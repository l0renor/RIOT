/*
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  unittests
 * @{
 *
 * @file
 * @brief       Unittests for the libcose package
 *
 */
#ifndef TESTS_LIBCOSE_H
#define TESTS_LIBCOSE_H

#include "embUnit/embUnit.h"
#include "random.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MANDATORY function for collecting random Bytes
 *        required by the tweetnacl package
 */
extern void randombytes(uint8_t *target, uint64_t n);

/**
*  @brief   The entry point of this test suite.
*/
void tests_libcose(void);

/**
 * @brief   Generates tests for libcose
 *
 * @return  embUnit tests if successful, NULL if not.
 */
Test *tests_libcose_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_LIBCOSE_H */
/** @} */

