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
 * @brief       Test application for the ili9431 tft display
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 *
 * @}
 */

#ifndef TEST_SPI
#error "TEST_SPI not defined"
#endif

#ifndef TEST_SPI_CS
#error "TEST_SPI_CS not defined"
#endif

#ifndef TEST_SPI_CLK
#error "TEST_SPI_CLK not defined"
#endif

#ifndef TEST_PIN_DC
#error "TEST_PIN_DC not defined"
#endif
#ifndef TEST_PIN_RST
#error "TEST_PIN_RST not defined"
#endif

#include <stdio.h>
#include "xtimer.h"
#include "ili9341.h"
#include "periph/spi.h"

int main(void)
{
    ili9341_t dev;
    ili9341_params_t params = {
        .spi = TEST_SPI,
        .spi_clk = TEST_SPI_CLK,
        .cs_pin = TEST_SPI_CS,
        .dc_pin = TEST_PIN_DC,
        .rst_pin = TEST_PIN_RST
    };

    puts("ili9341 TFT display test application\n");

    /* initialize the sensor */
    printf("Initializing display...");

    if (ili9341_init(&dev, &params) == 0) {
        puts("[OK]");
    }
    else {
        puts("[Failed]");
        return 1;
    }

    puts("ili9341 TFT display filling map\n");
    ili9341_fill(&dev, 0, 319, 0, 239, 0x0000);
    puts("ili9341 TFT display map filled\n");

    uint16_t i = 0x00ff;
    ili9341_fill(&dev, 10, 59, 10, 109, i);
    xtimer_sleep(1);
    ili9341_invert_on(&dev);
    puts("ili9341 TFT display inverted\n");
    xtimer_sleep(1);
    ili9341_invert_off(&dev);
    puts("ili9341 TFT display normal\n");
    while (1) {
        i+=0x0010;
        ili9341_fill(&dev, 10, 59, 10, 109, i);
        xtimer_usleep(10 * US_PER_MS);
    }

    return 0;
}
