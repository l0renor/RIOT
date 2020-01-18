/*
 * Copyright (C) 2020 Koen Zandberg
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
 * @brief       Test application for the CST816S touch screen driver
 *
 * @author      koen Zandberg <koen@bergzand.net>
 *
 * @}
 */

#include <stdlib.h>
#include <stdio.h>

#include "cst816s_params.h"
#include "cst816s.h"
#include "xtimer.h"
#include "fmt.h"
#include "thread.h"
#include "thread_flags.h"

static kernel_pid_t _pid;

#define CST816S_THREAD_FLAG     (1 << 8)
#define CST816S_NUM_TOUCHES     5

static void _cb(cst816s_t *dev, void *arg)
{
    (void)dev;
    (void)arg;
    thread_flags_set((thread_t *)sched_threads[_pid], CST816S_THREAD_FLAG);
}

static int _dump_cst816s(cst816s_t *dev)
{
    puts("Reading data:");
    cst816s_touch_data_t touches;
    int num = cst816s_read(dev, &touches);
    if (num == 0) {
        printf("Touch at %03u, %03u with %02x\n", touches.x,
               touches.y, touches.gesture);
    }
    else {
        puts("Device not responding");
    }
    return num;
}

int main(void)
{
    cst816s_t dev;

    _pid = sched_active_pid;

    puts("CST816S test application\n");
    cst816s_init(&dev, &cst816s_params[0], _cb, NULL);

    while (1) {
        thread_flags_t flags = thread_flags_wait_any(CST816S_THREAD_FLAG);
        if (flags & CST816S_THREAD_FLAG) {
            _dump_cst816s(&dev);
        }
    }
    return 0;
}
