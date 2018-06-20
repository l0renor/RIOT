/*
 * Copyright (C) 2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_rdcli_common
 * @{
 *
 * @file
 * @brief       Implementation of common functions for CoRE RD clients
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "fmt.h"
#include "luid.h"

#include "net/nanocoap.h"
#include "net/rdcli_common.h"

#define ENABLE_DEBUG (0)
#include "debug.h"


#ifdef RDCLI_EP
#define BUFSIZE         (sizeof(RDCLI_EP))  /* contains \0 termination char */
#else
#define PREFIX_LEN      (sizeof(RDCLI_EP_PREFIX))       /* contains \0 char */
#define BUFSIZE         (PREFIX_LEN + RDCLI_EP_SUFFIX_LEN)
#endif

char rdcli_ep[BUFSIZE];
char rdcli_path[NANOCOAP_URI_MAX];

void rdcli_common_init(void)
{
#ifdef RDCLI_EP
    memcpy(rdcli_ep, RDCLI_EP, BUFSIZE);
#else
    uint8_t luid[RDCLI_EP_SUFFIX_LEN / 2];

    if (PREFIX_LEN > 1) {
        memcpy(rdcli_ep, RDCLI_EP_PREFIX, (PREFIX_LEN - 1));
    }

    luid_get(luid, sizeof(luid));
    fmt_bytes_hex(&rdcli_ep[PREFIX_LEN - 1], luid, sizeof(luid));
    rdcli_ep[BUFSIZE - 1] = '\0';
#endif
    memset(rdcli_path, 0, sizeof(rdcli_path));
}

int rdcli_common_add_qstring(coap_pkt_t *pkt)
{
    char buf[64];
    char *pos = buf;
    *pos++ = '&';

    size_t len = strlen("lt");
    strncpy(pos, "lt", len);
    pos += 2;
    *pos++ = '=';
    pos += fmt_u32_dec(pos, RDCLI_LT);
    if (!rdcli_path[1]) {
        *pos++ = '&';
        len = strlen("ep");
        strcpy(pos, "ep");
        pos += len;
        *pos++ = '=';
        strcpy(pos, rdcli_ep);
        pos += strlen(rdcli_ep);

        *pos++ = '&';

#ifdef RDCLI_D
        *pos++ = '&';

        *pos++ = 'd';

        strncpy(pos, RDCLI_D);
        pos += strlen(RDCLI_D);
#endif
#ifdef MODULE_RDCLI_LWM2M
        *pos++ = '&';
        strcpy(pos, "lwm2m=1.0");
        pos += strlen("lwm2m=1.0");
        *pos++ = '&';
        strcpy(pos, "b=U");
        pos += strlen("b=U");
#endif
    }
    *pos++ = '\0';
    /* extend the url with some query string options */
    return coap_opt_add_string(pkt, COAP_OPT_URI_QUERY, buf, '&');
}
