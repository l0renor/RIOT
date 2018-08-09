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
 * @brief   USB manager thread, handles driver interaction and EP0
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include "thread.h"
#include "usb/usbdev.h"
#include "usb/usbman.h"

#include <string.h>
#define ENABLE_DEBUG    (0)
#include "debug.h"

#define _USBMAN_MSG_QUEUE_SIZE    (8)

static usbman_t _usbman;
static uint8_t in_buf[1024];
static uint8_t out_buf[1024];

void _event_cb(usbdev_t *usbdev, usbdev_event_t event);
void _event_ep_in_cb(usbdev_ep_t *ep, usbdev_event_t event);
void _event_ep_out_cb(usbdev_ep_t *ep, usbdev_event_t event);
static void *_usbman_thread(void *args);

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

static void *_usbman_thread(void *args)
{
    usbman_t *usbman = (usbman_t*)args;
    usbdev_t *dev = usbman->dev;
    usbdev_ep_t *ep0_in, *ep0_out;
    usbman->pid = sched_active_pid;
    usbman->addr = 0;
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

    usbopt_enable_t enable = USBOPT_ENABLE;
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
    dev->driver->set(dev, USBOPT_ATTACH, &enable, sizeof(usbopt_enable_t));

    while (1) {
        msg_receive(&msg);
        /* dispatch netdev, MAC and gnrc_netapi messages */
        switch (msg.type) {
            case USBDEV_MSG_TYPE_EVENT:
                dev->driver->esr(dev);
                break;
            case USBDEV_MSG_TYPE_EP0OUT_EVENT:
                usbman->out->driver->esr(usbman->out);
                break;
            case USBDEV_MSG_TYPE_EP0IN_EVENT:
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
    (void)pkt;
    printf("usbman: EP%d, %s: setup packet type 0x%x. request: 0x%x, value: 0x%x\n", ep->num, ep->dir == USBDEV_DIR_OUT? "out" : "in", pkt->type, pkt->request, pkt->value);
}

void _req_descriptor(usbman_t *usbman)
{
    usb_descriptor_device_t *desc = (usb_descriptor_device_t*)in_buf;
    desc->length = sizeof(usb_descriptor_device_t);
    desc->type = USB_TYPE_DESCRIPTOR_DEVICE;
    desc->bcd_usb = 0x0101;
    desc->max_packet_size = 64;
    desc->vendor_id = 0x03eb;
    desc->product_id = 0x0001;
    usbman->in->driver->ready(usbman->in, sizeof(usb_descriptor_device_t));
}

void _req_status(usbman_t *usbman)
{
    memset(in_buf, 0, 2);
    usbman->in->driver->ready(usbman->in, 2);
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
                _req_descriptor(usbman);
                break;
            default:
                break;
        }
    }
    else{
        switch (pkt->request) {
            case 0x05:
                usbman->addr = pkt->value;
                break;
            default:
                break;
        }
        /* Signal zlp */
        usbman->in->driver->ready(usbman->in, 0);
    }
    usbman->out->driver->ready(usbman->out, 0);
    _print_setup(ep, pkt);
}

void _event_cb(usbdev_t *usbdev, usbdev_event_t event)
{
    usbman_t *usbman = (usbman_t *)usbdev->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = USBDEV_MSG_TYPE_EVENT,
                      .content = { .ptr = usbdev } };

        if (msg_send(&msg, usbman->pid) <= 0) {
            puts("usbman: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_RESET:
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
        msg_t msg = { .type = USBDEV_MSG_TYPE_EP0OUT_EVENT,
                      .content = { .ptr = ep } };

        if (msg_send(&msg, usbman->pid) <= 0) {
            puts("usbman_ep: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_TR_COMPLETE:
                usbman->out->driver->ready(usbman->out, 0);
                if (usbman->addr) {
                    usbman->dev->driver->set(usbman->dev, USBOPT_ADDRESS, &usbman->addr, sizeof(uint8_t));
                    usbman->addr = 0;
                }
                puts("out rcvd\n");
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
        msg_t msg = { .type = USBDEV_MSG_TYPE_EP0IN_EVENT,
                      .content = { .ptr = ep} };

        if (msg_send(&msg, usbman->pid) <= 0) {
            puts("usbman_ep: possibly lost interrupt.");
        }
    }
    else {
        switch (event) {
            case USBDEV_EVENT_TR_COMPLETE:
                break;
            default:
                break;
        }
    }
}
