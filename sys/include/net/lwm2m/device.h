/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <stdlib.h>

ssize_t lwm2m_coap_device_manufacturer(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context);
ssize_t lwm2m_coap_device_model(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context);

ssize_t lwm2m_coap_device_reboot(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context);

ssize_t lwm2m_coap_device_err(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context);
ssize_t lwm2m_coap_device_bind(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context);
#define LWM2M_COAP_DEVICE_RESOURCES \
    { \
        .path = "/3/0/0", \
        .methods = COAP_GET, \
        .handler = lwm2m_coap_device_manufacturer \
    }, \
    { \
        .path = "/3/0/1", \
        .methods = COAP_GET, \
        .handler = lwm2m_coap_device_model \
    }, \
    { \
        .path = "/3/0/4", \
        .methods = COAP_POST, \
        .handler = lwm2m_coap_device_reboot \
    }, \
    { \
        .path = "/3/0/11", \
        .methods = COAP_GET, \
        .handler = lwm2m_coap_device_err \
    }, \
    { \
        .path = "/3/0/16", \
        .methods = COAP_GET, \
        .handler = lwm2m_coap_device_bind \
    }
