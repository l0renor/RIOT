/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *               2019 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @defgroup    sys_suit SUIT manifest parsers
 * @ingroup     sys
 * @brief       SUIT manifest parser for CBOR based manifests
 *
 * This is a simple suit manifest parser for RIOT. The high level assumption is
 * that the raw manifest data is stored in a buffered location where raw values
 * or strings can be left during the lifetime of the suit_v4_manifest_t struct.
 * This assumption is valid in the case where a (network based) transfer
 * mechanism is used to transfer the manifest to the node and an intermediate
 * buffer is necessary to validate the manifest.
 *
 * The parser is based on draft version 4 of the specification, restricted to
 * handling CBOR based manifests.
 *
 * @see https://tools.ietf.org/html/draft-moran-suit-manifest-04
 *
 * @{
 *
 * @brief       Decoding functions for a CBOR encoded SUIT manifest
 * @author      Koen Zandberg <koen@bergzand.net>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 */

#ifndef SUIT_V4_CBOR_H
#define SUIT_V4_CBOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "cbor.h"
#include "uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief TinyCBOR validation mode to use
 */
#define SUIT_TINYCBOR_VALIDATION_MODE       CborValidateStrictMode

/**
 * @brief SUIT conditionals
 */
enum {
    SUIT_COND_VENDOR_ID     = 1,    /**< Vendor ID match conditional */
    SUIT_COND_CLASS_ID      = 2,    /**< Class ID match conditional */
    SUIT_COND_DEV_ID        = 3,    /**< Device ID match conditional */
    SUIT_COND_BEST_BEFORE   = 4,    /**< Best before conditional */
};

/**
 * @brief SUIT payload digest algorithms
 *
 * Unofficial list from
 * [suit-manifest-generator](https://github.com/ARMmbed/suit-manifest-generator)
 */
typedef enum {
    SUIT_DIGEST_NONE    = 0,    /**< No digest algo supplied */
    SUIT_DIGEST_SHA256  = 1,    /**< SHA256 */
    SUIT_DIGEST_SHA384  = 2,    /**< SHA384 */
    SUIT_DIGEST_SHA512  = 3,    /**< SHA512 */
} suit_v4_cbor_digest_t;

/**
 * @brief SUIT payload digest types
 *
 * Unofficial list from
 * [suit-manifest-generator](https://github.com/ARMmbed/suit-manifest-generator)
 */
typedef enum {
    SUIT_DIGEST_TYPE_RAW        = 1,    /**< Raw payload digest */
    SUIT_DIGEST_TYPE_INSTALLED  = 2,    /**< Installed firmware digest */
    SUIT_DIGEST_TYPE_CIPHERTEXT = 3,    /**< Ciphertext digest */
    SUIT_DIGEST_TYPE_PREIMAGE   = 4     /**< Pre-image digest */
} suit_v4_cbor_digest_type_t;

/**
 * @brief SUIT manifest struct
 */
typedef struct {
    const uint8_t *buf; /**< ptr to the buffer of the manifest */
    size_t len;         /**< length of the manifest */
} suit_v4_cbor_manifest_t;

/**
 * @brief Parse a buffer containing a cbor encoded manifest into a
 * suit_v4_cbor_manifest_t struct
 *
 * @param   manifest    manifest to fill
 * @param   buf         Buffer to read from
 * @param   len         Size of the buffer
 *
 * return               @ref SUIT_OK on success
 * return               negative on parsing error
 */
int suit_v4_cbor_parse(suit_v4_cbor_manifest_t *manifest, const uint8_t *buf,
                       size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SUIT_V4_CBOR_H */
/** @} */
