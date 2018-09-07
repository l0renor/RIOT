/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    usb_plumbum_hdrs Plumbum USB header functions
 * @ingroup     usb_plumbum
 *
 * @{
 *
 * @file
 * @brief       Plumbum basic interface
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef USB_PLUMBUM_HDRS_H
#define USB_PLUMBUM_HDRS_H

#include <stdint.h>
#include <stdlib.h>
#include "usb/plumbum.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t plumbum_hdrs_config_size(plumbum_t *plumbum);
size_t plumbum_hdrs_fmt_additional(plumbum_t *plumbum, plumbum_hdr_gen_t *hdr);
size_t plumbum_hdrs_fmt_ifaces(plumbum_t *plumbum);
size_t plumbum_hdrs_fmt_endpoints(plumbum_t *plumbum, plumbum_interface_t *iface);

#ifdef __cplusplus
}
#endif
#endif /* USB_PLUMBUM_HDRS_H */
/** @} */
