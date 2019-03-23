/*
 * Copyright (C) 2019 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
/**
 * @ingroup     sys_suit_handlers
 * @brief       SUIT v4 manifest handlers
 *
 * @{
 *
 * @brief       Handler functions for SUIT manifests
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef SUIT_V4_HANDLERS_H
#define SUIT_V4_HANDLERS_H

#include <stddef.h>
#include <stdint.h>

#include "suit/v4/cbor.h"
#include "uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief suit handler prototype
 *
 * @param manifest  SUIT v4 manifest context
 * @param it        CborValue iterator to the content the handler must handle
 *
 * @return          1 on success
 * @return          negative on error
 */
typedef int (*suit_manifest_handler)(suit_manifest_t *manifest, int key, CborValue *it);

#ifdef __cplusplus
}
#endif

#endif /* SUIT_V4_HANDLERS_H */
/** @} */

