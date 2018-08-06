/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_USBDEV_EP_H
#define USB_USBDEV_EP_H

#ifdef __cplusplus
extern "C" {
#endif

ssize_t usbdev_ep_get_out(usbdev_ep_pair_t *ep);
ssize_t usbdev_ep_set_in(usbdev_ep_pair_t *ep);

#ifdef __cplusplus
}
#endif

#endif /* USB_USBDEV_EP_H */
/** @} */
