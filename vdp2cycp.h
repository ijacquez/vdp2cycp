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

        struct scroll_screen {
                struct scrn_format format;
                uint8_t pnd_bitmap;
                uint8_t vcs_bitmap;
        };

        struct scroll_screen nbg0;
        struct scroll_screen nbg1;
        struct scroll_screen nbg2;
        struct scroll_screen nbg3;
        struct scroll_screen rbg0;
        struct scroll_screen rbg1;

        struct scroll_screen *scroll_screens[SCRN_COUNT];
};

int32_t vdp2cycp(const struct state *);

#endif /* !VDP2CYCP_H_ */
