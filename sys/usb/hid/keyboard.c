/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup usb_hid HID Keyboard functions
 * @{
 * @file
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include "thread.h"
#include "kernel_types.h"
#include "msg.h"
#include "mutex.h"
#include "usb/plumbum.h"
#include "usb/hid.h"
#include "usb/message.h"

#include "usb/hid/keyboard.h"
#include "periph/gpio.h"
#include "board.h"

#include <string.h>

#define ENABLE_DEBUG    (1)
#include "debug.h"

//#define _KEYBOARD_MSG_QUEUE_SIZE    (8)
//#define KEYBOARD_STACKSIZE           (THREAD_STACKSIZE_SMALL)
//#define KEYBOARD_PRIO                (THREAD_PRIORITY_MAIN - 5)
//#define KEYBOARD_TNAME               "keyboard"

//static char _stack[KEYBOARD_STACKSIZE];

//static void *_keyboard_thread(void *args);
//void keyboard_create(char *stack, int stacksize, char priority,
//                   const char *name);

static uint8_t buf[64];

static int event_handler(plumbum_t *plumbum, plumbum_handler_t *handler, uint16_t event, void *arg);
static int _init(plumbum_t *plumbum, plumbum_handler_t *handler);

static const uint8_t report_descriptor[] = {
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x01,        // Usage (Consumer control)
0xA1, 0x01,        // Collection (Application)
0x05, 0x0C,        //   Usage Page (Consumer devices)
0x75, 0x01,        //   Report Size (1)
0x95, 0x07,        //   Report Count (7)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x09, 0xB5,        //   Usage (Scan Next Track) */ 
0x09, 0xB6,        //   Usage (Scan Previous Track) */ 
0x09, 0xB7,        //   Usage (Stop) */ 
0x09, 0xCD,        //   Usage (Play / Pause) */ 
0x09, 0xE2,        //   Usage (Mute) */ 
0x09, 0xE9,        //   Usage (Volume Up) */ 
0x09, 0xEA,        //   Usage (Volume Down) */ 
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0
};


void _gpio_cb(void *arg)
{
    plumbum_hid_device_t *hid = (plumbum_hid_device_t*)arg;
    hid->prev_state = hid->state;
    hid->state = gpio_read(BTN0_PIN);
    if (hid->state != hid->prev_state) {
        if (hid->state) {
            memset(buf, 0 , sizeof(buf));
        }
        else {
            memset(buf, 0 , sizeof(buf));
            buf[0] = 0x01;
        }
    }
    hid->ep.ep->driver->ready(hid->ep.ep, 1);
}

const plumbum_handler_driver_t hid_driver = {
    .init = _init,
    .event_handler = event_handler,
};

static size_t _gen_hid_descriptor(plumbum_t *plumbum, void *arg, uint8_t *buf, size_t max_len)
{
    (void)plumbum;
    (void)arg;
    if (max_len >= sizeof(usb_descriptor_hid_t)) {
        usb_descriptor_hid_t*hid = (usb_descriptor_hid_t*)buf;
        hid->length = sizeof(usb_descriptor_hid_t);
        hid->bcd_hid = 0x0110;
        hid->type = USB_TYPE_DESCRIPTOR_HID; 
        hid->country_code = USB_HID_COUNTRYCODE_NONE;
        hid->num_descriptors = 1;
        hid->report_type = USB_HID_DESCRIPTOR_TYPE_REPORT;
        hid->report_length = sizeof(report_descriptor);
    }
    return sizeof(usb_descriptor_hid_t);
}

static size_t _hid_size(plumbum_t *plumbum, void *arg)
{
    (void)plumbum;
    (void)arg;
    return sizeof(usb_descriptor_hid_t);
}

static plumbum_hid_device_t handler;

void keyboard_init(plumbum_t *plumbum)
{
    memset(&handler, 0 , sizeof(plumbum_hid_device_t));
    handler.handler.driver = &hid_driver;
    plumbum_register_event_handler(plumbum, (plumbum_handler_t*)&handler);
}

//
//void keyboard_create(char *stack, int stacksize, char priority,
//                   const char *name)
//{
//    int res = thread_create(stack, stacksize, priority, THREAD_CREATE_STACKTEST,
//                        _keyboard_thread, NULL, name);
//    (void)res;
//    assert(res > 1);
//}

static int _init(plumbum_t *plumbum, plumbum_handler_t *handler)
{
    plumbum_hid_device_t *hid = (plumbum_hid_device_t*)handler;

    hid->hid_hdr.next = NULL;
    hid->hid_hdr.gen_hdr = _gen_hid_descriptor;
    hid->hid_hdr.hdr_len = _hid_size;
    hid->hid_hdr.arg = NULL;

    /* Instantiate interface */
    memset(&hid->iface, 0, sizeof(plumbum_interface_t));
    /* Get EP's for interface */
    hid->iface.class = USB_CLASS_HID;
    hid->iface.subclass = USB_HID_SUBCLASS_NONE;
    hid->iface.protocol = USB_HID_PROTOCOL_NONE;
    hid->iface.hdr_gen = &hid->hid_hdr;
    hid->iface.handler = handler;
    
    plumbum_add_interface(plumbum, &hid->iface);
    if (plumbum_add_endpoint(plumbum, &hid->iface, &hid->ep, USB_EP_TYPE_INTERRUPT, USB_EP_DIR_IN) < 0)
    {
        DEBUG("hid_keyboard: error getting interface\n");
        return -1;
    }
    size_t len = sizeof(buf);
    static const usbopt_enable_t enable = USBOPT_ENABLE;
    uint8_t *bufptr = buf;
    hid->ep.ep->driver->set(hid->ep.ep, USBOPT_EP_BUF_ADDR, &bufptr, sizeof(uint8_t*));
    hid->ep.ep->driver->set(hid->ep.ep, USBOPT_EP_BUF_SIZE, &len, sizeof(len));
    hid->ep.ep->driver->set(hid->ep.ep, USBOPT_EP_STALL, &enable, sizeof(usbopt_enable_t));
    hid->ep.ep->driver->set(hid->ep.ep, USBOPT_EP_ENABLE, &enable, sizeof(usbopt_enable_t));

    memset(buf, 0, sizeof(buf));
    gpio_init_int(BTN0_PIN, BTN0_MODE, GPIO_BOTH, _gpio_cb, hid); 

    return 0;
}

static int _handle_hid_report(plumbum_t *plumbum, usb_setup_t *pkt)
{
    (void)pkt;
    memcpy(plumbum->buf_in, report_descriptor, sizeof(report_descriptor));
    plumbum->in->driver->ready(plumbum->in, sizeof(report_descriptor));
    return 0;
}

static int _handle_setup(plumbum_t *plumbum, plumbum_handler_t *handler, usb_setup_t *pkt)
{
    (void)handler;
    switch(pkt->request) {
        case 0x06:
            return _handle_hid_report(plumbum, pkt);
        case USB_SETUP_REQUEST_TYPE_IDLE:
            puts("setidle");
            //plumbum->in->driver->ready(plumbum->in, 0);
            return 0;
        default:
            return -1;
    }
}

static int _handle_tr_complete(plumbum_t *plumbum, plumbum_handler_t *handler, plumbum_endpoint_t *ep)
{
    (void)plumbum;
    (void)handler;
    (void)ep;
    return 0;
}

static int event_handler(plumbum_t *plumbum, plumbum_handler_t *handler, uint16_t event, void *arg)
{
    (void)plumbum;
    (void)handler;

    switch(event) {
            //case PLUMBUM_MSG_EP_EVENT:
            case PLUMBUM_MSG_TYPE_SETUP_RQ:
                return _handle_setup(plumbum, handler, (usb_setup_t*)arg);

            case PLUMBUM_MSG_TYPE_TR_COMPLETE:
                return _handle_tr_complete(plumbum, handler, arg);

        default:
            return -1;
            break;
    }
    return 0;
}

