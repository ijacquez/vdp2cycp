#include <sys/cdefs.h>

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <byteswap.h>

#include "vdp2cycp.h"
#include "debug.h"

#ifdef DEBUG
#define DEBUG_PATTERN(pv) do {                                                 \
        char *_output_buffer;                                                  \
        _output_buffer = debug_print_pattern(pv);                              \
        DEBUG_PRINTF("%s", _output_buffer);                                    \
        free(_output_buffer);                                                  \
} while (false)
#else
#define DEBUG_PATTERN(...)
#endif /* DEBUG */

static const char *_timing_mnemonics[] __unused = {
        "PNDR_NBG0",    /* 0x0 */
        "PNDR_NBG1",    /* 0x1 */
        "PNDR_NBG2",    /* 0x2 */
        "PNDR_NBG3",    /* 0x3 */
        "CHPNDR_NBG0",  /* 0x4 */
        "CHPNDR_NBG1",  /* 0x5 */
        "CHPNDR_NBG2",  /* 0x6 */
        "CHPNDR_NBG3",  /* 0x7 */
        "---",
        "---",
        "---",
        "---",
        "VCSTDR_NBG0",  /* 0xC */
        "VCSTDR_NBG1",  /* 0xD */
        "CPU_RW",       /* 0xE */
        "NO_ACCESS",    /* 0xF */
        NULL
};

#ifdef DEBUG
static char *debug_print_pattern(uint32_t) __unused;
#endif /* DEBUG */

/* Table representing number of VRAM accesses required for pattern name
 * data */
static const int8_t _timing_count_pnd[2][3] = {
        /* PND 1-word */
        {
                1,      /* No reduction */
                2,      /* 1/2 reduction */
                4       /* 1/4 reduction */
        },
        /* PND 2-word */
        {
                1,      /* No reduction */
                2,      /* 1/2 reduction */
                4       /* 1/4 reduction */
        }
};

/* Table representing number of VRAM accesses required for character
 * pattern data */
static const int8_t _timing_count_cpd[5][3] = {
        /* Character color count: 16 (palette) */
        {
                 1,     /* No reduction */
                 2,     /* 1/2 reduction */
                 4      /* 1/4 reduction */
        },
        /* Character color count: 256 (palette) */
        {
                 2,     /* No reduction */
                 4,     /* 1/2 reduction */
                -1      /* 1/4 reduction (invalid) */
        },
        /* Character color count: 2048 (palette)*/
        {
                 4,     /* No reduction */
                -1,     /* 1/2 reduction (invalid) */
                -1      /* 1/4 reduction (invalid) */
        },
        /* Character color count: 32,768 (RGB) */
        {
                 4,     /* No reduction */
                -1,     /* 1/2 reduction (invalid) */
                -1      /* 1/4 reduction (invalid) */
        },
        /* Character color count: 16,770,000 (RGB) */
        {
                 8,     /* No reduction */
                -1,     /* 1/2 reduction (invalid) */
                -1      /* 1/4 reduction (invalid) */
        }
};

/* Table representing range of access timings for normal TV screen mode
 *
 * Example:
 * If T0 is selected as the pattern name data access timing, the range
 * T0,T1,T2 and T4,T5,T6,T7 can be selected for character pattern data
 */
static const uint8_t _timing_range_normal[8] = {
        /* T0 -> T0, T1, T2, T4, T5, T6, T7 */
        0xF7,
        /* T1 -> T0, T1, T2, T3, T5, T6, T7 */
        0xEF,
        /* T2 -> T0, T1, T2, T3, T6, T7 */
        0xCF,
        /* T3 -> T0, T1, T2, T3, T7 */
        0x8F,
        /* T4 -> T0, T1, T2, T3 */
        0x0F,
        /* T5 -> T1, T2, T3 */
        0x0E,
        /* T6 -> T2, T3 */
        0x0C,
        /* T7 -> T3 */
        0x08
};

/* Table representing range of access timings for hi-res TV screen mode
 *
 * Example:
 * If T0 is selected as the pattern name data access timing, the range
 * T0,T1,T2 and T4,T5,T6,T7 can be selected for character pattern data
 */
static const uint8_t _timing_range_hires[8] = {
        /* T0 -> T0, T1, T2 */
        0x07,
        /* T1 -> T1, T2, T3 */
        0x0E,
        /* T2 -> T0, T2, T3 */
        0x0D,
        /* T3 -> T0, T1, T3 */
        0x0B,
        /* T4 -> Invalid */
        0x00,
        /* T5 -> Invalid */
        0x00,
        /* T6 -> Invaild */
        0x00,
        /* T7 -> Invalid */
        0x00
};

/* Table representing range of access timings for vertical cell scrolling
 *
 * Note, access for NBG0 and NBG1 must be by the same bank, and NBG0
 * access must be selected first
 */
static const uint8_t _timing_range_vcs[2] = {
        /* NBG0 */
        0x03,
        /* NBG1 */
        0x07
};

static int pnd_bitmap_calculate(struct scrn_cell_format *);
static bool pnd_bitmap_validate(uint16_t, uint8_t);

int
main(int argc __unused, char *argv[] __unused)
{
        DEBUG_PRINTF("sizeof(union vram_cycp): %lu bytes(s)\n", sizeof(union vram_cycp));
        DEBUG_PRINTF("sizeof(struct scrn_cell_format): %lu byte(s)\n", sizeof(struct scrn_cell_format));
        DEBUG_PRINTF("sizeof(_timing_count_pnd): %lu byte(s)\n", sizeof(_timing_count_pnd));
        DEBUG_PRINTF("sizeof(_timing_count_cpd): %lu byte(s)\n", sizeof(_timing_count_cpd));
        DEBUG_PRINTF("sizeof(_timing_range_normal): %lu byte(s)\n", sizeof(_timing_range_normal));
        DEBUG_PRINTF("sizeof(_timing_range_hires): %lu byte(s)\n", sizeof(_timing_range_hires));
        DEBUG_PRINTF("sizeof(_timing_range_vcs): %lu byte(s)\n", sizeof(_timing_range_vcs));

        struct scrn_cell_format configs[4];
        memset(&configs, 0x00, sizeof(configs));
        union vram_cycp vram_cycp;

        vdp2cycp(SCRN_NBG0, configs, &vram_cycp);

        /* config_nbg0.scf_scroll_screen = SCRN_NBG0; */
        /* config_nbg0.scf_map.plane_a = VRAM_ADDR_4MBIT(2, 0x00000); */
        /* config_nbg0.scf_map.plane_b = VRAM_ADDR_4MBIT(2, 0x00000); */
        /* config_nbg0.scf_map.plane_c = VRAM_ADDR_4MBIT(3, 0x00000); */
        /* config_nbg0.scf_map.plane_d = VRAM_ADDR_4MBIT(3, 0x00000); */

        /* pnd_bitmap_validate(0x0300, bitmap); */

        return 0;
}

/*-
 * Calculate VDP2 VRAM cycle patterns given selected VDP2 screens SCRNS.
 *
 * If successful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - VRAM_CYCP is NULL
 *   - SCRNS has an invalid screen
 *   - CONFIGS is NULL
 *   - Case 4
 *   - Case 5
 *   - Case 6
 *   - Case 7
 *   - Case 8
 */
int
vdp2cycp(uint32_t scrns, const struct scrn_cell_format *configs, union vram_cycp *vram_cycp)
{
        if (vram_cycp == NULL) {
                return -1;
        }

        if (configs == NULL) {
                return -1;
        }

        if (scrns == 0x00000000) {
                return -1;
        }

        if ((scrns & 0xFFFFFFC0) != 0x00000000) {
                return -1;
        }

        /* Loop through every screen and calculate PND bitmap. Bitwise OR
         * it and validate it */

        /* Go in order: NBG0, NBG1, NBG2, then NBG3 */

        /* Determine if VCS is being used for NBG0 and/or NBG1? */
        // XXX: Determine in which VRAM bank VCST is stored in
        // XXX: Is there a restriction as to where VCST can be stored?

        /* Determine how many PND access timings are needed (due to reduction) */

        /* Allocate PND access timing and store timing */

        // XXX: For example, If 1/4 reduction is used, PNDT should be on
        //      different banks, as well as VCST

        /* Use timing from PND to determine range where CPD access
         * timing should be and allocate */

        // XXX: Allocation function should take in a mask on the "constraint"
        // XXX: Allocation function should take which bank

        return 0;
}

/*-
 * Generate an 8-bit bitmap BITMAP of where pattern name data is stored
 * amongst the 4 banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is return
 * for the following cases:
 *
 *   - CONFIG is NULL
 */
static int
pnd_bitmap_calculate(struct scrn_cell_format *config)
{
#define BANK_BIT(b) (1 << (4 - (b) - 1))

        if (config == NULL) {
                return -1;
        }

        config->priv_pnd_bitmap = 0x00;

        uint8_t bank;

        switch (config->scf_scroll_screen) {
        case SCRN_NBG0:
        case SCRN_NBG1:
        case SCRN_NBG2:
        case SCRN_NBG3:
                /* XXX: 4 or 8-Mbit? */
                bank = VRAM_BANK_4MBIT(config->scf_map.planes[0]);
                config->priv_pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(config->scf_map.planes[1]);
                config->priv_pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(config->scf_map.planes[2]);
                config->priv_pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(config->scf_map.planes[3]);
                config->priv_pnd_bitmap |= BANK_BIT(bank);

                return 0;
        case SCRN_RBG1:
        case SCRN_RBG0: {
                uint32_t i;
                for (i = 0; i < 16; i++) {
                        /* XXX: 4 or 8-Mbit? */
                        bank = VRAM_BANK_4MBIT(config->scf_map.planes[i]);

                        config->priv_pnd_bitmap |= BANK_BIT(bank);
                }
        } return 0;
        default:
                return -1;
        }

#undef BANK_BIT
}

/*-
 * Validate 8-bit bitmap BITMAP, which represent where pattern name data
 * is stored amongst the 4 VRAM banks.
 *
 * If successful, true is returned. Otherwise, false.
 */
static bool
pnd_bitmap_validate(uint16_t ramctl, uint8_t bitmap)
{
        /*-
         * Where the pattern name data can be placed:
         * +--------+--------+--------+--------+--------+--------+
         * | VRAM A | VRAM B | A0     | A1     | B0     | B1     |
         * +--------+--------+--------+--------+--------+--------+
         * | 0      | 0      | YYYYYYYYYYYYYYY | NNNNNNNNNNNNNNN |
         * |        |        | NNNNNNNNNNNNNNN | YYYYYYYYYYYYYYY |
         * +--------+--------+--------+--------+-----------------+
         * | 1      | 0      | NNNNNN | YYYYYY | YYYYYYYYYYYYYYY |
         * |        |        | YYYYYY | YYYYYY | NNNNNNNNNNNNNNN |
         * +--------+--------+--------+--------+--------+--------+
         * | 0      | 1      | YYYYYYYYYYYYYYY | NNNNNN | YYYYYY |
         * |        |        | NNNNNNNNNNNNNNN | YYYYYY | YYYYYY |
         * +--------+--------+--------+--------+--------+--------+
         * | 1      | 1      | YYYYYY | YYYYYY | NNNNNN | NNNNNN |
         * |        |        | YYYYYY | NNNNNN | NNNNNN | YYYYYY |
         * |        |        | NNNNNN | YYYYYY | YYYYYY | NNNNNN |
         * |        |        | NNNNNN | NNNNNN | YYYYYY | YYYYYY |
         * +--------+--------+--------+--------+--------+--------+ */

        /*-
         * The bitmap is composed of 6 bits.
         *
         * The first 4 represent the four VRAM banks A0, A1, B0, and
         * B1.
         *
         * The upper 2 of the 6 bits represent whether bank A and/or
         * bank B are split, respectively.
         *
         * The bits for each bank represent where pattern name data can
         * be stored.
         *
         * +-------+----+----+----+----+-------+------+
         * | Bank  | A0 | A1 | B0 | B1 | Value | Mask |
         * +-------+----+----+----+----+-------+------+
         * | 0   0 | 1    1    0    0  | 0x0C  | 0x33 |
         * +-------+-------------------+--------------+
         * | 0   0 | 0    0    1    1  | 0x03  | 0x3C |
         * +-------+-------------------+--------------+
         *
         * +-------+-------------------+--------------+
         * | 1   0 | 0    1    1    1  | 0x27  | 0x18 |
         * +-------+-------------------+--------------+
         * | 1   0 | 1    1    0    0  | 0x2C  | 0x13 |
         * +-------+-------------------+--------------+
         *
         * +-------+-------------------+--------------+
         * | 0   1 | 1    1    0    1  | 0x0D  | 0x22 |
         * +-------+-------------------+--------------+
         * | 0   1 | 0    0    1    1  | 0x13  | 0x2C |
         * +-------+-------------------+--------------+
         *
         * +-------+-------------------+--------------+
         * | 1   1 | 1    1    0    0  | 0x3C  | 0x03 |
         * +-------+-------------------+--------------+
         * | 1   1 | 1    0    0    1  | 0x39  | 0x06 |
         * +-------+-------------------+--------------+
         * | 1   1 | 0    1    1    0  | 0x36  | 0x09 |
         * +-------+-------------------+--------------+
         * | 1   1 | 0    0    1    1  | 0x33  | 0x0C |
         * +-------+-------------------+--------------+
         */

        static const uint8_t pnd_bank_mask[4][5] = {
                {
                        2, /* Count */
                        0x33,
                        0x3C,
                        0xFF,
                        0xFF
                }, {
                        2, /* Count */
                        0x18,
                        0x13,
                        0xFF,
                        0xFF
                }, {
                        2, /* Count */
                        0x22,
                        0x2C,
                        0xFF,
                        0xFF
                }, {
                        4, /* Count */
                        0x03,
                        0x06,
                        0x09,
                        0x0C
                }
        };

        /* XXX: This would move elsewhere */
        uint8_t bank;
        bank = (ramctl & 0x0300) >> 8;

        uint32_t mask_count;
        mask_count = pnd_bank_mask[bank][0];

        uint32_t i;
        for (i = 0; i < (mask_count / 2); i++) {
                if ((bitmap & pnd_bank_mask[bank][1 + i]) != 0x00) {
                        return false;
                }

                if ((bitmap & pnd_bank_mask[bank][2 + i]) != 0x00) {
                        return false;
                }
        }

        return true;
}

#ifdef DEBUG
static char *
debug_print_pattern(uint32_t pv)
{
        char *output_buffer;

        /* Header: 8 strings of length 11, each spaced out (7 spaces), plus newline*/
        uint32_t line;
        line = (8 * 11) + 7 + 1;

        /* Two lines plus extra newline and NUL byte */
        uint32_t bytes;
        bytes = (2 * line) + 1 + 1;

        output_buffer = malloc(bytes);
        assert(output_buffer != NULL);
        memset(output_buffer, '\0', bytes);

        (void)sprintf(output_buffer,
            "\n"
            "%-11s %-11s %-11s %-11s %-11s %-11s %-11s %-11s\n"
            "%-11s %-11s %-11s %-11s %-11s %-11s %-11s %-11s\n",
            "T7",
            "T6",
            "T5",
            "T4",
            "T3",
            "T2",
            "T1",
            "T0",
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 7)],
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 6)],
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 5)],
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 4)],
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 3)],
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 2)],
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 1)],
            _timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 0)]);

        return output_buffer;
}
#endif /* DEBUG */
