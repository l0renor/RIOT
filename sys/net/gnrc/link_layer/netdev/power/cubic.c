#include <math.h>
#include "net/gnrc/netdev/power.h"
#include "cubic.h"
#define ENABLE_DEBUG    (1)
#include "debug.h"

static uint8_t calc_attenuation(netstats_nb_t* peer);
static uint8_t callback(netstats_nb_t* peer, uint8_t num_success, uint8_t num_failed);

gnrc_netdev_power_t cubic = {
    0x02,
    "Cubic",
    calc_attenuation,
    callback,
    NULL,
    NULL
};

gnrc_netdev_power_t *gnrc_netdev_power_cubic(void){
    return &cubic;
}

static uint8_t calc_attenuation(netstats_nb_t* peer) {
    return peer->tx_attenuation; /* Return power calculated at the callback */
}

static uint8_t callback(netstats_nb_t* peer, uint8_t num_success, uint8_t num_failed)
{
    uint16_t att = 0;
    uint8_t cur_att = peer->tx_attenuation;
    peer->transmissions++;
    /* decrease when at least 2 failed */
    if (num_failed > 0) {
       peer->transmissions = 0;
       peer->max_attenuation = cur_att;
       peer->k_factor = cbrt(cur_att * POWER_CUBIC_BETA / (1.0 * POWER_CUBIC_SCALE));
    }
    /* cubic increase in attenuation */
    att = (POWER_CUBIC_SCALE / 100.0) * pow(peer->transmissions - peer->k_factor, 3) + peer->max_attenuation;
    att = att > 255 ? 255 : att;
    if (att > cur_att && att - cur_att > POWER_CUBIC_SLEWLIMIT)
        att = cur_att + POWER_CUBIC_SLEWLIMIT;
    DEBUG("pwrctl: New transmission power is: %u\n", att);
    peer->tx_attenuation = (uint8_t)att;
    return (uint8_t)att;
}
