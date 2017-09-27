/*
 * Copyright (C) 2017 Tobias Heider <heidert@nm.ifi.lmu.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>
#include "memarray.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

void memarray_init(memarray_t *mem, void *data, size_t size, size_t num)
{
    DEBUG("memarray: Initialize memarray of %zu times %zu Bytes at %p\n",
          num, size, data);

    mem->free_data = data;
    mem->size = size;
    mem->num = num;

    for (size_t i = 0; i < (mem->num - 1); i++) {
        void *next = ((char *)mem->free_data) + ((i + 1) * mem->size);
        memcpy(((char *)mem->free_data) + (i * mem->size), &next, sizeof(void *));
    }
}

void *memarray_alloc(memarray_t *mem)
{
    if (mem->free_data == NULL) {
        return NULL;
    }
    void *free = mem->free_data;
    mem->free_data = *((void **)mem->free_data);
    DEBUG("memarray: Allocate %zu Bytes at %p\n", mem->size, free);
    return free;
}

void memarray_free(memarray_t *mem, void *ptr)
{
    memcpy(ptr, &mem->free_data, sizeof(void *));
    mem->free_data = ptr;
    DEBUG("memarray: Free %zu Bytes at %p\n", mem->size, ptr);
}
