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
 *  - 64 bits datatype support
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


#define PICOCBOR_TYPE_OFFSET    (5U)
#define PICOCBOR_TYPE_MASK      0xE0
#define PICOCBOR_VALUE_MASK     0x1F

#define PICOCBOR_TYPE_UINT      (0x00)
#define PICOCBOR_TYPE_NINT      (0x01)
#define PICOCBOR_TYPE_BSTR      (0x02)
#define PICOCBOR_TYPE_TSTR      (0x03)
#define PICOCBOR_TYPE_ARR       (0x04)
#define PICOCBOR_TYPE_MAP       (0x05)
#define PICOCBOR_TYPE_TAG       (0x06)
#define PICOCBOR_TYPE_FLOAT     (0x07)

/**
 * @name CBOR major types
 * @{
 */
#define PICOCBOR_MASK_UINT      (PICOCBOR_TYPE_UINT  << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_MASK_NINT      (PICOCBOR_TYPE_NINT  << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_MASK_BSTR      (PICOCBOR_TYPE_BSTR  << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_MASK_TSTR      (PICOCBOR_TYPE_TSTR  << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_MASK_ARR       (PICOCBOR_TYPE_ARR   << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_MASK_MAP       (PICOCBOR_TYPE_MAP   << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_MASK_TAG       (PICOCBOR_TYPE_TAG   << PICOCBOR_TYPE_OFFSET)
#define PICOCBOR_MASK_FLOAT     (PICOCBOR_TYPE_FLOAT << PICOCBOR_TYPE_OFFSET)
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
#define PICOCBOR_SIZE_INDEFINITE    31U
/** @} */

typedef enum {
    PICOCBOR_OK = 0,
    PICOCBOR_ERR_OVERFLOW = -1,
    PICOCBOR_ERR_INVALID_TYPE = -2,
    PICOCBOR_ERR_END = -3,
    PICOCBOR_ERR_INVALID = -4,
} picocbor_error_t;

typedef struct picocbor_value {
    const uint8_t *start;
    const uint8_t *end;
    uint32_t remaining;
    uint8_t flags;
} picocbor_value_t;

#define PICOCBOR_DECODER_FLAG_CONTAINER  0x01
#define PICOCBOR_DECODER_FLAG_INDEFINITE 0x02

void picocbor_decoder_init(picocbor_value_t *value,
                          uint8_t *buf, size_t len);
uint8_t picocbor_get_type(picocbor_value_t *value);
bool picocbor_at_end(picocbor_value_t *it);
int picocbor_get_uint32(picocbor_value_t *cvalue, uint32_t *value);
int picocbor_get_int32(picocbor_value_t *cvalue, int32_t *value);
int picocbor_get_bstr(picocbor_value_t *cvalue, const uint8_t **buf, size_t *len);
int picocbor_get_tstr(picocbor_value_t *cvalue, const uint8_t **buf, size_t *len);
int picocbor_enter_array(picocbor_value_t *it, picocbor_value_t *array);
int picocbor_enter_map(picocbor_value_t *it, picocbor_value_t *map);
void picocbor_leave_container(picocbor_value_t *it, picocbor_value_t *array);
int picocbor_get_null(picocbor_value_t *cvalue);
int picocbor_get_bool(picocbor_value_t *cvalue, bool *value);
int picocbor_skip_float(picocbor_value_t *cvalue);

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

/**
 * @brief Write an array indicator with @ref len items
 *
 * @param[in]   buf     buffer to write into
 * @param[in]   len     Number of items in the array
 *
 * @return              Number of bytes written
 */
size_t picocbor_fmt_array(uint8_t *buf, size_t len);

/**
 * @brief Write a map indicator with @ref len pairs
 *
 * @param[in]   buf     buffer to write into
 * @param[in]   len     Number of pairs in the map
 *
 * @return              Number of bytes written
 */
size_t picocbor_fmt_map(uint8_t *buf, size_t len);

/**
 * @brief Write an indefinite-length array indicator
 *
 * @param[in]   buf     buffer to write into
 *
 * @return              Number of bytes written
 */
size_t picocbor_fmt_array_indefinite(uint8_t *buf);

/**
 * @brief Write an indefinite-length map indicator
 *
 * @param[in]   buf     buffer to write the map
 *
 * @return              Number of bytes written
 */
size_t picocbor_fmt_map_indefinite(uint8_t *buf);

size_t picocbor_fmt_float(uint8_t *buf, float num);
#endif /* PICOCBOR_H */
