/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup sam0_common
 * @{
 * @file
 * @brief   muSB manager thread, handles driver interaction and EP0 management
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include "sam_usb.h"
#include "thread.h"
#include "usb/usbdev.h"
#include "usb/usbman.h"

#include "usb.h"
#include "cpu.h"

#include <string.h>
#include <wchar.h>
#define ENABLE_DEBUG    (0)
#include "debug.h"

#define _USBMAN_MSG_QUEUE_SIZE    (8)
#define USBMAN_STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#define USBMAN_PRIO                (THREAD_PRIORITY_MAIN - 6)
#define USBMAN_TNAME               "usb"

static usbman_t _usbman;
extern const usbdev_driver_t driver;
static sam0_common_usb_t usbdev;
static char _stack[USBMAN_STACKSIZE];
static uint8_t in_buf[1024];
static uint8_t out_buf[1024];

void _event_cb(usbdev_t *usbdev, usbdev_event_t event);
void _event_ep_in_cb(usbdev_ep_t *ep, usbdev_event_t event);
void _event_ep_out_cb(usbdev_ep_t *ep, usbdev_event_t event);
static void *_usbman_thread(void *args);

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

void usbman_init(void)
{
    usbdev.usbdev.driver = &driver;
    usbman_create(_stack, USBMAN_STACKSIZE, USBMAN_PRIO,
                   USBMAN_TNAME, &usbdev.usbdev );
}

void usbman_create(char *stack, int stacksize, char priority,
                   const char *name, usbdev_t *usbdev)
{
    usbman_t *usbman = &_usbman;
    usbman->dev = usbdev;
    int res = thread_create(stack, stacksize, priority, THREAD_CREATE_STACKTEST,
                        _usbman_thread, (void *)usbman, name);
    (void)res;
    assert(res > 0);
}

uint16_t usbman_add_string_descriptor(usbman_t *usbman, usbman_string_t *desc, const char *str)
{
    mutex_lock(&usbman->lock);
    desc->next = usbman->strings;
    usbman->strings = desc;
    desc->idx = usbman->str_idx++;
    desc->str = str;
    mutex_unlock(&usbman->lock);
    return desc->idx;
}

usbman_string_t *_get_descriptor(usbman_t *usbman, uint16_t idx)
{
    for (usbman_string_t * str = usbman->strings; str; str = str->next) {
        if (str->idx == idx) {
            return str;
        }
    }
    return NULL;
}

uint16_t usbman_add_iface(usbman_t *usbman, usbman_interface_t *iface)
{
    mutex_lock(&usbman->lock);
    iface->next = usbman->iface;
    usbman->iface = iface;
    mutex_unlock(&usbman->lock);
    return iface->idx;
}

static void _usbman_config_ep0(usbman_t *usbman)
{
    static const usbopt_enable_t enable = USBOPT_ENABLE;
    size_t len = 64;
    uint8_t *buf = in_buf;
    usbman->in->driver->set(usbman->in, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
    usbman->in->driver->set(usbman->in, USBOPT_EP_BUF_ADDR, &buf, sizeof(buf));
    usbman->in->driver->set(usbman->in, USBOPT_EP_BUF_SIZE, &len, sizeof(len));
    buf = out_buf;
    usbman->out->driver->set(usbman->out, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
    usbman->out->driver->set(usbman->out, USBOPT_EP_BUF_ADDR, &buf, sizeof(buf));
    usbman->out->driver->set(usbman->out, USBOPT_EP_BUF_SIZE, &len, sizeof(len));
    usbman->out->driver->ready(usbman->out, 0);
}

static void *_usbman_thread(void *args)
{
    usbman_t *usbman = (usbman_t*)args;
    usbdev_t *dev = usbman->dev;
    usbdev_ep_t *ep0_in, *ep0_out;
    usbman->pid = sched_active_pid;
    usbman->addr = 0;
    usbman->strings = NULL;
    usbman->str_idx = 1;
    mutex_init(&usbman->lock);
    msg_t msg, msg_queue[_USBMAN_MSG_QUEUE_SIZE];
    DEBUG("usbman: starting thread %i\n", sched_active_pid);
    /* setup the link-layer's message queue */
    msg_init_queue(msg_queue, _USBMAN_MSG_QUEUE_SIZE);
    /* register the event callback with the device driver */
    dev->cb = _event_cb;
    /* initialize low-level driver */
    dev->driver->init(dev);
    dev->context = usbman;

    dev->driver->get(dev, USBOPT_EP0_IN, &usbman->in, sizeof(ep0_in));
    dev->driver->get(dev, USBOPT_EP0_OUT, &usbman->out, sizeof(ep0_out));
    usbman->in->cb = _event_ep_in_cb;
    usbman->out->cb = _event_ep_out_cb;
    usbman->in->context = usbman;
    usbman->out->context = usbman;

    usbman->in->driver->init(usbman->in);
    usbman->out->driver->init(usbman->out);
    _usbman_config_ep0(usbman);
    usbman_add_string_descriptor(usbman, &usbman->manuf, USB_CONFIG_MANUF_STR);
    usbman_add_string_descriptor(usbman, &usbman->product, USB_CONFIG_PRODUCT_STR);

    usbman->state = USBMAN_STATE_DISCONNECT;

    usbopt_enable_t enable = USBOPT_ENABLE;
    dev->driver->set(dev, USBOPT_ATTACH, &enable, sizeof(usbopt_enable_t));

    while (1) {
        msg_receive(&msg);
        /* dispatch netdev, MAC and gnrc_netapi messages */
        switch (msg.type) {
            case USBMAN_MSG_TYPE_EVENT:
                dev->driver->esr(dev);
                break;
            case USBMAN_MSG_TYPE_EP0OUT_EVENT:
                usbman->out->driver->esr(usbman->out);
                break;
            case USBMAN_MSG_TYPE_EP0IN_EVENT:
                usbman->in->driver->esr(usbman->in);
                break;
            default:
                DEBUG("usbman: unhandled event\n");
                break;
        }
    }
    return NULL;
}

void _print_setup(usbdev_ep_t* ep, usb_setup_t *pkt)
{
    printf("usbman: EP%d, %s: setup packet type 0x%x. request: 0x%x, value: 0x%x\n", ep->num, ep->dir == USBDEV_DIR_OUT? "out" : "in", pkt->type, pkt->request, pkt->value);
}

static void _req_descriptor(usbman_t *usbman)
{
    usb_descriptor_device_t *desc = (usb_descriptor_device_t*)in_buf;
    memset(desc, 0, sizeof(usb_descriptor_device_t));
    desc->length = sizeof(usb_descriptor_device_t);
    desc->type = USB_TYPE_DESCRIPTOR_DEVICE;
    desc->bcd_usb = 0x0101;
    desc->max_packet_size = 64;
    desc->vendor_id = USB_CONFIG_VID;
    desc->product_id = USB_CONFIG_PID;
    desc->manufacturer_idx = usbman->manuf.idx;
    desc->product_idx = usbman->product.idx;
    desc->num_configurations = 1;
    usbman->in->driver->ready(usbman->in, sizeof(usb_descriptor_device_t));
}

static void _req_config(usbman_t *usbman)
{
    usb_descriptor_configuration_t *conf = (usb_descriptor_configuration_t*)in_buf;
    memset(conf, 0 ,sizeof(usb_descriptor_configuration_t));
    conf->length = sizeof(usb_descriptor_configuration_t);
    conf->type = USB_TYPE_DESCRIPTOR_CONFIGURATION;
    conf->total_length = sizeof(usb_descriptor_configuration_t);
    conf->val = 0;
    conf->attributes = USB_CONF_ATTR_RESERVED;
    if (USB_CONFIG_SELF_POWERED) {
        conf->attributes |= USB_CONF_ATTR_SELF_POWERED;
    }
    /* Todo: upper bound */
    conf->max_power = USB_CONFIG_MAX_POWER/2;
    usbman->in->driver->ready(usbman->in, sizeof(usb_descriptor_configuration_t));
}

void _req_status(usbman_t *usbman)
{
    memset(in_buf, 0, 2);
    usbman->in->driver->ready(usbman->in, 2);
}

void _req_str(usbman_t *usbman, uint16_t idx)
{
    printf("idx: %d\n", idx);
    if (idx == 0) {
        usb_descriptor_string_t *pkt = (usb_descriptor_string_t*)in_buf;
        pkt->length = sizeof(uint16_t)+sizeof(usb_descriptor_string_t);
        pkt->type = USB_TYPE_DESCRIPTOR_STRING;
        /* Only one language ID supported */
        uint16_t us = USB_CONFIG_DEFAULT_LANGID;
        memcpy(in_buf+sizeof(usb_descriptor_string_t),
             &us, sizeof(uint16_t));
        usbman->in->driver->ready(usbman->in, pkt->length);
    }
    else {
        mutex_lock(&usbman->lock);
        usb_descriptor_string_t *pkt = (usb_descriptor_string_t*)in_buf;
        usbman_string_t *str = _get_descriptor(usbman, idx);
        if (str) {
            pkt->type = USB_TYPE_DESCRIPTOR_STRING;
            pkt->length = sizeof(usb_descriptor_string_t);
            pkt->length += _cpy_str(in_buf+sizeof(usb_descriptor_string_t), str->str);
            usbman->in->driver->ready(usbman->in, pkt->length);
        }
        else {
            usbman->in->driver->ready(usbman->in, 0);
        }
        mutex_unlock(&usbman->lock);
    }
}

void recv_setup(usbman_t *usbman, usbdev_ep_t *ep)
{
    (void)usbman;
    usb_setup_t *pkt = (usb_setup_t*)out_buf;
    if (pkt->type & 0x80) {
        switch (pkt->request) {
            case 0x00:
                _req_status(usbman);
                puts("status request recv\n");
                break;
            case 0x06:
                {
                    uint8_t type = pkt->value >> 8;
                    uint8_t idx = (uint8_t)pkt->value;
                switch (type) {
                    case 0x1:
                        _req_descriptor(usbman);
                        break;
                    case 0x2:
                        _req_config(usbman);
                        break;
                    case 0x3:
                        _req_str(usbman, idx);
                        break;
                    default:
                        puts("unhandled");
                        _print_setup(ep, pkt);
                        break;
                }
                }
                break;
            default:
                break;
        }
    }
    else{
        switch (pkt->request) {
            case 0x05:
                usbman->addr = (uint8_t)pkt->value;
                break;
            default:
                break;
        }
        /* Signal zlp */
        usbman->in->driver->ready(usbman->in, 0);
    }
    usbman->out->driver->ready(usbman->out, 0);
}

void _event_cb(usbdev_t *usbdev, usbdev_event_t event)
{
    usbman_t *usbman = (usbman_t *)usbdev->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = USBMAN_MSG_TYPE_EVENT,
                      .content = { .ptr = usbdev } };

        if (msg_send(&msg, usbman->pid) <= 0) {
            puts("usbman: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_RESET:
                usbman->state = USBMAN_STATE_RESET;
                usbman->addr = 0;
                usbman->dev->driver->set(usbman->dev, USBOPT_ADDRESS, &usbman->addr, sizeof(uint8_t));
                break;
            default:
                DEBUG("usbman: unhandled event\n");
                break;
        }
    }
}

void _event_ep_out_cb(usbdev_ep_t *ep, usbdev_event_t event)
{
    usbman_t *usbman = (usbman_t *)ep->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = USBMAN_MSG_TYPE_EP0OUT_EVENT,
                      .content = { .ptr = ep } };

        if (msg_send(&msg, usbman->pid) <= 0) {
            puts("usbman_ep: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_TR_COMPLETE:
                usbman->out->driver->ready(usbman->out, 0);
                break;
            case USBDEV_EVENT_RX_SETUP:
                recv_setup(usbman, ep);
                break;
            default:
                break;
        }
    }
}

void _event_ep_in_cb(usbdev_ep_t *ep, usbdev_event_t event)
{
    usbman_t *usbman = (usbman_t *)ep->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = USBMAN_MSG_TYPE_EP0IN_EVENT,
                      .content = { .ptr = ep} };

        if (msg_send(&msg, usbman->pid) <= 0) {
            puts("usbman_ep: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_TR_COMPLETE:
                /* Configure address if we have received one and handled the zlp */
                if (usbman->addr && usbman->state == USBMAN_STATE_RESET) {
                    usbman->dev->driver->set(usbman->dev, USBOPT_ADDRESS, &usbman->addr, sizeof(uint8_t));
                    /* Address configured */
                    usbman->state = USBMAN_STATE_ADDR;
                }
                break;
            default:
                break;
        }
    }
}
