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

#include "bitarithm.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static sam0_common_usb_t *_usbdev;

static UsbDeviceDescBank banks[16];

static usbdev_ep_t endpoints[16];

int usbdev_ep_init(usbdev_ep_t *ep);
int usbdev_ep_get(usbdev_ep_t *ep, usbopt_ep_t opt, void *value, size_t max_len);
int usbdev_ep_set(usbdev_ep_t *ep, usbopt_ep_t opt, const void *value, size_t value_len);
int usbdev_ep_ready(usbdev_ep_t *ep, size_t len);
void usbdev_ep_esr(usbdev_ep_t *ep);

const usbdev_ep_driver_t driver_ep = {
    .init = usbdev_ep_init,
    .get = usbdev_ep_get,
    .set = usbdev_ep_set,
    .esr = usbdev_ep_esr,
    .ready = usbdev_ep_ready,
};

static inline unsigned _get_ep_num(unsigned num, usbdev_dir_t dir)
{
    return 2*num + (dir == USBDEV_DIR_OUT ? 0 : 1);
}

static inline usbdev_ep_t* _get_ep(unsigned num, usbdev_dir_t dir)
{
    return &endpoints[_get_ep_num(num, dir)];
}

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
    /* Set GPIO */
    gpio_init(GPIO_PIN(PA,24), GPIO_IN);
    gpio_init(GPIO_PIN(PA,25), GPIO_IN);
    gpio_init_mux(GPIO_PIN(PA, 24), GPIO_MUX_G);
    gpio_init_mux(GPIO_PIN(PA, 25), GPIO_MUX_G);
    poweron();
    /* Reset peripheral */
    USB->DEVICE.CTRLA.reg |= USB_CTRLA_SWRST;
    while(usb_swrst_syncing()) {}
    while(USB->DEVICE.CTRLA.bit.SWRST) {}

    /* Enable USB device */
    USB->DEVICE.DESCADD.reg = (uint32_t)banks;
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

    USB->DEVICE.CTRLB.bit.SPDCONF = 0x0;

    /* Interrupt configuration */
    USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST;
    NVIC_EnableIRQ(USB_IRQn);

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
    if (USB->DEVICE.EPINTSMRY.reg) {
        unsigned ep_num = bitarithm_lsb(USB->DEVICE.EPINTSMRY.reg);
        UsbDeviceEndpoint *ep_reg = &USB->DEVICE.DeviceEndpoint[ep_num];
        if (ep_reg->EPINTFLAG.bit.RXSTP || ep_reg->EPINTFLAG.bit.TRCPT0 || ep_reg->EPINTFLAG.bit.TRFAIL0)
        {
            endpoints[ep_num].cb(&endpoints[ep_num + 1], USBDEV_EVENT_ESR);
        }
        else if (ep_reg->EPINTFLAG.bit.TRCPT1 || ep_reg->EPINTFLAG.bit.TRFAIL1)
        {
            endpoints[ep_num + 1].cb(&endpoints[ep_num + 1], USBDEV_EVENT_ESR);
        }
    }
    else {
        _usbdev->usbdev.cb(&_usbdev->usbdev, USBDEV_EVENT_ESR);
    }
}

int usbdev_get(usbdev_t *usbdev, usbopt_t opt, void *value, size_t max_len)
{
    sam0_common_usb_t *dev = (sam0_common_usb_t*) usbdev;

    (void)dev;
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case USBOPT_EP0_IN:
            {
                assert(max_len == sizeof(usbdev_ep_t*));
                usbdev_ep_t *ep = _get_ep(0, USBDEV_DIR_IN);
                ep->dir = USBDEV_DIR_IN;
                ep->num = 0;
                ep->type = USBDEV_EP_TYPE_CONTROL;
                ep->cb = NULL;
                ep->driver = &driver_ep;
                *(usbdev_ep_t**)value = ep;
                res = sizeof(usbdev_ep_t*);
            }
            break;
        case USBOPT_EP0_OUT:
            {
                assert(max_len == sizeof(usbdev_ep_t*));
                usbdev_ep_t *ep = _get_ep(0, USBDEV_DIR_OUT);
                ep->dir = USBDEV_DIR_OUT;
                ep->num = 0;
                ep->type = USBDEV_EP_TYPE_CONTROL;
                ep->cb = NULL;
                ep->driver = &driver_ep;
                *(usbdev_ep_t**)value = ep;
                res = sizeof(usbdev_ep_t*);
            }
            break;
        default:
            break;
    }
    return res;
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
            assert(value_len == sizeof(uint8_t));
            uint8_t addr = (*((uint8_t*)value));
            USB->DEVICE.DADD.bit.DADD = addr;
            if (addr) {
                USB->DEVICE.DADD.bit.ADDEN = 1;
            }
            else {
                USB->DEVICE.DADD.bit.ADDEN = 0;
            }
            break;
        case USBOPT_ATTACH:
            assert(value_len == sizeof(usbopt_enable_t));
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

static void _ep_disable(usbdev_ep_t *ep)
{
    UsbDeviceEndpoint *ep_reg = &USB->DEVICE.DeviceEndpoint[ep->num];
    if (ep->dir == USBDEV_DIR_OUT) {
        ep_reg->EPCFG.bit.EPTYPE0 = 0;
    }
    else {
        ep_reg->EPCFG.bit.EPTYPE1 = 0;
    }
}

static void _ep_enable(usbdev_ep_t *ep)
{
    UsbDeviceEndpoint *ep_reg = &USB->DEVICE.DeviceEndpoint[ep->num];
    uint8_t type = 0;
    switch (ep->type) {
       case USBDEV_EP_TYPE_CONTROL:
           type = 0x01;
           break;
       case USBDEV_EP_TYPE_ISOCHRONOUS:
           type = 0x02;
           break;
       case USBDEV_EP_TYPE_BULK:
           type = 0x03;
           break;
       case USBDEV_EP_TYPE_INTERRUPT:
           type = 0x04;
           break;
    }
    if (ep->dir == USBDEV_DIR_OUT) {
        ep_reg->EPCFG.bit.EPTYPE0 = type;
    }
    else {
        ep_reg->EPCFG.bit.EPTYPE1 = type;
    }
}

void usbdev_esr(usbdev_t *dev)
{
    (void)dev;
    if (USB->DEVICE.INTFLAG.reg & USB->DEVICE.INTENSET.reg) {
        if (USB->DEVICE.INTFLAG.bit.EORST) {
            /* Clear flag */
            USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST;
            usbdev_ep_init(&endpoints[0]);
            _ep_enable(&endpoints[0]);
            usbdev_ep_init(&endpoints[1]);
            _ep_enable(&endpoints[1]);
            _usbdev->usbdev.cb(&_usbdev->usbdev, USBDEV_EVENT_RESET);
        }
        /* Re-enable the USB IRQ */
    NVIC_EnableIRQ(USB_IRQn);
    }
}


void _ep_address(usbdev_ep_t *ep, char* buf)
{
    UsbDeviceDescBank *bank = &banks[_get_ep_num(ep->num, ep->dir)];
    bank->ADDR.reg = (uint32_t)buf;
}

void _ep_size(usbdev_ep_t *ep, size_t size)
{
    UsbDeviceDescBank *bank = &banks[_get_ep_num(ep->num, ep->dir)];
    unsigned val = 0x00;
    switch(size) {
        case 8:
            val = 0x0;
            break;
        case 16:
            val = 0x1;
            break;
        case 32:
            val = 0x2;
            break;
        case 64:
            val = 0x3;
            break;
        case 128:
            val = 0x4;
            break;
        case 256:
            val = 0x5;
            break;
        case 512:
            val = 0x6;
            break;
        case 1023:
            val = 0x7;
            break;
        default:
            return;
    }
    bank->PCKSIZE.bit.SIZE = val;
}

int usbdev_ep_init(usbdev_ep_t *ep)
{
    /* Enable interrupt for endpoint */
    UsbDeviceEndpoint *ep_reg = &USB->DEVICE.DeviceEndpoint[ep->num];
    if (ep->dir == USBDEV_DIR_OUT) {
        ep_reg->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0 | USB_DEVICE_EPINTENSET_TRFAIL0;
        if (ep->num == 0) {
            ep_reg->EPINTENSET.reg = USB_DEVICE_EPINTENSET_RXSTP;
        }
    }
    else {
        ep_reg->EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT1 | USB_DEVICE_EPINTENSET_TRFAIL1;
    }
    DEBUG("ep_init: 0x%x\n", ep_reg->EPSTATUS.reg);
    return 0;
}

int usbdev_ep_get(usbdev_ep_t *ep, usbopt_ep_t opt, void *value, size_t max_len)
{
    (void)ep;
    (void)value;
    (void)max_len;
    int res = -ENOTSUP;
    assert(ep != NULL);
    switch (opt) {
        case USBOPT_EP_ENABLE:
        case USBOPT_EP_BUF_ADDR:
            break;
        default:
            break;
    }
    return res;
}

int usbdev_ep_set(usbdev_ep_t *ep, usbopt_ep_t opt, const void *value, size_t value_len)
{
    (void)ep;
    (void)value;
    (void)value_len;
    int res = -ENOTSUP;
    assert(ep != NULL);
    switch (opt) {
        case USBOPT_EP_ENABLE:
            assert(value_len == sizeof(usbopt_enable_t));
            if (*((usbopt_enable_t *)value)) {
                _ep_enable(ep);
            }
            else {
                _ep_disable(ep);
            }
            break;
        case USBOPT_EP_BUF_ADDR:
            assert(value_len == sizeof(uint8_t*));
            _ep_address(ep, *((char**)value));
            res = sizeof(char*);
            break;
        case USBOPT_EP_BUF_SIZE:
            assert(value_len == sizeof(size_t));
            _ep_size(ep, *((size_t*)value));
            break;
        default:
            break;
    }
    return res;
}

int usbdev_ep_ready(usbdev_ep_t *ep, size_t len)
{
    UsbDeviceDescBank *bank = &banks[_get_ep_num(ep->num, ep->dir)];
    UsbDeviceEndpoint *ep_reg = &USB->DEVICE.DeviceEndpoint[ep->num];
    if (ep->dir == USBDEV_DIR_IN) {
        bank->PCKSIZE.bit.BYTE_COUNT = len;
        ep_reg->EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK1RDY;
    }
    else {
        ep_reg->EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;
    }
    return 0;
}

void usbdev_ep_esr(usbdev_ep_t *ep)
{
    UsbDeviceEndpoint *ep_reg = &USB->DEVICE.DeviceEndpoint[ep->num];
    DEBUG("st %s :%x - %x\n", ep->dir == USBDEV_DIR_OUT ? "out" : "in", ep_reg->EPSTATUS.reg, ep_reg->EPINTFLAG.reg);
    signed event = -1;
    if (ep->dir == USBDEV_DIR_OUT) {
        if (ep_reg->EPINTFLAG.bit.TRCPT0) {
            ep_reg->EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT0;
            event = USBDEV_EVENT_TR_COMPLETE;
        }
        else if (ep_reg->EPINTFLAG.bit.RXSTP) {
            event = USBDEV_EVENT_RX_SETUP;
            ep_reg->EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;
        }
        else if (ep_reg->EPINTFLAG.bit.TRFAIL0) {
            ep_reg->EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
            DEBUG("tf0\n");
        }
    }
    else {
        if (ep_reg->EPINTFLAG.bit.TRCPT1) {
            ep_reg->EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1;
            event = USBDEV_EVENT_TR_COMPLETE;
        }
        else if (ep_reg->EPINTFLAG.bit.TRFAIL1) {
            ep_reg->EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL1;
            DEBUG("tf1\n");
        }
    }
    if (event >= 0) {
        DEBUG("cb called\n");
        ep->cb(ep, event);
    }
    NVIC_EnableIRQ(USB_IRQn);
}

const usbdev_driver_t driver = {
    .init = usbdev_init,
    .get = usbdev_get,
    .set = usbdev_set,
    .esr = usbdev_esr,
};
