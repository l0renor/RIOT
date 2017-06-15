/*
 * Copyright (C) 2017 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

/**
 * @defgroup    net_gnrc_netdev_stats  Netdev stats group
 * @ingroup     net_gnrc_netdev
 * @brief       Records statistics about link layer peers
 * @{
 *
 * @file
 * @brief       peer stats definitions
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */
#ifndef NETSTATS_PEER_H
#define NETSTATS_PEER_H

#include <string.h>
#include "net/netdev.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Max length of a L2 address
 */
#define NETSTATS_PEER_L2_ADDR_MAX_SIZE      (8)

/**
 * @name @ref EWMA parameters
 * @{
 */
#define NETSTATS_PEER_EWMA_SCALE            128 /**< Multiplication factor of the EWMA */
#define NETSTATS_PEER_EWMA_ALPHA             15 /**< Alpha factor of the EWMA */
/** @} */

/**
 * @name @ref ETX parameters
 * @{
 */
#define NETSTATS_PEER_ETX_NOACK_PENALTY      10 /**< ETX penalty for not receiving any ACK */
#define NETSTATS_PEER_ETX_DIVISOR           128 /**< ETX fixed point divisor (rfc 6551) */
#define NETSTATS_PEER_ETX_INIT                2 /**< Initial ETX, assume a perfect link */
/** @} */

/**
 * @brief Initialize the peer stats
 *
 * @param[in] dev  ptr to netdev device
 *
 */
void netstats_peer_init(netdev_t *dev);

/**
 * @brief Find a peer by the mac address, NULL if not found. Returns a pointer to the stats
 *
 * @param[in] dev       ptr to netdev device
 * @param[in] l2_addr   ptr to the L2 address
 * @param[in] len       length of the L2 address
 *
 */
netstats_peer_t *netstats_peer_getbymac(netdev_t *dev, const uint8_t *l2_addr, uint8_t len);

/**
 * @brief Iterator over the recorded peers, returns the next non-zero record. NULL if none remaining
 *
 * @param[in] first     ptr to the first record
 * @param[in] prev      ptr to the previous record
 *
 * @return ptr to the record
 * @return NULL if no records remain
 */
netstats_peer_t *netstats_peer_get_next(netstats_peer_t *first, netstats_peer_t *prev);

/**
 * @brief Store this peer as next in the transmission queue.
 *
 * Set len to zero if a nop record is needed, for example if the
 * transmission has a multicast address as a destination.
 *
 * @param[in] dev       ptr to netdev device
 * @param[in] l2_addr   ptr to the L2 address
 * @param[in] len       length of the L2 address
 *
 */
void netstats_peer_record(netdev_t *dev, const uint8_t *l2_addr, uint8_t len);

/**
 * @brief Get the first available peer in the transmission queue and increment pointer.
 *
 * @param[in] dev       ptr to netdev device
 *
 * @return ptr to the record
 */
netstats_peer_t *netstats_peer_get_recorded(netdev_t *dev);

/**
 * @brief Update the next recorded peer with the provided numbers
 *
 * This only increments the statistics if the length of the l2-address of the retrieved record
 * is non-zero. see also @ref netstats_peer_record. The numbers indicate the number of transmissions
 * the radio had to perform before a successful transmission was performed. For example: in the case
 * of a single send operation needing 3 tries before an ACK was received, there are 2 failed
 * transmissions and 1 successful transmission.
 *
 * @param[in] dev         ptr to netdev device
 * @param[in] num_success number of successful radio transmissions
 * @param[in] num_failed  number of failed radio transmissions
 *
 * @return ptr to the record
 */
netstats_peer_t *netstats_peer_update_tx(netdev_t *dev, uint8_t num_success, uint8_t num_failed);

/**
 * @brief Record rx stats for the l2_addr
 *
 * @param[in] dev         ptr to netdev device
 * @param[in] l2_addr ptr to the L2 address
 * @param[in] l2_addr_len length of the L2 address
 * @param[in] rssi        RSSI of the received transmission in abs([dBm])
 * @param[in] lqi         Link Quality Indication provided by the radio
 *
 * @return ptr to the updated record
 */
netstats_peer_t *netstats_peer_update_rx(netdev_t *dev, const uint8_t *l2_addr, uint8_t l2_addr_len, uint8_t rssi, uint8_t lqi);

void netstats_peer_update_etx(netstats_peer_t *stats, uint8_t success, uint8_t failures);

#ifdef __cplusplus
}
#endif

#endif /* NETSTATS_PEER_H */
/**
 * @}
 */
