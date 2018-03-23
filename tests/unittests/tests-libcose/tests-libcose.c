/*
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
  * @brief      Unit tests for pkg libcose
  *
  * @author     Koen Zandberg <koen@bergzand.net>
  */

#define MAX_NUMBER_BLOCKS 128

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tweetnacl.h>
#include "cn-cbor/cn-cbor.h"
#include "cose.h"
#include "cose/crypto.h"
#include "memarray.h"
#include "embUnit.h"

#include "tests-libcose.h"

static char kid[] = "koen@riot-os.org";
static char kid2[] = "paco@riot-os.org";
static unsigned char pk[crypto_sign_PUBLICKEYBYTES];
static unsigned char sk[crypto_sign_SECRETKEYBYTES];
static unsigned char pk2[crypto_sign_PUBLICKEYBYTES];
static unsigned char sk2[crypto_sign_SECRETKEYBYTES];

#define MLEN (sizeof(payload))
#define SMLEN (sizeof(payload)+ crypto_sign_BYTES)
#define NUM_TESTS (sizeof(tests)/sizeof(struct test))
static uint8_t buf[2048];


static cn_cbor block_storage_data[MAX_NUMBER_BLOCKS];
static memarray_t storage;

static void setUp(void)
{
    /* Initialize */
    random_init(0);
}

static void tearDown(void)
{
    /* Finalize */
}

static void print_bytestr(uint8_t *bytes, size_t len)
{
    for(unsigned int idx=0; idx < len; idx++)
    {
        printf("%02X", bytes[idx]);
    }
}


/* CN_CBOR calloc functions */
static void *cose_calloc(size_t count, size_t size, void *memblock)
{
    (void)count;
    (void)size;
    void *block = memarray_alloc(memblock);
    if (block) {
        memset(block, 0, sizeof(cn_cbor));
        printf("MALL address: %p\n", block);
    }
    return block;

}

static void cose_free(void *ptr, void *memblock)
{
    /* TODO: BUG? */
    printf("FREE address: %p\n", ptr);
    memarray_free(memblock, ptr);
}

static cn_cbor_context ct =
{
    .calloc_func = cose_calloc,
    .free_func = cose_free,
    .context = &storage,
};

static void test_libcose_01(void)
{
    printf("Running test 01\n");
    char sign1_payload[] = "Input string";
    memset(buf, 0, sizeof(buf));
    cose_sign_t sign, verify;
    cose_signer_t signer;
    cn_cbor_errback errp;
    /* Initialize struct */
    printf("Start init\n");
    cose_sign_init(&sign, COSE_FLAGS_UNTAGGED);
    cose_sign_init(&verify, 0);

    printf("Initialized, adding payload\n");
    /* Add payload */
    cose_sign_set_payload(&sign, sign1_payload, strlen(sign1_payload));

    /* First signer */
    printf("Payload added, generating keypair\n");
    crypto_sign_keypair(pk, sk);
    //cose_crypto_keypair_ed25519(pk, sk);
    printf("Keypair ready, building signer\n");
    cose_signer_init(&signer);
    cose_signer_set_keys(&signer, COSE_EC_CURVE_ED25519, pk, NULL, sk);
    cose_signer_set_kid(&signer, (uint8_t*)kid, sizeof(kid) - 1);

    cose_sign_add_signer(&sign, &signer);

    /* Encode COSE sign object */
    ssize_t encode_size = cose_sign_encode(&sign, buf, sizeof(buf), &ct, &errp);
    printf("Encoded size for sign1: %u\n", encode_size);
    print_bytestr(buf+64, encode_size);
    printf("\n");
    printf("Signature: ");
    print_bytestr(buf, 64);
    printf("\n");
    TEST_ASSERT(encode_size > 0);
    /* Decode again */
    int decode_success = cose_sign_decode(&verify, buf + 64, encode_size, &ct, &errp);
    /* Verify with signature slot 0 */
    TEST_ASSERT_EQUAL_INT(decode_success, 0);
    int verification = cose_sign_verify(&verify, &signer, 0, &ct);
    printf("Verification: %d\n", verification);
    TEST_ASSERT_EQUAL_INT(verification, 0);
    /* Modify payload */
    ((int*)(verify.payload))[0]++;
    verification = cose_sign_verify(&verify, &signer, 0, &ct);
    /* Should fail due to modified payload */
    TEST_ASSERT(verification != 0);
}

static void test_libcose_02(void)
{
    char payload[] = "Input string";
    cose_sign_t sign, verify;
    cose_signer_t signer, signer2;
    cn_cbor_errback errp;
    /* Initialize struct */
    cose_sign_init(&sign, 0);
    cose_sign_init(&verify, 0);

    /* Add payload */
    cose_sign_set_payload(&sign, payload, sizeof(payload));

    /* First signer */
    cose_crypto_keypair_ed25519(pk, sk);
    cose_signer_init(&signer);
    cose_signer_set_keys(&signer, COSE_EC_CURVE_ED25519, pk, NULL, sk);
    cose_signer_set_kid(&signer, (uint8_t*)kid, sizeof(kid) - 1);

    /* Second signer */
    cose_crypto_keypair_ed25519(pk2, sk2);
    cose_signer_init(&signer2);
    cose_signer_set_keys(&signer2, COSE_EC_CURVE_ED25519, pk2, NULL, sk2);
    cose_signer_set_kid(&signer2, (uint8_t*)kid2, sizeof(kid2) - 1);
    cose_sign_add_signer(&sign, &signer);
    cose_sign_add_signer(&sign, &signer2);

    TEST_ASSERT(cose_signer_serialize_protected(&signer, NULL, 0, &ct, &errp) > 0);

    size_t len = cose_sign_encode(&sign, buf, sizeof(buf), &ct, &errp);


    TEST_ASSERT_EQUAL_INT(cose_sign_decode(&verify, buf + 128, len, &ct, &errp), 0);
    /* Test correct signature with correct signer */
    TEST_ASSERT_EQUAL_INT(cose_sign_verify(&verify, &signer, 0, &ct), 0);
    TEST_ASSERT(cose_sign_verify(&verify, &signer, 1, &ct) != 0);
    TEST_ASSERT(cose_sign_verify(&verify, &signer2, 0, &ct) != 0);
    TEST_ASSERT_EQUAL_INT(cose_sign_verify(&verify, &signer2, 1, &ct), 0);
    /* Modify payload */
    ((int*)verify.payload)[0]++;
    TEST_ASSERT(cose_sign_verify(&verify, &signer, 0, &ct) != 0);
    TEST_ASSERT(cose_sign_verify(&verify, &signer, 1, &ct) != 0);
    TEST_ASSERT(cose_sign_verify(&verify, &signer2, 0, &ct) != 0);
    TEST_ASSERT(cose_sign_verify(&verify, &signer2, 1, &ct) != 0);
}

Test *tests_libcose_all(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_libcose_01),
        new_TestFixture(test_libcose_02),
    };

    EMB_UNIT_TESTCALLER(libcose_tests, setUp, tearDown, fixtures);
    return (Test*)&libcose_tests;
}

void tests_libcose(void)
{
    printf("Running tests\n");
    memarray_init(&storage, block_storage_data, sizeof(cn_cbor), MAX_NUMBER_BLOCKS);
    TESTS_RUN(tests_libcose_all());
}
