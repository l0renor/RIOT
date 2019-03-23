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

#include <inttypes.h>

#include "suit/v4/suit.h"
#include "suit/v4/handlers.h"
#include "suit/v4/policy.h"
#include "suit/v4/suit.h"
#include "riotboot/hdr.h"
#include "riotboot/slot.h"
#include "cbor.h"

#define HELLO_HANDLER_MAX_STRLEN 32

static int _hello_handler(suit_v4_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;

    char buf[HELLO_HANDLER_MAX_STRLEN];
    size_t len = HELLO_HANDLER_MAX_STRLEN;

    if (cbor_value_is_text_string(it)) {
        cbor_value_copy_text_string(it, buf, &len, NULL);
        printf("HELLO: \"%.*s\"\n", len, buf);
        return SUIT_OK;
    }
    else {
        printf("_hello_handler(): unexpected value type: %u\n", cbor_value_get_type(
                   it));
        return -1;
    }
}

static int _version_handler(suit_v4_manifest_t *manifest, int key,
                            CborValue *it)
{
    (void)manifest;
    (void)key;
    /* Validate manifest version */
    int version = -1;
    if (cbor_value_is_integer(it) &&
        (cbor_value_get_int(it, &version) == CborNoError)) {
        if (version == SUIT_VERSION) {
            manifest->validated |= SUIT_VALIDATED_VERSION;
            return 0;
            puts("suit: validated manifest version");
        }
        else {
            return -1;
        }
    }
    return -1;
}

static int _seq_no_handler(suit_v4_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;

    uint32_t seq_nr;

    if (cbor_value_is_unsigned_integer(it) &&
        (cbor_value_get_int_checked(it, (int*)&seq_nr) == CborNoError)) {

        const riotboot_hdr_t *hdr = riotboot_slot_get_hdr(riotboot_slot_current());
        if (seq_nr <= hdr->version) {
            printf("%"PRIu32" <= %"PRIu32"\n", seq_nr, hdr->version);
            puts("seq_nr <= running image");
            return -1;
        }

        hdr = riotboot_slot_get_hdr(riotboot_slot_other());
        if (riotboot_hdr_validate(hdr) == 0) {
            if (seq_nr <= hdr->version) {
                printf("%"PRIu32" <= %"PRIu32"\n", seq_nr, hdr->version);
                puts("seq_nr <= other image");
                return -1;
            }
        }

        puts("suit: validated sequence number");
        manifest->validated |= SUIT_VALIDATED_SEQ_NR;
        return 0;
    }
    return -1;
}

static int _dependencies_handler(suit_v4_manifest_t *manifest, int key,
                                 CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;
    /* No dependency support */
    return 0;
}

static int _common_handler(suit_v4_manifest_t *manifest, int key, CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;
    /* Check common section */
    return 1;
}

static int _component_handler(suit_v4_manifest_t *manifest, int key,
                              CborValue *it)
{
    (void)manifest;
    (void)key;
    (void)it;
    return 1;
}

/* begin{code-style-ignore} */
static suit_manifest_handler_t global_handlers[] = {
    [ 0] = _hello_handler,
    [ 1] = _version_handler,
    [ 2] = _seq_no_handler,
    [ 3] = _dependencies_handler,
    [ 4] = _common_handler,
    [ 5] = _component_handler,
    [ 6] = NULL, /* dependency resolution */
#if 0
    [ 7] = _payload_fetch,
    [ 8] = _install,
    [ 9] = _validate,
    [10] = _load,
    [11] = _run,
#endif
};
/* end{code-style-ignore} */

static const unsigned global_handlers_len = sizeof(global_handlers) /
                                            sizeof(global_handlers[0]);

suit_manifest_handler_t _suit_manifest_get_handler(int key,
                                                   const suit_manifest_handler_t *handlers,
                                                   size_t len)
{
    if (key < 0 || (size_t)key >= len) {
        return NULL;
    }
    return handlers[key];
}

suit_manifest_handler_t suit_manifest_get_handler(int key)
{
    return _suit_manifest_get_handler(key, global_handlers,
                                      global_handlers_len);
}
