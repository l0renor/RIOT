/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    usb_audio USB audio 3.0 definitions
 * @ingroup     usb
 *
 * @{
 *
 * @file
 * @brief       USB audio device class 3.0 descriptors
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef USB_AUDIO_H
#define USB_AUDIO_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint16_t bcdadc;            /**< Audio class specification release                  */
    uint8_t category;
    uint16_t totallength;
    uint8_t controls;
} usb_descriptor_ac_interface_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t nrinpins;
} usb_descriptor_ac_clock

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t terminalid;
    uint16_t terminaltype;
    uint8_t assocterminal;
    uint8_t clocksourceid;
    uint32_t controls;
    uint16_t clusterdescridx;
    uint16_t terminaldescridx;
    uint16_t connectordercridx;;
    uint16_t terminalidx;
} usb_descriptor_ac_input_terminal_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t terminalid;
    uint16_t terminaltype;
    uint8_t assocterminal;
    uint8_t sourceid;
    uint8_t clocksourceid;
    uint32_t controls;
    uint16_t terminaldescridx;
    uint16_t connectordercridx;;
    uint16_t terminalidx;
} usb_descriptor_ac_output_terminal_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t unitid;
    uint8_t nrpins;
} usb_descriptor_ac_mixer_unit_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t attributes;
    uint8_t controls;
    uint8_t referenceterminal;
    uint8_t clocksourceidx;
} usb_descriptor_ac_clock_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t clocksourceid;
    uint8_t controls;
    uint8_t multiplieridx;
} usb_decriptor_ac_clock_mult_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t pwrdomainid;
    uint16_t recoverytime1;     /**< Recovery time from D1 to D0 in 50us increments     */
    uint16_t recoverytime2;     /**< Recovery time from D2 to D0 in 50us increments     */
    uint8_t nrentities;
} usb_descriptor_ac_power_domain_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint16_t bcdadc;            /**< Audio class specification release                  */
    uint8_t category;
    uint16_t totallength;
    uint8_t controls;
} usb_descriptor_as_interface_t;

#ifdef __cplusplus
}
#endif
#endif /* USB_AUDIO_H */
/** @} */
