/*
 * Copyright (C) 2017 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     netdev_power
 * @{
 * @file
 * @brief       Full power static power control
 *
 */

#ifndef POWER_FULL_H
#define POWER_FULL_H

#include "net/gnrc/netdev/power.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POWER_STATIC_POWER 255 /**< Static power output attenuation */

/**
 * @brief   Return the address to the reno power function
 *
 * @return  Address of the reno power control function
 */
gnrc_netdev_power_t *gnrc_netdev_power_static(void);

#ifdef __cplusplus
}
#endif

#endif /* POWER_FULL_H */
/**
 * @}
 */
