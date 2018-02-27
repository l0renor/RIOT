/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_xpt2046 XPT2046 touch screen driver
 * @ingroup     drivers_sensors
 *
 * @{
 * @file
 * @brief       Device driver interface for the XPT2046 touch screen sensor.
 *
 * @details     There are three sensor values that can be read: x, y and z
 *              values. This sensor will read all three values in one
 *              measurement sequence
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef XPT2046_H
#define XPT2046_H

#include "periph/spi.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   Parameters for the xpt2046 sensor
 *
 * These parameters are needed to configure the device at startup.
 */
typedef struct {
    spi_t spi;              /**< SPI device which is used */
    spi_clk_t spi_clk;      /**< SPI speed to use */
    spi_cs_t cs_pin;        /**< GPIO pin connected to chip select */
    gpio_t int_pin;         /**< GPIO pin connected to the interrupt pin */
} xpt2046_params_t;

/**
 * @brief   Device descriptor for the xpt2046 sensor
 */
typedef struct {
    xpt2046_params_t params;             /**< Device Parameters */
} xpt2046_t;

/**
 * @name Device measurement struct
 */
typedef struct {
    uint16_t x;                         /**< X position of the measurement */
    uint16_t y;                         /**< Y position of the measurement */
    uint16_t z;                         /**< Pressure measurement */
} xpt2046_xyz_t;

/**
 * @brief   Status and error return codes
 */
enum {
    XPT2046_OK           =  0,     /**< everything was fine */
    XPT2046_ERR_SPI      = -1,     /**< error initializing the SPI bus */
};

/**
 * @brief   Initialize the given XPT2046 device
 *
 * @param[out] dev          Initialized device descriptor of XPT2046 device
 * @param[in]  params       The parameters for the XPT2046 device (sampling rate, etc)
 *
 * @return                  XPT2046_OK on success
 * @return                  XPT2046_ERR_SPI
 */
int xpt2046_init(xpt2046_t* dev, const xpt2046_params_t* params);

/**
 * @brief   Do a touch screen measurement
 *
 * @param[in]  dev          XPT2046 device descriptor
 * @param[out] measure      Measurement struct for the output values
 */
int xpt2046_get_xyz(const xpt2046_t* dev, xpt2046_xyz_t* measure);

uint16_t xpt2046_get_temp(const xpt2046_t* dev);


#ifdef __cplusplus
}
#endif

#endif /* XPT2046_H */
/** @} */
