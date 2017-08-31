/**
 * Native CPU peripheral configuration
 *
 * Copyright (C) 2014 Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * @ingroup native_cpu
 * @{
 * @file
 * @author  Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @name hardware timer clock skew avoidance
 * @{
 */
#define NATIVE_TIMER_MIN_RES 200
/** @} */

/**
 * @name Random Number Generator configuration
 * @{
 */
#define RANDOM_NUMOF        (1U)
/** @} */

/**
 * @name RealTime Clock configuration
 * @{
 */
#define RTC_NUMOF (1)
/** @} */

/**
 * @name GPIO sysfs dir
 * @{
 */
#define GPIO_SYSFS_DIR      "/sys/class/gpio"
/** @} */

/**
 * @name Maximum number of GPIO supported (also highest GPIO number supported)
 * @{
 */
#define GPIO_NATIVE_NUMOF   42
/** @} */

/**
 * @name GPIO sysfs dir
 * @{
 */
#define GPIO_SYSFS_DIR      "/sys/class/gpio"
/** @} */

/**
 * @brief   Available SPI clock speeds
 *
 * The actual speed of the bus can vary to some extend, as the combination of
 * CPU clock and available prescaler values on certain platforms may not make
 * the exact values possible.
 */
#define HAVE_SPI_CLK_T
typedef enum {
    SPI_CLK_100KHZ =   100000U,     /**< drive the SPI bus with 100KHz clock speed */
    SPI_CLK_400KHZ =   400000U,     /**< drive the SPI bus with 400KHz clock speed*/
    SPI_CLK_1MHZ   =  1000000U,     /**< drive the SPI bus with 1MHz clock speed */
    SPI_CLK_5MHZ   =  5000000U,     /**< drive the SPI bus with 5MHz clock speed */
    SPI_CLK_10MHZ  = 10000000U      /**< drive the SPI bus with 10MHz clock speed */
} spi_clk_t;

/**
 * @name Maximum number of GPIO supported (also highest GPIO number supported)
 * @{
 */
#define GPIO_NATIVE_NUMOF   42
/** @} */

#define SPI_NUMOF	2
#define SPI_HWCS_MASK (0xffffff00)
#define PERIPH_SPI_NEEDS_TRANSFER_BYTE
#define PERIPH_SPI_NEEDS_TRANSFER_REG

/**
 * @name Timer peripheral configuration
 * @{
 */
#define TIMER_NUMOF        (1U)
#define TIMER_0_EN         1

/**
 * @brief xtimer configuration
 * @{
 */
#define XTIMER_OVERHEAD 14

/* timer_set_absolute() has a high margin for possible underflow if set with
 * value not far in the future. To prevent this, we set high backoff values
 * here.
 */
#define XTIMER_BACKOFF      200
#define XTIMER_ISR_BACKOFF  200

/** @} */

/**
 * @brief UART configuration
 * @{
 */
#ifndef UART_NUMOF
#define UART_NUMOF (1U)
#endif
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
