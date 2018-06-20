/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_rdcli_simple
 * @{
 *
 * @file
 * @brief       Simplified CoAP resource directory client implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <string.h>

#include "fmt.h"
#include "net/sock/udp.h"
#include "sock_types.h"
#include "net/nanocoap.h"
#include "net/nanocoap_sock.h"
#include "net/rdcli_config.h"
#include "net/rdcli_common.h"
#include "net/rdcli_simple.h"
#include "net/ipv6/addr.h"

#define BUFSIZE             (128U)

#define OPTIONS             "<3/0>,<5/0>"

/* we don't want to allocate the CoAP packet and scratch buffer on the stack,
 * as they are too large for that. */
static coap_pkt_t pkt;
static uint8_t buf[BUFSIZE];
static uint16_t mid = 3;


int rdcli_simple_register(void)
{
    sock_udp_ep_t remote = {
        .family    = AF_INET6,
        .netif     = SOCK_ADDR_ANY_NETIF,
        .port      = RDCLI_SERVER_PORT,
    };

    /* parse RD server address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&remote.addr.ipv6,
                           RDCLI_SERVER_ADDR) == NULL) {
        return RDCLI_SIMPLE_NOADDR;
    }

    pkt.hdr = (coap_hdr_t*)buf;
    size_t len = 0;
    if (rdcli_path[1]) {
        len = coap_build_hdr(pkt.hdr, COAP_REQ, NULL, 0, COAP_METHOD_PUT, mid++);
    }
    else {
        len = coap_build_hdr(pkt.hdr, COAP_REQ, NULL, 0, COAP_METHOD_POST, mid++);
    }
    coap_pkt_init(&pkt, buf, sizeof(buf), len);
    /* make packet confirmable */
    coap_hdr_set_type(pkt.hdr, COAP_TYPE_CON);
    if (rdcli_path[0]) {
        coap_opt_add_string(&pkt, COAP_OPT_URI_PATH, rdcli_path, '/');
    }
    else {
        coap_opt_add_string(&pkt, COAP_OPT_URI_PATH, "/rd", '/');
    }
    coap_opt_add_uint(&pkt, COAP_OPT_CONTENT_FORMAT, COAP_FORMAT_TEXT);

    /* add Uri-Query options */
    rdcli_common_add_qstring(&pkt);
#ifdef MODULE_RDCLI_LWM2M
    coap_opt_finish(&pkt, COAP_OPT_FINISH_PAYLOAD);
    memcpy(pkt.payload, OPTIONS, sizeof(OPTIONS));
    pkt.payload_len = sizeof(OPTIONS);
#else
    coap_opt_finish(&pkt, 0);
#endif


    sock_udp_ep_t local = { .port=COAP_PORT, .family=AF_INET6 };
    int res = nanocoap_request(&pkt, &local, &remote, sizeof(buf) );
    if (res > 0) {
        if (coap_get_code_raw(&pkt) == COAP_CODE_CREATED) {
           coap_get_location(&pkt, (uint8_t*)rdcli_path);
        }
    }

    return RDCLI_SIMPLE_OK;
}
