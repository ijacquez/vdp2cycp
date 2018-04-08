/*
 * Copyright (c) 2012-2016 Israel Jacquez
 * See LICENSE for details.
 *
 * Israel Jacquez <mrkotfw@gmail.com>
 */

#ifndef VDP2_H_
#define VDP2_H_

#define SCRN_NBG0               0 /* Normal background (NBG0) */
#define SCRN_RBG1               5 /* Rotational background (RBG1) */
#define SCRN_NBG1               1 /* Normal background (NBG1) */
#define SCRN_NBG2               2 /* Normal background (NBG2) */
#define SCRN_NBG3               3 /* Normal background (NBG3) */
#define SCRN_RBG0               4 /* Rotational background (RBG0) */

#define SCRN_CCC_PALETTE_16     0
#define SCRN_CCC_PALETTE_256    1
#define SCRN_CCC_PALETTE_2048   2
#define SCRN_CCC_RGB_32768      3
#define SCRN_CCC_RGB_16770000   4

#define VRAM_ADDR_4MBIT(x, y)   (0x25E00000 + ((x) << 17) + (y))

struct scrn_cell_format {
        uint32_t scf_scroll_screen; /* Normal/rotational background */
        uint32_t scf_cc_count; /* Character color count */
        uint32_t scf_cp_table; /* Character pattern table lead address*/

        struct {
                uint32_t plane_a;
                uint32_t plane_b;
                uint32_t plane_c;
                uint32_t plane_d;
                uint32_t plane_e; /* For RBG0 and RBG1 use only */
                uint32_t plane_f; /* For RBG0 and RBG1 use only */
                uint32_t plane_g; /* For RBG0 and RBG1 use only */
                uint32_t plane_h; /* For RBG0 and RBG1 use only */
                uint32_t plane_i; /* For RBG0 and RBG1 use only */
                uint32_t plane_j; /* For RBG0 and RBG1 use only */
                uint32_t plane_k; /* For RBG0 and RBG1 use only */
                uint32_t plane_l; /* For RBG0 and RBG1 use only */
                uint32_t plane_m; /* For RBG0 and RBG1 use only */
                uint32_t plane_n; /* For RBG0 and RBG1 use only */
                uint32_t plane_o; /* For RBG0 and RBG1 use only */
                uint32_t plane_p; /* For RBG0 and RBG1 use only */
        } scf_map; /* Map lead addresses */
};

#endif /* !VDP2_H_ */
