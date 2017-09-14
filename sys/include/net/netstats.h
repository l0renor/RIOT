/*
 * Copyright (C) 2016 INRIA
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_netstats Packet statistics per module
 * @ingroup     net
 * @brief       Each module may store information about sent and received packets
 * @{
 *
 * @file
 * @brief       Definition of net statistics
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 */

#include <stdint.h>

#ifndef NET_NETSTATS_H
#define NET_NETSTATS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name @ref net_netstats module names
 * @{
 */
#define NETSTATS_LAYER2     (0x01)
#define NETSTATS_IPV6       (0x02)
#define NETSTATS_RPL        (0x03)
#define NETSTATS_ALL        (0xFF)
/** @} */

#ifndef NETSTATS_NB_SIZE
/**
 * @brief   The max number of entries in the peer stats table
 */
#define NETSTATS_NB_SIZE           (8)
#endif

/**
 * @brief   The queue size for tx correlation
 */
#define NETSTATS_NB_QUEUE_SIZE     (4)

/**
 * @brief       Global statistics struct
 */
typedef struct {
    uint32_t tx_unicast_count;  /**< packets sent via unicast */
    uint32_t tx_mcast_count;    /**< packets sent via multicast
                                     (including broadcast) */
    uint32_t tx_success;        /**< successful sending operations
                                     (either acknowledged or unconfirmed
                                     sending operation, e.g. multicast) */
    uint32_t tx_failed;         /**< failed sending operations */
    uint32_t tx_bytes;          /**< sent bytes */
    uint32_t rx_count;          /**< received (data) packets */
    uint32_t rx_bytes;          /**< received bytes */
} netstats_t;

typedef struct netstats_nb_hook netstats_nb_hook_t;
/**
 * @brief       Stats per peer struct
 */
typedef struct netstats_nb {
    uint8_t l2_addr[8];     /**< Link layer address of the neighbor */
    uint8_t l2_addr_len;    /**< Length of netstats_nb::l2_addr */
    uint16_t etx;           /**< ETX of this peer */
#ifdef MODULE_NETSTATS_NEIGHBOR_EXT
    int16_t rssi;           /**< Average RSSI of received frames in abs([dBm]) */
    uint8_t lqi;            /**< Average LQI of received frames */
    uint32_t tx_count;      /**< Number of sent frames to this peer */
    uint32_t tx_failed;     /**< Number of failed transmission tries to this peer */
    uint32_t rx_count;      /**< Number of received frames */
    uint32_t tx_bytes;      /**< Bytes sent */
    uint32_t rx_bytes;      /**< Bytes received */
#endif
    uint8_t  freshness;     /**< Freshness counter */
    uint32_t last_updated;  /**< seconds timestamp of last update */
    uint32_t last_halved;   /**< seconds timestamp of last halving */
    netstats_nb_hook_t *hooks; /**< notification hook */

    /* Power control attributes */
#ifdef MODULE_GNRC_NETDEV_POWER
    uint8_t  tx_attenuation;     /**< Current TX attenuation used for this peer */
    uint8_t  power_control; /**< integer indicating the used power control function */
    uint8_t  max_attenuation; /**< Maximum reached attenuation before decreasing (local maximum) */
    uint8_t  transmissions;  /**< Number of transmissions since last decrease */
    float  k_factor;   /**< Cubic calculated parameter */
#endif
} netstats_nb_t;


struct netstats_nb_hook {
    netstats_nb_hook_t *next;
    void *arg;
    void (*callback)(netstats_nb_t*, void*);
    uint16_t threshold; 
    uint16_t last_etx;
};

#ifdef __cplusplus
}
#endif

#endif /* NET_NETSTATS_H */
/** @} */
