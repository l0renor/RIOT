/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup sys_picocbor
 * @{
 * @file
 * @brief   Minimalistic CBOR decoder implementation
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "picocbor.h"
#include "byteorder.h"

void picocbor_decoder_init(picocbor_value_t *value,
                           uint8_t *buf, size_t len)
{
    value->start = buf;
    value->end = buf + len;
    value->flags = 0;
}

static inline uint8_t _get_type(picocbor_value_t *value)
{
    return (*value->start & PICOCBOR_TYPE_MASK);
}

uint8_t picocbor_get_type(picocbor_value_t *value)
{
    return (_get_type(value) >> PICOCBOR_TYPE_OFFSET);
}

bool picocbor_at_end(picocbor_value_t *it)
{
    if (it->flags & PICOCBOR_DECODER_FLAG_CONTAINER) {
        if (it->flags & PICOCBOR_DECODER_FLAG_INDEFINITE &&
            *it->start == 0xFF) {
            it->start++;
            return true;
        }
        return it->remaining == 0;
    }
    else {
        return it->start >= it->end;
    }
}

/* Doesn't type check */
static int _get_uint64(picocbor_value_t *cvalue, uint64_t *value)
{
    uint64_t tmp = 0;
    unsigned bytelen = *cvalue->start & PICOCBOR_VALUE_MASK;

    if (bytelen < 24) {
        *value = bytelen;
        /* Ptr should advance 1 pos */
        return 1;
    }
    if (bytelen > 27) {
        return PICOCBOR_ERR_INVALID;
    }
    unsigned bytes = 1 << (bytelen - 24);
    if (cvalue->start + bytes > cvalue->end) {
        return PICOCBOR_ERR_END;
    }
    /* Copy the value from cbor to the least significant bytes */
    memcpy(((uint8_t *)&tmp) + sizeof(uint64_t) - bytes, cvalue->start + 1, bytes);
    *value = ntohll(tmp);
    return 1 + bytes;
}

static int _get_uint32(picocbor_value_t *cvalue, uint32_t *value)
{
    uint64_t tmp = 0;
    int res = _get_uint64(cvalue, &tmp);

    if (tmp <= UINT32_MAX) {
        *value = (uint32_t)tmp;
        return res;
    }
    return PICOCBOR_ERR_OVERFLOW;
}

static int _get_nint32(picocbor_value_t *cvalue, int32_t *value)
{
    uint64_t tmp = 0;
    int res = _get_uint64(cvalue, &tmp);

    if (tmp <= INT32_MAX) {
        *value = (-(int32_t)tmp) - 1;
        return res;
    }
    return PICOCBOR_ERR_OVERFLOW;
}

static void _advance(picocbor_value_t *cvalue, int res)
{
    cvalue->start += res;
    cvalue->remaining--;
}

static int _advance_if(picocbor_value_t *cvalue, int res)
{
    if (res > 0) {
        _advance(cvalue, res);
    }
    return res;
}

/* Includes check */
int picocbor_get_uint32(picocbor_value_t *cvalue, uint32_t *value)
{
    uint8_t type = _get_type(cvalue);
    int res = PICOCBOR_ERR_INVALID_TYPE;

    if (type == PICOCBOR_MASK_UINT) {
        res = _get_uint32(cvalue, value);
    }
    return _advance_if(cvalue, res);
}

/* Read the int and advance past the int */
int picocbor_get_int32(picocbor_value_t *cvalue, int32_t *value)
{
    uint8_t type = _get_type(cvalue);

    int res = 0;

    if (type == PICOCBOR_MASK_UINT) {
        uint32_t intermediate = 0;
        res = _get_uint32(cvalue, &intermediate);
        *value = intermediate;
    }
    else if (type == PICOCBOR_MASK_NINT) {
        res = _get_nint32(cvalue, value);
    }
    else {
        return PICOCBOR_ERR_INVALID_TYPE;
    }
    return _advance_if(cvalue, res);
}

static int _get_str(picocbor_value_t *cvalue, const uint8_t **buf, size_t *len, uint8_t type)
{
    uint8_t ctype = _get_type(cvalue);

    if (ctype != type) {
        return PICOCBOR_ERR_INVALID_TYPE;
    }
    uint64_t blen = 0;
    int res = _get_uint64(cvalue, &blen);
    if (blen > SIZE_MAX) {
        return PICOCBOR_ERR_OVERFLOW;
    }
    if (cvalue->end - cvalue->start < (int64_t)blen) {
        return PICOCBOR_ERR_END;
    }
    *len = blen;
    *buf = (cvalue->start);
    if (_advance_if(cvalue, res) < 0) {
        return res;
    }
    cvalue->start += blen;
    return PICOCBOR_OK;
}

int picocbor_get_bstr(picocbor_value_t *cvalue, const uint8_t **buf, size_t *len)
{
    return _get_str(cvalue, buf, len, PICOCBOR_MASK_BSTR);
}

int picocbor_get_tstr(picocbor_value_t *cvalue, const uint8_t **buf, size_t *len)
{
    return _get_str(cvalue, buf, len, PICOCBOR_MASK_TSTR);
}

int picocbor_get_null(picocbor_value_t *cvalue)
{
    if (*cvalue->start == (PICOCBOR_MASK_FLOAT | PICOCBOR_SIMPLE_NULL)) {
        _advance(cvalue, 1);
        return PICOCBOR_OK;
    }
    return PICOCBOR_ERR_INVALID_TYPE;
}

int picocbor_get_bool(picocbor_value_t *cvalue, bool *value)
{
    if ((*cvalue->start & (PICOCBOR_TYPE_MASK | (PICOCBOR_VALUE_MASK - 1))) ==
        (PICOCBOR_MASK_FLOAT | PICOCBOR_SIMPLE_FALSE)) {
        *value = (*cvalue->start & 0x01);
        _advance(cvalue, 1);
        return PICOCBOR_OK;
    }
    return PICOCBOR_ERR_INVALID_TYPE;
}

int picocbor_skip_float(picocbor_value_t *cvalue)
{
    uint8_t type = _get_type(cvalue);

    if (type == PICOCBOR_MASK_FLOAT) {
        uint64_t tmp;
        int res = _get_uint64(cvalue, &tmp);
        (void)tmp;
        return _advance_if(cvalue, res);
    }
    return PICOCBOR_ERR_INVALID_TYPE;
}

int picocbor_enter_container(picocbor_value_t *it, picocbor_value_t *container)
{
    container->end = it->end;

    /* Indefinite container */
    if ((*it->start & 0x1F) == 0x1F) {
        container->flags = PICOCBOR_DECODER_FLAG_INDEFINITE |
                           PICOCBOR_DECODER_FLAG_CONTAINER;
        container->start = it->start + 1;
        container->remaining = UINT32_MAX;
    }
    else {
        int res = _get_uint32(it, &container->remaining);
        container->flags = PICOCBOR_DECODER_FLAG_CONTAINER;
        if (res < 0) {
            return res;
        }
        container->start = it->start + res;
    }
    return PICOCBOR_OK;
}

int picocbor_enter_array(picocbor_value_t *it, picocbor_value_t *array)
{
    uint8_t type = _get_type(it);

    if (type != PICOCBOR_MASK_ARR) {
        return PICOCBOR_ERR_INVALID_TYPE;
    }
    return picocbor_enter_container(it, array);
}

int picocbor_enter_map(picocbor_value_t *it, picocbor_value_t *map)
{
    uint8_t type = picocbor_get_type(it);

    if (type != PICOCBOR_MASK_MAP) {
        return PICOCBOR_ERR_INVALID_TYPE;
    }
    int res = picocbor_enter_container(it, map);
    if (map->remaining > UINT32_MAX / 2) {
        return PICOCBOR_ERR_OVERFLOW;
    }
    map->remaining = map->remaining * 2;
    return res;
}

void picocbor_leave_container(picocbor_value_t *it, picocbor_value_t *array)
{
    it->start = array->start;
    if (it->flags == PICOCBOR_DECODER_FLAG_CONTAINER) {
        it->remaining--;
    }
}


/* E_NOTIMPLEMENTED */
int picocbor_advance(picocbor_value_t *it)
{
    uint8_t type = _get_type(it);


    if (type == PICOCBOR_MASK_BSTR || type == PICOCBOR_MASK_TSTR) {
        const uint8_t *tmp;
        size_t len;
        if (_get_str(it, tmp, len) < 0) {
            return PICOCBOR_ERR_END;
        }
    }
    /* map or array */
    return 0;
}
