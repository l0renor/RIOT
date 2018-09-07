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
} usb_descriptor_audio_interface_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t attributes;
    uint8_t controls;
    uint8_t assocterminal;
    uint8_t clocksource;
} usb_descriptor_audio_clock_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t nrinpins;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t terminalid;
    uint16_t terminaltype;
    uint8_t assocterminal;
    uint8_t clocksourceid;
    uint8_t nrchannels;
    uint32_t channelconfig;
    uint8_t channelnamesidx;
    uint16_t controls;
    uint8_t terminalidx;
} usb_descriptor_audio_input_terminal_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t terminalid;
    uint16_t terminaltype;
    uint8_t assocterminal;
    uint8_t sourceid;
    uint8_t clocksourceid;
    uint16_t controls;
    uint8_t terminalidx;
} usb_descriptor_audio_output_terminal_t;



#ifdef __cplusplus
}
#endif
#endif /* USB_AUDIO_H */
/** @} */
