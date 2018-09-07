/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup usb_plumbum_hdrs Plumbum USB header functions
 * @{
 * @file
 * @brief   Plumbum USB header formatting functions
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <string.h>
#include <stdio.h>
#include "usb/plumbum/hdrs.h"
#include "usb/message.h"

size_t plumbum_hdrs_config_size(plumbum_t *plumbum)
{
    size_t len = sizeof(usb_descriptor_configuration_t);
    for (plumbum_interface_t *iface = plumbum->iface;
            iface;
            iface = iface->next) {
        len += sizeof(usb_descriptor_interface_t);
        for (plumbum_hdr_gen_t *hdr = iface->hdr_gen; hdr; hdr = hdr->next) {
            len += hdr->hdr_len(plumbum, hdr->arg);
        }
        for (plumbum_endpoint_t *ep = iface->ep;
                ep; ep = ep->next) {
            len += sizeof(usb_descriptor_endpoint_t);
        }
    }
    return len;
}

size_t plumbum_hdrs_fmt_additional(plumbum_t *plumbum, plumbum_hdr_gen_t *hdr)
{
    size_t len = 0;
    for (; hdr; hdr = hdr->next) {
        len += hdr->gen_hdr(plumbum, hdr->arg);
    }
    return len;
}

size_t plumbum_hdrs_fmt_ifaces(plumbum_t *plumbum)
{
    size_t len = 0;
    for (plumbum_interface_t *iface = plumbum->iface;
            iface;
            iface = iface->next) {
        usb_descriptor_interface_t usb_iface;
        memset(&usb_iface, 0 , sizeof(usb_descriptor_interface_t));
        usb_iface.length = sizeof(usb_descriptor_interface_t);
        usb_iface.type = USB_TYPE_DESCRIPTOR_INTERFACE;
        usb_iface.interface_num = iface->idx;
        usb_iface.alternate_setting = 0;
        usb_iface.class = iface->class;
        usb_iface.subclass = iface->subclass;
        usb_iface.protocol = iface->protocol;
        usb_iface.num_endpoints = 1;
        if (iface->descr) {
            usb_iface.idx = iface->descr->idx;
        }
        else {
            usb_iface.idx = 0;
        }
        plumbum_put_bytes(plumbum, (uint8_t*)&usb_iface, sizeof(usb_descriptor_interface_t));
        len += sizeof(usb_descriptor_interface_t);
        len += plumbum_hdrs_fmt_additional(plumbum, iface->hdr_gen);
        len += plumbum_hdrs_fmt_endpoints(plumbum, iface);
    }
    return len;
}


size_t plumbum_hdrs_fmt_endpoints(plumbum_t *plumbum, plumbum_interface_t *iface)
{
    size_t len = 0;
    for (plumbum_endpoint_t *ep = iface->ep;
            ep; ep = ep->next) {
        usb_descriptor_endpoint_t usb_ep;
        memset(&usb_ep, 0 , sizeof(usb_descriptor_endpoint_t));
        usb_ep.length = sizeof(usb_descriptor_endpoint_t);
        usb_ep.type = USB_TYPE_DESCRIPTOR_ENDPOINT;
        usb_ep.address = ep->ep->num;
        if (ep->ep->dir == USB_EP_DIR_OUT) {
            usb_ep.address |= 0x80;
        }
        usb_ep.attributes = 3;
        usb_ep.max_packet_size = 64;
        usb_ep.interval = 20;
        usb_ep.address = 0x81;
        plumbum_put_bytes(plumbum, (uint8_t*)&usb_ep, sizeof(usb_descriptor_endpoint_t));
        len += usb_ep.length;
    }
    return len;
}

