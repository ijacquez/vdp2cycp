#include <sys/cdefs.h>

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <byteswap.h>

#include "vdp2cycp.h"

#include "math.h"
#include "debug.h"

/* Table representing number of VRAM accesses required for pattern name
 * data. */
static const int8_t _timings_count_pnd[3][3] = {
        /* Invalid */
        {
                -1,
                -1,
                -1,
        },
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
 * pattern data. */
static const int8_t _timings_count_cell_cpd[5][3] = {
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
                -1,     /* Invalid */
                -1,     /* Invalid */
                -1      /* Invalid */
        },
        /* Character color count: 16,770,000 (RGB) */
        {
                -1,     /* Invalid */
                -1,     /* Invalid */
                -1      /* Invalid */
        }
};

/* Table representing number of VRAM accesses required for character
 * pattern data. */
static const int8_t _timings_count_bitmap_cpd[5][3] = {
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

/* Table representing range of access timings for normal TV screen mode.
 *
 * For example, if T0 is selected as the pattern name data access
 * timing, the range T0,T1,T2 and T4,T5,T6,T7 can be selected for
 * character pattern data.
 */
static const uint8_t _timings_range_normal[8] = {
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

/* Table representing range of access timings for hi-res TV screen mode. */
static const uint8_t _timings_range_hires[8] = {
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

/* Table representing range of access timings for vertical cell scrolling.
 *
 * Note, access for NBG0 and NBG1 must be by the same bank, and NBG0
 * access must be selected first.
 */
static const uint8_t _timings_range_vcs[2] = {
        /* NBG0 */
        0x03,
        /* NBG1 */
        0x07
};

static void state_init(struct state *) __unused;
static void state_configs_build(struct state *, struct scrn_format *) __unused;

static int pnd_bitmap_calculate(struct scrn_format *) __unused;
static bool pnd_bitmap_validate(const struct state *state, uint8_t) __unused;

int
main(int argc __unused, char *argv[] __unused)
{
        DEBUG_PRINTF("sizeof(union vram_cycp): %lu bytes(s)\n", sizeof(union vram_cycp));
        DEBUG_PRINTF("sizeof(struct scrn_cell_format): %lu byte(s)\n", sizeof(struct scrn_cell_format));
        DEBUG_PRINTF("sizeof(struct scrn_bitmap_format): %lu byte(s)\n", sizeof(struct scrn_bitmap_format));
        DEBUG_PRINTF("sizeof(_timings_count_pnd): %lu byte(s)\n", sizeof(_timings_count_pnd));
        DEBUG_PRINTF("sizeof(_timings_count_cell_cpd): %lu byte(s)\n", sizeof(_timings_count_cell_cpd));
        DEBUG_PRINTF("sizeof(_timings_count_bitmap_cpd): %lu byte(s)\n", sizeof(_timings_count_bitmap_cpd));
        DEBUG_PRINTF("sizeof(_timings_range_normal): %lu byte(s)\n", sizeof(_timings_range_normal));
        DEBUG_PRINTF("sizeof(_timings_range_hires): %lu byte(s)\n", sizeof(_timings_range_hires));
        DEBUG_PRINTF("sizeof(_timings_range_vcs): %lu byte(s)\n", sizeof(_timings_range_vcs));

        struct state state;

        state_init(&state);

        /* XXX: Place holder */
        state.ramctl = 0x03000;

        struct scrn_format configs[SCRN_COUNT];
        struct scrn_cell_format config_nbg0;
        struct scrn_cell_format config_nbg1;
        struct scrn_cell_format config_nbg2;
        struct scrn_cell_format config_nbg3;

        memset(&configs, 0x00, sizeof(configs));
        memset(&config_nbg0, 0x00, sizeof(config_nbg0));
        memset(&config_nbg1, 0x00, sizeof(config_nbg1));
        memset(&config_nbg2, 0x00, sizeof(config_nbg2));
        memset(&config_nbg3, 0x00, sizeof(config_nbg3));

        configs[0].sf_scroll_screen = SCRN_NBG0;
        configs[0].sf_format = SCRN_FORMAT_CELL;
        configs[0].sf_cc_count = SCRN_CCC_PALETTE_16;
        configs[0].sf_config = &config_nbg0;

        config_nbg0.scf_pnd_size = 1;
        config_nbg0.scf_cp_table = VRAM_ADDR_4MBIT(0, 0x00000);
        config_nbg0.scf_vcs_table = VRAM_ADDR_4MBIT(0, 0x00000);
        config_nbg0.scf_reduction = SCRN_REDUCTION_NONE;
        config_nbg0.scf_map.plane_a = VRAM_ADDR_4MBIT(2, 0x00000);
        config_nbg0.scf_map.plane_b = VRAM_ADDR_4MBIT(2, 0x00000);
        config_nbg0.scf_map.plane_c = VRAM_ADDR_4MBIT(3, 0x00000);
        config_nbg0.scf_map.plane_d = VRAM_ADDR_4MBIT(3, 0x00000);

        /* XXX:
         * Loop through every screen and calculate PND bit-map. This
         * should be done when configuring VDP2 cell format */
        uint32_t i;
        for (i = 0; i < SCRN_COUNT; i++) {
                pnd_bitmap_calculate(&configs[i]);
        }

        DEBUG_FORMAT(&configs[0]);

        state_configs_build(&state, configs);

        union vram_cycp vram_cycp;

        int error;
        error = vdp2cycp(&state, SCRN_NBG0, &vram_cycp);
        DEBUG_PRINTF("vdp2cycp: %i\n", error);

        return 0;
}

/*-
 * Calculate VDP2 VRAM cycle patterns given selected VDP2 screens SCRNS.
 *
 * If successful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - STATE is NULL
 *   - SCRNS has an invalid screen
 *   - VRAM_CYCP is NULL
 *   - Case 4
 *   - Case 5
 *   - Case 6
 *   - Case 7
 *   - Case 8
 */
int
vdp2cycp(struct state *state, uint8_t scrns, union vram_cycp *vram_cycp)
{
        if (state == NULL) {
                return -1;
        }

        if (vram_cycp == NULL) {
                return -1;
        }

        if (scrns == 0x00) {
                return -1;
        }

        if ((scrns & 0xC0) != 0x00) {
                return -1;
        }

        /* Go in order: NBG0, NBG1, NBG2, then NBG3 */

        uint32_t scrn;

        /* Bitwise OR it and validate PND bit-maps */
        uint8_t pnd_bitmap;
        pnd_bitmap = 0x00;

        for (scrn = 0; scrn < state->cell_count; scrn++) {
                if ((scrns & (1 << scrn)) == 0) {
                        continue;
                }

                const struct scrn_format *config;
                config = state->cell_configs[scrn];

                if (config == NULL) {
                        continue;
                }

                const struct scrn_cell_format *cell_config;
                cell_config = config->sf_config;

                pnd_bitmap |= cell_config->priv_pnd_bitmap;
        }

        if ((state->cell_count > 0) && (!(pnd_bitmap_validate(state, pnd_bitmap)))) {
                return -1;
        }

        for (scrn = 0; scrn < state->cell_count; scrn++) {
                if ((scrns & (1 << scrn)) == 0) {
                        continue;
                }

                const struct scrn_format *config;
                config = state->cell_configs[scrn];

                if (config == NULL) {
                        continue;
                }

                const struct scrn_cell_format *cell_config;
                cell_config = config->sf_config;

                /* Determine how many PND access timings are needed (due
                 * to reduction) */
                int8_t tpnd;
                tpnd = _timings_count_pnd[cell_config->scf_pnd_size][cell_config->scf_reduction];

                /* Invalid number of PND access timings */
                if (tpnd < 0) {
                        return -1;
                }

                /* Determine how many CPD access timings are needed (due
                 * to reduction) and character color count */
                int8_t tcpd;
                tcpd = _timings_count_cell_cpd[config->sf_cc_count][cell_config->scf_reduction];

                /* Invalid number of PND access timings */
                if (tcpd < 0) {
                        return -1;
                }

                DEBUG_PRINTF("tpnd: %i access timing required\n", tpnd);
                DEBUG_PRINTF("tcpd: %i access timing required\n", tcpd);
        }

        /* Determine if VCS is being used for NBG0 and/or NBG1? */
        // XXX: Determine in which VRAM bank VCST is stored in
        // XXX: Is there a restriction as to where VCST can be stored?

        /* Allocate PND access timing and store timing */

        // XXX: For example, If 1/4 reduction is used, PNDT should be on
        //      different banks, as well as VCST because there just
        //      isn't enough access timings available. 1/4 reduction
        //      requires 4.

        /* Use timing from PND to determine range where CPD access
         * timing should be and allocate */

        // XXX: Allocation function should take in a mask on the "constraint"
        // XXX: Allocation function should take which bank to allocate access timing on

        return 0;
}

/*-
 * Initialize HW state.
 */
static void
state_init(struct state *state)
{
        if (state == NULL) {
                return;
        }

        memset(state->cell_configs, 0x00, sizeof(state->cell_configs));
        state->cell_count = 0;

        memset(state->bitmap_configs, 0x00, sizeof(state->bitmap_configs));
        state->bitmap_count = 0;
}

/*-
 * Given an array of configurations, Generate two table look ups of cell
 * and bitmap configs, respectively.
 */
static void
state_configs_build(struct state *state, struct scrn_format *configs)
{
        if (configs == NULL) {
                return;
        }

        state->cell_count = 0;
        state->bitmap_count = 0;

        uint32_t scrn;
        for (scrn = 0; scrn < SCRN_COUNT; scrn++) {
                struct scrn_format *config;
                config = &configs[scrn];

                switch (config->sf_format) {
                case SCRN_FORMAT_CELL:
                        state->cell_configs[scrn] = config;
                        state->cell_count++;
                        break;
                case SCRN_FORMAT_BITMAP:
                        state->bitmap_configs[scrn] = config;
                        state->bitmap_count++;
                        break;
                }
        }
}

/*-
 * Generate an 8-bit bit-map BITMAP of where pattern name data is stored
 * amongst the 4 banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is return
 * for the following cases:
 *
 *   - CONFIG is NULL
 *   - CONFIG doesn't point to valid format
 *   - CONFIG is not a cell format
 */
static int
pnd_bitmap_calculate(struct scrn_format *config)
{
#define BANK_BIT(b) (1 << (4 - (b) - 1))

        if (config == NULL) {
                return -1;
        }

        if (config->sf_config == NULL) {
                return -1;
        }

        if (config->sf_format != SCRN_FORMAT_CELL) {
                return -1;
        }

        struct scrn_cell_format *cell_config;
        cell_config = config->sf_config;

        cell_config->priv_pnd_bitmap = 0x00;

        uint8_t bank;

        switch (config->sf_scroll_screen) {
        case SCRN_NBG0:
        case SCRN_NBG1:
        case SCRN_NBG2:
        case SCRN_NBG3:
                /* XXX: 4 or 8-Mbit? */
                bank = VRAM_BANK_4MBIT(cell_config->scf_map.planes[0]);
                cell_config->priv_pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(cell_config->scf_map.planes[1]);
                cell_config->priv_pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(cell_config->scf_map.planes[2]);
                cell_config->priv_pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(cell_config->scf_map.planes[3]);
                cell_config->priv_pnd_bitmap |= BANK_BIT(bank);

                return 0;
        case SCRN_RBG1:
        case SCRN_RBG0: {
                uint32_t i;
                for (i = 0; i < 16; i++) {
                        /* XXX: 4 or 8-Mbit? */
                        bank = VRAM_BANK_4MBIT(cell_config->scf_map.planes[i]);

                        cell_config->priv_pnd_bitmap |= BANK_BIT(bank);
                }
        } return 0;
        default:
                return -1;
        }

#undef BANK_BIT
}

/*-
 * Validate 8-bit bit-map BITMAP, which represent where pattern name data
 * is stored amongst the 4 VRAM banks.
 *
 * If successful, true is returned. Otherwise, false.
 */
static bool
pnd_bitmap_validate(const struct state *state, uint8_t bitmap)
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
         * The bit-map is composed of 6 bits.
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
         * +---------------------------+
         * | Bit                       |
         * +-------+-------------------+
         * | 5   4 |  3    2    1    0 |
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

#define VALIDATE_BITMAP(bitmap, bank, i)                                       \
        ((((uint8_t)(bitmap) & pnd_bank_mask[(bank)][1 + (i)]) != 0x00))

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
        bank = (state->ramctl & 0x0300) >> 8;

        uint32_t mask_count;
        mask_count = pnd_bank_mask[bank][0];

        bool valid;
        valid = false;

        uint32_t i;
        for (i = 0; i < (mask_count / 2); i++) {
                valid = valid || VALIDATE_BITMAP(bitmap, bank, i);
                valid = valid || VALIDATE_BITMAP(bitmap, bank, i + 1);
        }

        return valid;

#undef VALIDATE_BITMAP
}
