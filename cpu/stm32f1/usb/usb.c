/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_stm32f1
 * @{
 * @file
 * @brief   USB interface functions
 *
 * ST terminology for the USB device can be a tad confusing as it uses
 * different terms compared to the USB specification. 
 * OUT -> RX
 * IN  -> TX
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <errno.h>

#include "cpu.h"
#include "periph/gpio.h"
#include "usb/usbdev.h"
#include "xtimer.h"

/**
 * @brief masks bits that require a read before write
 */
#define USB_EP_INVARIANT_MASK   (USB_EP_T_FIELD | USB_EP_KIND | USB_EPADDR_FIELD)

void usbdev_ep_init(usbdev_ep_t *ep);
int usbdev_ep_get(usbdev_ep_t *ep, usbopt_ep_t opt, void *value, size_t max_len);
int usbdev_ep_set(usbdev_ep_t *ep, usbopt_ep_t opt, const void *value, size_t value_len);
int usbdev_ep_ready(usbdev_ep_t *ep, size_t len);
void usbdev_ep_esr(usbdev_ep_t *ep);

static inline uint16_t* usb_ep_reg(uint8_t ep)
{
    return (uint16_t*)&USB->EP0R + (2U * ep);
}

/**
 * @brief   returns the value of an enpoint register that can be written to an
 *          endpoint without modifying anything
 *
 * @param[in]   ep  endpoint number
 * @returns         endpoint register invariant value
 */
static inline uint16_t usb_ep_reg_unmodified(uint8_t ep)
{
    return (*usb_ep_reg(ep) & USB_EP_INVARIANT_MASK) | USB_EP_CTR_RX | USB_EP_CTR_TX;
}

static inline uint32_t* usb_ep_pma_txaddr(uint8_t ep)
{
    return (uint32_t*)USB_PMAADDR + (ep * 16U);
}

static inline uint32_t* usb_get_pma_txcount(uint8_t ep)
{
    return (uint32_t*)USB_PMAADDR + (ep * 16U + 4U);
}

static inline uint32_t* usb_get_pma_rxaddr(uint8_t ep)
{
    return (uint32_t*)USB_PMAADDR + (ep * 16U + 8U);
}

static inline uint32_t* usb_get_pma_rxcount(uint8_t ep)
{
    return (uint32_t*)USB_PMAADDR + (ep * 16U + 8U);
}

const usbdev_ep_driver_t driver_ep = {
    .init = usbdev_ep_init,
    .get = usbdev_ep_get,
    .set = usbdev_ep_set,
    .esr = usbdev_ep_esr,
    .ready = usbdev_ep_ready,
};

void usbdev_init(usbdev_t *usbdev)
{
    (void)usbdev;
    /* Enable clock */
    periph_clk_en(APB1, RCC_APB1ENR_USBEN);
    /* Deassert reset */
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;

    /* Deassert power down bit */
    USB->CNTR |= ~ USB_CNTR_PDWN;

    USB->ISTR = 0;

    /* Start of the packet buffer */
    USB->BTABLE = 0;

    /* t_startup delay */
    xtimer_usleep(1);
    /* Datasheet claims that USB pins are automatically connected when device
     * is enabled */
    gpio_init(GPIO_PIN(PORT_B, 9), GPIO_OUT);

    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

static void usb_attach(void)
{
    gpio_set(GPIO_PIN(PORT_B, 9));
}

static void usb_detach(void)
{
    gpio_clear(GPIO_PIN(PORT_B, 9));
}

static void _set_address(uint8_t address)
{
    USB->DADDR = USB_DADDR_EF | address;
}

int usbdev_set(usbdev_t *dev, usbopt_t opt, const void *value, size_t value_len)
{
    (void)dev;
    int res = -ENOTSUP;

    if (dev == NULL) {
        return -ENODEV;
    }

    switch (opt) {
        case USBOPT_ADDRESS:
            assert(value_len == sizeof(uint8_t));
            uint8_t addr = (*((uint8_t*)value));
            _set_address(addr);
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

static inline bool _is_in_stall(usbdev_ep_t *ep)
{
    return ((*usb_ep_reg(ep->num)) & USB_EPTX_STAT) == USB_EP_TX_STALL;
}

static inline bool _is_out_stall(usbdev_ep_t *ep)
{
        return ((*usb_ep_reg(ep->num)) & USB_EPRX_STAT) == USB_EP_RX_STALL;
}

static bool _is_stall(usbdev_ep_t *ep)
{
    if (ep->dir == USB_EP_DIR_IN) {
        return _is_in_stall(ep);
    }
    else {
        return _is_out_stall(ep);
    }
}

static void _set_stall(usbdev_ep_t *ep)
{
    (void)ep;
}

static int _ep_unready(usbdev_ep_t *ep)
{
    /* TODO: check safety of function */
    if (ep->dir == USB_EP_DIR_IN) {
        /* IN direction (TX in ST wording) */
        uint16_t change = USB_EPTX_DTOG1;
        /* Toggle from VALID to NAK */
        *usb_ep_reg(ep->num) = usb_ep_reg_unmodified(ep->num) | change;
    }
    else {
        uint16_t change = USB_EPRX_DTOG1;
        /* Toggle to NAK */
        *usb_ep_reg(ep->num) = usb_ep_reg_unmodified(ep->num) | change;
    }
    return 0;
}

void usbdev_ep_init(usbdev_ep_t *ep)
{
    uint16_t type = ep->type << 9; 
    *usb_ep_reg(ep->num) = usb_ep_reg_unmodified(ep->num) | type | ep->num;
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
            break;
        case USBOPT_EP_BUF_ADDR:
            break;
        case USBOPT_EP_STALL:
            *(usbopt_enable_t*)value = _is_stall(ep) ? USBOPT_ENABLE : USBOPT_DISABLE;
            res = sizeof(usbopt_enable_t);
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
            res = sizeof(char*);
            break;
        case USBOPT_EP_BUF_SIZE:
            assert(value_len == sizeof(size_t));
            break;
        case USBOPT_EP_STALL:
            assert(value_len == sizeof(usbopt_enable_t));
            _ep_set_stall(ep, *(usbopt_enable_t*)value);
            res = sizeof(usbopt_enable_t);
        case USBOPT_EP_READY:
            assert(value_len == sizeof(usbopt_enable_t));
            if (*((usbopt_enable_t *)value)) {
                _ep_unready(ep);
            }
            else {
                usbdev_ep_ready(ep, 0);
            }
            break;
            res = sizeof(usbopt_enable_t);
        default:
            break;
    }
    return res;
}

int usbdev_ep_ready(usbdev_ep_t *ep, size_t len)
{
    uint16_t change = 0;
    if (ep->dir == USB_EP_DIR_IN) {
        change = USB_EPTX_DTOG1;
        /* IN direction (TX in ST wording) */
        if (_is_in_stall(ep)) {
           change = USB_EPTX_DTOG1 | USB_EPTX_DTOG2; 
        }
        *usb_ep_pma_txaddr(ep->num) = len;
        /* Toggle from NAK to VALID */
        *usb_ep_reg(ep->num) = usb_ep_reg_unmodified(ep->num) | change;
    }
    else {
        change = USB_EPRX_DTOG1;
        if (_is_in_stall(ep)) {
           change = USB_EPRX_DTOG1 | USB_EPRX_DTOG2; 
        }
        /* Toggle to VALID */
    }
    *usb_ep_reg(ep->num) = usb_ep_reg_unmodified(ep->num) | change;
    return 0;
}

void isr_usb_lp_can1_rx0(void)
{

}

const usbdev_driver_t driver = {
    .init = usbdev_init,
    .new_ep = usbdev_new_ep,
    .get = usbdev_get,
    .set = usbdev_set,
    .esr = usbdev_esr,
};
