/*
 * Copyright (c) 2012-2016 Israel Jacquez
 * See LICENSE for details.
 *
 * Israel Jacquez <mrkotfw@gmail.com>
 */

#ifndef VDP2CYCP_H_
#define VDP2CYCP_H_

#include <stdint.h>
#include <stdbool.h>

#include "vdp2.h"

struct state {
        uint16_t ramctl;
        union vram_cycp vram_cycp;

        struct scrn_format format_nbg0;
        struct scrn_format format_nbg1;
        struct scrn_format format_nbg2;
        struct scrn_format format_nbg3;
        struct scrn_format format_rbg0;
        struct scrn_format format_rbg1;

        struct scrn_format *formats[SCRN_COUNT];
};

int vdp2cycp(const struct state *);

#endif /* !VDP2CYCP_H_ */
