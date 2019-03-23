/*
 * Copyright (C) 2019 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @ingroup     sys_suit_v4
 * @{
 *
 * @file
 * @brief       SUIT v4
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 *
 * @}
 */

#include "suit/v4/handlers.h"

static int _version_handler(suit_v4_cbor_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;
    /* Validate manifest version */
    int version = -1;
    if (cbor_value_is_integer(it) &&
            (cbor_value_get_int(it, version) == CborNoError)) {
        return version == SUIT_VERSION ? 0 : -1;
    }
    return 1;
}

static int _seq_no_handler(suit_v4_cbor_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;
    /* Validate that all sequence numbers on this device are lower than the
     * supplied version number */
    return 1;
}

static int _dependencies_handler(suit_v4_cbor_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;
    /* No dependency support */
    return 0;
}

static int _common_handler(suit_v4_cbor_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;
    /* Check common section */
    return 1;
}

static int _component_handler(suit_v4_cbor_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;
    return 1;
}

static const suit_manifest_handler_t *global_handlers[] = {
    NULL,
    _version_handler,
    _seq_no_handler,
    _dependencies_handler,
    _common_handler,
    _component_handler,
    NULL, /* dependency resolution */
    _payload_fetch,
    _install,
    _validate,
    _load,
    _run,
}
