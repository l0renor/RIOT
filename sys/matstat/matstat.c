/*
 * Copyright (C) 2018 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include "matstat.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

void matstat_clear(matstat_state_t *state)
{
    *state = MATSTAT_STATE_INIT;
}

void matstat_add(matstat_state_t *state, int32_t value)
{
    if (value > state->max) {
        state->max = value;
    }
    if (value < state->min) {
        state->min = value;
    }
    /* Using Welford's algorithm for variance */
    if (!state->count) {
        state->sum_sq = 0;
    }
    else {
        int32_t new_mean = (state->sum + value) / (state->count + 1);
        state->sum_sq += (value - matstat_mean(state)) * (value - new_mean);
    }
    state->count++;
    state->sum += value;
}

uint64_t matstat_variance(const matstat_state_t *state)
{
    if (state->count < 2) {
        /* We don't have any way of returning an error */
        return 0;
    }
    uint64_t variance = state->sum_sq / (state->count - 1);
    DEBUG("Var: (%" PRIu64 " / (%" PRId32 " - 1)) = %" PRIu64 "\n",
        state->sum_sq, state->count, variance);
    return variance;
}

void matstat_merge(matstat_state_t *dest, const matstat_state_t *src)
{
    if (src->count == 0) {
        /* src is empty, no-op */
        return;
    }
    if (dest->count == 0) {
        /* dest is empty, straight copy */
        *dest = *src;
        return;
    }
    /* Combining the variance of the two samples needs some extra
     * handling if the means are different between the two states,
     * source: https://stats.stackexchange.com/a/43183
     * (using sum_sq = sigma2 * n)
     */
    dest->sum_sq = (dest->sum_sq + dest->sum * matstat_mean(dest) +
            src->sum_sq + src->sum * matstat_mean(src));
    dest->count += src->count;
    dest->sum += src->sum;
    dest->sum_sq = dest->sum_sq - matstat_mean(dest) * dest->sum;
    if (src->max > dest->max) {
        dest->max = src->max;
    }
    if (src->min < dest->min) {
        dest->min = src->min;
    }
}
