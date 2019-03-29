/*
 * Copyright (C) 2019 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_suit
 * @{
 *
 * @file
 * @brief       SUIT manifest handler
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include "suit/conditions.h"
#include "suit/v1/suit.h"
#include "suit/v1/cbor.h"
#include "uuid.h"
#include "luid.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SUIT_DEVID_BYTES 32

static int _validate_uuid(const suit_v1_cbor_manifest_t *manifest,
                          size_t idx, uuid_t *uuid)
{
    uuid_t parameter;
    size_t len = sizeof(parameter);
    int res = suit_v1_cbor_get_condition_parameter(manifest, idx,
                                                (uint8_t *)&parameter,
                                                &len);

    if (res < 0) {
        return res;
    }
    return uuid_equal(&parameter, uuid) ? SUIT_OK : SUIT_ERR_COND;
}

static int _validate_condition(const suit_v1_cbor_manifest_t *manifest,
                               size_t idx, int type)
{
    switch (type) {
        case SUIT_COND_VENDOR_ID:
            return _validate_uuid(manifest, idx, suit_get_vendor_id());
        case SUIT_COND_CLASS_ID:
            return _validate_uuid(manifest, idx, suit_get_class_id());
        case SUIT_COND_DEV_ID:
            return _validate_uuid(manifest, idx, suit_get_device_id());
        case SUIT_COND_BEST_BEFORE:
            DEBUG("suit: best before condition not supported\n");
            return SUIT_ERR_UNSUPPORTED;
        default:
            DEBUG("suit: unknown conditional type: %d\n", type);
            return SUIT_ERR_UNSUPPORTED;
    }
}

static int _validate_conditions(const suit_v1_cbor_manifest_t *manifest)
{
    size_t idx = 0;
    int type = 0;

    while (suit_v1_cbor_get_condition_type(manifest, idx, &type) > 0) {
        int res = _validate_condition(manifest, idx, type);
        if (res < 0) {
            DEBUG("suit: error validating conditional at index %u"
                  "of type %d: %d\n", (unsigned)idx, type, res);
            return res;
        }
        idx++;
    }
    return SUIT_OK;
}

int suit_v1_parse(suit_v1_cbor_manifest_t *manifest, uint8_t *buf, size_t len)
{
    return suit_v1_cbor_parse(manifest, buf, len);
}

int suit_v1_validate_manifest(const suit_v1_cbor_manifest_t *manifest,
                           uint32_t cur_seq_no)
{
    uint32_t version;
    uint32_t seq_no;

    /* Validate manifest version number */
    if (suit_v1_cbor_get_version(manifest, &version) < 0) {
        return SUIT_ERR_INVALID_MANIFEST;
    }

    if (version != SUIT_MANIFEST_VERSION) {
        return SUIT_ERR_UNSUPPORTED;
    }

    /* Compare sequence numbers */
    if (suit_v1_cbor_get_seq_no(manifest, &seq_no) < 0) {
        return SUIT_ERR_INVALID_MANIFEST;
    }

    if (seq_no <= cur_seq_no) {
        return SUIT_ERR_SEQUENCE_NUMBER;
    }

    int res = _validate_conditions(manifest);
    if (res < 0) {
        return res;
    }
    return SUIT_OK;
}
