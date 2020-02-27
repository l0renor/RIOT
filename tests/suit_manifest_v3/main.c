/*
 * Copyright (C) 2019 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup    tests
 * @{
 *
 * @file
 * @brief      Tests for module suit_v3
 *
 * @author     Kaspar Schleiser <kaspar@schleiser.de>
 */

#include <stdio.h>

#include "kernel_defines.h"
#include "log.h"

#include "suit/v3/suit.h"
#include "embUnit.h"

#include "blob/manifests/manifest0.bin.h"
#include "blob/manifests/manifest1.bin.h"
#include "blob/manifests/manifest2.bin.h"
#include "blob/manifests/manifest3.bin.h"

#define SUIT_URL_MAX            128

typedef struct {
    const unsigned char *data;
    size_t len;
    int expected;
} manifest_blob_t;

const manifest_blob_t manifest_blobs[] = {
    /* Older GCC can't handle manifestx_bin_len here */
    { manifest0_bin, sizeof(manifest0_bin), SUIT_ERR_SIGNATURE },
    { manifest1_bin, sizeof(manifest1_bin), SUIT_ERR_SEQUENCE_NUMBER },
    { manifest2_bin, sizeof(manifest2_bin), SUIT_ERR_COND },
    { manifest3_bin, sizeof(manifest3_bin), SUIT_OK },
};

const unsigned manifest_blobs_numof = ARRAY_SIZE(manifest_blobs);

static int test_suitv3_manifest(const unsigned char *manifest_bin,
                                size_t manifest_bin_len)
{
    char _url[SUIT_URL_MAX];
    suit_v3_manifest_t manifest;
    riotboot_flashwrite_t writer;

    memset(&writer, 0, sizeof(manifest));

    manifest.writer = &writer;
    manifest.urlbuf = _url;
    manifest.urlbuf_len = SUIT_URL_MAX;

    int res;
    if ((res =
             suit_v3_parse(&manifest, manifest_bin,
                           manifest_bin_len)) != SUIT_OK) {
        return res;
    }

    return res;
}

static void test_suitv3_manifest_01_manifests(void)
{
    for (unsigned i = 0; i < manifest_blobs_numof; i++) {
        printf("\n--- testing manifest %u\n", i);
        int res = \
            test_suitv3_manifest(manifest_blobs[i].data, manifest_blobs[i].len);
        printf("---- res=%i (expected=%i)\n", res, manifest_blobs[i].expected);
        TEST_ASSERT_EQUAL_INT(manifest_blobs[i].expected, res);
    }
}

Test *tests_suitv3_manifest(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_suitv3_manifest_01_manifests),
    };

    EMB_UNIT_TESTCALLER(suitv3_manifest_tests, NULL, NULL, fixtures);

    return (Test *)&suitv3_manifest_tests;
}

int main(void)
{
    TESTS_START();
    TESTS_RUN(tests_suitv3_manifest());
    TESTS_END();
    return 0;
}