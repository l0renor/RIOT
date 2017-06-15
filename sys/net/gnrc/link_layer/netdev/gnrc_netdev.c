/*
 * Copyright (C) 2015 Freie Universität Berlin
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     net
 * @file
 * @brief       Glue for netdev devices to netapi
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @}
 */

#include <errno.h>

#include "msg.h"
#include "thread.h"

#include "net/gnrc.h"
#include "net/gnrc/nettype.h"
#include "net/netdev.h"

#include "net/gnrc/netdev.h"
#include "net/ethernet/hdr.h"
#include "net/netstats/peer.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#if defined(MODULE_OD) && ENABLE_DEBUG
#include "od.h"
#endif

#define NETDEV_NETAPI_MSG_QUEUE_SIZE 8

static void _pass_on_packet(gnrc_pktsnip_t *pkt);

#ifdef MODULE_NETSTATS_PEER
static void _process_receive_stats(gnrc_netdev_t *netdev, gnrc_pktsnip_t *pkt);
static void _register_sender(netdev_t *dev, gnrc_pktsnip_t *pkt);
#endif

/**
 * @brief   Function called by the device driver on device events
 *
 * @param[in] event     type of event
 */
static void _event_cb(netdev_t *dev, netdev_event_t event, void* context)
{
    gnrc_netdev_t *gnrc_netdev = (gnrc_netdev_t*) dev->context;

    if (event == NETDEV_EVENT_ISR) {
        msg_t msg;

        msg.type = NETDEV_MSG_TYPE_EVENT;
        msg.content.ptr = gnrc_netdev;

        if (msg_send(&msg, gnrc_netdev->pid) <= 0) {
            puts("gnrc_netdev: possibly lost interrupt.");
        }
    }
    else {
        DEBUG("gnrc_netdev: event triggered -> %i\n", event);
        switch(event) {
            case NETDEV_EVENT_RX_COMPLETE:
                {
                    gnrc_pktsnip_t *pkt = gnrc_netdev->recv(gnrc_netdev);

                    if (pkt) {
#ifdef MODULE_NETSTATS_PEER
                        _process_receive_stats(gnrc_netdev, pkt);
#endif
                        _pass_on_packet(pkt);
                    }

                    break;
                }
            case NETDEV_EVENT_TX_NOACK:
#ifdef MODULE_NETSTATS_L2
                dev->stats.tx_failed++;
#endif
#ifdef MODULE_NETSTATS_PEER
                if(context) {
                    netdev_radio_tx_info_t *info = (netdev_radio_tx_info_t*)context;
                    /* All transmissions failed */
                    netstats_peer_update_tx(dev, 0, info->transmissions );
                }
#endif
            case NETDEV_EVENT_TX_MEDIUM_BUSY:
#ifdef MODULE_NETSTATS_L2
                dev->stats.tx_failed++;
#endif
                break;
            case NETDEV_EVENT_TX_COMPLETE:
#ifdef MODULE_NETSTATS_L2
                dev->stats.tx_success++;
#endif
#ifdef MODULE_NETSTATS_PEER
                if(context) {
                    netdev_radio_tx_info_t *info = (netdev_radio_tx_info_t*)context;
                    /* One successful transmission, transmissions - 1 failed */
                    netstats_peer_update_tx(dev, 1, info->transmissions - 1 );
                }
#endif
                break;
            default:
                DEBUG("gnrc_netdev: warning: unhandled event %u.\n", event);
        }
    }
}

static void _pass_on_packet(gnrc_pktsnip_t *pkt)
{
    /* throw away packet if no one is interested */
    if (!gnrc_netapi_dispatch_receive(pkt->type, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
        DEBUG("gnrc_netdev: unable to forward packet of type %i\n", pkt->type);
        gnrc_pktbuf_release(pkt);
        return;
    }
}

#ifdef MODULE_NETSTATS_PEER
static void _process_receive_stats(gnrc_netdev_t *netdev, gnrc_pktsnip_t *pkt)
{
    gnrc_netif_hdr_t *hdr;
    const uint8_t *src = NULL;
    gnrc_pktsnip_t *netif = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_NETIF);

    if (netif != NULL) {
        size_t src_len;
        hdr = netif->data;
        src = gnrc_netif_hdr_get_src_addr(hdr);
        src_len = hdr->src_l2addr_len;
        netstats_peer_update_rx(netdev->dev, src, src_len, hdr->rssi, hdr->lqi);
    }
}

/* record peer if useful*/
static void _register_sender(netdev_t *dev, gnrc_pktsnip_t *pkt)
{
    gnrc_netif_hdr_t *netif_hdr;
    const uint8_t *dst = NULL;

    if (pkt->type != GNRC_NETTYPE_NETIF) {
        DEBUG("l2 stats: first header is not generic netif header\n");
        return;
    }
    netif_hdr = pkt->data;
    if (!(netif_hdr->flags & /* Only process unicast */
          (GNRC_NETIF_HDR_FLAGS_BROADCAST | GNRC_NETIF_HDR_FLAGS_MULTICAST))) {
        size_t dst_len;
        DEBUG("l2 stats: recording transmission\n");
        dst = gnrc_netif_hdr_get_dst_addr(netif_hdr);
        dst_len = netif_hdr->dst_l2addr_len;
        netstats_peer_record(dev, dst, dst_len);
    }
    else {
        DEBUG("l2 stats: Destination is multicast or unicast, NULL recorded");
        netstats_peer_record(dev, NULL, 0);
    }
    return;
}
#endif

/**
 * @brief   Startup code and event loop of the gnrc_netdev layer
 *
 * @param[in] args  expects a pointer to the underlying netdev device
 *
 * @return          never returns
 */
static void *_gnrc_netdev_thread(void *args)
{
    DEBUG("gnrc_netdev: starting thread\n");

    gnrc_netdev_t *gnrc_netdev = (gnrc_netdev_t*) args;
    netdev_t *dev = gnrc_netdev->dev;

    gnrc_netdev->pid = thread_getpid();

    gnrc_netapi_opt_t *opt;
    int res;
    msg_t msg, reply, msg_queue[NETDEV_NETAPI_MSG_QUEUE_SIZE];

    /* setup the MAC layers message queue */
    msg_init_queue(msg_queue, NETDEV_NETAPI_MSG_QUEUE_SIZE);

    /* register the event callback with the device driver */
    dev->event_callback = _event_cb;
    dev->context = (void*) gnrc_netdev;

    /* register the device to the network stack*/
    gnrc_netif_add(thread_getpid());

    netstats_peer_init(dev);

    /* initialize low-level driver */
    dev->driver->init(dev);

    /* start the event loop */
    while (1) {
        DEBUG("gnrc_netdev: waiting for incoming messages\n");
        msg_receive(&msg);
        /* dispatch NETDEV and NETAPI messages */
        switch (msg.type) {
            case NETDEV_MSG_TYPE_EVENT:
                DEBUG("gnrc_netdev: GNRC_NETDEV_MSG_TYPE_EVENT received\n");
                dev->driver->isr(dev);
                break;
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("gnrc_netdev: GNRC_NETAPI_MSG_TYPE_SND received\n");
                gnrc_pktsnip_t *pkt = msg.content.ptr;
#ifdef MODULE_NETSTATS_PEER
                _register_sender(dev, pkt);
#endif
                gnrc_netdev->send(gnrc_netdev, pkt);
                break;
            case GNRC_NETAPI_MSG_TYPE_SET:
                /* read incoming options */
                opt = msg.content.ptr;
                DEBUG("gnrc_netdev: GNRC_NETAPI_MSG_TYPE_SET received. opt=%s\n",
                        netopt2str(opt->opt));
                /* set option for device driver */
                res = dev->driver->set(dev, opt->opt, opt->data, opt->data_len);
                DEBUG("gnrc_netdev: response of netdev->set: %i\n", res);
                /* send reply to calling thread */
                reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
                /* read incoming options */
                opt = msg.content.ptr;
                DEBUG("gnrc_netdev: GNRC_NETAPI_MSG_TYPE_GET received. opt=%s\n",
                        netopt2str(opt->opt));
                /* get option from device driver */
                res = dev->driver->get(dev, opt->opt, opt->data, opt->data_len);
                DEBUG("gnrc_netdev: response of netdev->get: %i\n", res);
                /* send reply to calling thread */
                reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            default:
                DEBUG("gnrc_netdev: Unknown command %" PRIu16 "\n", msg.type);
                break;
        }
    }
    /* never reached */
    return NULL;
}

kernel_pid_t gnrc_netdev_init(char *stack, int stacksize, char priority,
                        const char *name, gnrc_netdev_t *gnrc_netdev)
{
    kernel_pid_t res;

    /* check if given netdev device is defined and the driver is set */
    if (gnrc_netdev == NULL || gnrc_netdev->dev == NULL) {
        return -ENODEV;
    }

    /* create new gnrc_netdev thread */
    res = thread_create(stack, stacksize, priority, THREAD_CREATE_STACKTEST,
                        _gnrc_netdev_thread, (void *)gnrc_netdev, name);
    if (res <= 0) {
        return -EINVAL;
    }

    return res;
}
