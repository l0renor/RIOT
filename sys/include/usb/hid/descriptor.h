/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */
/**
 * @defgroup    usb_hid_descriptor - USB HID report descriptor list
 * @ingroup     usb_hid
 * @brief       Defines for generating USB HID report descriptors.
 *
 * @{
 *
 * @file
 * @brief       Definitions of USB HID report descriptor components.
 *
 * List of defines to build HID report descriptors from. This list does not
 * pretend to be complete. Please extend where necessary for your application
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */
#ifndef USB_HID_DESCRIPTOR_H
#define USB_HID_DESCRIPTOR_H

#ifdef __cplusplus
extern "c" {
#endif

#define USB_HID_DATA_LEN_0          0x0
#define USB_HID_DATA_LEN_1          0x1
#define USB_HID_DATA_LEN_2          0x2
#define USB_HID_DATA_LEN_4          0x3

#define USB_HID_DESC_USAGE_PAGE          0x04
#define USB_HID_DESC_USAGE               0x08
#define USB_HID_DESC_USAGE_MINIMUM       0x18
#define USB_HID_DESC_USAGE_MAXIMUM       0x28
#define USB_HID_DESC_LOGICAL_MINIMUM     0x14
#define USB_HID_DESC_LOGICAL_MAXIMUM     0x24
#define USB_HID_DESC_PHYSICAL_MINIMUM    0x34
#define USB_HID_DESC_PHYSICAL_MAXIMUM    0x44
#define USB_HID_DESC_INPUT               0x80
#define USB_HID_DESC_OUTPUT              0x90
#define USB_HID_DESC_REPORT_ID           0x84

#define USB_HID_DESC_REPORT_COUNT        0x94
#define USB_HID_DESC_REPORT_SIZE         0x74

#define USB_HID_DESC_COLLECTION          0xA0
#define USB_HID_DESC_END_COLLECTION      0xC0


#define USB_HID_USAGE_CONSUMER_CONTROL  USB_HID_DESC_USAGE | USB_HID_DATA_LEN_1, 0x01

/**
 * @brief USB HID report descriptor usage page arguments
 */
#define USB_HID_USAGE_PAGE_GENERIC_DESKTOP      USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x01
#define USB_HID_USAGE_PAGE_SIMULATION_CONTROLS  USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x02
#define USB_HID_USAGE_PAGE_VR_CONTROLS          USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x03
#define USB_HID_USAGE_PAGE_SPORT_CONTROLS       USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x04
#define USB_HID_USAGE_PAGE_GAME_CONTROLS        USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x05
#define USB_HID_USAGE_PAGE_GENERIC_DEVICE       USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x06
#define USB_HID_USAGE_PAGE_KEYBOARD_KEYPAD      USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x07
#define USB_HID_USAGE_PAGE_LED                  USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x08
#define USB_HID_USAGE_PAGE_BUTTON               USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x09
#define USB_HID_USAGE_PAGE_ORDINAL              USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x0A
#define USB_HID_USAGE_PAGE_TELEPHONY            USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x0B
#define USB_HID_USAGE_PAGE_CONSUMER_CONTROL     USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x0C
#define USB_HID_USAGE_PAGE_DIGITIZER            USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x0D
#define USB_HID_USAGE_PAGE_ALPHANUMERIC_DISP    USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x14
#define USB_HID_USAGE_PAGE_SENSOR               USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x20
#define USB_HID_USAGE_PAGE_MEDICAL_INSTRUMENT   USB_HID_DESC_USAGE_PAGE|USB_HID_DATA_LEN_0, 0x40

#ifdef __cplusplus
}
#endif

#endif /* USB_HID_DESCRIPTOR_H */
/** @} */
