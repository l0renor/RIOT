/*
 * Copyright (C) 2017 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "static.h"
#include "reno.h"
#include "cubic.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static gnrc_netdev_power_t *power_functions[NETDEV_POWER_FUNCTIONS_NUMOF];

void gnrc_netdev_power_init(void)
{
    /* insert new power functions here */
    power_functions[0] = gnrc_netdev_power_static();
    power_functions[1] = gnrc_netdev_power_reno();
    power_functions[2] = gnrc_netdev_power_cubic();
}

gnrc_netdev_power_t *gnrc_netdev_power_get_default(void)
{
    return power_functions[NETDEV_POWER_DEFAULT_FUNC];
}

uint8_t gnrc_netdev_power_get_default_func(void)
{
    return NETDEV_POWER_DEFAULT_FUNC;
}

gnrc_netdev_power_t *gnrc_netdev_power_get(uint8_t function)
{
    if (function >= NETDEV_POWER_FUNCTIONS_NUMOF)
    {
        return power_functions[NETDEV_POWER_DEFAULT_FUNC];
    }
    else if (power_functions[function] != NULL)
    {
        return power_functions[function];
    }
    DEBUG("[pwrctl] returning default");
    return power_functions[NETDEV_POWER_DEFAULT_FUNC];
}
