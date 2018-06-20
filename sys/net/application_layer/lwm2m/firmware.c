/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>
#include "fmt.h"
#include "net/nanocoap.h"
#include "firmware/manifest.h"

static char uri[128];
extern firmware_manifest_t _fw_man;
uint8_t state = 0;

ssize_t lwm2m_coap_firmware_package(coap_pkt_t* pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    int result = COAP_CODE_CREATED;
    coap_block1_t block1;
    int blockwise = coap_get_block1(pkt, &block1);
    bool more = (!blockwise || !block1.more) ? false : true;

    if (firmware_manifest_putbytes(pkt->payload, pkt->payload_len, block1.offset, more) < 0) {
        return COAP_CODE_SERVICE_UNAVAILABLE;
    }
    if (more) {
        state = 1;
    }
    else {
        state = 2;
    }
    ssize_t reply_len = coap_build_reply(pkt, result, buf, len, 0);
    uint8_t *pkt_pos = (uint8_t*)pkt->hdr + reply_len;
    pkt_pos += coap_put_block1_ok(pkt_pos, &block1, 0);
    return pkt_pos - (uint8_t*)pkt->hdr;
}

ssize_t lwm2m_coap_firmware_uri(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context)
{
    (void)context;
    unsigned method_flag = coap_method2flag(coap_get_code_detail(pkt));
    switch(method_flag) {
        case COAP_GET:
            return coap_reply_simple(pkt, COAP_CODE_205, buf, len,
                COAP_FORMAT_TEXT, (uint8_t*)uri, strlen(uri));
        case COAP_PUT:
            memset(uri, 0, sizeof(uri));
            if(pkt->payload_len < sizeof(uri)) {
                memcpy(uri, (char*)pkt->payload, pkt->payload_len);
                return coap_reply_simple(pkt, COAP_CODE_204, buf, len,
                    COAP_FORMAT_NONE, NULL, 0);
            }
            else {
                return coap_reply_simple(pkt, COAP_CODE_BAD_REQUEST, buf, len,
                    COAP_FORMAT_NONE, NULL, 0);
            }
    }
    return -1;
}

ssize_t lwm2m_coap_firmware_update(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context)
{
    (void)context;
    if (state == 2) {
        firmware_manifest_update();
        return coap_reply_simple(pkt, COAP_CODE_CREATED, buf, len, COAP_FORMAT_NONE,
                NULL, 0);
        state = 3;
    }
    else {
        return coap_reply_simple(pkt, COAP_CODE_BAD_REQUEST, buf, len, COAP_FORMAT_NONE,
                NULL, 0);
    }
}

ssize_t lwm2m_coap_firmware_state(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context)
{
    (void)context;
    char payload[8];
    payload[fmt_u16_dec(payload, _fw_man.state)] = '\0';
    return coap_reply_simple(pkt, COAP_CODE_205, buf, len, COAP_FORMAT_TEXT,
            (uint8_t*)payload, strlen(payload));
}

ssize_t lwm2m_coap_firmware_result(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context)
{
    (void)context;
    char payload[8];
    payload[fmt_u16_dec(payload, _fw_man.result)] = '\0';
    return coap_reply_simple(pkt, COAP_CODE_205, buf, len, COAP_FORMAT_TEXT,
            (uint8_t*)payload, strlen(payload));
}

ssize_t lwm2m_coap_firmware_method(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context)
{
    (void)context;
    return coap_reply_simple(pkt, COAP_CODE_205, buf, len, COAP_FORMAT_TEXT,
            (uint8_t*)"1", strlen("1"));
}
