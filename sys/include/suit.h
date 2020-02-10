/*
 * Copyright (C) 2020 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @defgroup    sys_suit SUIT secure firmware updates
 * @ingroup     sys
 * @brief       SUIT secure firmware updates
 *
 * @experimental
 *
 * @{
 *
 * @brief       SUIT helper API
 * @author      Koen Zandberg <koen@bergzand.net>
 *
 */

#ifndef SUIT_H
#define SUIT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function pointers for SUIT update transport protocols
 */
typedef struct {
    int (*get)(const char *url, suit_storage_driver_t *storage),
} suit_transport_driver_t;

/**
 * @brief API for SUIT update storage
 */
typedef struct {
    int (*init)(void *arg, int target_slot);
    int (*update)(void *arg, size_t offset, uint8_t *buf, size_t len, int more);
    int (*verify)(const uint8_t *digest, size_t digest_len, size_t img_len, int target_slot);
} suit_storage_driver_t;

suit_transport_driver_t *suit_transport_driver;
suit_storage_driver_t *suit_storage_driver;

static inline void suit_set_transport_driver(suit_transport_driver_t *transport)
{
    suit_transport_driver = transport;
}

static inline void suit_set_storage_driver(suit_storage_driver_t *storage)
{
    suit_storage_driver = storage;
}

#ifdef __cplusplus
}
#endif

#endif /* SUIT_COAP_H */
/** @} */
