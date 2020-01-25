/*
 * Copyright (C) 2020  Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_mtd_mapper  MTD addres mapper
 * @ingroup     drivers_storage
 * @brief       Driver for address remap for flash devices
 *
 * This MTD module allows for remapping multiple different regions on a single
 * MTD device and present them as separate MTD devices. This is similar to
 * partitions on a hard drive, although this system only allows hardcoded
 * partitions and lacks a partition table.
 *
 * The use case for this module is to be able to split a single MTD device, for
 * example a SPI NOR flash chip into multiple separate regions which all can
 * contain their own content or file systems.
 *
 * ## Usage
 *
 * To use this module include it in your makefile:
 *
 * ```
 * USEMODULE += mtd_mapper
 * ```
 *
 * To define new regions with an existing MTD device the following is required:
 *
 * ```
 * mtd_mapper_parent_t parent = {
 *     .mtd = MTD_0,
 *     .mutex = MUTEX_INIT,
 *     .init = false,
 * };
 *
 * mtd_mapper_region_t region1 = {
 *     .mtd = {
 *         .driver = &mtd_mapper_driver,
 *         .sector_count = SECTOR_COUNT / 2,
 *         .pages_per_sector = PAGE_PER_SECTOR,
 *         .page_size = PAGE_SIZE,
 *     },
 *     .parent = &parent,
 *     .offset = PAGE_PER_SECTOR * PAGE_SIZE * SECTOR_COUNT / 2
 * };
 *
 * mtd_dev_t *dev = &region.mtd;
 * ```
 * The snippet here defines a region within an existing `MTD_0` device of half
 * the size of `MTD_0` and starting in the middle of the device.
 *
 * @{
 *
 * @brief       Interface definitions for mtd mapper support
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef MTD_MAPPER_H
#define MTD_MAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include "mtd.h"
#include "mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MTD mapper backing device context
 */
typedef struct {
    mtd_dev_t *mtd;     /**< Parent MTD device */
    mutex_t lock;       /**< Mutex for guarding the backing device access */
    bool init;          /**< Initialization flag */
} mtd_mapper_parent_t;

/**
 * @brief MTD mapped region
 */
typedef struct {
    mtd_dev_t mtd;                  /**< MTD context                         */
    mtd_mapper_parent_t *parent;    /**< MTD mapper parent device            */
    uint32_t offset;                /**< Offset address to start this region */
} mtd_mapper_region_t;

extern const mtd_desc_t mtd_mapper_driver;

#ifdef __cplusplus
}
#endif

#endif /* MTD_MAPPER_H */
/** @} */
