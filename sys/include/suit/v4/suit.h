/*
 * Copyright (C) 2019 Koen Zandberg
 *               2019 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @ingroup     sys_suit
 * @brief       SUIT manifest handling
 *
 * @{
 *
 * @brief       Handler functions for SUIT manifests
 * @author      Koen Zandberg <koen@bergzand.net>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 */

#ifndef SUIT_V4_SUIT_H
#define SUIT_V4_SUIT_H

#include <stddef.h>
#include <stdint.h>

#include "suit/v4/cbor.h"
#include "uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The SUIT vendor ID source
 *
 * The basis of the UUID must be the vendor domain, please change this when
 * using this module in a product
 */
#ifndef SUIT_VENDOR_DOMAIN
#define SUIT_VENDOR_DOMAIN  "riot-os.org" /**< Device vendor domain */
#endif

/**
 * @brief The SUIT class ID source
 *
 * By default the RIOT_VERSION define is used for this
 */
#ifndef SUIT_CLASS_ID
#define SUIT_CLASS_ID  RIOT_VERSION
#endif

/**
 * @brief Supported SUIT manifest version
 */
#define SUIT_MANIFEST_VERSION        4

/**
 * @brief SUIT error codes
 */
typedef enum {
    SUIT_OK                     = 0,    /**< Manifest parsed and validated */
    SUIT_ERR_INVALID_MANIFEST   = -1,   /**< Unexpected CBOR structure detected */
    SUIT_ERR_UNSUPPORTED        = -2,   /**< Unsupported SUIT feature detected */
    SUIT_ERR_NOT_SUPPORTED      = -3,   /**< Unsupported manifest features detected */
    SUIT_ERR_COND               = -4,   /**< Conditionals evaluate to false */
    SUIT_ERR_SEQUENCE_NUMBER    = -5,   /**< Sequence number less or equal to
                                             current sequence number */
} suit_v4_error_t;

/**
 * @brief Parse a manifest
 *
 * @note The buffer is still required after parsing, please don't reuse the
 * buffer while the @p manifest is used
 *
 * @param   manifest    manifest context to store information in
 * @param   buf         buffer to parse the manifest from
 * @param   len         length of the manifest data in the buffer
 *
 * @return              SUIT_OK on parseable manifest
 * @return              negative @ref suit_v4_error_t code on error
 */
int suit_v4_parse(suit_v4_cbor_manifest_t *manifest, uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* SUIT_V4_SUIT_H */
/** @} */