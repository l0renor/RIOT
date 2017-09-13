#include "net/gnrc/netdev/power.h"
#include "reno.h"
#define ENABLE_DEBUG    (0)
#include "debug.h"

static uint8_t calc_attenuation(netstats_nb_t* peer);
static uint8_t callback(netstats_nb_t* peer, uint8_t num_success, uint8_t num_failed);

gnrc_netdev_power_t reno = {
    0x01,
    "Reno",
    calc_attenuation,
    callback,
    NULL,
    NULL
};

gnrc_netdev_power_t *gnrc_netdev_power_reno(void){
    return &reno;
}

static uint8_t calc_attenuation(netstats_nb_t* peer) {
    return peer->tx_attenuation; /* Return power calculated at the callback */
}

static uint8_t callback(netstats_nb_t* peer, uint8_t num_success, uint8_t num_failed)
{
    uint8_t att = 0;
    uint8_t cur_att = peer->tx_attenuation;
    /* additive increase in attenuation if no drops */
    if (num_failed == 0)
    {
        att = cur_att == 255 ? cur_att : cur_att + POWER_RENO_INCREMENT;
    }
    /* Multiplicative decrease */
    /* All failed, half the attenuation */
    else if (num_success == 0) {
        att = cur_att >> 1;
    }
    else
    {
        /* decrease by failed packets */
        att = (1 - (num_failed * 0.125)) * cur_att;
    }
    DEBUG("pwrctl: New transmission power is: %u\n", att);
    peer->tx_attenuation = att;
    return att;
}
