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
 * @brief       Reno-style power control
 *
 */

#ifndef POWER_CUBIC_H
#define POWER_CUBIC_H

#include "net/gnrc/netdev/power.h"

#ifdef __cplusplus
extern "C" {
#endif

#define POWER_CUBIC_BETA 5
#define POWER_CUBIC_SCALE 1
#define POWER_CUBIC_SLEWLIMIT 40
/**
 * @brief   Return the address to the reno power function
 *
 * @return  Address of the reno power control function
 */
gnrc_netdev_power_t *gnrc_netdev_power_cubic(void);

#ifdef __cplusplus
}
#endif

#endif /* POWER_CUBIC_H */
/**
 * @}
 */
