/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_uuid RFC 4122 compliant UUID's
 * @ingroup     sys
 * @brief       Provides RFC 4122 compliant UUID's
 *
 * This module provides RFC 4122 compliant UUID generation. The UUID stored in
 * @ref uuid_t struct is stored in network byte order.
 *
 * @{
 *
 * @file
 * @brief       [RFC 4122](https://tools.ietf.org/html/rfc4122) UUID functions
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef USB_USBMAN_H
#define USB_USBMAN_H

#include <stdint.h>
#include <stdlib.h>
#include "usb/usbdev.h"
#include "usb.h"

#include "kernel_types.h"
#include "msg.h"
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USBMAN_MSG_TYPE_EVENT   (0x1234)
#define USBMAN_MSG_TYPE_EP0IN_EVENT   (0x1235)
#define USBMAN_MSG_TYPE_EP0OUT_EVENT   (0x1236)

typedef enum {
    USBMAN_STATE_DISCONNECT,
    USBMAN_STATE_RESET,
    USBMAN_STATE_ADDR,
    USBMAN_STATE_CONFIGURED,
    USBMAN_STATE_SUSPEND,
} usbman_state_t;

typedef struct usbman_string usbman_string_t;
typedef struct usbman_interface usbman_interface_t;
typedef struct usbman_endpoint usbman_endpoint_t;

struct usbman_string {
    struct usbman_string *next;
    uint16_t idx;
    const char *str;
    size_t len;
};

struct usbman_endpoint {
    struct usbman_endpoint *next;
    usbdev_ep_t *ep;
};

struct usbman_interface {
    struct usbman_interface *next;
    uint16_t idx;
    usbman_endpoint_t *ep;                   /** LL of endpoints */
};

typedef struct {
    usbman_string_t manuf;
    usbman_string_t product;
    usbdev_ep_t *out;                       /**< EP0 out endpoint */
    usbdev_ep_t *in;                        /**< EP0 in endpoint */
    usbdev_t *dev;                          /**< usb phy device of the usb manager */
    usbman_string_t *strings;
    usbman_interface_t *iface;              /**< Linked list of interfaces */
    kernel_pid_t pid;                       /**< PID of the usb manager's thread */
    uint16_t addr;
    usbman_state_t state;
    uint16_t str_idx;
    mutex_t lock;
} usbman_t;


void usbman_init(void);
void usbman_create(char *stack, int stacksize, char priority,
                   const char *name, usbdev_t *usbdev);


#ifdef __cplusplus
}
#endif
#endif /* USB_USBMAN_H */
/** @} */
