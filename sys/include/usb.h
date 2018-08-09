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

#include <stdint.h>

#define USB_TYPE_DESCRIPTOR_DEVICE          0x01
#define USB_TYPE_DESCRIPTOR_CONFIGURATION   0x02
#define USB_TYPE_DESCRIPTOR_STRING          0x03
#define USB_TYPE_DESCRIPTOR_INTERFACE       0x04

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint16_t bcd_usb;
    uint8_t class;
    uint8_t subclass;
    uint8_t protocol;
    uint8_t max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t bcd_device;
    uint8_t manufacturer_idx;
    uint8_t product_idx;
    uint8_t serial_idx;
    uint8_t num_configurations;
} usb_descriptor_device_t;

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint16_t total_length;
    uint8_t num_interfaces;
    uint8_t val;
    uint8_t idx;
    uint8_t attributes;
    uint8_t max_power;
} usb_descriptor_configuration_t;

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint8_t interface_num;
    uint8_t alternate_setting;
    uint8_t num_endpoints;
    uint8_t class;
    uint8_t subclass;
    uint8_t protocol;
    uint8_t idx;
} usb_descriptor_interface_t;

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint8_t address;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} usb_descriptor_endpoint_t;


typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
} usb_descriptor_string_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} usb_setup_t;


#ifdef __cplusplus
}
#endif

#endif /* USB_H */
/** @} */

