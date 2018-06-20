/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>
#include "net/nanocoap.h"
#include "xtimer.h"
#include "periph/pm.h"

static xtimer_t reboot_tm;

void reboot_cb(void *context)
{
    (void)context;
    pm_reboot();
}

ssize_t lwm2m_coap_device_manufacturer(coap_pkt_t *pkt, uint8_t *buf,  size_t len,
        void *context)
{
    (void)context;
    return coap_reply_simple(pkt, COAP_CODE_205, buf, len,
            COAP_FORMAT_TEXT, (uint8_t*)"RIOT-os", strlen("RIOT-os"));
}

ssize_t lwm2m_coap_device_model(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    return coap_reply_simple(pkt, COAP_CODE_205, buf, len,
            COAP_FORMAT_TEXT, (uint8_t*)RIOT_BOARD, strlen(RIOT_BOARD));
}

ssize_t lwm2m_coap_device_reboot(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    reboot_tm.callback = reboot_cb;
    xtimer_set(&reboot_tm, 200 * US_PER_MS);
    return coap_reply_simple(pkt, COAP_CODE_204, buf, len,
            COAP_FORMAT_NONE, NULL, 0);
}

ssize_t lwm2m_coap_device_err(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    return coap_reply_simple(pkt, COAP_CODE_205, buf, len,
            COAP_FORMAT_TEXT, (uint8_t*)"0", strlen("0"));
}

ssize_t lwm2m_coap_device_bind(coap_pkt_t *pkt, uint8_t *buf, size_t len, void *context)
{
    (void)context;
    return coap_reply_simple(pkt, COAP_CODE_205, buf, len,
            COAP_FORMAT_TEXT, (uint8_t*)"U", strlen("U"));
}
