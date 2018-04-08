/*
 * Copyright (c) 2012-2016 Israel Jacquez
 * See LICENSE for details.
 *
 * Israel Jacquez <mrkotfw@gmail.com>
 */

#ifndef VDP2_H_
#define VDP2_H_

#define SCRN_NBG0               (1 << 0) /* Normal background (NBG0) */
#define SCRN_RBG1               (1 << 5) /* Rotational background (RBG1) */
#define SCRN_NBG1               (1 << 1) /* Normal background (NBG1) */
#define SCRN_NBG2               (1 << 2) /* Normal background (NBG2) */
#define SCRN_NBG3               (1 << 3) /* Normal background (NBG3) */
#define SCRN_RBG0               (1 << 4) /* Rotational background (RBG0) */

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

struct scrn_cell_format {
        uint32_t scf_scroll_screen; /* Normal/rotational background */
        uint32_t scf_cc_count; /* Character color count */
        uint32_t scf_cp_table; /* Character pattern table lead address*/

        uint32_t scf_reduction; /* Background reduction */

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
        } scf_map; /* Map lead addresses */

        uint8_t priv_pnd_bitmap; /* Holds the pattern name data bitmap */
};

/*-
 * General guideline for number of accesses required
 *
 *   1 - Pattern name data (1-word or 2-words)
 *   1 - 16-color character pattern or bitmapped data
 *   2 - 256-color character pattern or bitmapped data
 *   4 - 2048-color character pattern or bitmapped data
 *   4 - 32K-color character pattern or bitmapped data
 *   8 - 16M-color character pattern or bitmapped data
 *   1 - Vertical cell scroll table data
 */

#define VRAM_CTL_CYCP_PNDR_NBG0         0x0 /* NBG0 pattern name data read */
#define VRAM_CTL_CYCP_PNDR_NBG1         0x1 /* NBG1 pattern name data read */
#define VRAM_CTL_CYCP_PNDR_NBG2         0x2 /* NBG2 pattern name data read */
#define VRAM_CTL_CYCP_PNDR_NBG3         0x3 /* NBG3 pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG0       0x4 /* NBG0 character pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG1       0x5 /* NBG1 character pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG2       0x6 /* NBG2 character pattern name data read */
#define VRAM_CTL_CYCP_CHPNDR_NBG3       0x7 /* NBG3 character pattern name data read */
#define VRAM_CTL_CYCP_VCSTDR_NBG0       0xC /* NBG0 vertical cell scroll table data read */
#define VRAM_CTL_CYCP_VCSTDR_NBG1       0xD /* NBG0 vertical cell scroll table data read */
#define VRAM_CTL_CYCP_CPU_RW            0xE /* CPU read/write */
#define VRAM_CTL_CYCP_NO_ACCESS         0xF /* No access */

/* Calculate starting bit for T */
#define VRAM_CTL_CYCP_TIMING_BIT(x)     (((x) & 0x7) << 2)

/* Calculate timing mask for T */
#define VRAM_CTL_CYCP_TIMING_MASK(t)    ((0x0000000F) << VRAM_CTL_CYCP_TIMING_BIT(t))

/* Extract timing value T */
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
                unsigned int t0:4; /* Timing T0 */
                unsigned int t1:4; /* Timing T1 */
                unsigned int t2:4; /* Timing T2 */
                unsigned int t3:4; /* Timing T3 */
                unsigned int t4:4; /* Timing T4 */
                unsigned int t5:4; /* Timing T5 */
                unsigned int t6:4; /* Timing T6 */
                unsigned int t7:4; /* Timing T7 */
        } __packed pt[4];
};

#endif /* !VDP2_H_ */
