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
#define ENABLE_DEBUG    (0)
#include "debug.h"

#define _USBMAN_MSG_QUEUE_SIZE    (8)

static usbman_t _usbman;
void _event_cb(usbdev_t *usbdev, usbdev_event_t event);
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
    usbman->pid = sched_active_pid;
    msg_t msg, msg_queue[_USBMAN_MSG_QUEUE_SIZE];
    DEBUG("usbman: starting thread %i\n", sched_active_pid);
    /* setup the link-layer's message queue */
    msg_init_queue(msg_queue, _USBMAN_MSG_QUEUE_SIZE);
    /* register the event callback with the device driver */
    dev->cb = _event_cb;
    /* initialize low-level driver */
    dev->driver->init(dev);

    usbopt_enable_t enable = USBOPT_ENABLE;
    dev->driver->set(dev, USBOPT_ATTACH, &enable, sizeof(usbopt_enable_t));
    while (1) {
        DEBUG("usbman: waiting for incoming messages\n");
        msg_receive(&msg);
        /* dispatch netdev, MAC and gnrc_netapi messages */
        switch (msg.type) {
            case USBDEV_MSG_TYPE_EVENT:
                DEBUG("gnrc_netif: GNRC_NETDEV_MSG_TYPE_EVENT received\n");
                dev->driver->esr(dev);
                break;
        }
    }
    return NULL;
}

void _event_cb(usbdev_t *usbdev, usbdev_event_t event)
{
    usbman_t *usbman = (usbman_t *)usbdev->context;
    if (event == USBDEV_EVENT_ESR) {
        msg_t msg = { .type = USBDEV_MSG_TYPE_EVENT,
                      .content = { .ptr = usbdev } };

        if (msg_send(&msg, usbman->pid) <= 0) {
            puts("gnrc_netif: possibly lost interrupt.");
        }
    }
}
