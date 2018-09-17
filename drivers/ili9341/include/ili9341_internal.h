/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ili9341
 * @{
 *
 * @file
 * @brief       Device driver implementation for the ili9341 display controller
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 *
 * @}
 */

#ifndef ILI9341_INTERNAL_H
#define ILI9341_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define ILI9341_CMD_SWRESET       0x01
#define ILI9341_CMD_SPLIN           0x10 /* Enter sleep mode */
#define ILI9341_CMD_SLPOUT          0x11 /* Sleep out */
#define ILI9341_CMD_DINVOFF         0x20 /* Display inversion off */
#define ILI9341_CMD_DINVON          0x21 /* Display inversion on */
#define ILI9341_CMD_


#define ILI9341_CMD_GAMSET          0x26 /* Gamma Set */
#define ILI9341_CMD_DISPOFF         0x28 /* Display OFF */
#define ILI9341_CMD_DISPON          0x29 /* Display ON */
#define ILI9341_CMD_CASET           0x2A /* Column Address Set */
#define ILI9341_CMD_PASET           0x2b /* Page Address Set */
#define ILI9341_CMD_RAMWR           0x2c /* Memory Write */
#define ILI9341_CMD_RAMRD           0x2e /* Memory Read */
#define ILI9341_CMD_MADCTL          0x36
#define ILI9341_CMD_IDMOFF          0x38 /* Idle Mode OFF */
#define ILI9341_CMD_IDMON           0x39 /* Idle Mode ON */
#define ILI9341_CMD_PIXSET          0x3A /* COLMOD: Pixel Format Set */
#define ILI9341_CMD_WRDISBV         0x51 /* Write Display Brightness */
#define ILI9341_CMD_WRCTRLD         0x53
#define ILI9341_CMD_RDCTRLD         0x54
#define ILI9341_CMD_FRAMECTL1       0xb1
#define ILI9341_CMD_FRAMECTL2       0xb2
#define ILI9341_CMD_FRAMECTL3       0xb3
#define ILI9341_CMD_DFUNC           0xb3
#define ILI9341_CMD_PWCTRL1         0xc0
#define ILI9341_CMD_PWCTRL2         0xc1
#define ILI9341_CMD_VMCTRL1         0xc5
#define ILI9341_CMD_VMCTRL2         0xc7
#define ILI9341_CMD_PGAMCTRL        0xe0
#define ILI9341_CMD_NGAMCTRL        0xe1
#define ILI9341_CMD_IFCTL           0xf6


#define ILI9341_MADCTL_MY           0x80
#define ILI9341_MADCTL_MX           0x40
#define ILI9341_MADCTL_MV           0x20
#define ILI9341_MADCTL_ML           0x10
#define ILI9341_MADCTL_BGR          0x08
#define ILI9341_MADCTL_MH           0x04

#define ILI9341_MADCTL_VERT         ILI9341_MADCTL_MX
#define ILI9341_MADCTL_VERT_FLIP    ILI9341_MADCTL_MY
#define ILI9341_MADCTL_HORZ         ILI9341_MADCTL_MV
#define ILI9341_MADCTL_HORZ_FLIP    ILI9341_MADCTL_MV | ILI9341_MADCTL_MY | ILI9341_MADCTL_MX

#define ILI9341_PIXSET_16BIT        0x55 /* MCU and RGB 16 bit interface */
#define ILI9341_PIXSET_18BIT        0x66 /* MCU and RGB 18 bit interface (not implemented) */

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_INTERNAL_H */
