/*
 * Copyright (C) 2016 Freie Universität Berlin
 *               2017 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_netdev_ieee802154
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 * @author  Koen Zandberg <koen@bergzand.net>
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include "net/eui64.h"
#include "net/ieee802154.h"
#include "net/netdev.h"

#include "net/netdev/ieee802154.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static int _get(netdev_t *dev, netopt_t opt, void *value, size_t max_len);
static int _set(netdev_t *dev, netopt_t opt, const void *value, size_t value_len);

const netdev_driver_t ieee802154_layer = {
    .send = netdev_send_pass,
    .recv = netdev_recv_pass,
    .init = netdev_init_pass,
    .isr = netdev_isr_pass,
    .get = _get,
    .set = _set,
};

netdev_t *netdev_ieee802154_add(netdev_t *head,
                                netdev_ieee802154_ct_t *dev_wpan) {
    dev_wpan->netdev.driver = &ieee802154_layer;
    head->event_callback = netdev_event_cb_pass;
    dev_wpan->hwdev = (netdev_ieee802154_t*)head;
    return netdev_add_layer(head, (netdev_t *)dev_wpan);
}

static int _get_iid(netdev_ieee802154_ct_t *dev, eui64_t *value, size_t max_len)
{
    (void)max_len;

    uint8_t *addr;
    uint16_t addr_len;

    assert(max_len >= sizeof(eui64_t));

    if (dev->hwdev->flags & NETDEV_IEEE802154_SRC_MODE_LONG) {
        addr_len = IEEE802154_LONG_ADDRESS_LEN;
        addr = dev->hwdev->long_addr;
    }
    else {
        addr_len = IEEE802154_SHORT_ADDRESS_LEN;
        addr = dev->hwdev->short_addr;
    }
    ieee802154_get_iid(value, addr, addr_len);

    return sizeof(eui64_t);
}

static int _get(netdev_t *dev, netopt_t opt,
                void *value, size_t max_len)
{
    netdev_ieee802154_ct_t *dev_wpan = (netdev_ieee802154_ct_t*)dev;
    netdev_ieee802154_t *hwdev = dev_wpan->hwdev;
    int res = -ENOTSUP;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(max_len >= sizeof(hwdev->short_addr));
            memcpy(value, hwdev->short_addr, sizeof(hwdev->short_addr));
            res = sizeof(hwdev->short_addr);
            break;
        case NETOPT_ADDRESS_LONG:
            assert(max_len >= sizeof(hwdev->long_addr));
            memcpy(value, hwdev->long_addr, sizeof(hwdev->long_addr));
            res = sizeof(hwdev->long_addr);
            break;
        case NETOPT_ADDR_LEN:
        case NETOPT_SRC_LEN:
            assert(max_len == sizeof(uint16_t));
            if (hwdev->flags & NETDEV_IEEE802154_SRC_MODE_LONG) {
                *((uint16_t *)value) = IEEE802154_LONG_ADDRESS_LEN;
            }
            else {
                *((uint16_t *)value) = IEEE802154_SHORT_ADDRESS_LEN;
            }
            res = sizeof(uint16_t);
            break;
        case NETOPT_NID:
            assert(max_len == sizeof(hwdev->pan));
            *((uint16_t *)value) = hwdev->pan;
            res = sizeof(hwdev->pan);
            break;
        case NETOPT_CHANNEL:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)value) = (uint16_t)hwdev->chan;
            res = sizeof(hwdev->chan);
            break;
        case NETOPT_ACK_REQ:
            assert(max_len == sizeof(netopt_enable_t));
            if (hwdev->flags & NETDEV_IEEE802154_ACK_REQ) {
                *((netopt_enable_t *)value) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)value) = NETOPT_DISABLE;
            }
            res = sizeof(netopt_enable_t);
            break;
        case NETOPT_RAWMODE:
            assert(max_len == sizeof(netopt_enable_t));
            if (hwdev->flags & NETDEV_IEEE802154_RAW) {
                *((netopt_enable_t *)value) = NETOPT_ENABLE;
            }
            else {
                *((netopt_enable_t *)value) = NETOPT_DISABLE;
            }
            res = sizeof(netopt_enable_t);
            break;
#ifdef MODULE_GNRC
        case NETOPT_PROTO:
            assert(max_len == sizeof(gnrc_nettype_t));
            *((gnrc_nettype_t *)value) = hwdev->proto;
            res = sizeof(gnrc_nettype_t);
            break;
#endif
        case NETOPT_DEVICE_TYPE:
            assert(max_len == sizeof(uint16_t));
            *((uint16_t *)value) = NETDEV_TYPE_IEEE802154;
            res = sizeof(uint16_t);
            break;
        case NETOPT_IPV6_IID:
            res = _get_iid(dev_wpan, value, max_len);
            break;
#ifdef MODULE_NETSTATS_L2
        case NETOPT_STATS:
            assert(max_len == sizeof(uintptr_t));
            *((netstats_t **)value) = &hwdev->netdev.stats;
            res = sizeof(uintptr_t);
            break;
#endif
#ifdef MODULE_L2FILTER
        case NETOPT_L2FILTER:
            assert(max_len >= sizeof(l2filter_t **));
            *((l2filter_t **)value) = hwdev->netdev.filter;
            res = sizeof(l2filter_t **);
            break;
#endif
        default:
            break;
    }
    if (res == -ENOTSUP) {
        res = dev_wpan->netdev.lower->driver->get(dev_wpan->netdev.lower, opt, value, max_len);
    }
    return res;
}

int _set(netdev_t *dev, netopt_t opt, const void *value,
         size_t len)
{
    netdev_ieee802154_ct_t *dev_wpan = (netdev_ieee802154_ct_t*)dev;
    netdev_ieee802154_t *hwdev = dev_wpan->hwdev;
    int res = -ENOTSUP;

    switch (opt) {
        case NETOPT_CHANNEL:
        {
            assert(len == sizeof(uint16_t));
            uint16_t chan = *((uint16_t *)value);
            /* real validity needs to be checked by device, since sub-GHz and
             * 2.4 GHz band radios have different legal values. Here we only
             * check that it fits in an 8-bit variabl*/
            assert(chan <= UINT8_MAX);
            hwdev->chan = chan;
            res = sizeof(uint16_t);
            break;
        }
        case NETOPT_ADDRESS:
            assert(len <= sizeof(hwdev->short_addr));
            memset(hwdev->short_addr, 0, sizeof(hwdev->short_addr));
            memcpy(hwdev->short_addr, value, len);
            res = sizeof(hwdev->short_addr);
            break;
        case NETOPT_ADDRESS_LONG:
            assert(len <= sizeof(hwdev->long_addr));
            memset(hwdev->long_addr, 0, sizeof(hwdev->long_addr));
            memcpy(hwdev->long_addr, value, len);
            res = sizeof(hwdev->long_addr);
            break;
        case NETOPT_ADDR_LEN:
        case NETOPT_SRC_LEN:
            assert(len == sizeof(uint16_t));
            res = sizeof(uint16_t);
            switch ((*(uint16_t *)value)) {
                case IEEE802154_SHORT_ADDRESS_LEN:
                    hwdev->flags &= ~NETDEV_IEEE802154_SRC_MODE_LONG;
                    break;
                case IEEE802154_LONG_ADDRESS_LEN:
                    hwdev->flags |= NETDEV_IEEE802154_SRC_MODE_LONG;
                    break;
                default:
                    res = -EAFNOSUPPORT;
                    break;
            }
            break;
        case NETOPT_NID:
            assert(len == sizeof(hwdev->pan));
            hwdev->pan = *((uint16_t *)value);
            res = sizeof(hwdev->pan);
            break;
        case NETOPT_ACK_REQ:
            if ((*(bool *)value)) {
                hwdev->flags |= NETDEV_IEEE802154_ACK_REQ;
            }
            else {
                hwdev->flags &= ~NETDEV_IEEE802154_ACK_REQ;
            }
            res = sizeof(uint16_t);
            break;
        case NETOPT_RAWMODE:
            if ((*(bool *)value)) {
                hwdev->flags |= NETDEV_IEEE802154_RAW;
            }
            else {
                hwdev->flags &= ~NETDEV_IEEE802154_RAW;
            }
            res = sizeof(uint16_t);
            break;
#ifdef MODULE_GNRC
        case NETOPT_PROTO:
            assert(len == sizeof(gnrc_nettype_t));
            hwdev->proto = *((gnrc_nettype_t *)value);
            res = sizeof(gnrc_nettype_t);
            break;
#endif
#ifdef MODULE_L2FILTER
        case NETOPT_L2FILTER:
            res = l2filter_add(hwdev->netdev.filter, value, len);
            break;
        case NETOPT_L2FILTER_RM:
            res = l2filter_rm(hwdev->netdev.filter, value, len);
            break;
#endif
        default:
            break;
    }
    if (res == -ENOTSUP) {
        res = dev_wpan->netdev.lower->driver->set(dev_wpan->netdev.lower, opt, value, len);
    }
    return res;
}

/** @} */
