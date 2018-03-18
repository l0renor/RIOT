/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup drivers_netdev_api
 * @brief
 * @{
 *
 * @file
 * @brief   Definitions for netdev common IEEE 802.15.4 code
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */
#ifndef NET_NETDEV_IEEE802154_H
#define NET_NETDEV_IEEE802154_H

#include "net/ieee802154.h"
#include "net/gnrc/nettype.h"
#include "net/netopt.h"
#include "net/netdev.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    IEEE 802.15.4 netdev flags
 * @brief   Flags for netdev_ieee802154_t::flags
 *
 * The flag-space `0xff00` is available for device-specific flags.
 * The flag-space `0x00ff` was chosen for global flags to be in accordance to
 * the IEEE 802.15.4 MAC header flags.
 * @{
 */

#define NETDEV_IEEE802154_SEND_MASK         (0x0028)    /**< flags to take for send packets */
#define NETDEV_IEEE802154_RAW               (0x0002)    /**< pass raw frame to upper layer */
/**
 * @brief   use long source addres (set) or short source address (unset)
 */
#define NETDEV_IEEE802154_SRC_MODE_LONG     (0x0004)
/**
 * @brief enable security
 */
#define NETDEV_IEEE802154_SECURITY_EN       (IEEE802154_FCF_SECURITY_EN)

/**
 * @brief   request ACK from receiver
 */
#define NETDEV_IEEE802154_ACK_REQ           (IEEE802154_FCF_ACK_REQ)

/**
 * @brief   set frame pending bit
 */
#define NETDEV_IEEE802154_FRAME_PEND        (IEEE802154_FCF_FRAME_PEND)
/**
 * @}
 */

/**
 * @brief   Option parameter to be used with @ref NETOPT_CCA_MODE to set
 *          the mode of the clear channel assessment (CCA) defined
 *          in Std 802.15.4.
 */
typedef enum {
    NETDEV_IEEE802154_CCA_MODE_1 = 1,   /**< Energy above threshold */
    NETDEV_IEEE802154_CCA_MODE_2,       /**< Carrier sense only */
    NETDEV_IEEE802154_CCA_MODE_3,       /**< Carrier sense with energy above threshold */
    NETDEV_IEEE802154_CCA_MODE_4,       /**< ALOHA */
    NETDEV_IEEE802154_CCA_MODE_5,       /**< UWB preamble sense based on the SHR of a frame */
    NETDEV_IEEE802154_CCA_MODE_6,       /**< UWB preamble sense based on the packet
                                         *   with the multiplexed preamble */
} netdev_ieee802154_cca_mode_t;

/**
 * @brief Extended structure to hold IEEE 802.15.4 driver state
 *
 * @extends netdev_t
 *
 * Supposed to be extended by driver implementations.
 * The extended structure should contain all variable driver state.
 */
typedef struct {
    netdev_t netdev;                        /**< @ref netdev_t base class */
    /**
     * @brief IEEE 802.15.4 specific fields
     * @{
     */
#ifdef MODULE_GNRC
    gnrc_nettype_t proto;                   /**< Protocol for upper layer */
#endif

    /**
     * @brief   PAN ID in network byte order
     */
    uint16_t pan;

    /**
     * @brief   Short address in network byte order
     */
    uint8_t short_addr[IEEE802154_SHORT_ADDRESS_LEN];

    /**
     * @brief   Long address in network byte order
     */
    uint8_t long_addr[IEEE802154_LONG_ADDRESS_LEN];
    uint8_t seq;                            /**< sequence number */
    uint8_t chan;                           /**< channel */
    uint16_t flags;                         /**< flags as defined above */
    /** @} */
} netdev_ieee802154_t;

/**
 * @brief   Received packet status information for IEEE 802.15.4 radios
 */
typedef struct netdev_radio_rx_info netdev_ieee802154_rx_info_t;

/**
 * @brief   Control layer descriptor for IEEE802.15.4 layer
 */
typedef struct {
    netdev_t netdev;            /**< netdev layer parent struct */
    netdev_ieee802154_t *hwdev; /**< Pointer to the hardware driver struct */
} netdev_ieee802154_ct_t;

/**
 * @brief   Add a ieee802.15.4 netdev layer to the top of the netdev stack
 *
 * @param[in] head    Top netdev device of the stack.
 * @param[in] layer   New ieee802.15.4 layer to push to the top of the stack.
 *
 * @return  The new top of the netdev stack.
 */
netdev_t *netdev_ieee802154_add(netdev_t *head,
                                netdev_ieee802154_ct_t *layer);

#ifdef __cplusplus
}
#endif

#endif /* NET_NETDEV_IEEE802154_H */
/** @} */
