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

#include "kernel_types.h"
#include "msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USBDEV_MSG_TYPE_EVENT   (0x1234)
#define USBDEV_MSG_TYPE_EP0IN_EVENT   (0x1235)
#define USBDEV_MSG_TYPE_EP0OUT_EVENT   (0x1236)

typedef struct {
    usbdev_ep_t *out;                       /**< EP0 out endpoint */
    usbdev_ep_t *in;                        /**< EP0 in endpoint */
    usbdev_t *dev;                          /**< usb phy device of the usb manager */
    kernel_pid_t pid;                       /**< PID of the usb manager's thread */
    uint16_t addr;
} usbman_t;

void usbman_create(char *stack, int stacksize, char priority,
                   const char *name, usbdev_t *usbdev);


#ifdef __cplusplus
}
#endif
#endif /* USB_USBMAN_H */
/** @} */
