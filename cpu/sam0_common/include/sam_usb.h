/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_sam0_common
 * @brief
 *
 * @{
 *
 * @file
 * @brief       brief
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef SAM_USB_H
#define SAM_USB_H

#include <stdint.h>
#include <stdlib.h>
#include "usb/usbdev.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAM_USB_BUF_SPACE   1024

typedef struct {
    usbdev_t usbdev;
    uint16_t int_flags;
    uint8_t buffer[SAM_USB_BUF_SPACE];
    size_t used;                        /**< Number of bytes from the buffer that are used */
} sam0_common_usb_t;

void usb_init(void);

void usb_attach(void);
void usb_detach(void);
#ifdef __cplusplus
}
#endif
#endif /* SAM_USB_H */
/** @} */
