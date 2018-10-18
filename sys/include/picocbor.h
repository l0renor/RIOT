/*
 * Copyright (C) 2018 Freie Universität Berlin
 * Copyright (C) 2018 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_picocbor minimalistic CBOR encoder
 * @ingroup     sys
 * @brief       Provides a minimal CBOR encoder
 *
 * PicoCBOR is a minimal CBOR encoder. For protocols such as CoAP, OSCORE,
 * SenML and CORECONF a well defined and thus predictable CBOR structure is
 * required. PicoCBOR tries to fill this requirement by providing a very
 * minimal CBOR encoder. Supported is:
 *  - All major types
 *  - Arrays including indefinite length arrays
 *  - Maps including indefinite length maps
 * Not included:
 *  - Floats
 *  - Date and time
 *  - Sanity checks and verifications
 *  - Decoding
 *
 * Assumptions:
 *  - The maximum data size is known
 *  - Input is already validated
 *
 * @{
 *
 * @file
 * @see         [rfc 7049](https://tools.ietf.org/html/rfc7049)
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef PICOCBOR_H
#define PICOCBOR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define PICOCBOR_TYPE_OFFSET        (5U)
#define PICOCBOR_TYPE_MASK          0xE0
#define PICOCBOR_VALUE_MASK         0x17

/**
 * @name CBOR major types
 * @{
 */
#define PICOCBOR_TYPE_UINT          (0x00 << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_TYPE_NINT          (0x01 << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_TYPE_BSTR          (0x02 << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_TYPE_TSTR          (0x03 << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_TYPE_ARR           (0x04 << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_TYPE_MAP           (0x05 << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_TYPE_TAG           (0x06 << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_TYPE_FLOAT         (0x07 << PICOCBOR_TYPE_OFFSET)
/** @} */

/**
 * @name CBOR simple data types
 * @{
 */
#define PICOCBOR_SIMPLE_FALSE       20U
#define PICOCBOR_SIMPLE_TRUE        21U
#define PICOCBOR_SIMPLE_NULL        22U
#define PICOCBOR_SIMPLE_UNDEF       23
/** @} */

/**
 * @name CBOR data sizes
 * @{
 */
#define PICOCBOR_SIZE_BYTE          24U
#define PICOCBOR_SIZE_SHORT         25U
#define PICOCBOR_SIZE_WORD          26U
#define PICOCBOR_SIZE_DOUBLE        27

/**
 * @brief Write a CBOR boolean value into a buffer
 *
 * @param[in]   buf     Buffer to write into
 * @param[in]   content Boolean value to write
 *
 * @return              Number of bytes written
 */
size_t picocbor_fmt_bool(uint8_t *buf, bool content);

/**
 * @brief Write an unsigned integer of at most sizeof uint32_t into the buffer
 *
 * @param[in]   buf     buffer to write into
 * @param[in]   num     unsigned integer to write
 *
 * @return  number of bytes written
 */
size_t picocbor_fmt_uint(uint8_t *buf, uint32_t num);

/**
 * @brief Write a signed integer of at most sizeof int32_t into the buffer
 *
 * If it is not certain if the data is signed, use this function.
 *
 * @param[in]   buf     buffer to write into
 * @param[in]   num     unsigned integer to write
 *
 * @return  number of bytes written
 */
size_t picocbor_fmt_int(uint8_t *buf, int32_t num);

size_t picocbor_fmt_bstr(uint8_t *buf, size_t len);
size_t picocbor_fmt_tstr(uint8_t *buf, size_t len);
size_t picocbor_fmt_array(uint8_t *buf, size_t len);

#endif /* PICOCBOR_H */
