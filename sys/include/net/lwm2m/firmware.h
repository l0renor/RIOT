/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <stdlib.h>

ssize_t lwm2m_coap_firmware_package(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context);
ssize_t lwm2m_coap_firmware_uri(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context);
ssize_t lwm2m_coap_firmware_method(coap_pkt_t *pkt, uint8_t *buf,  size_t len, void *context);
ssize_t lwm2m_coap_firmware_result(coap_pkt_t *pkt, uint8_t *buf,  size_t len, void *context);
ssize_t lwm2m_coap_firmware_state(coap_pkt_t *pkt, uint8_t *buf,  size_t len, void *context);
ssize_t lwm2m_coap_firmware_update(coap_pkt_t *pkt, uint8_t *buf,  size_t len, void *context);

#define LWM2M_COAP_FIRMWARE_RESOURCES \
    { \
        .path = "/5/0/0", \
        .methods = COAP_PUT, \
        .handler = lwm2m_coap_firmware_package \
    }, \
    { \
        .path = "/5/0/1", \
        .methods = COAP_GET | COAP_PUT, \
        .handler = lwm2m_coap_firmware_uri \
    }, \
    { \
        .path = "/5/0/2", \
        .methods = COAP_POST, \
        .handler = lwm2m_coap_firmware_update \
    }, \
    { \
        .path = "/5/0/3", \
        .methods = COAP_GET, \
        .handler = lwm2m_coap_firmware_state \
    }, \
    { \
        .path = "/5/0/5", \
        .methods = COAP_GET, \
        .handler = lwm2m_coap_firmware_result \
    }, \
    { \
        .path = "/5/0/9", \
        .methods = COAP_GET | COAP_GET, \
        .handler = lwm2m_coap_firmware_method\
    }

