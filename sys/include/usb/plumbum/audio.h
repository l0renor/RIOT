/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    usb_plumbum_audio Plubmum USB audio 3.0 interface
 * @ingroup     usb_plumbum
 *
 * @{
 *
 * @file
 * @brief       Plumbum USB audio 3.0 stack
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef USB_PLUMBUM_AUDIO_H
#define USB_PLUMBUM_AUDIO_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "usb/plumbum.h"

typedef enum {
    AUDIO_BLOCK_TYPE_TERMINAL_INPUT,
    AUDIO_BLOCK_TYPE_TERMINAL_OUTPUT,
    AUDIO_BLOCK_TYPE_CLOCK,
    AUDIO_BLOCK_TYPE_FEATURE,
} plumbum_audio_block_type_t;

typedef enum {
    PLUMBUM_AUDIO_CLOCK_EXTERNAL = 0x0,
    PLUMBUM_AUDIO_CLOCK_INTERNAL_FIXED = 0x1,
    PLUMBUM_AUDIO_CLOCK_INTERNAL_VARIABLE = 0x2,
    PLUMBUM_AUDIO_CLOCK_INTERNAL_PROGRAMMABLE = 0x3,
} plumbum_audio_clocktype_t;

typedef struct {
    plumbum_handler_t handler;
    struct plumbum_audio_block *blocks; /**< Building blocks used by the audio function */
    plumbum_t *plumbum;
    plumbum_hdr_gen_t assoc_hdr;
    plumbum_hdr_gen_t control_hdr;
    plumbum_hdr_gen_t stream_hdr;
    plumbum_hdr_gen_t stream_ep_hdr;
    plumbum_interface_t control;
    plumbum_interface_t stream;
    plumbum_interface_alt_t stream_alt;
    plumbum_endpoint_t stream_ep;
} plumbum_audio_t;

typedef struct plumbum_audio_block {
    struct plumbum_audio_block *next;   /**< Next block in the linked list */
    plumbum_audio_block_type_t type;    /**< Block type e.g. terminal, feature */
    uint8_t id;                         /**< Unique ID of this block */
} plumbum_audio_block_t;

typedef struct {
    plumbum_audio_block_t block;
    plumbum_audio_clocktype_t type;
} plumbum_audio_block_clock_t;

typedef struct {
    plumbum_audio_block_t block;
    plumbum_audio_block_clock_t *clock;
    uint16_t type;
    uint8_t channels;
} plumbum_audio_block_input_t;

typedef struct {
    plumbum_audio_block_t block;
    plumbum_audio_block_clock_t *clock;
    plumbum_audio_block_t *source;
    uint16_t type;
} plumbum_audio_block_output_t;


int plumbum_audio_init(plumbum_t *plumbum, plumbum_audio_t *audio);

int plumbum_audio_add_clock(plumbum_audio_t *audio,
        plumbum_audio_block_clock_t *clock,
        plumbum_audio_clocktype_t type);

int plumbum_audio_add_input(plumbum_audio_t *audio, plumbum_audio_block_input_t *input,
        uint16_t type);
int plumbum_audio_add_output(plumbum_audio_t *audio, plumbum_audio_block_output_t *output,
        uint16_t type);
int plumbum_audio_add_block(plumbum_audio_t *audio,
                            plumbum_audio_block_t *block);
#ifdef __cplusplus
}
#endif
#endif /* USB_PLUMBUM_AUDIO_H */
/** @} */
