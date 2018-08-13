/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_H
#define USB_H

#ifdef __cplusplus
extern "c" {
#endif

#include "usb/hdr.h"

#ifndef USB_CONFIG_VID
#define USB_CONFIG_VID          (0x1234)
#endif

#ifndef USB_CONFIG_PID
#define USB_CONFIG_PID          (0x5678)
#endif

#ifndef USB_CONFIG_MANUF_STR
#define USB_CONFIG_MANUF_STR   "RIOT-os.org"
#endif

#ifndef USB_CONFIG_PRODUCT_STR
#define USB_CONFIG_PRODUCT_STR  "USB device"
#endif

#ifndef USB_CONFIG_PRODUCT_BCDVERSION
#define USB_CONFIG_PRODUCT_BCDVERSION   "0x0100"
#endif

#ifndef USB_CONFIG_SELF_POWERED
#define USB_CONFIG_SELF_POWERED   (0)
#endif

#ifndef USB_CONFIG_DEFAULT_LANGID
#define USB_CONFIG_DEFAULT_LANGID   0x0409
#endif

/**
 * @brief USB device max power draw in mA, between 0 and 500mA
 */
#ifndef USB_CONFIG_MAX_POWER
#define USB_CONFIG_MAX_POWER   (100)
#endif

#ifdef __cplusplus
}
#endif

#endif /* USB_H */
/** @} */
