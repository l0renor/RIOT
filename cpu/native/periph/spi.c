/*
 * Copyright (C) 2017 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_periph_spi
 * @{
 *
 * @file
 * @brief       Low-level SPI driver implementation for native
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 *
 * @}
 */

#include "cpu.h"
#include "mutex.h"
#include "assert.h"
#include "periph/spi.h"
#include "periph/gpio.h"

/* Remove this ugly guard once we selectively build the periph drivers */
#ifdef SPI_NUMOF

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Allocate one lock per SPI device
 */
static mutex_t locks[SPI_NUMOF];

static int fds[SPI_NUMOF];

/* Initialize SPI bus, probably only check if it exists */
void spi_init(spi_t bus)
{
    assert(bus < SPI_NUMOF);
}

/* Initialize SPI bus with CS, export CS pin as an output */
int spi_init_cs(spi_t bus, spi_cs_t cs)
{
    if (bus >= SPI_NUMOF) {
        return SPI_NODEV;
    }
    if (cs == SPI_CS_UNDEF ||
        (((cs & SPI_HWCS_MASK) == SPI_HWCS_MASK) && (cs & ~(SPI_HWCS_MASK)))) {
        return SPI_NOCS;
    }
    gpio_init(cs, GPIO_OUT);
    return SPI_OK;
}

/* Open and configure the SPI bus, do the real initialization here*/
int spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
{
    char spidev[128];
    uint8_t spidev_mode = mode;
    uint8_t bits = 8;

    /* lock bus */
    mutex_lock(&locks[bus]);
    if ((cs != SPI_HWCS_MASK) && (cs != SPI_CS_UNDEF)) {
        cs = 0;
        spidev_mode |= SPI_NO_CS;
    }
    snprintf(spidev, sizeof(spidev), "/dev/spidev%u.%u", bus, cs);
    /* Open device */
    fds[bus] = open(spidev, O_RDWR);
    
    /* Configure SPIDEV */
    if (ioctl(fds[bus], SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {
        DEBUG("[SPI]: bits_per_word setup failed for %s\n", spidev);
        return SPI_NOMODE;
    }
    if (ioctl(fds[bus], SPI_IOC_WR_MAX_SPEED_HZ, &clk) < 0) {
        DEBUG("[SPI]: clock setup failed for %s\n", spidev);
        return SPI_NOCLK;
    }
    if (ioctl(fds[bus], SPI_IOC_WR_MODE, &mode) < 0) {
        DEBUG("[SPI]: Mode setup failed for %s\n", spidev);
        return SPI_NOMODE;
    }
    return SPI_OK;

}

/* Close the file descriptor */
void spi_release(spi_t bus)
{
    close(fds[bus]);
    mutex_unlock(&locks[bus]);
}

/* Transfer call. Store when cont=true, flush when cont=false */
void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len)
{
    /* TODO: assert CS pin */
    if ((cs != SPI_HWCS_MASK) && (cs != SPI_CS_UNDEF)) {
        gpio_clear((gpio_t)cs);
    }
    struct spi_ioc_transfer xfer;
	memset(&xfer, 0, sizeof(xfer));
    xfer.tx_buf = (unsigned long)out;
    xfer.rx_buf = (unsigned long)in;
    xfer.len = len;
    ioctl(fds[bus], SPI_IOC_MESSAGE(1), &xfer);
    if ((!cont) && (cs != SPI_CS_UNDEF)) {
        gpio_set((gpio_t)cs);
    }
}

#endif
