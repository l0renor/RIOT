/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup sam0_common
 * @{
 * @file
 * @brief   USB interface functions
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "cpu.h"
#include "periph/gpio.h"
#include "usb/usbdev.h"
#include "sam_usb.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static sam0_common_usb_t *_usbdev;

static bool usb_enable_syncing(void)
{
    if (USB->DEVICE.SYNCBUSY.reg & USB_SYNCBUSY_ENABLE) {
        return true;
    }
    return false;
}

static bool usb_swrst_syncing(void)
{
    if (USB->DEVICE.SYNCBUSY.reg & USB_SYNCBUSY_SWRST) {
        return true;
    }
    return false;
}

static inline void poweron(void)
{
#if defined(CPU_FAM_SAMD21)
    PM->AHBMASK.reg |= PM_AHBMASK_USB;
    PM->APBBMASK.reg |= PM_APBBMASK_USB;
    GCLK->CLKCTRL.reg = (uint32_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 |
                        (GCLK_CLKCTRL_ID(USB_GCLK_ID)));
#elif defined(CPU_FAM_SAML21)
    MCLK->AHBMASK.reg |= (MCLK_AHBMASK_USB);
    GCLK->PCHCTRL[USB_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK0;
#endif
}

static inline void poweroff(void)
{
#if defined(CPU_FAM_SAMD21)
    PM->AHBMASK.reg |= PM_AHBMASK_USB;
    GCLK->CLKCTRL.reg = (uint32_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 |
                        (GCLK_CLKCTRL_ID(USB_GCLK_ID)));
#elif defined(CPU_FAM_SAML21)
    MCLK->AHBMASK.reg |= (MCLK_AHBMASK_USB);
    GCLK->PCHCTRL[USB_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK0;
#endif
}

int usbdev_init(usbdev_t *dev)
{
    /* Only one usb device on this board */
    _usbdev = (sam0_common_usb_t*)dev;
    poweron();
    /* Reset peripheral */
    USB->DEVICE.CTRLA.reg |= USB_CTRLA_SWRST;
    while(usb_swrst_syncing()) {}
    while(USB->DEVICE.CTRLA.bit.SWRST) {}

    /* Enable USB device */
    USB->DEVICE.CTRLA.reg |= USB_CTRLA_ENABLE | USB_CTRLA_RUNSTDBY;
    while(usb_enable_syncing()) {}
    /* Callibration values */
    USB->DEVICE.PADCAL.reg = 
        USB_PADCAL_TRANSP((*(uint32_t*)USB_FUSES_TRANSP_ADDR >>
                            USB_FUSES_TRANSP_Pos)) |
        USB_PADCAL_TRANSN((*(uint32_t*)USB_FUSES_TRANSN_ADDR >>
                            USB_FUSES_TRANSN_Pos)) |
        USB_PADCAL_TRIM((*(uint32_t*)USB_FUSES_TRIM_ADDR >>
                            USB_FUSES_TRIM_Pos));
    
    /* Set GPIO */
    gpio_init(GPIO_PIN(PA,24), GPIO_IN);
    gpio_init(GPIO_PIN(PA,25), GPIO_IN);
    gpio_init_mux(GPIO_PIN(PA, 24), GPIO_MUX_G);
    gpio_init_mux(GPIO_PIN(PA, 25), GPIO_MUX_G);


    /* Interrupt configuration */
    USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_SOF;
    NVIC_EnableIRQ(USB_IRQn);
 
//    UsbDeviceEndpoint *ep = &USB->DEVICE.DeviceEndpoint[0];
//    ep->EPCFG.bit.EPTYPE0 = 0x01;
//    ep->EPCFG.bit.EPTYPE1 = 0x01;
    return 0;
}

void usb_attach(void)
{
    /* Datasheet is not clear whether device starts detached */
    USB->DEVICE.CTRLB.reg &= ~USB_DEVICE_CTRLB_DETACH;
    DEBUG("USB: ctrlb: %x\n", USB->DEVICE.CTRLB.reg);
}

void usb_detach(void)
{
    /* Datasheet is not clear whether device starts detached */
    USB->DEVICE.CTRLB.reg |= USB_DEVICE_CTRLB_DETACH;
}

void isr_usb(void)
{
    NVIC_DisableIRQ(USB_IRQn);
    _usbdev->usbdev.cb(&_usbdev->usbdev, USBDEV_EVENT_ESR);
}

int usbdev_get(usbdev_t *usbdev, usbopt_t opt, void *value, size_t max_len)
{
    (void)usbdev;
    (void)opt;
    (void)value;
    (void)max_len;
    return -ENOTSUP;
}

int usbdev_set(usbdev_t *usbdev, usbopt_t opt, const void *value, size_t value_len)
{
    sam0_common_usb_t *dev = (sam0_common_usb_t*) usbdev;

    (void)dev;
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case USBOPT_ADDRESS:
            break;
        case USBOPT_ATTACH:
            assert(value_len >= sizeof(usbopt_enable_t));
            if (*((usbopt_enable_t*)value)) {
                usb_attach();
            }
            else {
                usb_detach();
            }
            res = sizeof(usbopt_enable_t);
            break;
        default:
            break;
    }
    return res;
}

void usbdev_esr(usbdev_t *dev)
{
    (void)dev;
    if (USB->DEVICE.INTFLAG.reg) {
    }
    else if (USB->DEVICE.EPINTSMRY.reg) {
    }
    /* Re-enable the USB IRQ */
    NVIC_EnableIRQ(USB_IRQn);
}

const usbdev_driver_t driver = {
    .init = usbdev_init,
    .get = usbdev_get,
    .set = usbdev_set,
    .esr = usbdev_esr,
};
