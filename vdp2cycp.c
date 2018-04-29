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
static const int8_t _timings_count_pnd[3][4] = {
        /* Invalid */
        {
                -1,     /* Invalid */
                -1,     /* Invalid */
                -1,     /* Invalid */
        },
        /* PND 1-word */
        {
                 1,     /* No reduction */
                 2,     /* 1/2 reduction */
                 4,     /* 1/4 reduction */
                -1      /* Invalid */
        },
        /* PND 2-word */
        {
                 1,     /* No reduction */
                 2,     /* 1/2 reduction */
                 4,     /* 1/4 reduction */
                -1      /* Invalid */
        }
};

/* Table representing number of VRAM accesses required for character
 * pattern data. */
static const int8_t _timings_count_cpd[2][5][4] = {
        /* Cell */
        {
                /* Character color count: 16 (palette) */
                {
                        1,     /* No reduction */
                        2,     /* 1/2 reduction */
                        4,     /* 1/4 reduction */
                        -1      /* Invalid */
                },
                /* Character color count: 256 (palette) */
                {
                        2,     /* No reduction */
                        4,     /* 1/2 reduction */
                        -1,     /* 1/4 reduction (invalid) */
                        -1      /* Invalid */
                },
                /* Character color count: 2048 (palette)*/
                {
                        4,     /* No reduction */
                        -1,     /* 1/2 reduction (invalid) */
                        -1,     /* 1/4 reduction (invalid) */
                        -1      /* Invalid */
                },
                /* Character color count: 32,768 (RGB) */
                {
                        -1,     /* Invalid */
                        -1,     /* Invalid */
                        -1,     /* Invalid */
                        -1      /* Invalid */
                },
                /* Character color count: 16,770,000 (RGB) */
                {
                        -1,     /* Invalid */
                        -1,     /* Invalid */
                        -1,     /* Invalid */
                        -1      /* Invalid */
                }
        },
        /* Bitmap */
        {
                /* Character color count: 16 (palette) */
                {
                        1,     /* No reduction */
                        2,     /* 1/2 reduction */
                        4,     /* 1/4 reduction */
                        -1      /* Invalid */
                },
                /* Character color count: 256 (palette) */
                {
                        2,     /* No reduction */
                        4,     /* 1/2 reduction */
                        -1,     /* 1/4 reduction (invalid) */
                        -1      /* Invalid */
                },
                /* Character color count: 2048 (palette)*/
                {
                        4,     /* No reduction */
                        -1,     /* 1/2 reduction (invalid) */
                        -1,     /* 1/4 reduction (invalid) */
                        -1      /* Invalid */
                },
                /* Character color count: 32,768 (RGB) */
                {
                        4,     /* No reduction */
                        -1,     /* 1/2 reduction (invalid) */
                        -1,     /* 1/4 reduction (invalid) */
                        -1      /* Invalid */
                },
                /* Character color count: 16,770,000 (RGB) */
                {
                        8,     /* No reduction */
                        -1,     /* 1/2 reduction (invalid) */
                        -1,     /* 1/4 reduction (invalid) */
                        -1      /* Invalid */
                }
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
static const uint32_t _timings_range_normal[8] = {
        /* T0 -> T0, T1, T2, T4, T5, T6, T7 */
        0xFFFF0FFF,
        /* T1 -> T0, T1, T2, T3, T5, T6, T7 */
        0xFFF0FFFF,
        /* T2 -> T0, T1, T2, T3, T6, T7 */
        0xFF00FFFF,
        /* T3 -> T0, T1, T2, T3, T7 */
        0xF000FFFF,
        /* T4 -> T0, T1, T2, T3 */
        0x0000FFFF,
        /* T5 -> T1, T2, T3 */
        0x0000FFF0,
        /* T6 -> T2, T3 */
        0x0000FF00,
        /* T7 -> T3 */
        0x0000F000
};

/* Table representing range of access timings for hi-res TV screen mode. */
static const uint32_t _timings_range_hires[8] = {
        /* T0 -> T0, T1, T2 */
        0x00000FFF,
        /* T1 -> T1, T2, T3 */
        0x0000FFF0,
        /* T2 -> T0, T2, T3 */
        0x0000FF0F,
        /* T3 -> T0, T1, T3 */
        0x0000F0FF,
        /* T4 -> Invalid */
        0x00000000,
        /* T5 -> Invalid */
        0x00000000,
        /* T6 -> Invaild */
        0x00000000,
        /* T7 -> Invalid */
        0x00000000
};

/* Table representing range of access timings for vertical cell scrolling.
 *
 * Note, access for NBG0 and NBG1 must be by the same bank, and NBG0
 * access must be selected first.
 */
static const uint32_t _timings_range_vcs[2] = {
        /* NBG0 */
        0x000000FF,
        /* NBG1 */
        0xF0000FFF
};

static int32_t cycp_calculate_timings(const struct scrn_format *, uint8_t *, uint8_t *, uint8_t *);

static void state_init(struct state *state, const struct scrn_format **) __unused;

static int32_t pnd_bitmap_calculate(const struct scrn_format *, uint8_t *) __unused;
static int32_t pnd_bitmap_validate(uint8_t, uint8_t) __unused;
static int32_t pnd_bitmap_validate_all(const struct state *) __unused;

static int32_t vcs_bitmap_calculate(const struct scrn_format *, uint8_t *) __unused;
static int32_t vcs_bitmap_validate_all(const struct state *) __unused;

static int32_t scrn_plane_count_get(const struct scrn_format *) __unused;

int
main(int argc __unused, char *argv[] __unused)
{
        DEBUG_PRINTF("sizeof(union vram_cycp): %lu bytes(s)\n", sizeof(union vram_cycp));
        DEBUG_PRINTF("sizeof(struct scrn_format): %lu byte(s)\n", sizeof(struct scrn_format));
        DEBUG_PRINTF("sizeof(struct scrn_cell_format): %lu byte(s)\n", sizeof(struct scrn_cell_format));
        DEBUG_PRINTF("sizeof(struct scrn_bitmap_format): %lu byte(s)\n", sizeof(struct scrn_bitmap_format));
        DEBUG_PRINTF("sizeof(_timings_count_pnd): %lu byte(s)\n", sizeof(_timings_count_pnd));
        DEBUG_PRINTF("sizeof(_timings_count_cpd): %lu byte(s)\n", sizeof(_timings_count_cpd));
        DEBUG_PRINTF("sizeof(_timings_count_vcs): %lu byte(s)\n", sizeof(_timings_count_vcs));

        DEBUG_PRINTF("sizeof(_timings_range_normal): %lu byte(s)\n", sizeof(_timings_range_normal));
        DEBUG_PRINTF("sizeof(_timings_range_hires): %lu byte(s)\n", sizeof(_timings_range_hires));
        DEBUG_PRINTF("sizeof(_timings_range_vcs): %lu byte(s)\n", sizeof(_timings_range_vcs));
        DEBUG_PRINTF("sizeof(struct state): %lu byte(s)\n", sizeof(struct state));

        struct state state;

        state_init(&state, bg_formats);

        /* XXX: Place holder */
        state.ramctl = 0x0000;

        int32_t error;
        error = vdp2cycp(&state);
        DEBUG_PRINTF("vdp2cycp: %i\n", error);

        return error;
}

/*-
 * Calculate VDP2 VRAM cycle patterns.
 *
 * If successful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - -1 STATE is NULL
 *   - -2 Vertical cell scroll is stored in an invalid bank
 *   - -3 Pattern name data is stored in an invalid bank
 *   - -4 Insufficient number of vertical cell scroll access timings
 *   - -5 Insufficient number of pattern name data access timings
 *   - -6 Insufficient number of character pattern data access timings
 */
int32_t
vdp2cycp(const struct state *state)
{
        if (state == NULL) {
                return -1;
        }

        if ((vcs_bitmap_validate_all(state)) < 0) {
                return -2;
        }

        if ((pnd_bitmap_validate_all(state)) < 0) {
                return -3;
        }

        /* Go in order: NBG0, NBG1, NBG2, then NBG3 */

        uint32_t scrn;
        for (scrn = 0; scrn < SCRN_COUNT; scrn++) {
                struct scrn_format *format;
                format = &state->scroll_screens[scrn]->format;

                if (format == NULL) {
                        continue;
                }

                /* Check if scroll screen is enabled */
                if (!format->sf_enable) {
                        continue;
                }

                uint8_t tvcs;
                uint8_t tpnd;
                uint8_t tcpd;

                int32_t ret;
                if ((ret = cycp_calculate_timings(format, &tvcs, &tpnd, &tcpd)) < 0) {
                        return ret;
                }

                DEBUG_PRINTF("--------------------------------------------------------------------------------\n");
                DEBUG_FORMAT(format);

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

static int32_t
cycp_calculate_timings(
        const struct scrn_format *format,
        uint8_t *tvcs,
        uint8_t *tpnd,
        uint8_t *tcpd)
{
        *tvcs = 0;
        *tpnd = 0;
        *tcpd = 0;

        /* Determine if vertical cell scroll is used */
        if (VRAM_BANK_ADDRESS(format->sf_vcs_table)) {
                *tvcs = _timings_count_vcs[format->sf_scroll_screen];

                if ((int8_t)*tvcs < 0) {
                        return -4;
                }
        }

        /* Determine how many PND access timings are needed (due
         * to reduction) */
        if (format->sf_type == SCRN_TYPE_CELL) {
                const struct scrn_cell_format *cell_format;
                cell_format = &format->sf_format.cell;

                *tpnd = _timings_count_pnd[cell_format->scf_pnd_size][format->sf_reduction];

                /* Invalid number of PND access timings */
                if ((int8_t)*tpnd < 0) {
                        return -5;
                }
        }

        /* Determine how many CPD access timings are needed (due
         * to reduction) and character color count */
        *tcpd = _timings_count_cpd[format->sf_type][format->sf_cc_count][format->sf_reduction];

        /* Invalid number of PND access timings */
        if ((int8_t)*tcpd < 0) {
                return -6;
        }

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

        state->scroll_screens[0] = &state->nbg0;
        state->scroll_screens[1] = &state->nbg1;
        state->scroll_screens[2] = &state->nbg2;
        state->scroll_screens[3] = &state->nbg3;
        state->scroll_screens[4] = &state->rbg0;
        state->scroll_screens[5] = &state->rbg1;

        uint32_t i;
        for (i = 0; (i < SCRN_COUNT) && (formats[i] != NULL); i++) {
                uint8_t scrn;
                scrn = formats[i]->sf_scroll_screen;

                struct scroll_screen *scroll_screen;
                scroll_screen = state->scroll_screens[scrn];

                (void)memcpy(&scroll_screen->format, formats[i], sizeof(*formats[i]));

                vcs_bitmap_calculate(&scroll_screen->format, &scroll_screen->vcs_bitmap);
                pnd_bitmap_calculate(&scroll_screen->format, &scroll_screen->pnd_bitmap);
        }
}

/*-
 * Calculate an 8-bit bit-map PND_BITMAP of where pattern name data is
 * stored amongst the 4 banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - -1 PND_BITMAP is NULL
 *   - -2 FORMAT is NULL
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
                return 0;
        }

        if (!format->sf_enable) {
                return 0;
        }

        const struct scrn_cell_format *cell_format;
        cell_format = &format->sf_format.cell;

        int32_t plane_count;
        plane_count = scrn_plane_count_get(format);

        int32_t i;
        for (i = 0; i < plane_count; i++) {
                /* XXX: 4 or 8-Mbit? */
                uint8_t bank;
                bank = VRAM_BANK_4MBIT(cell_format->scf_map.planes[i]);

                *pnd_bitmap |= BANK_BIT(bank);

                DEBUG_PRINTF("p: 0x%08X, %i, BANK_BIT(bank): 0x%02X\n",
                    cell_format->scf_map.planes[i],
                    bank,
                    BANK_BIT(bank));
        }

        return 0;

#undef BANK_BIT
}

/*-
 * Validate 8-bit bit-map BITMAP, which represent where pattern name data
 * is stored amongst the 4 VRAM banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - -1 Pattern name data is stored in an invalid bank
 */
static int32_t
pnd_bitmap_validate(uint8_t bank_config, uint8_t pnd_bitmap)
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

        if ((pnd_bitmap & 0x0F) == 0x00) {
                return 0;
        }

        uint32_t i;
        for (i = 0; ; i++) {
                uint8_t pnd_bank_mask;
                pnd_bank_mask = pnd_bank_masks[bank_config & 0x03][i];

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
 * Validate the pattern name bit-map of all configured scroll screens.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - -1 STATE is NULL
 *   - -2 Pattern name data is stored in an invalid bank
 */
static int32_t
pnd_bitmap_validate_all(const struct state *state)
{
        if (state == NULL) {
                return -1;
        }

        uint8_t pnd_bitmap;
        pnd_bitmap = 0x00;

        uint32_t scrn;
        for (scrn = 0; scrn < SCRN_COUNT; scrn++) {
                struct scroll_screen *scroll_screen;
                scroll_screen = state->scroll_screens[scrn];

                struct scrn_format *format;
                format = &scroll_screen->format;

                if (!format->sf_enable) {
                        continue;
                }

                pnd_bitmap |= scroll_screen->pnd_bitmap;
        }

        DEBUG_PRINTF("pnd_bitmap_all: 0x%02X\n", pnd_bitmap);

        /* XXX: This needs to be moved elsewhere */
        uint8_t bank_config;
        bank_config = ((state->ramctl & 0x0300) >> 8) & 0x03;

        return pnd_bitmap_validate(bank_config, pnd_bitmap);
}

/*-
 * Calculate an 8-bit bit-map VCS_BITMAP of where vertical cell scroll
 * data is stored amongst the 4 banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - -1 VCS_BITMAP is NULL
 *   - -2 FORMAT is NULL
 */
static int
vcs_bitmap_calculate(const struct scrn_format *format, uint8_t *vcs_bitmap)
{
        if (vcs_bitmap == NULL) {
                return -1;
        }

        *vcs_bitmap = 0x00;

        if (format == NULL) {
                return -2;
        }

        if (format->sf_type != SCRN_TYPE_CELL) {
                return 0;
        }

        if (!format->sf_enable) {
                return 0;
        }

        *vcs_bitmap = VRAM_BANK_4MBIT(format->sf_vcs_table);

        return 0;
}

/*-
 * Validate the vertical cell scroll bit-map of NBG0 and NBG1.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - -1 STATE is NULL
 *   - -2 Vertical cell scroll data is stored in an invalid bank
 */
static int32_t
vcs_bitmap_validate_all(const struct state *state)
{
        if (state == NULL) {
                return -1;
        }

        const uint8_t *vcs_bitmap_nbg0;
        vcs_bitmap_nbg0 = &state->nbg0.vcs_bitmap;

        const uint8_t *vcs_bitmap_nbg1;
        vcs_bitmap_nbg1 = &state->nbg1.vcs_bitmap;

        if ((*vcs_bitmap_nbg0 == 0x00) && (*vcs_bitmap_nbg1 == 0x00)) {
                return 0;
        }

        return ((*vcs_bitmap_nbg0 & *vcs_bitmap_nbg1) != 0x00) ? 0 : -2;
}

/*-
 * Return the number of planes available from the screen format FORMAT.
 *
 * If succesful, the number of planes is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - -1 FORMAT is NULL
 *   - -2 FORMAT is not a cell type
 */
static int32_t
scrn_plane_count_get(const struct scrn_format *format)
{
        if (format == NULL) {
                return -1;
        }

        if (format->sf_type != SCRN_TYPE_CELL) {
                return -2;
        }

        switch (format->sf_scroll_screen) {
        case SCRN_NBG0:
        case SCRN_NBG1:
        case SCRN_NBG2:
        case SCRN_NBG3:
                return 4;
        case SCRN_RBG0:
        case SCRN_RBG1:
                return 16;
        }

        return -1;
}
