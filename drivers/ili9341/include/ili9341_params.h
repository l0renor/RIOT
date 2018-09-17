/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ili9341
 *
 * @{
 * @file
 * @brief       Default configuration for ili9341
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef ILI9341_PARAMS_H
#define ILI9341_PARAMS_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Default parameters for ILI9341 display */
/**
 * @name    Set default configuration parameters for the ILI9341
 * @{
 */
#ifndef ILI9341_PARAM_SPI
#define ILI9341_PARAM_SPI          (SPI_DEV(0))
#endif
#ifndef ILI9341_PARAM_SPI_CLK
#define ILI9341_PARAM_SPI_CLK      (SPI_CLK_5MHZ)
#endif
#ifndef ILI9341_PARAM_CS
#define ILI9341_PARAM_CS           (GPIO_PIN(0, 0))
#endif
#ifndef ILI9341_PARAM_DX
#define ILI9341_PARAM_DX           (GPIO_PIN(0, 1))
#endif

#ifndef ILI9341_PARAMS
#define ILI9341_PARAMS              { .spi = ILI9341_PARAM_SPI, \
                                      .spi_clk = ILI9341_PARAM_SPI_CLK, \
                                      .cs_pin = ILI9341_PARAM_CS, \
                                      .dc_pin = ILI9341_PARAM_INT, \
#endif
/**@}*/

/**
 * @brief   Configure ILI9341
 */
static const ili9341_params_t ili9341_params[] =
{
    ILI9341_PARAMS,
};

#endif /* ILI9341_PARAMS_H */
