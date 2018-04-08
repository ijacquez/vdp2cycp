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

int vdp2cycp(uint32_t, const struct scrn_cell_format *, union vram_cycp *);

#endif /* !VDP2CYCP_H_ */
