/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_USBOPT_H
#define USB_USBOPT_H

#ifdef __cplusplus
extern "c" {
#endif

#include <stdint.h>

typedef enum
{
    USBOPT_DISABLE = 0,
    USBOPT_ENABLE  = 1,
} usbopt_enable_t;

typedef enum {
    /**
     * @brief Set the USB device address
     */
    USBOPT_ADDRESS,
    /**
     * @brief getter for endpoint 0
     */
    USBOPT_EP0_IN,
    USBOPT_EP0_OUT,
    USBOPT_ATTACH,
} usbopt_t;

typedef enum {
    USBOPT_EP_BUF_ADDR,
    USBOPT_EP_BUF_SIZE,
    USBOPT_EP_ENABLE,
} usbopt_ep_t;

#ifdef __cplusplus
}
#endif

#endif /* USB_H */
/** @} */
