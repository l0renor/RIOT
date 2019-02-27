/*
 * Copyright (C) 2018 Dylan Laduranty <dylan.laduranty@mesotic.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_CDC_H
#define USB_CDC_H

#include <stdint.h>
#include "usb.h"

#ifdef __cplusplus
extern "c" {
#endif

#define USB_CDC_VERSION_BCD          0x0120 /**< USB CDC version in BCD */

/**
 * @name USB CDC subclass types
 * @{
 */
#define USB_CDC_SUBCLASS_NONE          0x00 /**< No subclass */
#define USB_CDC_SUBCLASS_DLCM          0x01 /**< Direct Line Control Model */
#define USB_CDC_SUBCLASS_ACM           0x02 /**< Abstract Control Model */
#define USB_CDC_SUBCLASS_TCM           0x03 /**< Telephone Control Model */
#define USB_CDC_SUBCLASS_MCCM          0x04 /**< Multi-Channel Control Model */
#define USB_CDC_SUBCLASS_CCM           0x05 /**< CAPI Control Mode */
#define USB_CDC_SUBCLASS_ENCM          0x06 /**< Eth Networking Control Model */
#define USB_CDC_SUBCLASS_ANCM          0x07 /**< ATM Networking Control Model */
#define USB_CDC_SUBCLASS_WHCM          0x08 /**< Wireless Handset Control Model */
#define USB_CDC_SUBCLASS_DM            0x09 /**< Device Management */
#define USB_CDC_SUBCLASS_MDLM          0x0A /**< Mobile Direct Line Model */
#define USB_CDC_SUBCLASS_OBEX          0x0B /**< OBEX */
#define USB_CDC_SUBCLASS_EEM           0x0C /**< Ethernet Emulation Model */
#define USB_CDC_SUBCLASS_NCM           0x0D /**< Network Control Model */
/** @} */

/**
 * @name USB CDC protocol types
 * @{
 */
#define USB_CDC_PROTOCOL_NONE          0x00 /**< No protocol required */
#define USB_CDC_PROTOCOL_ITU           0x01 /**< AT Commands: V.250 etc */
#define USB_CDC_PROTOCOL_PCCA          0x02 /**< AT Commands defined by PCCA-101 */
#define USB_CDC_PROTOCOL_PCCA_A        0x03 /**< AT Commands defined by PCCA-101 & Annex O */
#define USB_CDC_PROTOCOL_GSM           0x04 /**< AT Commands defined by GSM 07.07 */
#define USB_CDC_PROTOCOL_3GPP          0x05 /**< AT Commands defined by 3GPP 27.007 */
#define USB_CDC_PROTOCOL_CS            0x06 /**< AT Commands defined by TIA for CDMA */
#define USB_CDC_PROTOCOL_EEM           0x07 /**< Ethernet Emulation Model */
#define USB_CDC_PROTOCOL_EXT           0xFE /**< External Protocol */
#define USB_CDC_PROTOCOL_VENDOR        0xFF /**< Vendor-specific */
/** @} */
/**
 * @name USB CDC descriptor subtypes
 */
#define USB_CDC_DESCR_SUBTYPE_FUNCTIONAL    0x00 /**< Header functional
                                                   *  descriptor */
#define USB_CDC_DESCR_SUBTYPE_UNION         0x06 /** Union descriptor */
#define USB_CDC_DESCR_SUBTYPE_ETH_NET       0x0f /** Ethernet descriptor */
/** @} */

/**
 * @name USB CDC management requests
 * @{
 */

/**
 * @brief Set ethernet multicast filter request
 */
#define USB_CDC_MGNT_REQUEST_SET_ETH_MULTICAST_FILTER   0x40

/**
 * @brief Set ethernet power management pattern filter
 */
#define USB_CDC_MGNT_REQUEST_SET_ETH_PM_PATTERN_FILTER  0x41

/**
 * @brief Get ethernet power management pattern filter
 */
#define USB_CDC_MGNT_REQUEST_GET_ETH_PM_PATTERN_FILTER  0x42

/**
 * @brief Set ethernet packet filter
 */
#define USB_CDC_MGNT_REQUEST_SET_ETH_PACKET_FILTER      0x43

/**
 * @brief Get ethernet statistics
 */
#define USB_CDC_MGNT_REQUEST_GET_ETH_STATISTICS         0x44
/** @} */

/**
 * @name USB CDC management notifications
 * @{
 */

/**
 * @brief Network connection status notification
 */
#define USB_CDC_MGNT_NOTIF_NETWORK_CONNECTION   0x00

/**
 * @brief Response available notification
 */
#define USB_CDC_MGNT_NOTIF_RESPONSE_AVAILABLE   0x01

/**
 * @brief Hook on the auxiliary phone changed notification
 */
#define USB_CDC_MGNT_NOTIF_AUX_JACK_HOOK_STATE  0x08

/**
 * @brief Ring voltage on the POTS line interface notification
 */
#define USB_CDC_MGNT_NOTIF_RING_DETECT          0x09

/**
 * @brief Asynchronous UART status notification
 */
#define USB_CDC_MGNT_NOTIF_SERIAL_STATE         0x20

/**
 * @brief Call state change notification
 */
#define USB_CDC_MGNT_NOTIF_CALL_STATE_CHANGE    0x28

/**
 * @brief Line state change notification
 */
#define USB_CDC_MGNT_NOTIF_LINE_STATE_CHANGE    0x29

/**
 * @brief Throughput change notification
 */
#define USB_CDC_MGNT_NOTIF_CONN_SPEED_CHANGE    0x2A

/** @} */

#define USB_HID_COUNTRYCODE_NONE        0x00

#define USB_HID_DESCRIPTOR_TYPE_REPORT  0x22

#define USB_TYPE_DESCRIPTOR_CDC         0x24 /**< CDC descriptor         */

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint8_t subtype;
    uint8_t macaddress;
    uint32_t ethernetstatistics;
    uint16_t maxsegmentsize;
    uint16_t numbermcfilters;
    uint8_t numberpowerfilters;
} usb_desc_ecm_t;

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint8_t subtype;
    uint8_t capabalities;
} usb_desc_acm_t;

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint8_t subtype;
    uint16_t bcd_hid;
} usb_desc_cdc_t;

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint8_t subtype;
    uint8_t master_if;
    uint8_t slave_if;
} usb_desc_union_t;

typedef struct __attribute__((packed)) {
    uint8_t length;
    uint8_t type;
    uint8_t subtype;
    uint8_t capabalities;
    uint8_t data_if;
} usb_desc_call_mngt_t;

#ifdef __cplusplus
}
#endif

#endif /* USB_CDC_H */
/** @} */
