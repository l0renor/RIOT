#include <stddef.h>
#include "net/gnrc/netdev/power.h"
#include "static.h"

static uint8_t calc_attenuation(netstats_nb_t* peer);

gnrc_netdev_power_t pwr_static = {
    0x00,
    "Static",
    calc_attenuation,
    NULL,
    NULL,
    NULL
};

gnrc_netdev_power_t *gnrc_netdev_power_static(void){
    return &pwr_static;
}

static uint8_t calc_attenuation(netstats_nb_t* peer) {
    peer->tx_attenuation = POWER_STATIC_POWER;
    return POWER_STATIC_POWER; /* Always return no attenuation */
}
