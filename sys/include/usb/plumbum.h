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
    usbdev_ep_t *ep;
    uint8_t interval;                   /**< Poll interval for interrupt endpoints */
};

struct plumbum_interface {
    struct plumbum_interface *next;
    plumbum_handler_t* handler;
    plumbum_hdr_gen_t *hdr_gen;
    plumbum_endpoint_t *ep;                   /** LL of endpoints */
    plumbum_string_t *descr;
    uint16_t idx;
    uint8_t class;
    uint8_t subclass;
    uint8_t protocol;
};

typedef struct {
    plumbum_string_t manuf;         /**< Manufacturer string                */
    plumbum_string_t product;       /**< Product string                     */
    plumbum_string_t config;        /**< Configuration string               */
    usbdev_ep_t *out;               /**< EP0 out endpoint                   */
    usbdev_ep_t *in;                /**< EP0 in endpoint                    */
    uint8_t *buf_in;                /**< TODO: removeme */
    uint8_t *buf_out;
    usbdev_t *dev;                  /**< usb phy device of the usb manager  */
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
    size_t (*gen_hdr)(plumbum_t *plumbum, void *arg, uint8_t *buf, size_t max_len);
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
int plumbum_add_endpoint(plumbum_t *plumbum, plumbum_interface_t *iface, plumbum_endpoint_t* ep, usb_ep_type_t type, usb_ep_dir_t dir);
plumbum_t *plumbum_get_ctx(void);
void plumbum_register_event_handler(plumbum_t *plumbum, plumbum_handler_t *handler);
void plumbum_init(void);
void plumbum_create(char *stack, int stacksize, char priority,
                   const char *name, usbdev_t *usbdev);


#ifdef __cplusplus
}
#endif
#endif /* USB_PLUMBUM_H */
/** @} */
