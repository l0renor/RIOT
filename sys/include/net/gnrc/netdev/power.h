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
 * @brief       Power control functions
 *
 */

#ifndef NETDEV_POWER_H
#define NETDEV_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "net/netstats.h"

#define NETDEV_POWER_FUNCTIONS_NUMOF	3
#define NETDEV_POWER_DEFAULT_FUNC	2

/**
 * @brief Power control function representation
 */
typedef struct {
    uint16_t power_function;   /**< objective code point */
    char* name;
    uint8_t (*calc_att)(netstats_nb_t*); /**< Calculate transmit attenuation */
    uint8_t (*callback)(netstats_nb_t*, uint8_t, uint8_t); /**< Callback from a succesful transmit */
    void (*reset)(netstats_nb_t*);    /**< resets the OF */
    void (*init)(void);  /**< Power control specific init function */
} gnrc_netdev_power_t;


void gnrc_netdev_power_init(void);
gnrc_netdev_power_t *gnrc_netdev_power_get_default(void);
uint8_t gnrc_netdev_power_get_default_func(void);
gnrc_netdev_power_t *gnrc_netdev_power_get(uint8_t function);

#ifdef __cplusplus
}
#endif

#endif /* NETDEV_POWER_H */
/**
 * @}
 */
