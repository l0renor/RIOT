/*
 * Copyright (C) Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     net
 * @file
 * @brief       Peer level stats for netdev
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <errno.h>

#include "xtimer.h"
#include "net/netdev.h"
#include "net/netstats/peer.h"
#include "net/gnrc/netdev/power.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"
//static char addr_str[30];

static bool l2_addr_equal(uint8_t *a, uint8_t a_len, uint8_t *b, uint8_t b_len);

void netstats_peer_init(netdev_t *dev)
{
    memset(dev->pstats, 0, sizeof(netstats_peer_t) * NETSTATS_PEER_SIZE);
    dev->send_index = 0;
    dev->cb_index = 0;
}

netstats_peer_t *netstats_peer_get(netdev_t *dev, netstats_peer_t *newstats)
{
    netstats_peer_t *found_entry = NULL;
    netstats_peer_t *stats = dev->pstats;

    /* TODO parameter check */

    for (int i = 0; i < NETSTATS_PEER_SIZE; i++) {
        if (l2_addr_equal(stats[i].l2_addr, stats[i].l2_addr_len, newstats->l2_addr, newstats->l2_addr_len)) {
            found_entry = &stats[i];
        }

        if (stats[i].l2_addr_len == 0 && !found_entry) {
            /* found the first free entry */
            found_entry = &stats[i];
            break;
        }
    }
    return found_entry;
}

netstats_peer_t *netstats_peer_getbymac(netdev_t *dev, const uint8_t *l2_addr, uint8_t len)
{
    netstats_peer_t *found_entry = NULL;
    netstats_peer_t *stats = dev->pstats;

    /* TODO parameter check */
    for (int i = 0; i < NETSTATS_PEER_SIZE; i++) {
        if (stats[i].l2_addr_len == 0) {
            DEBUG("L2 peerstats: Building new entry for addr with len %d\n", len);
            /* found the first free entry */
            found_entry = &stats[i];
            /* fill entry */
            memcpy(found_entry->l2_addr, l2_addr, len);
            found_entry->l2_addr_len = len;
            found_entry->etx = NETSTATS_PEER_ETX_INIT * NETSTATS_PEER_ETX_DIVISOR;
            found_entry->power_control = gnrc_netdev_power_get_default_func();
            break;
        }
        if (l2_addr_equal(stats[i].l2_addr, stats[i].l2_addr_len, (uint8_t *)l2_addr, len)) {
            found_entry = &stats[i];
            break;
        }

    }
    if (found_entry == NULL) {
        DEBUG("L2 peerstats: No entry found.\n");
    }
    return found_entry;
}

netstats_peer_t *netstats_peer_get_next(netstats_peer_t *first, netstats_peer_t *prev)
{
    prev++;                                         /* get next entry */
    while (prev < (first + NETSTATS_PEER_SIZE)) {   /* while not reached end */
        if (prev->l2_addr_len) {
            return prev;
        }

        prev++;
    }
    return NULL;
}

netstats_peer_t *netstats_peer_record(netdev_t *dev, const uint8_t *l2_addr, uint8_t len)
{
    if (!(len)) {
        /* Fill queue with a NOP */
        dev->stats_queue[dev->send_index] = NULL;
    }
    else {
        dev->stats_queue[dev->send_index] = netstats_peer_getbymac(dev, l2_addr, len);
    }
    dev->send_index++;
    if (dev->send_index == 4) {
        dev->send_index = 0;
    }
    return dev->stats_queue[dev->send_index];
}

netstats_peer_t *netstats_peer_get_recorded(netdev_t *dev)
{

    netstats_peer_t *stats = dev->stats_queue[dev->cb_index];

    dev->cb_index++;
    if (dev->cb_index == 4) {
        dev->cb_index = 0;
    }
    return stats;
}

netstats_peer_t *netstats_peer_update_tx(netdev_t *dev, uint8_t num_success, uint8_t num_failed)
{
    /* Receive transmit statistics */
    /* Update: counters, ETX */
    netstats_peer_t *stats = netstats_peer_get_recorded(dev);

    if (stats) {
        stats->tx_count += num_success + num_failed;
        stats->tx_failed += num_failed;
	if (num_success || num_failed) {
            netstats_peer_update_etx(stats, num_success, num_failed);
        }
    }
    return stats;
}



netstats_peer_t *netstats_peer_update_rx(netdev_t *dev, const uint8_t *l2_addr, uint8_t l2_addr_len, uint8_t rssi, uint8_t lqi)
{
    netstats_peer_t *stats = netstats_peer_getbymac(
        dev, l2_addr, l2_addr_len
        );

    if (stats->rx_count == 0) {
        stats->rssi = rssi;
        stats->lqi = lqi;
    }
    else {
        /* Exponential weighted moving average */
        stats->rssi = ((uint32_t)stats->rssi *
                       (NETSTATS_PEER_EWMA_SCALE - NETSTATS_PEER_EWMA_ALPHA) +
                       (uint32_t)rssi * NETSTATS_PEER_EWMA_ALPHA
                      ) / NETSTATS_PEER_EWMA_SCALE;
        stats->lqi = ((uint32_t)stats->lqi *
                       (NETSTATS_PEER_EWMA_SCALE - NETSTATS_PEER_EWMA_ALPHA) +
                       (uint32_t)lqi * NETSTATS_PEER_EWMA_ALPHA
                      ) / NETSTATS_PEER_EWMA_SCALE;
    }
    stats->rx_count++;
    return stats;
}

void netstats_peer_update_etx(netstats_peer_t *stats, uint8_t success, uint8_t failures)
{
    uint16_t packet_etx = 0;
    if( success == 0)
    {
        packet_etx = NETSTATS_PEER_ETX_NOACK_PENALTY * NETSTATS_PEER_ETX_DIVISOR;
    }
    else
    {
        packet_etx = (failures+1)* 2 * NETSTATS_PEER_ETX_DIVISOR;
    }
    /* Exponential weighted moving average */
    stats->etx = ((uint32_t)stats->etx *
                  (NETSTATS_PEER_EWMA_SCALE - NETSTATS_PEER_EWMA_ALPHA) +
                  (uint32_t)packet_etx * NETSTATS_PEER_EWMA_ALPHA
                  ) / NETSTATS_PEER_EWMA_SCALE;

    DEBUG("L2-statuxvrzfqn: time: %lu ETX: %u, EWMA-ETX: % 2.2f, Att: %u\n", xtimer_now_usec(), packet_etx, stats->etx/128.0, stats->tx_attenuation);
}

static bool l2_addr_equal(uint8_t *a, uint8_t a_len, uint8_t *b, uint8_t b_len)
{
    bool equal = true;

    if (a_len != b_len) {
        return false;
    }
    for (int i = 0; i < a_len; i++) {
        if (a[i] != b[i]) {
            equal = false;
        }
    }
    ;
    return equal;
}
