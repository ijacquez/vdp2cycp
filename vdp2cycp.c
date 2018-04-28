#include <sys/cdefs.h>

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <byteswap.h>

#include "vdp2cycp.h"

#include "math.h"
#include "debug.h"

extern const struct scrn_format *bg_formats[];

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

/* Table representing number of VRAM accesses required for vertical cell
 * scroll. */
static const int8_t _timings_count_vcs[2] = {
        /* NBG0 */
        1,
        /* NBG1 */
        1
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

static void state_init(struct state *state, const struct scrn_format **) __unused;

static int pnd_bitmap_calculate(const struct scrn_format *, uint8_t *) __unused;
static int pnd_bitmap_validate(const struct state *, uint8_t) __unused;
static int pnd_bitmap_validate_all(const struct state *);

int
main(int argc __unused, char *argv[] __unused)
{
        DEBUG_PRINTF("sizeof(union vram_cycp): %lu bytes(s)\n", sizeof(union vram_cycp));
        DEBUG_PRINTF("sizeof(struct scrn_format): %lu byte(s)\n", sizeof(struct scrn_format));
        DEBUG_PRINTF("sizeof(struct scrn_cell_format): %lu byte(s)\n", sizeof(struct scrn_cell_format));
        DEBUG_PRINTF("sizeof(struct scrn_bitmap_format): %lu byte(s)\n", sizeof(struct scrn_bitmap_format));
        DEBUG_PRINTF("sizeof(_timings_count_pnd): %lu byte(s)\n", sizeof(_timings_count_pnd));
        DEBUG_PRINTF("sizeof(_timings_count_cell_cpd): %lu byte(s)\n", sizeof(_timings_count_cell_cpd));
        DEBUG_PRINTF("sizeof(_timings_count_bitmap_cpd): %lu byte(s)\n", sizeof(_timings_count_bitmap_cpd));
        DEBUG_PRINTF("sizeof(_timings_count_vcs): %lu byte(s)\n", sizeof(_timings_count_vcs));

        DEBUG_PRINTF("sizeof(_timings_range_normal): %lu byte(s)\n", sizeof(_timings_range_normal));
        DEBUG_PRINTF("sizeof(_timings_range_hires): %lu byte(s)\n", sizeof(_timings_range_hires));
        DEBUG_PRINTF("sizeof(_timings_range_vcs): %lu byte(s)\n", sizeof(_timings_range_vcs));
        DEBUG_PRINTF("sizeof(struct state): %lu byte(s)\n", sizeof(struct state));

        struct state state;

        state_init(&state, bg_formats);

        /* XXX: Place holder */
        state.ramctl = 0x0300;

        if ((pnd_bitmap_validate_all(&state)) < 0) {
                return -1;
        }

        return 0;

        int error;
        error = vdp2cycp(&state);
        DEBUG_PRINTF("vdp2cycp: %i\n", error);

        return -1;
}

/*-
 * Calculate VDP2 VRAM cycle patterns.
 *
 * If successful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - STATE is NULL
 */
int
vdp2cycp(struct state *state)
{
        if (state == NULL) {
                return -1;
        }

        /* Go in order: NBG0, NBG1, NBG2, then NBG3 */

        uint32_t scrn;
        for (scrn = 0; scrn < SCRN_COUNT; scrn++) {
                struct scrn_format *format;
                format = state->formats[scrn];

                if (format == NULL) {
                        continue;
                }

                /* Check if scroll screen is enabled */
                if (!format->sf_enable) {
                        continue;
                }

                const struct scrn_cell_format *cell_format;
                cell_format = &format->sf_format.cell;

                int8_t tvcs;
                tvcs = 0;

                /* Determine if vertical cell scroll is used */
                if (VRAM_BANK_ADDRESS(cell_format->scf_vcs_table)) {
                        tvcs = _timings_count_vcs[format->sf_scroll_screen];
                }

                /* Determine how many PND access timings are needed (due
                 * to reduction) */
                int8_t tpnd;
                tpnd = _timings_count_pnd[cell_format->scf_pnd_size][cell_format->scf_reduction];

                /* Invalid number of PND access timings */
                if (tpnd < 0) {
                        return -1;
                }

                /* Determine how many CPD access timings are needed (due
                 * to reduction) and character color count */
                int8_t tcpd;
                tcpd = _timings_count_cell_cpd[format->sf_cc_count][cell_format->scf_reduction];

                /* Invalid number of PND access timings */
                if (tcpd < 0) {
                        return -1;
                }

                DEBUG_PRINTF("tvcs: %i access timing required\n", tvcs);
                DEBUG_PRINTF("tpnd: %i access timing required\n", tpnd);
                DEBUG_PRINTF("tcpd: %i access timing required\n", tcpd);
        }

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
 * Initialize pseudo HW state via scaffolding.
 */
static void
state_init(struct state *state, const struct scrn_format **formats)
{
        if (state == NULL) {
                return;
        }

        memset(state, 0x00, sizeof(*state));

        if (formats == NULL) {
                return;
        }

        /*  */

        state->formats[0] = &state->format_nbg0;
        state->formats[1] = &state->format_nbg1;
        state->formats[2] = &state->format_nbg2;
        state->formats[3] = &state->format_nbg3;
        state->formats[4] = &state->format_rbg0;
        state->formats[5] = &state->format_rbg1;

        uint32_t i;
        for (i = 0; i < SCRN_COUNT; i++) {
                if (formats[i] == NULL) {
                        break;
                }

                switch (formats[i]->sf_scroll_screen) {
                case SCRN_NBG0:
                        (void)memcpy(&state->format_nbg0, formats[i], sizeof(*formats[i]));
                        break;
                case SCRN_NBG1:
                        (void)memcpy(&state->format_nbg1, formats[i], sizeof(*formats[i]));
                        break;
                case SCRN_NBG2:
                        (void)memcpy(&state->format_nbg2, formats[i], sizeof(*formats[i]));
                        break;
                case SCRN_NBG3:
                        (void)memcpy(&state->format_nbg3, formats[i], sizeof(*formats[i]));
                        break;
                case SCRN_RBG0:
                        (void)memcpy(&state->format_rbg0, formats[i], sizeof(*formats[i]));
                        break;
                case SCRN_RBG1:
                        (void)memcpy(&state->format_rbg1, formats[i], sizeof(*formats[i]));
                        break;
                }
        }
}

/*-
 * Calculate an 8-bit bit-map PND_BITMAP of where pattern name data is
 * stored amongst the 4 banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is return
 * for the following cases:
 *
 *   - -1 PND_BITMAP is NULL
 *   - -2 FORMAT is NULL
 *   - -3 FORMAT is not a cell format
 */
static int
pnd_bitmap_calculate(const struct scrn_format *format, uint8_t *pnd_bitmap)
{
#define BANK_BIT(b) (1 << (4 - (b) - 1))

        if (pnd_bitmap == NULL) {
                return -1;
        }

        *pnd_bitmap = 0x00;

        if (format == NULL) {
                return -2;
        }

        if (format->sf_type != SCRN_TYPE_CELL) {
                return -3;
        }

        if (!format->sf_enable) {
                return 0;
        }

        const struct scrn_cell_format *cell_format;
        cell_format = &format->sf_format.cell;

        switch (format->sf_scroll_screen) {
        case SCRN_NBG0:
        case SCRN_NBG1:
        case SCRN_NBG2:
        case SCRN_NBG3: {
                uint8_t bank;

                /* XXX: 4 or 8-Mbit? */
                bank = VRAM_BANK_4MBIT(cell_format->scf_map.planes[0]);
                *pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(cell_format->scf_map.planes[1]);
                *pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(cell_format->scf_map.planes[2]);
                *pnd_bitmap |= BANK_BIT(bank);

                bank = VRAM_BANK_4MBIT(cell_format->scf_map.planes[3]);
                *pnd_bitmap |= BANK_BIT(bank);
        } break;
        case SCRN_RBG1:
        case SCRN_RBG0:
                break;
        default:
                return -1;
        }

        return 0;

#undef BANK_BIT
}

/*-
 * Validate 8-bit bit-map BITMAP, which represent where pattern name data
 * is stored amongst the 4 VRAM banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is return
 * for the following cases:
 *
 *   - -1 STATE is NULL
 *   - -2 Pattern name data is stored in an invalid bank
 */
static int
pnd_bitmap_validate(const struct state *state, uint8_t pnd_bitmap)
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
         * +-------+----+----+----+----+-------+
         * | Bank  | A0 | A1 | B0 | B1 | Value |
         * +-------+----+----+----+----+-------+
         * | 0   0 | 0    1    0    0  | 0x04  |
         * | 0   0 | 1    0    0    0  | 0x08  |
         * | 0   0 | 1    1    0    0  | 0x0C  |
         * +-------+-------------------+-------+
         * | 0   0 | 0    0    0    1  | 0x01  |
         * | 0   0 | 0    0    1    0  | 0x02  |
         * | 0   0 | 0    0    1    1  | 0x03  |
         * +-------+-------------------+-------+
         *
         * +-------+-------------------+-------+
         * | 1   0 | 0    0    0    1  | 0x21  |
         * | 1   0 | 0    0    1    0  | 0x22  |
         * | 1   0 | 0    0    1    1  | 0x23  |
         * | 1   0 | 0    1    0    0  | 0x24  |
         * | 1   0 | 0    1    0    1  | 0x25  |
         * | 1   0 | 0    1    1    0  | 0x26  |
         * | 1   0 | 0    1    1    1  | 0x27  |
         * +-------+-------------------+-------+
         * | 1   0 | 0    1    0    0  | 0x24  |
         * | 1   0 | 1    0    0    0  | 0x28  |
         * | 1   0 | 1    1    0    0  | 0x2C  |
         * +-------+-------------------+-------+
         *
         * +-------+-------------------+-------+
         * | 0   1 | 0    1    0    0  | 0x14  |
         * | 0   1 | 1    0    0    0  | 0x18  |
         * | 0   1 | 1    1    0    0  | 0x1C  |
         * | 0   1 | 0    0    0    1  | 0x11  |
         * | 0   1 | 0    1    0    1  | 0x15  |
         * | 0   1 | 1    0    0    1  | 0x19  |
         * | 0   1 | 1    1    0    1  | 0x1D  |
         * +-------+-------------------+-------+
         * | 0   1 | 0    0    0    1  | 0x11  |
         * | 0   1 | 0    0    1    0  | 0x12  |
         * | 0   1 | 0    0    1    1  | 0x13  |
         * +-------+-------------------+-------+
         *
         * +-------+-------------------+-------+
         * | 1   1 | 0    1    0    0  | 0x34  |
         * | 1   1 | 1    0    0    0  | 0x38  |
         * | 1   1 | 1    1    0    0  | 0x3C  |
         * +-------+-------------------+-------+
         * | 1   1 | 0    0    0    1  | 0x31  |
         * | 1   1 | 1    0    0    0  | 0x38  |
         * | 1   1 | 1    0    0    1  | 0x39  |
         * +-------+-------------------+-------+
         * | 1   1 | 0    0    1    0  | 0x32  |
         * | 1   1 | 0    1    0    0  | 0x34  |
         * | 1   1 | 0    1    1    0  | 0x36  |
         * +-------+-------------------+-------+
         * | 1   1 | 0    0    0    1  | 0x31  |
         * | 1   1 | 0    0    1    0  | 0x32  |
         * | 1   1 | 0    0    1    1  | 0x33  |
         * +-------+-------------------+-------+
         */

        static const uint8_t pnd_bank_masks[4][10] = {
                /* Bank A: No split
                 * Bank B: No split */
                {
                        0x01,
                        0x02,
                        0x03,
                        0x04,
                        0x08,
                        0x0C
                },
                /* Bank A: Split
                 * Bank B: No split */
                {
                        0x01,
                        0x02,
                        0x03,
                        0x04,
                        0x05,
                        0x06,
                        0x07,
                        0x08,
                        0x0C
                },
                /* Bank A: No split
                 * Bank B: Split */
                {
                        0x01,
                        0x02,
                        0x03,
                        0x04,
                        0x05,
                        0x08,
                        0x09,
                        0x0C,
                        0x0D
                },
                /* Bank A: Split
                 * Bank B: Split */
                {
                        0x01,
                        0x02,
                        0x03,
                        0x04,
                        0x06,
                        0x08,
                        0x09,
                        0x0C
                }
        };

        if (state == NULL) {
                return -1;
        }

        if ((pnd_bitmap & 0x0F) == 0x00) {
                return 0;
        }

        /* XXX: This needs to be moved elsewhere */
        uint8_t bank_config;
        bank_config = (state->ramctl & 0x0300) >> 8;

        uint32_t i;
        for (i = 0; ; i++) {
                uint8_t pnd_bank_mask;
                pnd_bank_mask = pnd_bank_masks[bank_config][i];

                if (pnd_bank_mask == 0x00) {
                        break;
                }

                if ((pnd_bitmap & 0x0F) == pnd_bank_mask) {
                        return 0;
                }
        }

        return -2;
}

/*-
 * Calculate and validate concatenated pattern name bit-map, of all
 * configured scroll screens.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is return
 * for the following cases:
 *
 *   - -1 STATE is NULL
 *   - -2 Pattern name data is stored in an invalid bank
 */
static int
pnd_bitmap_validate_all(const struct state *state)
{
        if (state == NULL) {
                return -1;
        }

        uint8_t pnd_bitmap_all;
        pnd_bitmap_all = 0x00;

        uint32_t scrn;
        for (scrn = 0; scrn < SCRN_COUNT; scrn++) {
                const struct scrn_format *format;
                format = state->formats[scrn];

                if (format == NULL) {
                        continue;
                }

                if (!format->sf_enable) {
                        continue;
                }

                if (format->sf_type != SCRN_TYPE_CELL) {
                        continue;
                }

                DEBUG_PRINTF("*\n");

                uint8_t pnd_bitmap;

                int ret;
                if ((ret = pnd_bitmap_calculate(format, &pnd_bitmap)) < 0) {
                        DEBUG_PRINTF("TEST2\n");
                        return ret;
                }

                pnd_bitmap_all |= pnd_bitmap;
        }

        DEBUG_PRINTF("pnd_bitmap_all: 0x%02X\n", pnd_bitmap_all);

        return pnd_bitmap_validate(state, pnd_bitmap_all);
}
