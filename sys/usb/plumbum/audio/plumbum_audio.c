/*
 * Copyright (C) 2018 Koen Zandberg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup usb_plumbum_audio Plumbum USB audio device class 2.0 stack
 * @{
 * @file
 * @brief   Plumbum USB audio stack, responsible for interface management
 *
 * @author  Koen Zandberg <koen@bergzand.net>
 * @}
 */

#include <string.h>
#include "usb/plumbum/audio.h"
#include "usb/plumbum.h"
#include "usb/audio.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

static int _init(plumbum_t *plumbum, plumbum_handler_t *handler);
static int event_handler(plumbum_t *plumbum, plumbum_handler_t *handler, uint16_t event, void *arg);

const plumbum_handler_driver_t audiov3_driver = {
    .init = _init,
    .event_handler = event_handler,
};

/* Temp functions to ease development */
int plumbum_audio_add_clock(plumbum_audio_t *audio,
        plumbum_audio_block_clock_t *clock,
        plumbum_audio_clocktype_t type)
{
    clock->block.type = AUDIO_BLOCK_TYPE_CLOCK;
    clock->type = type;
    return plumbum_audio_add_block(audio, (plumbum_audio_block_t*)clock);
}

int plumbum_audio_add_input(plumbum_audio_t *audio, plumbum_audio_block_input_t *input,
        uint16_t type)
{
    input->block.type = AUDIO_BLOCK_TYPE_TERMINAL_INPUT;
    input->type = type;
    return plumbum_audio_add_block(audio, (plumbum_audio_block_t*)input);
}

int plumbum_audio_add_output(plumbum_audio_t *audio, plumbum_audio_block_output_t *output,
        uint16_t type)
{
    output->block.type = AUDIO_BLOCK_TYPE_TERMINAL_OUTPUT;
    output->type = type;
    return plumbum_audio_add_block(audio, (plumbum_audio_block_t*)output);
}

uint8_t _get_new_id(plumbum_audio_t *audio)
{
    uint8_t id = 1;
    for(plumbum_audio_block_t *blk = audio->blocks;
            blk; blk = blk->next) {
        if(id <= blk->id) {
            id = blk->id + 1;
        }
    }
    return id;
}

int plumbum_audio_add_block(plumbum_audio_t *audio,
                            plumbum_audio_block_t *block)
{
    block->id = _get_new_id(audio);
    block->next = audio->blocks;
    audio->blocks = block;
    return block->id;
}

int plumbum_audio_init(plumbum_t *plumbum, plumbum_audio_t *audio)
{
    audio->plumbum = plumbum; 
    audio->handler.driver = &audiov3_driver;
    plumbum_register_event_handler(plumbum, (plumbum_handler_t *)audio);
    return 0;
}

size_t _audio_assoc_descriptor(plumbum_t *plumbum, void *arg)
{
    plumbum_audio_t *audio = (plumbum_audio_t *)arg;
    usb_descriptor_interface_association_t descr;
    descr.length = sizeof(usb_descriptor_interface_association_t);
    descr.type = USB_TYPE_DESCRIPTOR_INTERFACE_ASSOC;
    descr.first_interface = audio->control.idx;
    descr.interface_count = 2;
    descr.class = USB_CLASS_AUDIO;
    descr.subclass = 0x00; /* audio subclass undefined */
    descr.protocol = USB_AUDIO_PROTOCOL_V2;  
    descr.idx = 0;
    plumbum_put_bytes(plumbum, (uint8_t*)&descr, sizeof(descr));
    return sizeof(descr);
}

size_t _audio_assoc_descriptor_size(plumbum_t *plumbum, void *arg)
{
    (void)plumbum;
    (void)arg;
    return sizeof(usb_descriptor_interface_association_t);
}

size_t _audio_control_block_size(plumbum_audio_block_t *block)
{
    switch(block->type) {
        case AUDIO_BLOCK_TYPE_TERMINAL_INPUT:
            return sizeof(usb_descriptor_ac2_input_terminal_t);
        case AUDIO_BLOCK_TYPE_TERMINAL_OUTPUT:
            return sizeof(usb_descriptor_ac2_output_terminal_t);
        case AUDIO_BLOCK_TYPE_CLOCK:
            return sizeof(usb_descriptor_ac2_clock_t);
        default:
            return 0;
    }
}

static size_t _audio_descriptor_input(plumbum_audio_t *audio, plumbum_audio_block_input_t *input)
{
    usb_descriptor_ac2_input_terminal_t in_descr;
    memset(&in_descr, 0, sizeof(in_descr));

    in_descr.length = sizeof(in_descr);
    in_descr.type = USB_AUDIO_CS_INTERFACE;
    in_descr.subtype = USB_AUDIO_AC_SUBTYPE_INPUT_TERMINAL;
    in_descr.terminalid = input->block.id;
    in_descr.terminaltype = input->type;
    in_descr.assocterminal = 0; /* Not supported at the moment */
    in_descr.clocksourceid = input->clock->block.id;
    in_descr.nrchannels = 2;
    in_descr.channelconfig = 0;
    in_descr.channelidx = 0;
    in_descr.controls = 0;
    in_descr.terminalidx = 0;
    plumbum_put_bytes(audio->plumbum, (uint8_t*)&in_descr, sizeof(in_descr));
    return sizeof(in_descr);
}

static size_t _audio_descriptor_clock(plumbum_audio_t *audio, plumbum_audio_block_clock_t *clock)
{
    usb_descriptor_ac2_clock_t clk_descr;
    memset(&clk_descr, 0, sizeof(clk_descr));

    clk_descr.length = sizeof(clk_descr);
    clk_descr.type = USB_AUDIO_CS_INTERFACE;
    clk_descr.subtype = USB_AUDIO_AC_SUBTYPE_CLOCK_SOURCE;
    clk_descr.clockid = clock->block.id;
    clk_descr.attributes = clock->type;
    clk_descr.assocterminal = 0; /* Not supported at the moment */
    clk_descr.controls = 0;
    clk_descr.idx = 0;
    plumbum_put_bytes(audio->plumbum, (uint8_t*)&clk_descr, sizeof(clk_descr));
    return sizeof(clk_descr);
}

static size_t _audio_descriptor_output(plumbum_audio_t *audio, plumbum_audio_block_output_t *output)
{
    usb_descriptor_ac2_output_terminal_t out_descr;
    memset(&out_descr, 0, sizeof(out_descr));

    out_descr.length = sizeof(out_descr);
    out_descr.type = USB_AUDIO_CS_INTERFACE;
    out_descr.subtype = USB_AUDIO_AC_SUBTYPE_OUTPUT_TERMINAL;
    out_descr.terminalid = output->block.id;
    out_descr.terminaltype = output->type;
    out_descr.assocterminal = 0; /* Not supported at the moment */
    out_descr.sourceid = output->source->id;
    out_descr.clocksourceid = output->clock->block.id;
    out_descr.controls = 0;
    out_descr.terminalidx = 0;
    plumbum_put_bytes(audio->plumbum, (uint8_t*)&out_descr, sizeof(out_descr));
    return sizeof(out_descr);
}

size_t _audio_control_block_descriptor(plumbum_audio_t *audio, plumbum_audio_block_t *block)
{
    switch(block->type) {
        case AUDIO_BLOCK_TYPE_TERMINAL_INPUT:
            return _audio_descriptor_input(audio, (plumbum_audio_block_input_t *)block);
        case AUDIO_BLOCK_TYPE_TERMINAL_OUTPUT:
            return _audio_descriptor_output(audio, (plumbum_audio_block_output_t *)block);
        case AUDIO_BLOCK_TYPE_CLOCK:
            return _audio_descriptor_clock(audio, (plumbum_audio_block_clock_t *)block);
        default:
            return 0;
    }
}

size_t _audio_control_descriptor(plumbum_t *plumbum, void *arg)
{
    plumbum_audio_t *audio = (plumbum_audio_t *)arg;
    usb_descriptor_ac2_interface_t ac_iface;
    size_t len = sizeof(ac_iface);
    ac_iface.length = sizeof(usb_descriptor_ac2_interface_t);
    ac_iface.type = USB_AUDIO_CS_INTERFACE;
    ac_iface.subtype = USB_AUDIO_AC_SUBTYPE_HEADER;
    ac_iface.bcdadc = 0x0200;
    ac_iface.category = 0x0E; /* generic speaker (FIXME) */
    ac_iface.totallength = sizeof(usb_descriptor_ac2_interface_t);
    ac_iface.controls = 0;
    for(plumbum_audio_block_t *blk = audio->blocks;
            blk; blk = blk->next) {
        ac_iface.totallength += _audio_control_block_size(blk);
    }
    plumbum_put_bytes(plumbum, (uint8_t*)&ac_iface, sizeof(ac_iface));
    for(plumbum_audio_block_t *blk = audio->blocks;
            blk; blk = blk->next) {
        len += _audio_control_block_descriptor(audio, blk);
    }

    return len;
}

size_t _audio_control_descriptor_size(plumbum_t *plumbum, void *arg)
{
    (void)plumbum;
    plumbum_audio_t *audio = (plumbum_audio_t *)arg;
    size_t len = sizeof(usb_descriptor_ac2_interface_t);
    for(plumbum_audio_block_t *blk = audio->blocks;
            blk; blk = blk->next) {
        len += _audio_control_block_size(blk);
    }
    return len;
}

size_t _audio_stream_descriptor(plumbum_t *plumbum, void *arg)
{
    size_t len = 0;
    (void)arg;
    usb_descriptor_as2_interface_t descr;
    descr.length = sizeof(usb_descriptor_as2_interface_t);
    descr.type = USB_AUDIO_CS_INTERFACE;
    descr.subtype = USB_AUDIO_AS_SUBTYPE_STREAMING;
    descr.terminallink = 2; /* TODO: dynamic */
    descr.controls = 0x00;
    descr.formattype = 0x01; /* Format 1 */
    descr.formats = 0x01; /* PCM */
    descr.nrchannels = 2;
    descr.channelconf = 0x03;
    descr.channelidx = 0; 
    plumbum_put_bytes(plumbum, (uint8_t*)&descr, sizeof(descr));
    len += sizeof(descr);

    usb_descriptor_as2_format1_t format;
    format.length = sizeof(format);
    format.type = USB_AUDIO_CS_INTERFACE;
    format.subtype = USB_AUDIO_AS_SUBTYPE_FORMAT;
    format.formattype = 0x01;
    format.subslotsize = 3;
    format.bitres = 24;
    plumbum_put_bytes(plumbum, (uint8_t*)&format, sizeof(format));
    len += sizeof(format);
    return len;
}

size_t _audio_stream_descriptor_size(plumbum_t *plumbum, void *arg)
{
    (void)plumbum;
    (void)arg;
    return sizeof(usb_descriptor_as2_interface_t)
        + sizeof(usb_descriptor_as2_format1_t);
}

size_t _audio_stream_ep_descriptor(plumbum_t *plumbum, void *arg)
{
    (void)arg;
    usb_descriptor_as2_endpoint_t descr;
    descr.length = sizeof(descr);
    descr.type = USB_AUDIO_CS_ENDPOINT;
    descr.subtype = USB_AUDIO_AS_EP_SUBTYPE_EP_GENERAL;
    descr.attributes = 0x00;
    descr.controls = 0;
    descr.lockdelayunits = 0;
    descr.lockdelay = 0;
    plumbum_put_bytes(plumbum, (uint8_t*)&descr, sizeof(descr));
    return sizeof(descr);

}

size_t _audio_stream_ep_descriptor_size(plumbum_t *plumbum, void *arg)
{
    (void)plumbum;
    (void)arg;
    return sizeof(usb_descriptor_as2_endpoint_t);
}

static void _setup_hdrs(plumbum_audio_t *audio)
{
    (void)audio;
}

static int _init(plumbum_t *plumbum, plumbum_handler_t *handler)
{
    DEBUG("Initializing audio subsystem\n");
    plumbum_audio_t *audio = (plumbum_audio_t*)handler;
    audio->blocks = NULL;
    memset(&audio->control, 0, sizeof(plumbum_interface_t));
    audio->control.class = USB_CLASS_AUDIO;
    audio->control.idx = 1;
    audio->control.subclass = USB_AUDIO_SUBCLASS_CONTROL;
    audio->control.protocol = USB_AUDIO_PROTOCOL_V2;
    audio->control.hdr_gen = &audio->control_hdr;
    audio->control.handler = handler;
    
    memset(&audio->stream, 0, sizeof(plumbum_interface_t));
    audio->stream.class = USB_CLASS_AUDIO;
    audio->stream.subclass = USB_AUDIO_SUBCLASS_STREAM;
    audio->stream.protocol = USB_AUDIO_PROTOCOL_V2;
    audio->stream.idx = 2;
    audio->stream.hdr_gen = NULL;
    audio->stream.handler = handler;
    audio->stream.alts = &audio->stream_alt;
    

    audio->stream_alt.next = NULL;
    audio->stream_alt.hdr_gen = &audio->stream_hdr;
    audio->stream_alt.ep = &audio->stream_ep;
    
    plumbum_add_endpoint(plumbum, &audio->stream, &audio->stream_ep, USB_EP_TYPE_ISOCHRONOUS, USB_EP_DIR_OUT, 512);
    audio->stream.ep = NULL;

    audio->stream_ep.interval = 1;
    audio->stream_ep.hdr_gen = &audio->stream_ep_hdr;

    audio->control_hdr.next = NULL;
    audio->control_hdr.gen_hdr = _audio_control_descriptor;
    audio->control_hdr.hdr_len = _audio_control_descriptor_size;
    audio->control_hdr.arg = audio;
    
    audio->stream_hdr.next = NULL;
    audio->stream_hdr.gen_hdr = _audio_stream_descriptor;
    audio->stream_hdr.hdr_len = _audio_stream_descriptor_size;
    audio->stream_hdr.arg = audio;

    audio->assoc_hdr.gen_hdr = _audio_assoc_descriptor;
    audio->assoc_hdr.hdr_len = _audio_assoc_descriptor_size;
    audio->assoc_hdr.arg = audio;
    
    audio->stream_ep_hdr.next = NULL;
    audio->stream_ep_hdr.gen_hdr = _audio_stream_ep_descriptor;
    audio->stream_ep_hdr.hdr_len = _audio_stream_ep_descriptor_size;
    audio->stream_ep_hdr.arg = audio;
    
    plumbum_add_interface(plumbum, &audio->stream);
    plumbum_add_interface(plumbum, &audio->control);
    plumbum_add_conf_descriptor(plumbum, &audio->assoc_hdr);
    _setup_hdrs(audio);
    return 0;
}

static void _print_setup(usb_setup_t *pkt)
{
    printf("plumbum: setup t:0x%.2x r:0x%x, v:0x%x l:%u\n", pkt->type, pkt->request, pkt->value, pkt->length);
}

static int _handle_setup(plumbum_t *plumbum, plumbum_handler_t *handler, usb_setup_t *pkt)
{
    (void)plumbum;
    (void)handler;
    _print_setup(pkt);
    switch(pkt->request) {
        default:
            return -1;
    }
}

static int event_handler(plumbum_t *plumbum, plumbum_handler_t *handler, uint16_t event, void *arg)
{
    DEBUG("event received\n");
    switch(event) {
            //case PLUMBUM_MSG_EP_EVENT:
        case PLUMBUM_MSG_TYPE_SETUP_RQ:
            return _handle_setup(plumbum, handler, (usb_setup_t*)arg);
        default:
            return -1;
            break;
    }
    return 0;
}

