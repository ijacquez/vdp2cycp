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
        /* XXX: Place holder */
        uint16_t ramctl;

        struct scrn_format *cell_configs[SCRN_COUNT];
        uint8_t cell_count;

        struct scrn_format *bitmap_configs[SCRN_COUNT];
        uint8_t bitmap_count;
};

int vdp2cycp(struct state *, uint8_t, union vram_cycp *);

#endif /* !VDP2CYCP_H_ */
