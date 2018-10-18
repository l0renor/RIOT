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
 * @brief   Minimalistic CBOR encoder implementation
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "picocbor.h"
#include "byteorder.h"

size_t picocbor_fmt_bool(uint8_t *buf, bool content)
{
    uint8_t value = content ? PICOCBOR_SIMPLE_TRUE : PICOCBOR_SIMPLE_FALSE;
    *buf = PICOCBOR_TYPE_FLOAT | value;
    return 1;
}

static size_t _fmt_uint32(uint8_t *buf, uint32_t num, uint8_t type)
{
    if (num < 23) {
        *buf = num | type;
        return 1;
    }
    else if (num <= UINT8_MAX) {
        *buf++ = type | PICOCBOR_SIZE_BYTE;
        *buf = num;
        return 2;
    }
    else if (num <= UINT16_MAX) {
        *buf++ = type | PICOCBOR_SIZE_SHORT;
        *buf++ = ((num & 0xff00) >> 8) & 0xff;
        *buf =  num & 0x00ff;
        return 3;
    }
    else {
        *buf++ = type | PICOCBOR_SIZE_WORD;
        uint32_t bnum = htonl(num);
        memcpy(buf, &bnum, sizeof(bnum));
        return 5;
    }
}

size_t picocbor_fmt_uint(uint8_t *buf, uint32_t num)
{
    return _fmt_uint32(buf, num, PICOCBOR_TYPE_UINT);
}

size_t picocbor_fmt_int(uint8_t *buf, int32_t num)
{
    if (num < 0) {
        num = abs(num) - 1;
        return _fmt_uint32(buf, num, PICOCBOR_TYPE_NINT);
    }
    else {
        return picocbor_fmt_uint(buf, (uint32_t)num);
    }
}

size_t picocbor_fmt_bstr(uint8_t *buf, size_t len)
{
    return _fmt_uint32(buf, (uint32_t)len, PICOCBOR_TYPE_BSTR);
}

size_t picocbor_fmt_tstr(uint8_t *buf, size_t len)
{
    return _fmt_uint32(buf, (uint32_t)len, PICOCBOR_TYPE_TSTR);
}

size_t picocbor_put_tstr(uint8_t *buf, char* str)
{
    size_t len = strlen(str);
    size_t hdrlen = picocbor_fmt_tstr(buf, len);
    memcpy(buf + hdrlen, str, len);
    return hdrlen + len;
}

size_t picocbor_put_bstr(uint8_t *buf, char* str, size_t len)
{
    size_t hdrlen = picocbor_fmt_bstr(buf, len);
    memcpy(buf+hdrlen, str, len);
    return hdrlen + len;
}

size_t picocbor_fmt_array(uint8_t *buf, size_t len)
{
    return _fmt_uint32(buf, (uint32_t)len, PICOCBOR_TYPE_ARR);
}
