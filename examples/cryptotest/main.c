/*
 * Copyright (C) 2018 FU berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     unittests
 * @{
 *
 * @file
 * @brief       crypto speed test
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "xtimer.h"
#include "thread.h"
#include "ps.h"

#include "random.h"

#ifdef MODULE_TWEETNACL
#include <tweetnacl.h>
#endif
#ifdef MODULE_HACL
#include <haclnacl.h>
#endif
#ifdef MODULE_MBEDTLS
#include <mbedtls/ecdsa.h>
#include <mbedtls/sha256.h>
#elif MODULE_TINYCRYPT
#include <tinycrypt/ecc.h>
#include <tinycrypt/ecc_dsa.h>
#include <tinycrypt/ecc_dh.h>
#include <tinycrypt/sha256.h>
#elif MODULE_C25519
#include "edsign.h"
#include "ed25519.h"
#elif MODULE_MONOCYPHER
#include "monocypher.h"
#elif MODULE_QDSA
#include "sign.h"
#elif MODULE_LIBHYDROGEN
#include "hydrogen.h"
#elif MODULE_WOLFSSL
#include <wolfssl/wolfcrypt/settings.h>
#include "wolfssl/wolfcrypt/ed25519.h"
#endif


#if defined(MODULE_TWEETNACL) || defined(MODULE_HACL)
#define SECRETKEYBYTES crypto_sign_SECRETKEYBYTES
#define PUBLICKEYBYTES crypto_sign_PUBLICKEYBYTES
#define SMLEN (sizeof(message) + crypto_sign_BYTES)
#elif defined(MODULE_TINYCRYPT)
#define SECRETKEYBYTES  32
#define PUBLICKEYBYTES  64
#elif defined(MODULE_C25519)
#define SECRETKEYBYTES  EDSIGN_SECRET_KEY_SIZE
#define PUBLICKEYBYTES  EDSIGN_PUBLIC_KEY_SIZE
#elif defined(MODULE_MONOCYPHER)
#define SECRETKEYBYTES  32
#define PUBLICKEYBYTES  32
#elif defined(MODULE_QDSA)
#define  SECRETKEYBYTES 64
#define  PUBLICKEYBYTES 32
#define SMLEN (sizeof(message) + 64)
#endif

static uint8_t message[200];
static size_t mlen = sizeof(message);

#ifndef MODULE_MBEDTLS
#ifndef MODULE_LIBHYDROGEN
#ifndef MODULE_WOLFSSL
#ifdef DO_SIGN
static unsigned char sign_sk[SECRETKEYBYTES];
static unsigned char sign_pk[PUBLICKEYBYTES];
#else
static unsigned char sign_pk[] = {
    0x37, 0xcc, 0x72, 0x62, 0x84, 0xb2, 0x68, 0xce, 0x7e, 0x3d, 0x14, 0xaf,
    0x82, 0xc9, 0x31, 0x5c, 0x59, 0xb0, 0x3f, 0x92, 0xb9, 0xf1, 0xbb, 0xd4,
    0x01, 0x8f, 0x6d, 0x25, 0xfa, 0x6f, 0xfd, 0xf6
};
#endif
#endif
#endif
#endif

#if  defined(MODULE_HACL) || defined(MODULE_TWEETNACL)
static unsigned char verify_result[SMLEN];
static unsigned long long int smlen = SMLEN;
static unsigned long long int verify_result_len;
#ifdef DO_SIGN
static unsigned char sm[SMLEN];
#else
static unsigned char sm[SMLEN] = {
    0x48, 0x55, 0xcc, 0xca, 0x51, 0xcc, 0x2e, 0x29, 0x0e, 0x6e, 0x1e, 0x34,
    0x72, 0xc6, 0xb0, 0x29, 0xb7, 0x42, 0xe7, 0x76, 0x93, 0x50, 0x1c, 0x46,
    0xa2, 0x86, 0xd5, 0x5f, 0x3d, 0x0a, 0x1b, 0x05, 0xae, 0xa9, 0x06, 0x6b,
    0x86, 0xb0, 0x2c, 0x31, 0x25, 0x19, 0xc6, 0x42, 0xf0, 0xf7, 0xb7, 0xe5,
    0xc7, 0x06, 0x48, 0x79, 0x85, 0x67, 0x43, 0xab, 0x0e, 0x56, 0x7b, 0x25,
    0xf2, 0x28, 0x65, 0x0c, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
    0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa
};
#endif /* DO_SIGN */
#endif

#ifdef MODULE_WOLFSSL
static ed25519_key key;
//static WC_RNG rng;
#ifdef DO_SIGN
static size_t sig_len;
#endif
static uint8_t signature[ED25519_SIG_SIZE];
static uint8_t skey[ED25519_KEY_SIZE] = {0x13, 0x97, 0x3b, 0x7d, 0xaa, 0x43, 0xcb, 0x9f, 0x2f, 0x91, 0xe9, 0xa7, 0xb0, 0x46, 0x72, 0x66, 0xef, 0x04, 0x8b, 0x81, 0xf2, 0xfd, 0x5a, 0x9f, 0xc9, 0x96, 0x26, 0x70, 0xb0, 0x59, 0x27, 0xf5};
static uint8_t pkey[ED25519_KEY_SIZE] = {0x32, 0xe7, 0x32, 0x31, 0x23, 0xd8, 0xee, 0x89, 0x85, 0x46, 0xa3, 0x90, 0x5b, 0x3a, 0x02, 0x4f, 0x20, 0xf1, 0xdf, 0x34, 0x6b, 0xa2, 0xac, 0x1c, 0x3f, 0xb6, 0xea, 0x9f, 0x55, 0xb8, 0x49, 0x11};
#endif

static volatile uint32_t middle, after = 0;
#ifdef DEVELHELP
static volatile int tstart, tmiddle, tafter = 0;
static volatile int stacksz;
#endif
#ifdef DO_SIGN
static volatile uint32_t before = 0;
#ifdef DEVELHELP
static volatile int tbefore = 0;
#endif
#endif

#ifdef MODULE_TINYCRYPT
static uint8_t signature[128];
static uint8_t digest[32];
static struct tc_sha256_state_struct sha;
#endif

#ifdef MODULE_LIBHYDROGEN
static hydro_sign_keypair keypair;
static uint8_t signature[hydro_sign_BYTES];
#define CONTEXT "RIOT"
#endif

#ifdef MODULE_MBEDTLS
#ifdef DO_SIGN
static uint8_t signature[MBEDTLS_ECDSA_MAX_LEN];
#else
static uint8_t signature[] = {0x30, 0x46, 0x02, 0x21, 0x00, 0x99, 0x1d, 0x15, 0x6b, 0xf9, 0xda, 0x23, 0x2e, 0xe6, 0x28, 0x16, 0xa7, 0x21, 0x13, 0x7e, 0xce, 0xf3, 0x12, 0x97, 0x8e, 0x88, 0x1e, 0xdd, 0x41, 0xda, 0xa2, 0x68, 0x36, 0xd2, 0xe5, 0x5c, 0xfd, 0x02, 0x21, 0x00, 0xae, 0x05, 0xcc, 0x0e, 0x2b, 0x79, 0x8f, 0x42, 0xf8, 0x86, 0x08, 0x43, 0x57, 0x4c, 0xb0, 0xdb, 0x1a, 0xb2, 0xf1, 0x22, 0x18, 0x22, 0xeb, 0x74, 0x44, 0x64, 0xce, 0x9a, 0xd2, 0xe1, 0xc3, 0x5e };
#endif
mbedtls_ecdsa_context ctx_sign, ctx_verify;
mbedtls_sha256_context sha256_ctx;
static uint8_t digest[32];
static size_t sig_len = sizeof(signature);

static int getrandom(void *ctx, unsigned char* buf, size_t len)
{
    (void)ctx;
    random_bytes((uint8_t*)buf, len);
    return 0;
}
void mbedtls_platform_zeroize(void *buf, size_t len)
{
        volatile unsigned char *p = (unsigned char*)buf; while( len-- ) *p++ = 0;
}


#endif

#ifdef MODULE_RELIC
static bn_t sign_sk;
static ec_t sign_pk;

static bn_t signature1;
static bn_t signature2;
#endif

#if defined(MODULE_C25519) || defined(MODULE_MONOCYPHER)
#ifdef DO_SIGN
static uint8_t signature[64];
#else
static uint8_t signature[64] = {
    0x48, 0x55, 0xcc, 0xca, 0x51, 0xcc, 0x2e, 0x29, 0x0e, 0x6e, 0x1e, 0x34,
    0x72, 0xc6, 0xb0, 0x29, 0xb7, 0x42, 0xe7, 0x76, 0x93, 0x50, 0x1c, 0x46,
    0xa2, 0x86, 0xd5, 0x5f, 0x3d, 0x0a, 0x1b, 0x05, 0xae, 0xa9, 0x06, 0x6b,
    0x86, 0xb0, 0x2c, 0x31, 0x25, 0x19, 0xc6, 0x42, 0xf0, 0xf7, 0xb7, 0xe5,
    0xc7, 0x06, 0x48, 0x79, 0x85, 0x67, 0x43, 0xab, 0x0e, 0x56, 0x7b, 0x25,
    0xf2, 0x28, 0x65, 0x0c
};
#endif
#endif

#ifdef MODULE_QDSA
static unsigned char verify_result[SMLEN];
static unsigned char sm[SMLEN];
long long unsigned int smlen = 0;
#endif

#ifndef MODULE_MBEDTLS
#ifndef MODULE_LIBHYDROGEN
#ifndef MODULE_WOLFSSL
#ifdef DO_SIGN
static void gen_keypair(uint8_t *pk, uint8_t *sk)
{
#if  defined(MODULE_HACL) || defined(MODULE_TWEETNACL)
    crypto_sign_keypair(pk, sk);
#elif defined(MODULE_TINYCRYPT)
    uECC_make_key(pk, sk, &curve_secp256r1);
#elif defined(MODULE_C25519)
    random_bytes(sk, sizeof(sk));
    ed25519_prepare(sk);
    edsign_sec_to_pub(pk, sk);
#elif defined(MODULE_MONOCYPHER)
    random_bytes(sk, sizeof(sk));
    crypto_sign_public_key(pk, sk);
#elif defined(MODULE_QDSA)
    random_bytes(sk, 32);
    keypair(pk, sk);
#endif
}
#endif
#endif
#endif
#endif

#ifdef MODULE_TINYCRYPT
int default_CSPRNG(uint8_t *buf, size_t len)
{
    random_bytes(buf, len);
    return 1;
}
#endif

void print_bstr(uint8_t *d, size_t l)
{
    for (size_t i = 0; i<l; i++) {
        printf("0x%.2x, ", d[i]);
    }
}

int main(void)
{
#ifdef DEVELHELP
    thread_t *p = (thread_t *)sched_threads[sched_active_pid];
    stacksz = p->stack_size;
    tstart = stacksz - thread_measure_stack_free(p->stack_start);
#endif

#ifdef MODULE_WOLFSSL
    wc_ed25519_init(&key);
    wc_ed25519_import_private_key(skey, 32, pkey, 32, &key);
//    wc_InitRng(&rng);
//    int rres = wc_ed25519_make_key(&rng, ED25519_KEY_SIZE, &key);
//    if (rres != 0) {
//        printf("Failed generating keys: %d\n", rres);
//    }
//    else {
//        unsigned int outlen = 32;
//        wc_ed25519_export_private_only(&key, skey, &outlen);
//        wc_ed25519_export_public(&key, pkey, &outlen);
//        printf("skey: ");
//        print_bstr(skey, 32);
//        printf("\n");
//        printf("pkey: ");
//        print_bstr(pkey, 32);
//        printf("\n");
//    }
#endif

    /* MBEDTLS initialization */
#ifdef MODULE_MBEDTLS
    mbedtls_ecdsa_init(&ctx_sign);
    mbedtls_ecdsa_init(&ctx_verify);
    mbedtls_sha256_init(&sha256_ctx);
    if (mbedtls_ecdsa_genkey(&ctx_sign, MBEDTLS_ECP_DP_SECP256R1, getrandom, NULL) != 0) {
        printf("keygen failed\n");
    }
#endif

    /* Libhydrogen keygen */
#ifdef MODULE_LIBHYDROGEN
    hydro_sign_keygen(&keypair);
#endif

    memset((uint8_t*)message, 0xaa, mlen);
#ifdef DO_SIGN
#if  defined(MODULE_HACL) || defined(MODULE_TWEETNACL)
    memset(sm, '\0', SMLEN);
#endif
    /* Generic keypair creation... */
#ifndef MODULE_WOLFSSL
#ifndef MODULE_MBEDTLS
#ifndef MODULE_LIBHYDROGEN
    gen_keypair(sign_pk, sign_sk);
#endif
#endif
#endif
    /* Measure stack */
#ifdef DEVELHELP
    tbefore = stacksz - thread_measure_stack_free(p->stack_start);
#endif
    /* Measure time before */
    before = xtimer_now_usec();

    /* Generic Sign */
#if  defined(MODULE_HACL) || defined(MODULE_TWEETNACL)
    crypto_sign(sm, &smlen, (const uint8_t *)message, mlen, sign_sk);
    /* Tinycrypt hash and sign */
#elif defined(MODULE_TINYCRYPT)
    tc_sha256_init(&sha);
    tc_sha256_update(&sha, message, mlen);
    tc_sha256_final(digest, &sha);
    uECC_sign(sign_sk, digest, 32, signature, &curve_secp256r1);
    /* MBEDTLS hash and sign */
#elif defined(MODULE_MBEDTLS)
    mbedtls_sha256_starts( &sha256_ctx, 0 );
    mbedtls_sha256_update( &sha256_ctx, message, mlen );
    mbedtls_sha256_finish( &sha256_ctx, digest);
    sig_len = MBEDTLS_ECDSA_MAX_LEN;
    int sres = mbedtls_ecdsa_write_signature(&ctx_sign, MBEDTLS_MD_SHA256, digest, sizeof(digest), signature, &sig_len, NULL, NULL);
    if (sres != 0) {
        printf("Signing failed %d\n", sres);
    }
    else {
        print_bstr(signature, sig_len);
        printf("\n");
    }

    /* C25519 sign */
#elif defined(MODULE_C25519)
	edsign_sign(signature, sign_pk, sign_sk, message, mlen);
    /* C25519 sign */
#elif defined(MODULE_MONOCYPHER)
    crypto_sign(signature, sign_sk, sign_pk, message, mlen);
    /* QDSA sign */
#elif defined(MODULE_QDSA)
    sign(sm, &smlen, message, mlen, sign_pk, sign_sk);
    /* libhydrogen sign */
#elif defined(MODULE_LIBHYDROGEN)
    hydro_sign_create(signature, message, mlen, CONTEXT, keypair.sk);
    /* WolfSSL signing */
#elif defined(MODULE_WOLFSSL)
    sig_len = sizeof(signature);
    int sres = wc_ed25519_sign_msg((uint8_t*)message, mlen, signature, &sig_len, &key);
    if (sres != 0) {
        printf("sign fail %d\n", sres);
    }
#endif
#endif /* DO_SIGN */

    /* Measure time after signing */
    middle = xtimer_now_usec();
#ifdef DEVELHELP
    /* Measure stack after signing */
    tmiddle = stacksz - thread_measure_stack_free(p->stack_start);
#endif
    /* Verifying... */
#if  defined(MODULE_HACL) || defined(MODULE_TWEETNACL)
    int res = crypto_sign_open(verify_result, &verify_result_len, sm, smlen, sign_pk);
#elif defined(MODULE_TINYCRYPT)
    /* Tinycrypt hash and verify */
    tc_sha256_init(&sha);
    tc_sha256_update(&sha, message, sizeof(message));
    tc_sha256_final(digest, &sha);
    int res = uECC_verify(sign_pk, digest, 32, signature, &curve_secp256r1);
#elif defined(MODULE_MBEDTLS)
    /* MBEDTLS hash and verify */
    mbedtls_sha256_starts( &sha256_ctx, 0 );
    mbedtls_sha256_update( &sha256_ctx, message, sizeof(message));
    mbedtls_sha256_finish( &sha256_ctx, digest);
    int res = mbedtls_ecdsa_read_signature( &ctx_sign, digest, sizeof(digest), signature, sig_len);
#elif defined(MODULE_C25519)
    /* C25519 verify */
    int res = edsign_verify(signature, sign_pk, message, sizeof(message));
#elif defined(MODULE_MONOCYPHER)
    /* Monocypher verify */
    int res = crypto_check(signature, sign_pk, message, sizeof(message));
#elif defined(MODULE_QDSA)
    /* QDSA verify */
    int res = verify(verify_result, 0, sm, smlen, sign_pk);
#elif defined(MODULE_LIBHYDROGEN)
    /* libhydrogen verify */
    int res = verify(verify_result, 0, sm, smlen, sign_pk);
    int res = hydro_sign_verify(signature, message, sizeof(message), CONTEXT, keypair.pk);
#elif defined(MODULE_WOLFSSL)
    int res;

    if (wc_ed25519_verify_msg(signature, ED25519_SIG_SIZE, (uint8_t*)message, mlen, &res, &key) < 0) {
        printf("Signature verify failed\n");
    }
#endif
    /* Measure time */
    after = xtimer_now_usec();
#ifdef DEVELHELP
    /* Measure stack */
    tafter = stacksz - thread_measure_stack_free(p->stack_start);
#endif

    /* Print results */
    printf("Res: %d, "
#ifdef DO_SIGN
            "before: %" PRIu32
#endif
            "middle: %" PRIu32", after: %" PRIu32"\n", res,
#ifdef DO_SIGN
            before,
#endif
            middle, after);

#ifdef DEVELHELP
    printf("Stack start: %i, "
#ifdef DO_SIGN
           "before: %i, "
#endif
           "middle: %i, after: %i\n", tstart,
#ifdef DO_SIGN
           tbefore,
#endif
           tmiddle, tafter);
#endif
    printf("\n");
    printf("Timing:"
#ifdef DO_SIGN
        "Result:\t%luus\t"
#endif
        "%luus\n",
#ifdef DO_SIGN
        (long unsigned)((middle - before)),
#endif
        (long unsigned)((after - middle)));
#ifdef DEVELHELP
    printf("stack :"
#ifdef DO_SIGN
           "Result:\t%iB\t"
#endif
           "%iB\n",
#ifdef DO_SIGN
           (tmiddle - tstart),
#endif
           (tafter - tstart));
#endif

    while(1) {}
    return 0;
}
