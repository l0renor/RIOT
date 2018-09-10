/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    usb_plumbum Plumbum USB device and endpoint manager
 * @ingroup     usb
 * @brief       Plumbum, a simple USB device management interface
 *
 * @{
 *
 * @file
 * @brief       Plumbum basic interface
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef USB_PLUMBUM_H
#define USB_PLUMBUM_H

#include <stdint.h>
#include <stdlib.h>
#include "usb/usbdev.h"
#include "usb.h"
#include "usb/message.h"

#include "kernel_types.h"
#include "msg.h"
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLUMBUM_MSG_TYPE_EVENT   (0x1234)
#define PLUMBUM_MSG_TYPE_EP_EVENT   (0x1235)
#define PLUMBUM_MSG_TYPE_SETUP_RQ   (0x1236)
#define PLUMBUM_MSG_TYPE_TR_COMPLETE   (0x1237)

typedef enum {
    PLUMBUM_STATE_DISCONNECT,
    PLUMBUM_STATE_RESET,
    PLUMBUM_STATE_ADDR,
    PLUMBUM_STATE_CONFIGURED,
    PLUMBUM_STATE_SUSPEND,
} plumbum_state_t;

typedef enum {
    PLUMBUM_SETUPRQ_READY,      /**< Ready for new request */
    PLUMBUM_SETUPRQ_INDATA,     /**< Request received with expected DATA IN stage */
    PLUMBUM_SETUPRQ_OUTACK,     /**< Expecting a ZLP ack from host */
    PLUMBUM_SETUPRQ_OUTDATA,    /**< Data out expected */
    PLUMBUM_SETUPRQ_INACK,      /**< ACK in request */
} plumbum_setuprq_state_t;

typedef struct plumbum_string plumbum_string_t;
typedef struct plumbum_hdr_gen plumbum_hdr_gen_t;
typedef struct plumbum_interface plumbum_interface_t;
typedef struct plumbum_endpoint plumbum_endpoint_t;
typedef struct plumbum_handler plumbum_handler_t;


struct plumbum_string {
    struct plumbum_string *next;
    uint16_t idx;
    const char *str;
    size_t len;
};

struct plumbum_endpoint {
    struct plumbum_endpoint *next;
    plumbum_hdr_gen_t *hdr_gen;
    usbdev_ep_t *ep;
    uint16_t maxpacketsize;             /**< Max packet size of this endpoint */
    uint8_t interval;                   /**< Poll interval for interrupt endpoints */
    bool active;                        /**< If the endpoint should be activated after reset */
};

struct plumbum_interface {
    struct plumbum_interface *next;
    struct plumbum_interface_alt *alts;
    plumbum_handler_t* handler;
    plumbum_hdr_gen_t *hdr_gen;     /**< Additional header generators */
    plumbum_endpoint_t *ep;         /**< Linked list of endpoints     */
    plumbum_string_t *descr;        /**< Descriptor string            */
    uint16_t idx;
    uint8_t class;
    uint8_t subclass;
    uint8_t protocol;
};

typedef struct plumbum_interface_alt {
    struct plumbum_interface_alt *next;
    plumbum_hdr_gen_t *hdr_gen;
    plumbum_endpoint_t *ep;
} plumbum_interface_alt_t;

typedef struct {
    size_t start;
    size_t cur;
    size_t len;
    size_t transfered;
    size_t reqlen;                  /**< Maximum length of request */
} plumbum_controlbuilder_t;

typedef struct {
    plumbum_string_t manuf;         /**< Manufacturer string                */
    plumbum_string_t product;       /**< Product string                     */
    plumbum_string_t config;        /**< Configuration string               */
    plumbum_controlbuilder_t builder;
    usbdev_ep_t *out;               /**< EP0 out endpoint                   */
    usbdev_ep_t *in;                /**< EP0 in endpoint                    */
    usbdev_t *dev;                  /**< usb phy device of the usb manager  */
    usb_setup_t setup;              /**< Last received setup packet         */
    plumbum_hdr_gen_t *hdr_gen;
    plumbum_string_t *strings;      /**< List of descriptor strings         */
    plumbum_interface_t *iface;     /**< List of USB interfaces             */
    plumbum_handler_t *handler;
    kernel_pid_t pid;               /**< PID of the usb manager's thread    */
    uint16_t addr;                  /**< Address of the USB port            */
    plumbum_state_t state;          /**< Current state                      */
    plumbum_setuprq_state_t setup_state; /**< Setup request state machine   */
    uint16_t str_idx;
    mutex_t lock;                   /**< Mutex for modifying the object     */
} plumbum_t;

struct plumbum_hdr_gen {
    struct plumbum_hdr_gen *next;
    size_t (*gen_hdr)(plumbum_t *plumbum, void *arg);
    size_t (*hdr_len)(plumbum_t *plumbum, void *arg);
    void *arg;
};

typedef struct {
    int (*init)(plumbum_t *plumbum, struct plumbum_handler *handler);
    int (*event_handler)(plumbum_t *plumbum, struct plumbum_handler *handler, uint16_t event, void *arg);
} plumbum_handler_driver_t;

struct plumbum_handler {
    struct plumbum_handler *next;
    const plumbum_handler_driver_t *driver;
    plumbum_interface_t *iface;
};


uint16_t plumbum_add_interface(plumbum_t *plumbum, plumbum_interface_t *iface);
int plumbum_add_endpoint(plumbum_t *plumbum, plumbum_interface_t *iface, plumbum_endpoint_t* ep, usb_ep_type_t type, usb_ep_dir_t dir, size_t len);
void plumbum_add_conf_descriptor(plumbum_t *plumbum, plumbum_hdr_gen_t* hdr_gen);
plumbum_t *plumbum_get_ctx(void);
void plumbum_register_event_handler(plumbum_t *plumbum, plumbum_handler_t *handler);
void plumbum_init(void);
void plumbum_create(char *stack, int stacksize, char priority,
                   const char *name, usbdev_t *usbdev);

size_t plumbum_put_bytes(plumbum_t *plumbum, const uint8_t *buf, size_t len);
size_t plumbum_put_char(plumbum_t *plumbum, char c);
void plumbum_ep0_ready(plumbum_t *plumbum);

static inline void plumbum_enable_endpoint(plumbum_endpoint_t *ep)
{
    ep->active = true;
}

static inline void plumbum_disable_endpoint(plumbum_endpoint_t *ep)
{
    ep->active = false;
}

#ifdef __cplusplus
}
#endif
#endif /* USB_PLUMBUM_H */
/** @} */
