/*
 * Copyright (C) 2019 Koen Zandberg
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
 * @brief       Tests cifra package
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 *
 * @}
 */

#include <string.h>

#include "embUnit.h"
#include "random.h"
#include "aes.h"
#include "curve25519.h"
#include "modes.h"

#define SECRET_KEY_LEN  32

/* AES-GCM test vector */
static uint8_t message[] = "0123456789abcdef";
static uint8_t nonce[] = "0123456789abcdef";

static uint8_t cipher[sizeof(message)];
static uint8_t tag[16];

static uint8_t decrypted[sizeof(message)];

static uint8_t key[SECRET_KEY_LEN];

/** Curve25519 test vector from cifra */
static uint8_t scalar[] = {0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d, 0x3c,
                           0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45, 0xdf, 0x4c,
                           0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a, 0xb1, 0x77, 0xfb,
                           0xa5, 0x1d, 0xb9, 0x2c, 0x2a};

static uint8_t public[] = {0xde, 0x9e, 0xdb, 0x7d, 0x7b, 0x7d, 0xc1, 0xb4, 0xd3,
                           0x5b, 0x61, 0xc2, 0xec, 0xe4, 0x35, 0x37, 0x3f, 0x83,
                           0x43, 0xc8, 0x5b, 0x78, 0x67, 0x4d, 0xad, 0xfc, 0x7e,
                           0x14, 0x6f, 0x88, 0x2b, 0x4f};

static uint8_t expect[] = {0x4a, 0x5d, 0x9d, 0x5b, 0xa4, 0xce, 0x2d, 0xe1, 0x72,
                           0x8e, 0x3b, 0xf4, 0x80, 0x35, 0x0f, 0x25, 0xe0, 0x7e,
                           0x21, 0xc9, 0x47, 0xd1, 0x9e, 0x33, 0x76, 0xf0, 0x9b,
                           0x3c, 0x1e, 0x16, 0x17, 0x42};

static uint8_t shared[sizeof(expect)];

static void setUp(void)
{
    /* Initialize */
    random_init(0);

    random_bytes(key, SECRET_KEY_LEN);
}

static void test_cifra_gcm(void)
{
    cf_aes_context aes;
    cf_aes_init(&aes, key, SECRET_KEY_LEN);

    cf_gcm_encrypt(&cf_aes, &aes,
                   message, sizeof(message),
                   NULL, 0,
                   nonce, sizeof(nonce),
                   cipher,
                   tag, 16);

    int res = cf_gcm_decrypt(&cf_aes, &aes,
                             cipher, sizeof(message),
                             NULL, 0,
                             nonce, sizeof(nonce),
                             tag, 16,
                             decrypted);

    TEST_ASSERT(res == 0);
    TEST_ASSERT_EQUAL_STRING((char *)message, (char *)decrypted);

    /* Flip bit in the ciphertext */
    cipher[0] ^= 0x01;

    res = cf_gcm_decrypt(&cf_aes, &aes,
                         cipher, sizeof(message),
                         NULL, 0,
                         nonce, sizeof(nonce),
                         tag, 16,
                         decrypted);

    TEST_ASSERT(res == 1);

    /* Flip bit in the ciphertext back */
    cipher[0] ^= 0x01;

    /* Flip bit in the key */
    key[0] ^= 0x01;

    /* Reimport the key */
    cf_aes_init(&aes, key, SECRET_KEY_LEN);

    res = cf_gcm_decrypt(&cf_aes, &aes,
                         cipher, sizeof(message),
                         NULL, 0,
                         nonce, sizeof(nonce),
                         tag, 16,
                         decrypted);

    TEST_ASSERT(res == 1);
}

static void test_cifra_c25519(void)
{
    cf_curve25519_mul(shared, scalar, public);
    TEST_ASSERT(memcmp(expect, shared, 32) == 0);
}

Test *tests_cifra(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_cifra_gcm),
        new_TestFixture(test_cifra_c25519),
    };

    EMB_UNIT_TESTCALLER(cifra_tests, setUp, NULL, fixtures);
    return (Test*)&cifra_tests;
}

int main(void)
{
    TESTS_START();
    TESTS_RUN(tests_cifra());
    TESTS_END();

    return 0;
}
