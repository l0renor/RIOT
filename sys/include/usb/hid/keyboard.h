/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_HID_KEYBOARD_H
#define USB_HID_KEYBOARD_H

#ifdef __cplusplus
extern "c" {
#endif

/**
 * @brief Keyboard output status flags
 * @{
 */
#define USB_HID_KEYBOARD_OUT_NUM_LOCK       0x01
#define USB_HID_KEYBOARD_OUT_CAPS_LOCK      0x02
#define USB_HID_KEYBOARD_OUT_SCROLL_LOCK    0x04
#define USB_HID_KEYBOARD_OUT_COMPOSE        0x08
#define USB_HID_KEYBOARD_OUT_KANA           0x10
/** @} */

/**
 * @brief Keyboard modifier key flags
 * @{
 */
#define USB_HID_KEYBOARD_FLAG_L_CTRL        0x01
#define USB_HID_KEYBOARD_FLAG_L_SHIFT       0x02
#define USB_HID_KEYBOARD_FLAG_L_ALT         0x04
#define USB_HID_KEYBOARD_FLAG_L_GUI         0x08
#define USB_HID_KEYBOARD_FLAG_R_CTRL        0x10
#define USB_HID_KEYBOARD_FLAG_R_SHIFT       0x20
#define USB_HID_KEYBOARD_FLAG_R_ALT         0x40
#define USB_HID_KEYBOARD_FLAG_R_GUI         0x80
/** @} */

#define USB_HID_KEYBOARD_NUM_KEYS           6

#define USB_SETUP_REQUEST_TYPE_HID_REPORT   0x22
#define USB_SETUP_REQUEST_TYPE_IDLE         0x0a

typedef struct __attribute__((packed)) {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t key[USB_HID_KEYBOARD_NUM_KEYS];
} usb_hid_keyboard_t;

void keyboard_init(plumbum_t *plumbum);

#ifdef __cplusplus
}
#endif

#endif /* USB_HID_KEYBOARD_H */
/** @} */
