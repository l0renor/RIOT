/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    usb_audio USB audio 2.0 and 3.0 definitions
 * @ingroup     usb
 *
 * @{
 *
 * @file
 * @brief       USB audio device class 2.0 and 3.0 descriptors
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

#define USB_AUDIO_SUBCLASS_CONTROL  0x01
#define USB_AUDIO_SUBCLASS_STREAM   0x02

#define USB_AUDIO_PROTOCOL_V1   0x10
#define USB_AUDIO_PROTOCOL_V2   0x20
#define USB_AUDIO_PROTOCOL_V3   0x30

#define USB_AUDIO_CS_UNDEFINED      0x20
#define USB_AUDIO_CS_DEVICE         0x21
#define USB_AUDIO_CS_CONFIGURATION  0x22
#define USB_AUDIO_CS_STRING         0x23
#define USB_AUDIO_CS_INTERFACE      0x24
#define USB_AUDIO_CS_ENDPOINT       0x25
#define USB_AUDIO_CS_CLUSTER        0x26

/* V2 defines (V3 is different) */
#define USB_AUDIO_AC_SUBTYPE_HEADER             0x01
#define USB_AUDIO_AC_SUBTYPE_INPUT_TERMINAL     0x02
#define USB_AUDIO_AC_SUBTYPE_OUTPUT_TERMINAL    0x03
#define USB_AUDIO_AC_SUBTYPE_CLOCK_SOURCE       0x0A

#define USB_AUDIO_AS_SUBTYPE_STREAMING          0x01
#define USB_AUDIO_AS_SUBTYPE_FORMAT             0x02
#define USB_AUDIO_AS_SUBTYPE_ENCODER            0x03

#define USB_AUDIO_AS_EP_SUBTYPE_UNDEFINED       0x00
#define USB_AUDIO_AS_EP_SUBTYPE_EP_GENERAL      0x01

#define USB_AUDIO_TERMINALTYPE_USB_STREAMING    0x0101
#define USB_AUDIO_TERMINALTYPE_USB_VENDOR       0x01FF

#define USB_AUDIO_TERMINALTYPE_MICROPHONE       0x0201

#define USB_AUDIO_TERMINALTYPE_SPEAKER          0x0301

#define USB_AUDIO_TERMINALTYPE_ANALOG           0x0601

/**
 * @brief Audio class control request layout
 */
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t request;
    uint8_t cn;             /**< Channel number or mixer control number */
    uint8_t cs;             /**< Control selector */
    uint8_t interface;
    uint8_t id;
    uint16_t length;
} usb_audio_control_request_t;

/* USB ADC v2 structs */

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint16_t bcdadc;            /**< 0x20 */
    uint8_t category;
    uint16_t totallength;
    uint8_t controls;
} usb_descriptor_ac2_interface_t;

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
    uint8_t channelidx;
    uint16_t controls;
    uint8_t terminalidx;
} usb_descriptor_ac2_input_terminal_t;

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
} usb_descriptor_ac2_output_terminal_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t unitid;
    uint8_t nrpins;
} usb_descriptor_ac2_mixer_unit_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t attributes;
    uint8_t controls;
    uint8_t assocterminal;
    uint8_t idx;
} usb_descriptor_ac2_clock_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t clocksourceid;
    uint8_t controls;
    uint8_t idx;
} usb_decriptor_ac2_clock_mult_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t pwrdomainid;
    uint16_t recoverytime1;     /**< Recovery time from D1 to D0 in 50us increments     */
    uint16_t recoverytime2;     /**< Recovery time from D2 to D0 in 50us increments     */
    uint8_t nrentities;
} usb_descriptor_ac2_power_domain_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t terminallink;            /**< Audio class specification release                  */
    uint8_t controls;
    uint8_t formattype;
    uint32_t formats;
    uint8_t nrchannels;
    uint32_t channelconf;
    uint8_t channelidx;
} usb_descriptor_as2_interface_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t formattype;         /**< FORMAT_TYPE_1 */
    uint8_t subslotsize;
    uint8_t bitres;
} usb_descriptor_as2_format1_t;

typedef struct __attribute__((packed)) {                                        
    uint8_t length;             /**< Size of this descriptor */                 
    uint8_t type;               /**< Descriptor type (@ref )  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t attributes;         /**< D7: maxpacketsonly */                           
    uint8_t controls;                                                        
    uint8_t lockdelayunits;
    uint16_t lockdelay;
} usb_descriptor_as2_endpoint_t; 
/* USB ADC v3 structs */

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t category;
    uint16_t totallength;
    uint32_t controls;
} usb_descriptor_ac3_interface_t;

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
    uint8_t terminalidx;
} usb_descriptor_ac3_input_terminal_t;

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
    uint8_t terminalidx;
} usb_descriptor_ac3_output_terminal_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t unitid;
    uint8_t nrpins;
} usb_descriptor_ac3_mixer_unit_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t attributes;
    uint8_t controls;
    uint8_t referenceterminal;
    uint8_t clocksourceidx;
} usb_descriptor_ac3_clock_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t clockid;
    uint8_t clocksourceid;
    uint8_t controls;
    uint8_t multiplieridx;
} usb_decriptor_ac3_clock_mult_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint8_t pwrdomainid;
    uint16_t recoverytime1;     /**< Recovery time from D1 to D0 in 50us increments     */
    uint16_t recoverytime2;     /**< Recovery time from D2 to D0 in 50us increments     */
    uint8_t nrentities;
} usb_descriptor_ac3_power_domain_t;

typedef struct __attribute__((packed)) {
    uint8_t length;             /**< Size of this descriptor */
    uint8_t type;               /**< Descriptor type (@ref USB_TYPE_DESCRIPTOR_DEVICE)  */
    uint8_t subtype;            /**< Descriptor subtype                                 */
    uint16_t bcdadc;            /**< Audio class specification release                  */
    uint8_t category;
    uint16_t totallength;
    uint8_t controls;
} usb_descriptor_as3_interface_t;

#ifdef __cplusplus
}
#endif
#endif /* USB_AUDIO_H */
/** @} */
