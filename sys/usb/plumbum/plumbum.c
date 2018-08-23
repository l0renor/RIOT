/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup usb Plumbum USB device stack
 * @{
 * @file
 * @brief   Plumbum USB manager thread, handles driver interaction and EP0 management
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include "sam_usb.h"
#include "thread.h"
#include "xtimer.h"
#include "byteorder.h"
#include "usb/usbdev.h"
#include "usb/message.h"
#include "usb/plumbum.h"
#include "usb/hid/keyboard.h"

#include "usb.h"
#include "cpu.h"

#include <string.h>
#include <errno.h>
#define ENABLE_DEBUG    (0)
#include "debug.h"

#define _PLUMBUM_MSG_QUEUE_SIZE    (8)
#define PLUMBUM_STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#define PLUMBUM_PRIO                (THREAD_PRIORITY_MAIN - 6)
#define PLUMBUM_TNAME               "plumbum"

static plumbum_t _plumbum;
extern const usbdev_driver_t driver;
static sam0_common_usb_t usbdev;
static char _stack[PLUMBUM_STACKSIZE];
static uint8_t in_buf[1024];
static uint8_t out_buf[1024];

void _event_cb(usbdev_t *usbdev, usbdev_event_t event);
void _event_ep0_cb(usbdev_ep_t *ep, usbdev_event_t event);
void _event_ep_cb(usbdev_ep_t *ep, usbdev_event_t event);
static void *_plumbum_thread(void *args);

static size_t _cpy_str(uint8_t *buf, const char *str)
{
    size_t len = 0;
    while(*str) {
        *buf++ = *str++;
        *buf++ = 0;
        len += 2;
    }
    return len;
}

plumbum_t *plumbum_get_ctx(void)
{
    return &_plumbum;
}

void plumbum_init(void)
{
    usbdev.usbdev.driver = &driver;
    plumbum_create(_stack, PLUMBUM_STACKSIZE, PLUMBUM_PRIO,
                   PLUMBUM_TNAME, &usbdev.usbdev );
}

void plumbum_create(char *stack, int stacksize, char priority,
                   const char *name, usbdev_t *usbdev)
{
    plumbum_t *plumbum = &_plumbum;
    plumbum->dev = usbdev;
    int res = thread_create(stack, stacksize, priority, THREAD_CREATE_STACKTEST,
                        _plumbum_thread, (void *)plumbum, name);
    (void)res;
    assert(res > 0);
}

uint16_t plumbum_add_string_descriptor(plumbum_t *plumbum, plumbum_string_t *desc, const char *str)
{
    mutex_lock(&plumbum->lock);
    desc->next = plumbum->strings;
    plumbum->strings = desc;
    desc->idx = plumbum->str_idx++;
    desc->str = str;
    mutex_unlock(&plumbum->lock);
    return desc->idx;
}

plumbum_interface_t *_ep_to_iface(plumbum_t *plumbum, usbdev_ep_t *ep)
{
    for (plumbum_interface_t *iface = plumbum->iface; iface; iface = iface->next) {
        for (plumbum_endpoint_t *pep = iface->ep; pep; pep = pep->next) {
            if (pep->ep == ep) {
                return iface;
            }
        }
    }
    return NULL;
}

plumbum_string_t *_get_descriptor(plumbum_t *plumbum, uint16_t idx)
{
    for (plumbum_string_t * str = plumbum->strings; str; str = str->next) {
        if (str->idx == idx) {
            return str;
        }
    }
    return NULL;
}

uint16_t plumbum_add_interface(plumbum_t *plumbum, plumbum_interface_t *iface)
{
    mutex_lock(&plumbum->lock);
    iface->next = plumbum->iface;
    plumbum->iface = iface;
    mutex_unlock(&plumbum->lock);
    return iface->idx;
}

void plumbum_register_event_handler(plumbum_t *plumbum, plumbum_handler_t *handler)
{
    mutex_lock(&plumbum->lock);
    handler->next = plumbum->handler;
    plumbum->handler = handler;
    mutex_unlock(&plumbum->lock);
    handler->driver->init(plumbum, handler);
}

int plumbum_add_endpoint(plumbum_t *plumbum, plumbum_interface_t *iface, plumbum_endpoint_t* ep, usb_ep_type_t type, usb_ep_dir_t dir)

{
    int res = -ENOMEM;
    mutex_lock(&plumbum->lock);
    usbdev_ep_t* usbdev_ep = plumbum->dev->driver->new_ep(plumbum->dev, type, dir);
    if (ep) {
        ep->ep = usbdev_ep;
        ep->next = iface->ep;
        iface->ep = ep;
        res = 0;
    }
    ep->ep->cb = _event_ep_cb;
    mutex_unlock(&plumbum->lock);
    return res;
}

static void _plumbum_config_ep0(plumbum_t *plumbum)
{
    static const usbopt_enable_t enable = USBOPT_ENABLE;
    size_t len = 64;
    uint8_t *buf = in_buf;
    plumbum->in->driver->set(plumbum->in, USBOPT_EP_BUF_ADDR, &buf, sizeof(buf));
    plumbum->in->driver->set(plumbum->in, USBOPT_EP_BUF_SIZE, &len, sizeof(len));
    plumbum->in->driver->set(plumbum->in, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
    buf = out_buf;
    plumbum->out->driver->set(plumbum->out, USBOPT_EP_BUF_ADDR, &buf, sizeof(buf));
    plumbum->out->driver->set(plumbum->out, USBOPT_EP_BUF_SIZE, &len, sizeof(len));
    plumbum->out->driver->set(plumbum->out, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
    plumbum->out->driver->ready(plumbum->out, 0);
}

void _req_status(plumbum_t *plumbum)
{
    memset(in_buf, 0, 2);
    plumbum->in->driver->ready(plumbum->in, 2);
}

void _req_str(plumbum_t *plumbum, uint16_t idx)
{
    if (idx == 0) {
        usb_descriptor_string_t *pkt = (usb_descriptor_string_t*)in_buf;
        pkt->length = sizeof(uint16_t)+sizeof(usb_descriptor_string_t);
        pkt->type = USB_TYPE_DESCRIPTOR_STRING;
        /* Only one language ID supported */
        uint16_t us = USB_CONFIG_DEFAULT_LANGID;
        memcpy(in_buf+sizeof(usb_descriptor_string_t),
             &us, sizeof(uint16_t));
        plumbum->in->driver->ready(plumbum->in, pkt->length);
    }
    else {
        mutex_lock(&plumbum->lock);
        usb_descriptor_string_t *pkt = (usb_descriptor_string_t*)in_buf;
        plumbum_string_t *str = _get_descriptor(plumbum, idx);
        if (str) {
            pkt->type = USB_TYPE_DESCRIPTOR_STRING;
            pkt->length = sizeof(usb_descriptor_string_t);
            pkt->length += _cpy_str(in_buf+sizeof(usb_descriptor_string_t), str->str);
            plumbum->in->driver->ready(plumbum->in, pkt->length);
        }
        else {
            plumbum->in->driver->ready(plumbum->in, 0);
        }
        mutex_unlock(&plumbum->lock);
    }
}

void _print_setup(usb_setup_t *pkt)
{
    printf("plumbum: setup packet type 0x%x. request: 0x%x, value: 0x%x\n", pkt->type, pkt->request, pkt->value);
}

static void _req_dev(plumbum_t *plumbum)
{
    usb_descriptor_device_t *desc = (usb_descriptor_device_t*)in_buf;
    memset(desc, 0, sizeof(usb_descriptor_device_t));
    desc->length = sizeof(usb_descriptor_device_t);
    desc->type = USB_TYPE_DESCRIPTOR_DEVICE;
    desc->bcd_usb = 0x0110;
    desc->max_packet_size = 64;
    desc->vendor_id = USB_CONFIG_VID;
    desc->product_id = USB_CONFIG_PID;
    desc->manufacturer_idx = plumbum->manuf.idx;
    desc->product_idx = plumbum->product.idx;
    desc->num_configurations = 1;
    plumbum->in->driver->ready(plumbum->in, sizeof(usb_descriptor_device_t));
}

static size_t _fmt_endpoints(plumbum_interface_t *iface, uint8_t *buf, size_t max_len)
{
    size_t len = 0;
    (void)max_len;
    for (plumbum_endpoint_t *ep = iface->ep;
            ep; ep = ep->next) {
        if (max_len < len + sizeof(usb_descriptor_endpoint_t)) {
            break;
        }
        uint8_t *write_pos = buf + len;
        usb_descriptor_endpoint_t *usb_ep = (usb_descriptor_endpoint_t*)write_pos;
        memset(usb_ep, 0 , sizeof(usb_descriptor_endpoint_t));
        usb_ep->length = sizeof(usb_descriptor_endpoint_t);
        usb_ep->type = USB_TYPE_DESCRIPTOR_ENDPOINT;
        usb_ep->address = ep->ep->num;
        if (ep->ep->dir == USB_EP_DIR_OUT) {
            usb_ep->address |= 0x80;
        }
        usb_ep->attributes = 3;
        usb_ep->max_packet_size = 64;
        usb_ep->interval = 20;
        usb_ep->address = 0x81;
        len += usb_ep->length;
    }
    return len;
}

static size_t _fmt_additional(plumbum_t *plumbum, plumbum_hdr_gen_t *hdr, uint8_t *buf, size_t max_len)
{
    size_t len = 0;
    for (; hdr; hdr = hdr->next) {
        len += hdr->gen_hdr(plumbum, hdr->arg, buf + len, max_len - len);
    }
    return len;
}

static size_t _fmt_ifaces(plumbum_t *plumbum, uint8_t *buf, size_t max_len)
{
    size_t len = 0;
    for (plumbum_interface_t *iface = plumbum->iface; 
            iface;
            iface = iface->next) {
        if (max_len < len + sizeof(usb_descriptor_interface_t)) {
            break;
        }
        uint8_t *write_pos = buf + len;
        usb_descriptor_interface_t *usb_iface = (usb_descriptor_interface_t*)write_pos;
        memset(usb_iface, 0 , sizeof(usb_descriptor_interface_t));
        usb_iface->length = sizeof(usb_descriptor_interface_t);
        usb_iface->type = USB_TYPE_DESCRIPTOR_INTERFACE;
        usb_iface->interface_num = iface->idx;
        usb_iface->alternate_setting = 0;
        usb_iface->class = iface->class;
        usb_iface->subclass = iface->subclass;
        usb_iface->protocol = iface->protocol;
        usb_iface->num_endpoints = 1;
        if (iface->descr) {
            usb_iface->idx = iface->descr->idx;
        }
        len += sizeof(usb_descriptor_interface_t);
        len += _fmt_additional(plumbum, iface->hdr_gen, write_pos + len, max_len);
        len += _fmt_endpoints(iface, write_pos + len, max_len);
    }
    return len;
}

static size_t _config_size(plumbum_t *plumbum)
{
    size_t len = sizeof(usb_descriptor_configuration_t);
    for (plumbum_interface_t *iface = plumbum->iface; 
            iface;
            iface = iface->next) {
        len += sizeof(usb_descriptor_interface_t);
        for (plumbum_hdr_gen_t *hdr = iface->hdr_gen; hdr; hdr = hdr->next) {
            len += hdr->hdr_len(plumbum, hdr->arg);
        }
        for (plumbum_endpoint_t *ep = iface->ep;
                ep; ep = ep->next) {
            len += sizeof(usb_descriptor_endpoint_t);
        }
    }
    return len;
}

static void _req_config(plumbum_t *plumbum, uint8_t *buf, size_t max_len)
{
    size_t len = 0;
    mutex_lock(&plumbum->lock);
    usb_descriptor_configuration_t *conf = (usb_descriptor_configuration_t*)buf;
    memset(conf, 0 ,sizeof(usb_descriptor_configuration_t));
    conf->length = sizeof(usb_descriptor_configuration_t);
    conf->type = USB_TYPE_DESCRIPTOR_CONFIGURATION;
    conf->total_length = sizeof(usb_descriptor_configuration_t);
    conf->val = 1;
    conf->attributes = USB_CONF_ATTR_RESERVED;
    if (USB_CONFIG_SELF_POWERED) {
        conf->attributes |= USB_CONF_ATTR_SELF_POWERED;
    }
    /* Todo: upper bound */
    conf->max_power = USB_CONFIG_MAX_POWER/2;
    conf->num_interfaces = 1;
    len += sizeof(usb_descriptor_configuration_t);
    /* TODO: add buffer upper bound */
    (void)max_len;
    len += _fmt_ifaces(plumbum, buf + len, max_len - len );
    conf->total_length = _config_size(plumbum);
    conf->idx = plumbum->config.idx;
    mutex_unlock(&plumbum->lock);
    plumbum->in->driver->ready(plumbum->in, len);
}

static void _req_dev_qualifier(plumbum_t *plumbum)
{
    usb_speed_t speed = USB_SPEED_LOW;
    plumbum->dev->driver->get(plumbum->dev, USBOPT_MAX_SPEED, &speed, sizeof(usb_speed_t));
    if (speed == USB_SPEED_HIGH) {
        /* TODO: implement device qualifier support */
    }
    /* Signal stall to indicate unsupported (USB 2.0 spec 9.6.2 */
    static const usbopt_enable_t enable = USBOPT_ENABLE;

    plumbum->in->driver->set(plumbum->in, USBOPT_EP_STALL, &enable, sizeof(usbopt_enable_t));
}

static void _req_descriptor(plumbum_t *plumbum, usb_setup_t *pkt)
{
    uint8_t type = pkt->value >> 8;
    uint8_t idx = (uint8_t)pkt->value;
    //_print_setup(pkt);
    switch (type) {
        case 0x1:
            _req_dev(plumbum);
            break;
        case 0x2:
            _req_config(plumbum, in_buf, pkt->length);
            break;
        case 0x03:
            _req_str(plumbum, idx);
            break;
        case 0x06:
            _req_dev_qualifier(plumbum);
            break;
        default:
            break;
    }
}

void recv_dev_setup(plumbum_t *plumbum, usbdev_ep_t *ep, usb_setup_t *pkt)
{
    (void)ep;
    if (pkt->type & 0x80) {
        switch (pkt->request) {
            case 0x00:
                _req_status(plumbum);
                break;
            case 0x06:
                _req_descriptor(plumbum, pkt);
                break;
            default:
                break;
        }
    }
    else{
        switch (pkt->request) {
            case 0x05:
                plumbum->addr = (uint8_t)pkt->value;
                break;
            default:
                break;
        }
        /* Signal zlp */
        plumbum->in->driver->ready(plumbum->in, 0);
    }
}

void recv_interface_setup(plumbum_t *plumbum, usbdev_ep_t *ep, usb_setup_t *pkt)
{
    uint16_t destination = pkt->index;
    (void)ep;
    /* Find interface handler */
    mutex_lock(&plumbum->lock);
    for (plumbum_interface_t *iface = plumbum->iface; iface; iface = iface->next) {
        if (destination == iface->idx) {
            iface->handler->driver->event_handler(plumbum, iface->handler, PLUMBUM_MSG_TYPE_SETUP_RQ, pkt);
        }
    }
    mutex_unlock(&plumbum->lock);
}

void recv_setup(plumbum_t *plumbum, usbdev_ep_t *ep)
{
    (void)ep;
    usb_setup_t *pkt = (usb_setup_t*)out_buf;
    if (pkt->type & 0x80) {
        plumbum->setup_state = PLUMBUM_SETUPRQ_INDATA;
    }
    else {
        if (pkt->length) {
            plumbum->setup_state = PLUMBUM_SETUPRQ_OUTDATA;
        }
        else {
            plumbum->setup_state = PLUMBUM_SETUPRQ_INACK;
            plumbum->in->driver->ready(plumbum->in, 0);
        }
    }
    uint8_t destination = pkt->type & USB_SETUP_REQUEST_RECIPIENT_MASK;
    switch(destination) {
        case USB_SETUP_REQUEST_RECIPIENT_DEVICE:
            recv_dev_setup(plumbum, ep, pkt);
            break;
        case USB_SETUP_REQUEST_RECIPIENT_INTERFACE:
            recv_interface_setup(plumbum, ep, pkt);
            break;
        default:
            DEBUG("plumbum: Unhandled setup request\n");
    }
    plumbum->out->driver->ready(plumbum->out, 0);
}

static void *_plumbum_thread(void *args)
{
    plumbum_t *plumbum = (plumbum_t*)args;
    mutex_lock(&plumbum->lock);
    usbdev_t *dev = plumbum->dev;
    plumbum->pid = sched_active_pid;
    plumbum->addr = 0;
    plumbum->strings = NULL;
    plumbum->iface = NULL;
    plumbum->str_idx = 1;
    plumbum->buf_in = in_buf;
    plumbum->setup_state = PLUMBUM_SETUPRQ_READY;
    mutex_init(&plumbum->lock);
    msg_t msg, msg_queue[_PLUMBUM_MSG_QUEUE_SIZE];
    DEBUG("plumbum: starting thread %i\n", sched_active_pid);
    /* setup the link-layer's message queue */
    msg_init_queue(msg_queue, _PLUMBUM_MSG_QUEUE_SIZE);
    /* register the event callback with the device driver */
    dev->cb = _event_cb;
    /* initialize low-level driver */
    dev->driver->init(dev);
    dev->context = plumbum;

    plumbum->in = plumbum->dev->driver->new_ep(plumbum->dev, USB_EP_TYPE_CONTROL, USB_EP_DIR_IN);
    plumbum->out = plumbum->dev->driver->new_ep(plumbum->dev, USB_EP_TYPE_CONTROL, USB_EP_DIR_OUT);
    plumbum->in->cb = _event_ep0_cb;
    plumbum->out->cb = _event_ep0_cb;
    plumbum->in->context = plumbum;
    plumbum->out->context = plumbum;

    plumbum->in->driver->init(plumbum->in);
    plumbum->out->driver->init(plumbum->out);
    _plumbum_config_ep0(plumbum);
    plumbum_add_string_descriptor(plumbum, &plumbum->manuf, USB_CONFIG_MANUF_STR);
    plumbum_add_string_descriptor(plumbum, &plumbum->product, USB_CONFIG_PRODUCT_STR);
    plumbum_add_string_descriptor(plumbum, &plumbum->config, USB_CONFIG_CONFIGURATION_STR);

    plumbum->state = PLUMBUM_STATE_DISCONNECT;
    mutex_unlock(&plumbum->lock);
    keyboard_init(plumbum);
    xtimer_sleep(1);
    usbopt_enable_t enable = USBOPT_ENABLE;
    dev->driver->set(dev, USBOPT_ATTACH, &enable, sizeof(usbopt_enable_t));

    while (1) {
        msg_receive(&msg);
        /* dispatch netdev, MAC and gnrc_netapi messages */
        switch (msg.type) {
            case PLUMBUM_MSG_TYPE_EVENT:
                dev->driver->esr(dev);
                break;
            case PLUMBUM_MSG_TYPE_EP_EVENT:
                {
                    usbdev_ep_t *ep = (usbdev_ep_t*)msg.content.ptr;
                    ep->driver->esr(ep);
                }
                break;
            default:
                DEBUG("plumbum: unhandled event\n");
                break;
        }
    }
    return NULL;
}

/* USB event callback */
void _event_cb(usbdev_t *usbdev, usbdev_event_t event)
{
    plumbum_t *plumbum = (plumbum_t *)usbdev->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = PLUMBUM_MSG_TYPE_EVENT,
                      .content = { .ptr = usbdev } };

        if (msg_send(&msg, plumbum->pid) <= 0) {
            puts("plumbum: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_RESET:
                {
                plumbum->state = PLUMBUM_STATE_RESET;
                plumbum->addr = 0;
                plumbum->setup_state = PLUMBUM_SETUPRQ_READY;
                plumbum->dev->driver->set(plumbum->dev, USBOPT_ADDRESS, &plumbum->addr, sizeof(uint8_t));
                plumbum_endpoint_t *ep = plumbum->iface->ep;
                static const usbopt_enable_t enable = USBOPT_ENABLE;
                ep->ep->driver->set(ep->ep, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
                }
                break;
            default:
                DEBUG("plumbum: unhandled event\n");
                break;
        }
    }
}

/* USB endpoint 0 callback */
void _event_ep0_cb(usbdev_ep_t *ep, usbdev_event_t event)
{
    plumbum_t *plumbum = (plumbum_t *)ep->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = PLUMBUM_MSG_TYPE_EP_EVENT,
                      .content = { .ptr = ep} };

        if (msg_send(&msg, plumbum->pid) <= 0) {
            puts("plumbum_ep: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_TR_COMPLETE:
                /* Configure address if we have received one and handled the zlp */
                if (plumbum->setup_state == PLUMBUM_SETUPRQ_INACK && ep->dir == USB_EP_DIR_IN) {
                    if (plumbum->addr && plumbum->state == PLUMBUM_STATE_RESET) {
                        plumbum->dev->driver->set(plumbum->dev, USBOPT_ADDRESS, &plumbum->addr, sizeof(uint8_t));
                        /* Address configured */
                        plumbum->state = PLUMBUM_STATE_ADDR;
                    }
                    DEBUG("plumbum_state: inack->ready\n");
                    plumbum->setup_state = PLUMBUM_SETUPRQ_READY;
                }
                else if (plumbum->setup_state == PLUMBUM_SETUPRQ_OUTACK && ep->dir == USB_EP_DIR_OUT) {
                    DEBUG("plumbum_state: outack->ready\n");
                    plumbum->setup_state = PLUMBUM_SETUPRQ_READY;
                }
                else if (plumbum->setup_state == PLUMBUM_SETUPRQ_INDATA && ep->dir == USB_EP_DIR_IN) {
                    /* Ready out ZLP */
                    DEBUG("plumbum_state: indata->outack\n");
                    plumbum->setup_state = PLUMBUM_SETUPRQ_OUTACK;
                    plumbum->out->driver->ready(plumbum->out, 0);
                }
                else if (plumbum->setup_state == PLUMBUM_SETUPRQ_OUTDATA && ep->dir == USB_EP_DIR_OUT) {
                    /* Ready in ZLP */
                    DEBUG("plumbum_state: outdata->inack\n");
                    plumbum->setup_state = PLUMBUM_SETUPRQ_INACK;
                    plumbum->in->driver->ready(plumbum->in, 0);
                }
                else if (ep->dir == USB_EP_DIR_OUT) {
                    recv_setup(plumbum, ep);
                }
                break;
            case USBDEV_EVENT_TR_FAIL:
                if (ep->dir == USB_EP_DIR_OUT) {
                    DEBUG("plumbum_ep: out fail");
                }
                else {
                    DEBUG("plumbum_ep: in fail");
                }
                break;
            case USBDEV_EVENT_TR_STALL:
                {
                    static const usbopt_enable_t disable = USBOPT_DISABLE;
                    ep->driver->set(ep, USBOPT_EP_STALL, &disable, sizeof(usbopt_enable_t));
                }
                break;
            default:
                break;
        }
    }
}

/* USB generic endpoint callback */
void _event_ep_cb(usbdev_ep_t *ep, usbdev_event_t event)
{
    plumbum_t *plumbum = (plumbum_t *)ep->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = PLUMBUM_MSG_TYPE_EP_EVENT,
                      .content = { .ptr = ep} };

        if (msg_send(&msg, plumbum->pid) <= 0) {
            puts("plumbum_ep: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_TR_COMPLETE:
                {
                    plumbum_interface_t *iface = _ep_to_iface(plumbum, ep);
                    if (iface) {
                        iface->handler->driver->event_handler(plumbum, iface->handler, PLUMBUM_MSG_TYPE_TR_COMPLETE, ep);
                    }
                    if (ep->dir == USB_EP_DIR_OUT)
                    {
                        ep->driver->ready(ep, 0);
                    }
                }
                break;
            case USBDEV_EVENT_TR_FAIL:
                break;
            case USBDEV_EVENT_TR_STALL:
                {
                    static const usbopt_enable_t disable = USBOPT_DISABLE;
                    ep->driver->set(ep, USBOPT_EP_STALL, &disable, sizeof(usbopt_enable_t));
                }
                break;
            default:
                break;
        }
    }
}
