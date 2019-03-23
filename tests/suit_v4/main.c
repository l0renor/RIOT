/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       minimal RIOT application, intended as size test
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 *
 * @}
 */

#include "periph/pm.h"

#include "suit/v4/cbor.h"
#include "manifest.cbor.h"

int main(void)
{
    suit_v4_cbor_manifest_t manifest;
    suit_v4_cbor_parse(&manifest, manifest_cbor, manifest_cbor_len);

    pm_off();

    return 0;
}
