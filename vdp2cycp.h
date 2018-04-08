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

int pnd_bitmap_calculate(uint8_t *, const struct scrn_cell_format *);
bool pnd_bitmap_validate(uint16_t, uint8_t);

int vdp2cycp(uint32_t, vram_cycp *);

#endif /* !VDP2CYCP_H_ */
