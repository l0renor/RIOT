/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "net/nanocoap.h"
#include "net/nanocoap_sock.h"
#include "net/lwm2m.h"
#ifdef MODULE_LWM2M_DEVICE
#include "net/lwm2m/device.h"
#endif
#ifdef MODULE_LWM2M_FIRMWARE
#include "net/lwm2m/firmware.h"
#endif


#define COAP_INBUF_SIZE (256U)

#define STACKSIZE           (THREAD_STACKSIZE_DEFAULT)
#define PRIO                (THREAD_PRIORITY_MAIN - 2)
#define TNAME               "lwm2m"


/* must be sorted by path (alphabetically) */
const coap_resource_t coap_resources[] = {
    COAP_WELL_KNOWN_CORE_DEFAULT_HANDLER,
#ifdef MODULE_LWM2M_DEVICE
    LWM2M_COAP_DEVICE_RESOURCES,
#endif
#ifdef MODULE_LWM2M_FIRMWARE
    LWM2M_COAP_FIRMWARE_RESOURCES,
#endif
};


const unsigned coap_resources_numof = sizeof(coap_resources) / sizeof(coap_resources[0]);

static char _stack[STACKSIZE];
#define LWM2M_QUEUE_SIZE    (4)
static msg_t _lwm2m_msg_queue[LWM2M_QUEUE_SIZE];

static void *lwm2m_runner(void *arg)
{
    (void)arg;
    msg_init_queue(_lwm2m_msg_queue, LWM2M_QUEUE_SIZE);
    uint8_t buf[COAP_INBUF_SIZE];

    sock_udp_ep_t local = { .port=COAP_PORT, .family=AF_INET6 };
    nanocoap_server(&local, buf, COAP_INBUF_SIZE);
    return NULL;
}

void lwm2m_run(void)
{
    thread_create(_stack, sizeof(_stack), PRIO, THREAD_CREATE_STACKTEST, lwm2m_runner, NULL, TNAME);
}
