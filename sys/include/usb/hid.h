/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_HID_H
#define USB_HID_H

#include <stdint.h>
#include "usb/plumbum.h"

#ifdef __cplusplus
extern "c" {
#endif

#define USB_HID_SUBCLASS_NONE   0x00
#define USB_HID_SUBCLASS_BOOT   0x01

#define USB_HID_PROTOCOL_NONE       0x00
#define USB_HID_PROTOCOL_KEYBOARD   0x01
#define USB_HID_PROTOCOL_MOUSE      0x02

#define USB_HID_COUNTRYCODE_NONE   0x00

#define USB_HID_DESCRIPTOR_TYPE_REPORT 0x22

#define USB_TYPE_DESCRIPTOR_HID     0x21 /**< HID descriptor         */


typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint16_t bcd_hid;
    uint8_t country_code;
    uint8_t num_descriptors;
    uint8_t report_type;
    uint16_t report_length;
} usb_descriptor_hid_t;
    
typedef struct {
    plumbum_handler_t handler;
    plumbum_interface_t iface;
    plumbum_endpoint_t ep;
    plumbum_hdr_gen_t hid_hdr;
    uint8_t state;
    uint8_t prev_state;
} plumbum_hid_device_t;

#ifdef __cplusplus
}
#endif

#endif /* USB_MESSAGE_H */
/** @} */

