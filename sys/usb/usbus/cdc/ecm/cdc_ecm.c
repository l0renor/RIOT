/*
 * Copyright (C) 2019 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup usbus_cdc_eem
 * @{
 * @file USBUS implementation for ethernet emulation model
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include "msg.h"
#include "tsrb.h"

#include "luid.h"
#include "net/ethernet.h"
#include "fmt.h"
#include "usb/descriptor.h"
#include "usb/cdc.h"
#include "usb/usbus.h"
#include "usb/usbus/cdc/ecm.h"

#include <string.h>

#define ENABLE_DEBUG    (0)
#include "debug.h"

static int event_handler(usbus_t *usbus, usbus_handler_t *handler, uint16_t event, void *arg);
static void _init(usbus_t *usbus, usbus_handler_t *handler);
void usbus_cdc_ecm_flush(usbus_cdcecm_device_t *cdcecm);

static size_t _gen_union_descriptor(usbus_t *usbus, void *arg);
static size_t _gen_union_size(usbus_t *usbus, void *arg);

static const usbus_hdr_gen_funcs_t _union_descriptor = {
    .get_header = _gen_union_descriptor,
    .get_header_len = _gen_union_size
};

static size_t _gen_ecm_descriptor(usbus_t *usbus, void *arg);
static size_t _gen_ecm_size(usbus_t *usbus, void *arg);

static const usbus_hdr_gen_funcs_t _ecm_descriptor = {
    .get_header = _gen_ecm_descriptor,
    .get_header_len = _gen_ecm_size
};

static size_t _gen_cdc_descriptor(usbus_t *usbus, void *arg);
static size_t _gen_cdc_size(usbus_t *usbus, void *arg);

static const usbus_hdr_gen_funcs_t _cdc_descriptor = {
    .get_header = _gen_cdc_descriptor,
    .get_header_len = _gen_cdc_size
};

static size_t _gen_union_descriptor(usbus_t *usbus, void *arg)
{
    usbus_cdcecm_device_t *cdc = (usbus_cdcecm_device_t*)arg;
    usb_desc_union_t uni;
    /* functional union descriptor */
    uni.length = sizeof(usb_desc_union_t);
    uni.type = USB_TYPE_DESCRIPTOR_CDC;
    uni.subtype = USB_CDC_DESCR_SUBTYPE_UNION;
    uni.master_if = cdc->iface_ctrl.idx;
    uni.slave_if = cdc->iface_data.idx;
    usbus_ctrlslicer_put_bytes(usbus, (uint8_t*)&uni, sizeof(uni));
    return sizeof(usb_desc_union_t);
}

static size_t _gen_union_size(usbus_t *usbus, void *arg)
{
    (void)usbus;
    (void)arg;
    return sizeof(usb_desc_union_t);
}

static size_t _gen_ecm_descriptor(usbus_t *usbus, void *arg)
{
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)arg;
    usb_desc_ecm_t ecm;
    /* functional cdc ecm descriptor */
    ecm.length = sizeof(usb_desc_ecm_t);
    ecm.type = USB_TYPE_DESCRIPTOR_CDC;
    ecm.subtype = USB_CDC_DESCR_SUBTYPE_ETH_NET;
    ecm.macaddress = cdcecm->mac_str.idx;
    ecm.ethernetstatistics = 0;
    ecm.maxsegmentsize = ETHERNET_FRAME_LEN;
    ecm.numbermcfilters = 0x0000; /* No filtering */
    ecm.numberpowerfilters = 0;
    usbus_ctrlslicer_put_bytes(usbus, (uint8_t*)&ecm, sizeof(ecm));
    return sizeof(usb_desc_ecm_t);
}

static size_t _gen_ecm_size(usbus_t *usbus, void *arg)
{
    (void)usbus;
    (void)arg;
    return sizeof(usb_desc_ecm_t);
}

static size_t _gen_cdc_descriptor(usbus_t *usbus, void *arg)
{
    (void)arg;
    usb_desc_cdc_t cdc;
    /* functional cdc descriptor */
    cdc.length = sizeof(usb_desc_cdc_t);
    cdc.bcd_hid = USB_CDC_VERSION_BCD;
    cdc.type = USB_TYPE_DESCRIPTOR_CDC;
    cdc.subtype = 0x00;
    usbus_ctrlslicer_put_bytes(usbus, (uint8_t*)&cdc, sizeof(cdc));
    return sizeof(usb_desc_cdc_t);
}

static size_t _gen_cdc_size(usbus_t *usbus, void *arg)
{
    (void)usbus;
    (void)arg;
    return sizeof(usb_desc_cdc_t);
}

static void _notify_link_speed(usbus_cdcecm_device_t *cdcecm)
{
    DEBUG("CDC ECM: sending link speed indication\n");
    usb_setup_t *notification = (usb_setup_t*)cdcecm->ep_ctrl.ep->buf;
    notification->type = USB_SETUP_REQUEST_DEVICE2HOST |
                         USB_SETUP_REQUEST_TYPE_CLASS |
                         USB_SETUP_REQUEST_RECIPIENT_INTERFACE;
    notification->request = USB_CDC_MGNT_NOTIF_CONN_SPEED_CHANGE;
    notification->value = 0;
    notification->index = cdcecm->iface_ctrl.idx;
    notification->length = 8;

    uint32_t *speed = (uint32_t*)(cdcecm->ep_ctrl.ep->buf + sizeof(usb_setup_t));
    *speed++ = USBUS_CDC_ECM_CONFIG_SPEED_DOWNSTREAM;
    *speed = USBUS_CDC_ECM_CONFIG_SPEED_UPSTREAM;
    usbdev_ep_ready(cdcecm->ep_ctrl.ep, sizeof(usb_setup_t) + 2 * sizeof(uint32_t));
    cdcecm->notif = USBUS_CDCECM_NOTIF_SPEED;
}

static void _notify_link_up(usbus_cdcecm_device_t *cdcecm)
{
    DEBUG("CDC ECM: sending link up indication\n");
    usb_setup_t *notification = (usb_setup_t*)cdcecm->ep_ctrl.ep->buf;
    notification->type = USB_SETUP_REQUEST_DEVICE2HOST |
                         USB_SETUP_REQUEST_TYPE_CLASS |
                         USB_SETUP_REQUEST_RECIPIENT_INTERFACE;
    notification->request = USB_CDC_MGNT_NOTIF_NETWORK_CONNECTION;
    notification->value = 1;
    notification->index = cdcecm->iface_ctrl.idx;
    notification->length = 0;
    usbdev_ep_ready(cdcecm->ep_ctrl.ep, sizeof(usb_setup_t));
    cdcecm->notif = USBUS_CDCECM_NOTIF_LINK_UP;
}

static const usbus_handler_driver_t cdcecm_driver = {
    .init = _init,
    .event_handler = event_handler,
};

static void _fill_ethernet(usbus_cdcecm_device_t *cdcecm)
{
    uint8_t ethernet[ETHERNET_ADDR_LEN];
    luid_get(ethernet, ETHERNET_ADDR_LEN);
    ethernet[0] |= 0x02;      /* locally administered address */
    ethernet[0] &= ~0x01;     /* unicast address */
    fmt_bytes_hex(cdcecm->mac_host, ethernet, sizeof(ethernet));
}

int usbus_cdcecm_init(usbus_t *usbus, usbus_cdcecm_device_t *handler)
{
    memset(handler, 0 , sizeof(usbus_cdcecm_device_t));
    mutex_init(&handler->out_lock);
    _fill_ethernet(handler);
    handler->usbus = usbus;
    handler->handler_ctrl.driver = &cdcecm_driver;
    usbus_register_event_handler(usbus, (usbus_handler_t*)handler);
    return 0;
}

static void _init(usbus_t *usbus, usbus_handler_t *handler)
{
    DEBUG("CDC ECM: intialization\n");
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)handler;


    cdcecm->ecm_hdr.next = NULL;
    cdcecm->ecm_hdr.funcs = &_ecm_descriptor;
    cdcecm->ecm_hdr.arg = cdcecm;

    cdcecm->union_hdr.next = &cdcecm->ecm_hdr;
    cdcecm->union_hdr.funcs = &_union_descriptor;
    cdcecm->union_hdr.arg = cdcecm;

    cdcecm->cdc_hdr.next = &cdcecm->union_hdr;
    cdcecm->cdc_hdr.funcs = &_cdc_descriptor;
    cdcecm->cdc_hdr.arg = cdcecm;

    /* Configure Interface 0 as control interface */
    cdcecm->iface_ctrl.class = USB_CLASS_CDC_CONTROL ;
    cdcecm->iface_ctrl.subclass = USB_CDC_SUBCLASS_ENCM;
    cdcecm->iface_ctrl.protocol = USB_CDC_PROTOCOL_NONE;
    cdcecm->iface_ctrl.hdr_gen = &cdcecm->cdc_hdr;
    cdcecm->iface_ctrl.handler = handler;
    /* Configure second interface to handle data endpoint */
    cdcecm->iface_data.class = USB_CLASS_CDC_DATA;
    cdcecm->iface_data.subclass = USB_CDC_SUBCLASS_NONE;
    cdcecm->iface_data.protocol = USB_CDC_PROTOCOL_NONE;
    cdcecm->iface_data.hdr_gen = NULL;
    cdcecm->iface_data.handler = handler;

    usbus_add_string_descriptor(usbus, &cdcecm->mac_str, cdcecm->mac_host);
    /* Create required endpoints */
    usbus_add_endpoint(usbus, &cdcecm->iface_ctrl, &cdcecm->ep_ctrl,
                       USB_EP_TYPE_INTERRUPT,
                       USB_EP_DIR_IN,
                       USBUS_CDCECM_EP_CTRL_SIZE);
    cdcecm->ep_ctrl.interval = 0x10;
    usbus_add_endpoint(usbus, (usbus_interface_t*)&cdcecm->iface_data_alt, &cdcecm->ep_out,
                       USB_EP_TYPE_BULK,
                       USB_EP_DIR_OUT,
                       USBUS_CDCECM_EP_DATA_SIZE);
    cdcecm->ep_out.interval = 0;
    usbus_add_endpoint(usbus, (usbus_interface_t*)&cdcecm->iface_data_alt,
                       &cdcecm->ep_in,
                       USB_EP_TYPE_BULK,
                       USB_EP_DIR_IN,
                       USBUS_CDCECM_EP_DATA_SIZE);
    cdcecm->ep_in.interval = 0;
    /* Add interfaces to the stack */
    usbus_add_interface(usbus, &cdcecm->iface_ctrl);
    usbus_add_interface(usbus, &cdcecm->iface_data);

    cdcecm->iface_data.alts = &cdcecm->iface_data_alt;

    //cdc->ep_data_in.ep->driver->ready(cdc->ep_data_in.ep, 0);
    usbus_enable_endpoint(&cdcecm->ep_out);
    usbus_enable_endpoint(&cdcecm->ep_in);
    usbus_enable_endpoint(&cdcecm->ep_ctrl);
    usbdev_ep_ready(cdcecm->ep_out.ep, 0);
    usbus_handler_set_flag(handler, USBUS_HANDLER_FLAG_RESET);
}

static int _handle_setup(usbus_t *usbus, usbus_handler_t *handler, usb_setup_t *pkt)
{
    (void)usbus;
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)handler;
    DEBUG("CDC ECM: Request: 0x%x\n", pkt->request);
    switch(pkt->request) {
        case USB_SETUP_REQ_SET_INTERFACE:
            DEBUG("CDC ECM: Changing active interface to alt %d\n", pkt->value);
            cdcecm->active_iface = (uint8_t)pkt->value;
            if (cdcecm->active_iface == 1) {
                _notify_link_up(cdcecm);
            }
            break;

        case USB_CDC_MGNT_REQUEST_SET_ETH_PACKET_FILTER:
            /* While we do answer the request, CDC ECM filters are not really
             * implemented */
            DEBUG("CDC ECM: Not modifying filter to 0x%x\n", pkt->value);
            break;

        default:
            return -1;
    }

    return 1;
}

static int _handle_in_complete(usbus_t *usbus, usbus_handler_t *handler)
{
    (void)usbus;
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)handler;
    mutex_unlock(&cdcecm->out_lock);
    return 0;
}

static int _handle_tx_xmit(usbus_t *usbus, usbus_handler_t *handler)
{
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)handler;
    DEBUG("CDC_ECM: Handling TX xmit from netdev\n");
    if (usbus->state != USBUS_STATE_CONFIGURED || cdcecm->active_iface == 0) {
        DEBUG("CDC ECM: not configured, unlocking\n");
        mutex_unlock(&cdcecm->out_lock);
        return 0;
    }
    /* Data prepared by netdev_send, signal ready to usbus */
    usbdev_ep_ready(cdcecm->ep_in.ep, cdcecm->tx_len);
    return 0;
}

static int _handle_rx_flush(usbus_t *usbus, usbus_handler_t *handler)
{
    (void)usbus;
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)handler;
    cdcecm->len = 0;
    usbdev_ep_ready(cdcecm->ep_out.ep, 0);
    return 0;
}

static void _store_frame_chunk(usbus_cdcecm_device_t *cdcecm)
{
    uint8_t *buf = cdcecm->ep_out.ep->buf;
    size_t len = 0;
    usbdev_ep_get(cdcecm->ep_out.ep, USBOPT_EP_AVAILABLE, &len, sizeof(size_t));
    memcpy(cdcecm->in_buf + cdcecm->len, buf, len);
    cdcecm->len += len;
    if (len < USBUS_CDCECM_EP_DATA_SIZE && cdcecm->netdev.event_callback) {
        cdcecm->netdev.event_callback(&cdcecm->netdev, NETDEV_EVENT_ISR);
    }
}

static int _handle_tr_complete(usbus_t *usbus, usbus_handler_t *handler, usbdev_ep_t *ep)
{
    (void)usbus;
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)handler;
    if (ep == cdcecm->ep_out.ep) {
        /* Retrieve incoming data */
        if (cdcecm->notif == USBUS_CDCECM_NOTIF_NONE) {
            _notify_link_up(cdcecm);
        }
        size_t len = 0;
        usbdev_ep_get(ep, USBOPT_EP_AVAILABLE, &len, sizeof(size_t));
        _store_frame_chunk(cdcecm);
        if (len == USBUS_CDCECM_EP_DATA_SIZE) {
            usbdev_ep_ready(ep, 0);
        }
        return 0;
    }
    else if (ep == cdcecm->ep_in.ep) {
        _handle_in_complete(usbus, handler);
    }
    else if (ep == cdcecm->ep_ctrl.ep && cdcecm->notif == USBUS_CDCECM_NOTIF_LINK_UP) {
        _notify_link_speed(cdcecm);
    }
    return 0;
}

static void _handle_reset(usbus_t *usbus, usbus_handler_t *handler)
{
    usbus_cdcecm_device_t *cdcecm = (usbus_cdcecm_device_t*)handler;
    DEBUG("CDC ECM: Reset\n");
    _handle_rx_flush(usbus, handler);
    _handle_in_complete(usbus, handler);
    cdcecm->notif = USBUS_CDCECM_NOTIF_NONE;
    cdcecm->active_iface = 0;
    mutex_unlock(&cdcecm->out_lock);
}

static int event_handler(usbus_t *usbus, usbus_handler_t *handler, uint16_t event, void *arg)
{
    switch(event) {
        case USBUS_MSG_TYPE_SETUP_RQ:
            return _handle_setup(usbus, handler, (usb_setup_t*)arg);

        case USBUS_MSG_TYPE_TR_COMPLETE:
            return _handle_tr_complete(usbus, handler, (usbdev_ep_t*)arg);

        case USBUS_MSG_CDCECM_RX_FLUSH:
            return _handle_rx_flush(usbus, handler);

        case USBUS_MSG_CDCECM_TX_XMIT:
            return _handle_tx_xmit(usbus, handler);

        case USBUS_MSG_TYPE_RESET:
            _handle_reset(usbus, handler);
            break;

        default:
            DEBUG("Unhandled event :0x%x\n", event);
            return -1;
    }
    return 0;
}
