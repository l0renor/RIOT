/*
 * Copyright (C) 2018 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

#ifndef USB_USBDEV_H
#define USB_USBDEV_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct usbdev {
} usbdev_t;

/**
 * @brief   Possible event types that are send from the device driver to the
 *          upper layer
 */
typedef enum {
    USBDEV_EVENT_ISR,                       /**< driver needs it's ISR handled */
    USBDEV_EVENT_OUT_READY,                 /**< endpoint out data  ready */
    /* expand this list if needed */
} usbdev_event_t;

typedef enum {
    USBDEV_EP_TYPE_CONTROL,
    USBDEV_EP_TYPE_INTERRUPT,
    USBDEV_EP_TYPE_BULK,
    USBDEV_EP_TYPE_ISOCHRONOUS,
    USBDEV_EP_TYPE_ASYNCRONOUS,
} usbdev_ep_type_t;

typedef struct usbdev_ep {
    uint8_t dir;
} usbdev_ep_t;

typedef struct usbdev_ep_pair {
    usbdev_ep_t out;
    usbdev_ep_t in;
    uint8_t num;                /* Endpoint pair numbers */
    usb_ep_type_t type;         /* Endpoint type (e.g. bulk, interrupt) */
} usbdev_ep_pair_t;


typedef struct {
    int (*init)(usbdev_ep_pair_t ep);

    /**
     * @brief   Get an option value from a given usb device endpoint
     *
     * @pre `(dev != NULL)`
     *
     * @param[in]   dev     network device descriptor
     * @param[in]   opt     option type
     * @param[out]  value   pointer to store the option's value in
     * @param[in]   max_len maximal amount of byte that fit into @p value
     *
     * @return              number of bytes written to @p value
     * @return              `< 0` on error, 0 on success
     */
    int (*get)(usbdev_ep_pair_t *ep, usbopt_t opt,
               void *value, size_t max_len);

    /**
     * @brief   Set an option value for a given usb device endpoint
     *
     * @pre `(dev != NULL)`
     *
     * @param[in] dev       network device descriptor
     * @param[in] opt       option type
     * @param[in] value     value to set
     * @param[in] value_len the length of @p value
     *
     * @return              number of bytes used from @p value
     * @return              `< 0` on error, 0 on success
     */
    int (*set)(usbdev_ep_pair_t *dev, netopt_t opt,
               const void *value, size_t value_len);
    /**
     * @brief a driver's user-space ISR handler
     *
     * @pre `(dev != NULL)`
     *
     * This function will be called from a network stack's loop when being
     * notified by netdev_isr.
     *
     * It is supposed to call
     * @ref netdev_t::event_callback "netdev->event_callback()" for each
     * occurring event.
     *
     * See receive packet flow description for details.
     *
     * @param[in]   dev     network device descriptor
     */
    void (*isr)(usbdev_ep_t *dev);

    /**
     * @brief Signal out data buffer (Host to device) ready for new data
     */
    int (*out_ready)(usbdev_ep_pair_t *ep);

    /**
     * @brief Signal in data buffer (device to host) ready for reading.
     */
    int (*in_ready)(usbdev_ep_pair_t *ep);
} usbdev_ep_driver_t;
/**
 * activate pull up to indicate device connected
 */
int usbdev_attach(usbdev_t *dev);

/**
 * deactivate pull up to indicate device connected
 */
int usbdev_detach(usbdev_t *dev);

/**
 * Get an USB endpoint struct of the indicated type
 *
 * @returns         NULL if the endpoint type is not available.
 */
usbdev_ep_pair_t *usbdev_get_ep(usbdev_t *dev, usb_ep_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* USB_USBDEV_H */
/** @} */
