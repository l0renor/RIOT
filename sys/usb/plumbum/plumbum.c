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
#include "usb/plumbum/hdrs.h"

#include "usb/hid/keyboard.h"
#include "usb/audio.h"
#include "usb/plumbum/audio.h"

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

#define PLUMBUM_MAX_SIZE            64

static plumbum_t _plumbum;
extern const usbdev_driver_t driver;
static sam0_common_usb_t usbdev;
static char _stack[PLUMBUM_STACKSIZE];

void _event_cb(usbdev_t *usbdev, usbdev_event_t event);
void _event_ep0_cb(usbdev_ep_t *ep, usbdev_event_t event);
void _event_ep_cb(usbdev_ep_t *ep, usbdev_event_t event);
static void *_plumbum_thread(void *args);

static size_t plumbum_cpy_str(plumbum_t *plumbum, const char *str)
{
    size_t len = 0;
    while(*str) {
        plumbum_put_char(plumbum, *str);
        plumbum_put_char(plumbum, 0);
        len += 2;
        str++;
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

size_t plumbum_put_bytes(plumbum_t *plumbum, const uint8_t *buf, size_t len)
{
    plumbum_controlbuilder_t *builder = &plumbum->builder;
    size_t end = builder->start + plumbum->in->len;
    size_t byte_len = 0;    /* Length of the string to copy */

    /* Calculate start offset of the supplied bytes */
    size_t byte_offset = (builder->start > builder->cur) ? builder->start - builder->cur : 0;

    /* Check for string before or beyond window */
    if ((builder->cur >= end) || (byte_offset > len)) {
        builder->cur += len;
        return 0;
    }
    /* Check if string is over the end of the window */
    if ((builder->cur + len) >= end) {
        byte_len = end - (builder->cur + byte_offset);
    }
    else {
        byte_len = len - byte_offset;
    }
    size_t start_offset = builder->cur - builder->start + byte_offset;
    builder->cur += len;
    builder->len += byte_len;
    //printf("%u+%u/%u@%u\n",byte_len, byte_offset, len, start_offset);
    memcpy(plumbum->in->buf + start_offset , buf + byte_offset, byte_len);
    return byte_len;
}

size_t plumbum_put_char(plumbum_t *plumbum, char c)
{
    plumbum_controlbuilder_t *builder = &plumbum->builder;
    size_t end = builder->start + plumbum->in->len;
    /* Only copy the char if it is within the window */
    if ((builder->start <=  builder->cur) && (builder->cur < end)) {
        uint8_t *pos = plumbum->in->buf + builder->cur - builder->start;
        *pos = c;
        builder->cur++;
        builder->len++;
        return 1;
    }
    builder->cur++;
    return 0;
}

uint16_t plumbum_add_string_descriptor(plumbum_t *plumbum, plumbum_string_t *desc, const char *str)
{
    mutex_lock(&plumbum->lock);
    desc->next = plumbum->strings;
    plumbum->strings = desc;
    desc->idx = plumbum->str_idx++;
    desc->str = str;
    mutex_unlock(&plumbum->lock);
    DEBUG("plumbum: Adding string descriptor number %u for: \"%s\"\n", desc->idx, str);
    return desc->idx;
}

void plumbum_add_conf_descriptor(plumbum_t *plumbum, plumbum_hdr_gen_t* hdr_gen)
{
    mutex_lock(&plumbum->lock);
    hdr_gen->next = plumbum->hdr_gen;
    plumbum->hdr_gen = hdr_gen;
    mutex_unlock(&plumbum->lock);
}

void plumbum_ep0_ready(plumbum_t *plumbum)
{
    plumbum_controlbuilder_t *bldr = &plumbum->builder;
    size_t len = bldr->len;
    len = len < bldr->reqlen - bldr->start ? len : bldr->reqlen - bldr->start;
    bldr->transfered += len;
    //printf("rdy %u, tx: %u\n", len, bldr->transfered);
    plumbum->in->driver->ready(plumbum->in, len);
}

plumbum_interface_t *_ep_to_iface(plumbum_t *plumbum, usbdev_ep_t *ep)
{
    for (plumbum_interface_t *iface = plumbum->iface; iface; iface = iface->next) {
        for (plumbum_endpoint_t *pep = iface->ep; pep; pep = pep->next) {
            if (pep->ep == ep) {
                return iface;
            }
        }
        for (plumbum_interface_alt_t *alt = iface->alts; alt; alt = alt->next) {
            for (plumbum_endpoint_t *pep = alt->ep; pep; pep = pep->next) {
                if (pep->ep == ep) {
                    return iface;
                }
            }
        }
    }
    return NULL;
}

int plumbum_update_builder(plumbum_t *plumbum)
{
    plumbum_controlbuilder_t *bldr = &plumbum->builder;
    size_t end = bldr->start + plumbum->in->len;
    if (bldr->cur > end && bldr->start < bldr->reqlen && bldr->transfered < bldr->reqlen) {
        bldr->start += plumbum->in->len;
        bldr->cur = 0;
        bldr->len = 0;
        //printf("pkt cur %u, start: %u, end %u, max %u\n", bldr->cur, bldr->start, end, bldr->reqlen);
        return 1;
    }
    return 0;
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

int plumbum_add_endpoint(plumbum_t *plumbum, plumbum_interface_t *iface, plumbum_endpoint_t* ep, usb_ep_type_t type, usb_ep_dir_t dir, size_t len)
{
    int res = -ENOMEM;
    mutex_lock(&plumbum->lock);
    usbdev_ep_t* usbdev_ep = plumbum->dev->driver->new_ep(plumbum->dev, type, dir, len);
    if (usbdev_ep) {
        ep->maxpacketsize = usbdev_ep->len;
        usbdev_ep->context = plumbum;
        usbdev_ep->cb = _event_ep_cb;
        ep->ep = usbdev_ep;
        ep->next = iface->ep;
        iface->ep = ep;
        res = 0;
    }
    mutex_unlock(&plumbum->lock);
    return res;
}

static void _activate_endpoints(plumbum_t *plumbum)
{
    for (plumbum_interface_t *iface = plumbum->iface; iface; iface = iface->next) {
        for (plumbum_endpoint_t *ep = iface->ep; ep; ep = ep->next) {
            if (ep->active) {
                static const usbopt_enable_t enable = USBOPT_ENABLE;
                ep->ep->driver->set(ep->ep, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
                printf("activated endpoint %d, dir %s\n", ep->ep->num, ep->ep->dir == USB_EP_DIR_OUT? "out" : "in");
            }
        }
        for (plumbum_interface_alt_t *alt = iface->alts; alt; alt = alt->next) {
            for (plumbum_endpoint_t *ep = alt->ep; ep; ep = ep->next) {
                if (ep->active) {
                    static const usbopt_enable_t enable = USBOPT_ENABLE;
                    ep->ep->driver->set(ep->ep, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
                    printf("activated endpoint %d, dir %s\n", ep->ep->num, ep->ep->dir == USB_EP_DIR_OUT? "out" : "in");
                }
            }
        }
    }
}

static void _plumbum_config_ep0(plumbum_t *plumbum)
{
    static const usbopt_enable_t enable = USBOPT_ENABLE;
    plumbum->in->driver->set(plumbum->in, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
    plumbum->out->driver->set(plumbum->out, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));
    plumbum->out->driver->ready(plumbum->out, 0);
}

void _req_status(plumbum_t *plumbum)
{
    uint8_t status[2];
    memset(status, 0, 2);
    plumbum_put_bytes(plumbum, status, sizeof(status));
    plumbum->in->driver->ready(plumbum->in, 2);
}

void _req_str(plumbum_t *plumbum, uint16_t idx)
{
    if (idx == 0) {
        usb_descriptor_string_t desc;
        desc.type = USB_TYPE_DESCRIPTOR_STRING;
        desc.length = sizeof(uint16_t)+sizeof(usb_descriptor_string_t);
        plumbum_put_bytes(plumbum, (uint8_t*)&desc, sizeof(desc));
        /* Only one language ID supported */
        uint16_t us = USB_CONFIG_DEFAULT_LANGID;
        plumbum_put_bytes(plumbum, (uint8_t*)&us, sizeof(uint16_t));
        plumbum_ep0_ready(plumbum);
    }
    else {
        usb_descriptor_string_t desc;
        desc.type = USB_TYPE_DESCRIPTOR_STRING;
        mutex_lock(&plumbum->lock);
        plumbum_string_t *str = _get_descriptor(plumbum, idx);
        if (str) {
            desc.length = sizeof(usb_descriptor_string_t);
            desc.length += 2*strlen(str->str);
            plumbum_put_bytes(plumbum, (uint8_t*)&desc, sizeof(desc));
            plumbum_cpy_str(plumbum, str->str);
            plumbum_ep0_ready(plumbum);
        }
        else {
            plumbum_ep0_ready(plumbum);
        }
        mutex_unlock(&plumbum->lock);
    }
}

void _print_setup(usb_setup_t *pkt)
{
    printf("plumbum: setup t:0x%.2x r:0x%x, v:0x%x l:%u\n", pkt->type, pkt->request, pkt->value, pkt->length);
}

static void _req_dev(plumbum_t *plumbum)
{
    usb_descriptor_device_t desc;
    memset(&desc, 0, sizeof(usb_descriptor_device_t));
    desc.length = sizeof(usb_descriptor_device_t);
    desc.type = USB_TYPE_DESCRIPTOR_DEVICE;
    desc.bcd_usb = 0x0110;
    desc.max_packet_size = PLUMBUM_MAX_SIZE;
    desc.vendor_id = USB_CONFIG_VID;
    desc.product_id = USB_CONFIG_PID;
    desc.manufacturer_idx = plumbum->manuf.idx;
    desc.product_idx = plumbum->product.idx;
    desc.num_configurations = 1;
    plumbum_put_bytes(plumbum, (uint8_t*)&desc, sizeof(usb_descriptor_device_t));
    plumbum_ep0_ready(plumbum);
}

static void _req_config(plumbum_t *plumbum)
{
    mutex_lock(&plumbum->lock);
    plumbum_hdrs_fmt_conf(plumbum);
    mutex_unlock(&plumbum->lock);
    plumbum_ep0_ready(plumbum);
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
    switch (type) {
        case 0x1:
            _req_dev(plumbum);
            break;
        case 0x2:
            _req_config(plumbum);
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
            case 0x09:
                _activate_endpoints(plumbum);
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
    uint16_t destination = pkt->index & 0x0f;
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


static inline size_t plumbum_pkt_maxlen(plumbum_t *plumbum, usb_setup_t *pkt)
{
    return pkt->length > plumbum->in->len ? plumbum->in->len : pkt->length;
}

void recv_setup(plumbum_t *plumbum, usbdev_ep_t *ep)
{
    (void)ep;
    usb_setup_t *pkt = &plumbum->setup;
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
}

static void *_plumbum_thread(void *args)
{
    plumbum_t *plumbum = (plumbum_t*)args;
    plumbum_audio_t audio;

    plumbum_audio_block_clock_t a_clock;
    plumbum_audio_block_input_t a_input;
    plumbum_audio_block_output_t a_output;

    mutex_lock(&plumbum->lock);
    usbdev_t *dev = plumbum->dev;
    plumbum->pid = sched_active_pid;
    plumbum->addr = 0;
    plumbum->strings = NULL;
    plumbum->iface = NULL;
    plumbum->str_idx = 1;
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

    plumbum->in = plumbum->dev->driver->new_ep(plumbum->dev, USB_EP_TYPE_CONTROL, USB_EP_DIR_IN, PLUMBUM_MAX_SIZE);
    plumbum->out = plumbum->dev->driver->new_ep(plumbum->dev, USB_EP_TYPE_CONTROL, USB_EP_DIR_OUT, PLUMBUM_MAX_SIZE);
    plumbum->in->cb = _event_ep0_cb;
    plumbum->out->cb = _event_ep0_cb;
    plumbum->in->context = plumbum;
    plumbum->out->context = plumbum;

    plumbum->in->driver->init(plumbum->in);
    plumbum->out->driver->init(plumbum->out);
    _plumbum_config_ep0(plumbum);
    plumbum_add_string_descriptor(plumbum, &plumbum->config, USB_CONFIG_CONFIGURATION_STR);
    plumbum_add_string_descriptor(plumbum, &plumbum->product, USB_CONFIG_PRODUCT_STR);
    plumbum_add_string_descriptor(plumbum, &plumbum->manuf, USB_CONFIG_MANUF_STR);

    plumbum->state = PLUMBUM_STATE_DISCONNECT;
    mutex_unlock(&plumbum->lock);
    plumbum_audio_init(plumbum, &audio);
    keyboard_init(plumbum);

    plumbum_audio_add_clock(&audio, &a_clock, PLUMBUM_AUDIO_CLOCK_INTERNAL_FIXED);
    plumbum_audio_add_input(&audio, &a_input, USB_AUDIO_TERMINALTYPE_USB_STREAMING);
    plumbum_audio_add_output(&audio, &a_output, USB_AUDIO_TERMINALTYPE_ANALOG);

    a_output.clock = &a_clock;
    a_input.clock = &a_clock;

    a_output.source = (plumbum_audio_block_t*)&a_input;

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
                puts("Reset");
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

        DEBUG("plumbum_ep: pid: %u\n", plumbum->pid);
        if (msg_send(&msg, plumbum->pid) <= 0) {
            puts("plumbum_ep0: possibly lost interrupt.");
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
                        DEBUG("Setting addres %u\n", plumbum->addr);
                    }
                    plumbum->setup_state = PLUMBUM_SETUPRQ_READY;
                }
                else if (plumbum->setup_state == PLUMBUM_SETUPRQ_OUTACK && ep->dir == USB_EP_DIR_OUT) {
                    memset(&plumbum->builder, 0, sizeof(plumbum_controlbuilder_t));
                    static const usbopt_enable_t disable = USBOPT_DISABLE;
                    plumbum->in->driver->set(plumbum->in, USBOPT_EP_READY, &disable, sizeof(usbopt_enable_t));
                    plumbum->setup_state = PLUMBUM_SETUPRQ_READY;
                }
                else if (plumbum->setup_state == PLUMBUM_SETUPRQ_INDATA && ep->dir == USB_EP_DIR_IN) {
                    if (plumbum_update_builder(plumbum)) {
                        recv_setup(plumbum, ep);
                        plumbum->setup_state = PLUMBUM_SETUPRQ_INDATA;
                    }
                    else {
                        /* Ready out ZLP */
                        plumbum->setup_state = PLUMBUM_SETUPRQ_OUTACK;
                    }
                }
                else if (plumbum->setup_state == PLUMBUM_SETUPRQ_OUTDATA && ep->dir == USB_EP_DIR_OUT) {
                    /* Ready in ZLP */
                    plumbum->setup_state = PLUMBUM_SETUPRQ_INACK;
                    plumbum->in->driver->ready(plumbum->in, 0);
                }
                else if (ep->dir == USB_EP_DIR_OUT) {
                    memset(&plumbum->builder, 0, sizeof(plumbum_controlbuilder_t));
                    memcpy(&plumbum->setup, plumbum->out->buf, sizeof(usb_setup_t));
                    plumbum->builder.reqlen = plumbum->setup.length;
                    plumbum->out->driver->ready(plumbum->out, 0);
                    recv_setup(plumbum, ep);
                }
                break;
            case USBDEV_EVENT_TR_FAIL:
                if (ep->dir == USB_EP_DIR_OUT) {
                }
                else {
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
                    else {
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
                puts("unhandled event");
                break;
        }
    }
}
