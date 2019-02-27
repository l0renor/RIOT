/*
 * Copyright (C) 2019 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_USBUS_CDC_ECM_H
#define USB_USBUS_CDC_ECM_H

#ifdef __cplusplus
extern "c" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include "net/ethernet.h"
#include "usb/descriptor.h"
#include "usb/usbus.h"
#include "net/netdev.h"
#include "mutex.h"

/**
 * @brief Link throughput as reported by the peripheral
 *
 * This defines a common up and down link throughput in bits/second. The USB
 * peripheral will report this to the host. This doesn't affect the actual
 * throughput, only what the peripheral reports.
 */
#ifndef USBUS_CDC_ECM_CONFIG_SPEED
#define USBUS_CDC_ECM_CONFIG_SPEED  1000000
#endif

/**
 * @brief Link download speed as reported by the peripheral
 */
#ifndef USBUS_CDC_ECM_CONFIG_SPEED_DOWNSTREAM
#define USBUS_CDC_ECM_CONFIG_SPEED_DOWNSTREAM USBUS_CDC_ECM_CONFIG_SPEED
#endif

/**
 * @brief Link upload speed as reported by the peripheral
 */
#ifndef USBUS_CDC_ECM_CONFIG_SPEED_UPSTREAM
#define USBUS_CDC_ECM_CONFIG_SPEED_UPSTREAM   USBUS_CDC_ECM_CONFIG_SPEED
#endif

/**
 * @brief Signal that the RX buffer can be flushed
 */
#define USBUS_MSG_CDCECM_RX_FLUSH (USBUS_MSG_TYPE_HANDLER | 0x01)
#define USBUS_MSG_CDCECM_TX_XMIT  (USBUS_MSG_TYPE_HANDLER | 0x02)

#define USBUS_CDCECM_EP_CTRL_SIZE  16
#define USBUS_CDCECM_EP_DATA_SIZE  64

typedef enum {
    USBUS_CDCECM_NOTIF_NONE,
    USBUS_CDCECM_NOTIF_LINK_UP,
    USBUS_CDCECM_NOTIF_SPEED,
} usbus_cdcecm_notif_t;

typedef struct __attribute__((packed)) {
    usb_setup_t setup;
    uint32_t down;
    uint32_t up;
} usbus_cdcecm_speed_notif_t;

typedef struct usbus_cdceem_device {
    usbus_handler_t handler_ctrl;
    usbus_interface_t iface_data;
    usbus_interface_t iface_ctrl;
    usbus_interface_alt_t iface_data_alt;
    usbus_endpoint_t ep_in;
    usbus_endpoint_t ep_out;
    usbus_endpoint_t ep_ctrl;
    usbus_hdr_gen_t cdc_hdr;
    usbus_hdr_gen_t ecm_hdr;
    usbus_hdr_gen_t union_hdr;
    usbus_hdr_gen_t call_mngt_hdr;
    netdev_t netdev;
    uint8_t mac_netdev[6]; /**< this device's MAC address */
    char mac_host[13];     /**< host side's MAC address as string */
    usbus_string_t mac_str;
    usbus_t *usbus;
    mutex_t out_lock;               /**< mutex used for locking concurrent sends */
    size_t tx_len;
    uint8_t in_buf[ETHERNET_FRAME_LEN];
    size_t len;                     /**< Length of the current frame */
    usbus_cdcecm_notif_t notif;   /**< Startup notification tracker */
    unsigned active_iface;
} usbus_cdcecm_device_t;

int usbus_cdcecm_init(usbus_t *usbus, usbus_cdcecm_device_t *handler);
#ifdef __cplusplus
}
#endif

#endif /* USB_USBUS_CDC_ECM_H */
/** @} */

