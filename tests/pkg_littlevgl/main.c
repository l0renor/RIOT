/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
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
 * @brief       Littlevgl LVGL test application
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <stdio.h>
#include "msg.h"
#include "xtimer.h"
#include "lvgl.h"

#include <SDL2/SDL.h>
#include "lvgl.h"
#include "monitor.h"
#include "mouse.h"
#include "demo.h"

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
 */
static void hal_init(void)
{
    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.disp_flush = monitor_flush;
    disp_drv.disp_fill = monitor_fill;
    disp_drv.disp_map = monitor_map;
    lv_disp_drv_register(&disp_drv);

    /* Add the mouse (or touchpad) as input device
     * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
    lv_indev_drv_register(&indev_drv);
}

int main(void)
{
    puts("RIOT littlevgl test application");

    /*Initialize LittlevGL*/
    lv_init();
    hal_init();

    /*Load a demo*/
    demo_create();
    while(1)
    {
        lv_task_handler();
        lv_tick_inc(1);
        xtimer_usleep(US_PER_MS);
    }
    /* should be never reached */
    return 0;
}
