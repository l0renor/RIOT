/*
 * Copyright (C) 2018 Koen Zandberg
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
 * @brief       Test the picocbor library
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 *
 * @}
 */

#include <float.h>
#include <arm_fp16.h>
#include "embUnit.h"
#include "picocbor.h"
/* tinycbor include */
#include "cbor.h"

static uint8_t buf[1024];

static void test_true(void)
{
    CborParser parser;
    CborValue it;
    size_t len = picocbor_fmt_bool(buf, true);
    CborError err = cbor_parser_init(buf, len, CborValidateStrictMode,
                                     &parser, &it);
    TEST_ASSERT_EQUAL_INT(err, 0);
    if (cbor_value_is_boolean(&it)) {
        bool result;
        err = cbor_value_get_boolean(&it, &result);
        TEST_ASSERT(result);
    }
    else {
        TEST_ASSERT(false);
    }
}

static void test_false(void)
{
    CborParser parser;
    CborValue it;
    size_t len = picocbor_fmt_bool(buf, false);
    CborError err = cbor_parser_init(buf, len, CborValidateStrictMode,
                                     &parser, &it);
    TEST_ASSERT_EQUAL_INT(err, 0);
    if (cbor_value_is_boolean(&it)) {
        bool result;
        err = cbor_value_get_boolean(&it, &result);
        TEST_ASSERT(!(result));
    }
    else {
        TEST_ASSERT(false);
    }
}

static void test_uint(void)
{
    for (uint32_t i = 0; i < 2 * UINT16_MAX; i++) {
        CborParser parser;
        CborValue it;

        size_t len = picocbor_fmt_uint(buf, i);

        CborError err = cbor_parser_init(buf, len, CborValidateStrictMode,
                                         &parser, &it);
        TEST_ASSERT_EQUAL_INT(err, 0);
        if (cbor_value_is_unsigned_integer(&it)) {
            uint64_t integer;
            err = cbor_value_get_uint64(&it, &integer);
            TEST_ASSERT_EQUAL_INT(integer, i);
        }
        else {
            TEST_ASSERT(false);
        }
    }
}

static void test_float(void)
{
    CborParser parser;
    CborValue it;
    
    size_t len = picocbor_fmt_float(buf, 1234e9);
    
    CborError err = cbor_parser_init(buf, len, CborValidateStrictMode,
                                     &parser, &it);
    TEST_ASSERT_EQUAL_INT(err, 0);
    if (cbor_value_is_float(&it)) {
        float half;
        err = cbor_value_get_float(&it, &half);
        printf("Answer: %f\n", half);
        TEST_ASSERT(half == 1234000019456.0f);
    }
    else {
        TEST_ASSERT(false);
    }
}


static void test_int(void)
{
    for (int32_t i = -2 * UINT16_MAX; i < 2 * UINT16_MAX; i++) {
        CborParser parser;
        CborValue it;

        size_t len = picocbor_fmt_int(buf, i);

        CborError err = cbor_parser_init(buf, len, CborValidateStrictMode,
                                         &parser, &it);
        TEST_ASSERT_EQUAL_INT(err, 0);
        if (cbor_value_is_integer(&it)) {
            int64_t integer;
            err = cbor_value_get_int64(&it, &integer);
            TEST_ASSERT_EQUAL_INT(integer, i);
        }
        else {
            TEST_ASSERT(false);
        }
    }
}

static void test_bstr(void)
{
    for (uint32_t i = 0; i < sizeof(buf) - 4; i++) {
        CborParser parser;
        CborValue it;

        size_t len = picocbor_fmt_bstr(buf, i);

        CborError err = cbor_parser_init(buf, len, CborValidateStrictMode,
                                         &parser, &it);
        TEST_ASSERT_EQUAL_INT(err, 0);
    }
}

static void test_tstr(void)
{
    for (unsigned i = 0; i < sizeof(buf) - 4; i++) {
        CborParser parser;
        CborValue it;

        size_t len = picocbor_fmt_tstr(buf, i);

        CborError err = cbor_parser_init(buf, len, CborValidateStrictMode,
                                         &parser, &it);
        TEST_ASSERT_EQUAL_INT(err, 0);
    }
}

static void test_array(void)
{
    for (unsigned i = 0; i < UINT8_MAX+10; i++) {
        CborParser parser;
        CborValue it, arr;
        uint8_t *pbuf = buf;

        /* create array of i len */
        pbuf += picocbor_fmt_array(pbuf, i);
        for (unsigned j = 0; j < i; j++) {
            pbuf += picocbor_fmt_uint(pbuf, i);
        }

        CborError err = cbor_parser_init(buf, pbuf - buf,
                                         CborValidateStrictMode,
                                         &parser, &it);
        TEST_ASSERT_EQUAL_INT(err, 0);
        TEST_ASSERT_EQUAL_INT(cbor_value_is_array(&it), 1);
        size_t len = 0;
        cbor_value_get_array_length(&it, &len);
        TEST_ASSERT_EQUAL_INT(i, len);
        cbor_value_enter_container(&it, &arr);
        for (unsigned j = 0; j < i; j++) {
            if (cbor_value_is_integer(&arr)) {
                int64_t integer;
                err = cbor_value_get_int64(&arr, &integer);
                TEST_ASSERT_EQUAL_INT(integer, i);
            }
            else {
                TEST_ASSERT(false);
            }
            cbor_value_advance_fixed(&arr);
        }
        TEST_ASSERT(cbor_value_at_end(&arr));
        cbor_value_leave_container(&it, &arr);
        TEST_ASSERT(cbor_value_at_end(&it));
    }
}

Test *tests_picocbor(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_true),
        new_TestFixture(test_false),
        new_TestFixture(test_uint),
        new_TestFixture(test_int),
        new_TestFixture(test_float),
        new_TestFixture(test_bstr),
        new_TestFixture(test_tstr),
        new_TestFixture(test_array),
    };
    EMB_UNIT_TESTCALLER(picocbor_tests, NULL, NULL, fixtures);
    return (Test *)&picocbor_tests;
}

int main(void)
{
    TESTS_START();
    TESTS_RUN(tests_picocbor());
    TESTS_END();
    return 0;
}
