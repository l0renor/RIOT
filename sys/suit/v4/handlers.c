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

static int _handle_command_sequence(suit_v4_manifest_t *manifest, CborValue *it,
        suit_manifest_handler_t handler);

int suit_cbor_get_string(const CborValue *it, const uint8_t **buf, size_t *len)
{
    if (!(cbor_value_is_text_string(it) || cbor_value_is_byte_string(it) || cbor_value_is_length_known(it))) {
        return -1;
    }
    CborValue next = *it;
    cbor_value_get_string_length(it, len);
    cbor_value_advance(&next);
    *buf = next.ptr - *len;
    return 0;
}

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

static int _common_sequence_handler(suit_v4_manifest_t *manifest, int key,
                                    CborValue *it)
{
    (void)manifest;
    printf("Received key %d\n", key);
    switch(key) {
        case SUIT_COND_VENDOR_ID:
        case SUIT_COND_CLASS_ID:
        case SUIT_COND_DEV_ID:
            {
                char uuid_str[UUID_STR_LEN + 1];
                uuid_t uuid;
                size_t len = sizeof(uuid);
                cbor_value_copy_byte_string(it, (uint8_t*)&uuid, &len, NULL);
                uuid_to_string(&uuid, uuid_str);
                printf("Attempting to validate uuid: %s\n", uuid_str);
            }
            break;
        default:
            printf("Unknown section in common\n");
    }
    return 0;
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

    (void)key;
    return _handle_command_sequence(manifest, it, _common_sequence_handler);
}

static int _component_handler(suit_v4_manifest_t *manifest, int key,
                              CborValue *it)
{
    (void)manifest;
    (void)key;

    CborValue arr;

    puts("storing components");
    if (!cbor_value_is_array(it)) {
        printf("components field not an array\n");
        return -1;
    }
    cbor_value_enter_container(it, &arr);

    unsigned n = 0;
    while (!cbor_value_at_end(&arr)) {
        CborValue map, key, value;

        cbor_map_iterate_init(&map, &arr);

        while (cbor_map_iterate(&map, &key, &value)) {
            // handle key, value
            printf("component %u (stub)\n", n);
        }

        cbor_value_advance(&arr);
    }

    manifest->state |= SIOT_MANIFEST_HAVE_COMPONENTS;
    cbor_value_enter_container(&arr, it);

    puts("storing components done");
    return 0;
}

/* begin{code-style-ignore} */
static suit_manifest_handler_t global_handlers[] = {
    [ 0] = _hello_handler,
    [ 1] = _version_handler,
    [ 2] = _seq_no_handler,
    [ 3] = _dependencies_handler,
    [ 4] = _component_handler,
    [ 5] = NULL,
    [ 6] = _common_handler,
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

static suit_manifest_handler_t _suit_manifest_get_handler(int key,
                                                   const suit_manifest_handler_t *handlers,
                                                   size_t len)
{
    if (key < 0 || (size_t)key >= len) {
        return NULL;
    }
    return handlers[key];
}

suit_manifest_handler_t suit_manifest_get_manifest_handler(int key)
{
    return _suit_manifest_get_handler(key, global_handlers,
                                      global_handlers_len);
}

int _handle_command_sequence(suit_v4_manifest_t *manifest, CborValue *bseq,
        suit_manifest_handler_t handler)
{

    if (!cbor_value_is_byte_string(bseq)) {
        printf("Not an byte array\n");
        return -1;
    }

    const uint8_t *sequence;
    size_t seq_len = 0;
    CborParser parser;
    CborValue it, arr;

    if (!cbor_value_is_byte_string(bseq)) {
        printf("Not an byte array\n");
        return -1;
    }
    suit_cbor_get_string(bseq, &sequence, &seq_len);
    CborError err = cbor_parser_init(sequence, seq_len, SUIT_TINYCBOR_VALIDATION_MODE,
                                     &parser, &it);
    if (err < 0) {
        return err;
    }
    if (!cbor_value_is_array(&it)) {
        printf("Not an byte array\n");
        return -1;
    }
    cbor_value_enter_container(&it, &arr);

    while (!cbor_value_at_end(&arr)) {
        CborValue map;
        if (!cbor_value_is_map(&arr)) {
            return SUIT_ERR_INVALID_MANIFEST;
        }
        cbor_value_enter_container(&arr, &map);
        int integer_key;
        if (cbor_value_get_int_checked(&map, &integer_key) == CborErrorDataTooLarge) {
            printf("integer key doesn't fit into int type\n");
            return SUIT_ERR_INVALID_MANIFEST;
        }
        cbor_value_advance(&map);
        int res = handler(manifest, integer_key, &map);
        if (res < 0) {
            return res;
        }
        cbor_value_leave_container(&arr, &map);
    }
    return 0;
}
