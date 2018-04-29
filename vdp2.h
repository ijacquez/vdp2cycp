/*
 * Copyright (c) 2012-2016 Israel Jacquez
 * See LICENSE for details.
 *
 * Israel Jacquez <mrkotfw@gmail.com>
 */

#ifndef VDP2_H_
#define VDP2_H_

#include <sys/cdefs.h>

#include <stdint.h>
#include <stdbool.h>

#define SCRN_NBG0               0 /* Normal background */
#define SCRN_NBG1               1 /* Normal background */
#define SCRN_NBG2               2 /* Normal background */
#define SCRN_NBG3               3 /* Normal background */
#define SCRN_RBG0               4 /* Rotational background */
#define SCRN_RBG1               5 /* Rotational background */
#define SCRN_EXBG               7 /* External input screen  */
#define SCRN_COUNT              6

#define SCRN_TYPE_CELL          0
#define SCRN_TYPE_BITMAP        1

#define SCRN_REDUCTION_NONE     0 /* No reduction */
#define SCRN_REDUCTION_HALF     1 /* 1/2 reduction */
#define SCRN_REDUCTION_QUARTER  2 /* 1/4 reduction */

#define SCRN_CCC_PALETTE_16     0
#define SCRN_CCC_PALETTE_256    1
#define SCRN_CCC_PALETTE_2048   2
#define SCRN_CCC_RGB_32768      3
#define SCRN_CCC_RGB_16770000   4

#define VRAM_ADDR_4MBIT(x, y)   (0x25E00000 + ((x) << 17) + (y))

#define VRAM_BANK_4MBIT(x)      (((x) >> 17) & 0x0007)

/* Determine if address is in VDP2 VRAM */
#define VRAM_BANK_ADDRESS(x)    ((((x) >> 20) & 0x000000FF) == 0x5E)

struct scrn_bitmap_format {
        struct {
                uint16_t width;
                uint16_t height;
        } sbf_bitmap_size;              /* Bitmap sizes
                                         * 512x256
                                         * 512x512
                                         * 1024x256,
                                         * 1024x512 */
        uint32_t sbf_color_palette;     /* Color palette lead address (if
                                         * applicable) */
        uint32_t sbf_bitmap_pattern;    /* Bitmap pattern lead address */

        uint8_t sbf_rp_mode;            /* RBG0 and RBG1 only
                                         *
                                         * Rotation parameter mode
                                         *   Mode 0: Rotation Parameter A
                                         *   Mode 1: Rotation Parameter B
                                         *   Mode 2: Swap Coefficient Data Read
                                         *   Mode 3: Swap via Rotation Parameter Window */
};

#ifdef SCRN_CELL_FORMAT_TEST_IMPL
struct scrn_cell_format_test_impl {
        struct {
                unsigned int scf_character_size:1;
                unsigned int scf_pnd_size:1;
                unsigned int scf_auxiliary_mode:1;
                unsigned int scf_reduction:2;
                unsigned int scf_plane_size:2;
                unsigned int scf_rp_mode:2;
                unsigned int reserved:23;
        } __packed;

        uint32_t scf_cp_table;          /* Character pattern table lead address */
        uint32_t scf_color_palette;     /* Color palette lead address */

        uint32_t scf_vcs_table;         /* Vertical cell scroll table lead address */
};
#endif /* SCRN_CELL_FORMAT_TEST_IMPL */

struct scrn_cell_format {
        uint8_t scf_character_size;     /* Character size: (1 * 1) or (2 * 2) cells */
        uint8_t scf_pnd_size;           /* Pattern name data size:
                                         * (1)-word
                                         * (2)-words */
        uint8_t scf_auxiliary_mode;     /* Auxiliary mode #0 (flip function)
                                         * auxiliary mode #1 (no flip function) */

        uint32_t scf_cp_table;          /* Character pattern table lead address */
        uint32_t scf_color_palette;     /* Color palette lead address */

        uint8_t scf_plane_size;         /* Plane size (1 * 1)
                                         * Plane size (2 * 1)
                                         * Plane size (2 * 2) */

        uint8_t scf_rp_mode;            /* RBG0 and RBG1 only
                                         *
                                         * Rotation parameter mode
                                         *   Mode 0: Rotation Parameter A
                                         *   Mode 1: Rotation Parameter B
                                         *   Mode 2: Swap Coefficient Data Read
                                         *   Mode 3: Swap via Rotation Parameter Window */

        union {
                uint32_t planes[16];

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
                } __packed;
        } scf_map;                      /* Map lead addresses */

        /* uint8_t priv_pnd_bitmap; /\* Holds the pattern name data bitmap *\/ */
};

struct scrn_format {
        bool sf_enable;
        uint8_t sf_scroll_screen;       /* Normal/rotational background */
        uint8_t sf_type;                /* Cell format type
                                         * Bitmap format type */
        uint8_t sf_cc_count;            /* Character color count */
        uint32_t sf_vcs_table;          /* Vertical cell scroll table lead address */
        uint8_t sf_reduction;           /* Background reduction
                                         * 1
                                         * 1/2 reduction
                                         * 1/4 reduction */

        union {
                struct scrn_cell_format cell;
                struct scrn_bitmap_format bitmap;
        } sf_format;
};

#define VRAM_CTL_CYCP_PNDR_NBG0         0x0 /* NBG0 pattern name data read */
#define VRAM_CTL_CYCP_PNDR_NBG1         0x1 /* NBG1 pattern name data read */
#define VRAM_CTL_CYCP_PNDR_NBG2         0x2 /* NBG2 pattern name data read */
#define VRAM_CTL_CYCP_PNDR_NBG3         0x3 /* NBG3 pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG0       0x4 /* NBG0 character pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG1       0x5 /* NBG1 character pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG2       0x6 /* NBG2 character pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG3       0x7 /* NBG3 character pattern name data read */
#define VRAM_CTL_CYCP_VCSTDR_NBG0       0xC /* NBG0 vertical cell scroll table data read */
#define VRAM_CTL_CYCP_VCSTDR_NBG1       0xD /* NBG1 vertical cell scroll table data read */
#define VRAM_CTL_CYCP_CPU_RW            0xE /* CPU read/write */
#define VRAM_CTL_CYCP_NO_ACCESS         0xF /* No access */

/* Calculate starting bit for T */
#define VRAM_CTL_CYCP_TIMING_BIT(x)     (((x) & 0x7) << 2)

/* Calculate 32-bit timing mask for T */
#define VRAM_CTL_CYCP_TIMING_MASK(t)    ((0x0000000F) << VRAM_CTL_CYCP_TIMING_BIT(t))

/* Extract timing value T from raw 32-bit cycle pattern value */
#define VRAM_CTL_CYCP_TIMING_VALUE(pv, t)                                      \
        (((uint32_t)(pv) & VRAM_CTL_CYCP_TIMING_MASK(t)) >> VRAM_CTL_CYCP_TIMING_BIT(t))

union vram_cycp {
        /*-
         * To byte swap (Big-endian):
         *
         * __bswap_32(t1.pv[0])
         */

        uint32_t pv[4]; /* VRAM cycle pattern value */

        struct {
                unsigned int t0:4;
                unsigned int t1:4;
                unsigned int t2:4;
                unsigned int t3:4;
                unsigned int t4:4;
                unsigned int t5:4;
                unsigned int t6:4;
                unsigned int t7:4;
        } __packed pt[4];
};

#endif /* !VDP2_H_ */
