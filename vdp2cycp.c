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

int
main(int argc __unused, char *argv[] __unused)
{
        struct scrn_cell_format config_nbg0;
        memset(&config_nbg0, 0x00, sizeof(struct scrn_cell_format));
        struct scrn_cell_format config_nbg1;
        memset(&config_nbg1, 0x00, sizeof(struct scrn_cell_format));
        struct scrn_cell_format config_nbg2;
        memset(&config_nbg2, 0x00, sizeof(struct scrn_cell_format));
        struct scrn_cell_format config_nbg3;
        memset(&config_nbg3, 0x00, sizeof(struct scrn_cell_format));

        config_nbg0.scf_scroll_screen = SCRN_NBG0;
        config_nbg0.scf_map.plane_a = VRAM_ADDR_4MBIT(2, 0x00000);
        config_nbg0.scf_map.plane_b = VRAM_ADDR_4MBIT(2, 0x00000);
        config_nbg0.scf_map.plane_c = VRAM_ADDR_4MBIT(3, 0x00000);
        config_nbg0.scf_map.plane_d = VRAM_ADDR_4MBIT(3, 0x00000);

        uint8_t bitmap_nbg0;
        pnd_bitmap_calculate(&bitmap_nbg0, &config_nbg0);
        uint8_t bitmap_nbg1;
        pnd_bitmap_calculate(&bitmap_nbg1, &config_nbg1);
        uint8_t bitmap_nbg2;
        pnd_bitmap_calculate(&bitmap_nbg2, &config_nbg2);
        uint8_t bitmap_nbg3;
        pnd_bitmap_calculate(&bitmap_nbg3, &config_nbg3);

        uint8_t bitmap;
        bitmap = bitmap_nbg0;

        pnd_bitmap_validate(0x0300, bitmap);

        DEBUG_PRINTF("0x%02X\n", bitmap);

        return 0;
}

/*-
 * Generate an 8-bit bitmap BITMAP of where pattern name data is stored
 * amongst the 4 banks.
 *
 * If succesful, 0 is returned. Otherwise, a negative value is return
 * for the following cases:
 *
 *   - BITMAP is NULL
 *   - CONFIG is NULL
 */
int
pnd_bitmap_calculate(uint8_t *bitmap, const struct scrn_cell_format *config)
{
        if (bitmap == NULL) {
                return -1;
        }

        if (config == NULL) {
                return -1;
        }

        uint32_t map_count;
        map_count = ((config->scf_scroll_screen & (SCRN_RBG0 | SCRN_RBG1)) == 0)
            ? 4
            : 16;

        *bitmap = 0x00;

        uint32_t i;
        for (i = 0; i < map_count; i++) {
                /* XXX: 4 or 8-Mbit? */
                uint8_t bank;
                bank = VRAM_BANK_4MBIT(config->scf_map.planes[i]);

                *bitmap |= 1 << (4 - bank - 1);
        }

        return 0;
}

/*-
 * Validate 8-bit bitmap BITMAP, which represent where pattern name data
 * is stored amongst the 4 VRAM banks.
 *
 * If successful, true is returned. Otherwise, false.
 */
bool
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
        for (i = 0; i < mask_count / 2; i++) {
                if ((bitmap & pnd_bank_mask[bank][1 + i]) != 0x00) {
                        return false;
                }

                if ((bitmap & pnd_bank_mask[bank][2 + i]) != 0x00) {
                        return false;
                }
        }

        return true;
}

/*-
 * Calculate VDP2 VRAM cycle patterns given selected VDP2 screens SCRNS.
 *
 * If successful, 0 is returned. Otherwise, a negative value is returned
 * for the following cases:
 *
 *   - P is NULL
 *   - SCRNS has an invalid screen
 *   - Case 3
 *   - Case 4
 *   - Case 5
 *   - Case 6
 *   - Case 7
 *   - Case 8
 */
int
vdp2cycp(uint32_t scrns __unused, vram_cycp *p __unused)
{
        if (p == NULL) {
                return -1;
        }

        if (scrns == 0x00000000) {
                return -1;
        }

        if ((scrns & 0xFFFFFFC0) != 0x00000000) {
                return -1;
        }

        return 0;
}

#ifdef DEBUG
static char *
debug_print_pattern(uint32_t pv __unused)
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
