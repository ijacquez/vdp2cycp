#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"

#include "vdp2.h"
#include "math.h"

#ifdef DEBUG
char *
debug_print_pattern(uint32_t pv)
{
        static const char *timing_mnemonics[] = {
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
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 7)],
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 6)],
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 5)],
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 4)],
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 3)],
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 2)],
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 1)],
            timing_mnemonics[VRAM_CTL_CYCP_TIMING_VALUE(pv, 0)]);

        return output_buffer;
}
#endif /* DEBUG */

#ifdef DEBUG
char *
debug_print_cell_format(const struct scrn_cell_format *config __unused)
{
        static const char *scroll_screen_names[] __unused = {
                "NBG0",
                "NBG1",
                "NBG2",
                "NBG3",
                "RGB0",
                "RGB1",
                NULL
        };

        static const char *cc_count_names[] __unused = {
                "16 colors (palette)",
                "256 colors (palette)",
                "2,048 colors (palette)",
                "32,768 colors (RGB)",
                "16,770,000 colors (RGB)",
                NULL,
        };

        static const char *reduction_names[] __unused = {
                "1 reduction",
                "1/2 reduction",
                "1/4 reduction",
                NULL
        };

        static const char *pnd_size_names[] __unused = {
                "Invalid",
                "1-word",
                "2-words",
                NULL
        };

        char *output_buffer;

        /* XXX: Hard coded for now */
        uint32_t bytes;
        bytes = 2048;

        output_buffer = malloc(bytes);
        assert(output_buffer != NULL);
        memset(output_buffer, '\0', bytes);

        (void)sprintf(output_buffer,
            "\n"
            "scf_scroll_screen: %s\n"
            "     scf_cc_count: %s\n"
            "     scf_pnd_size: %s\n"
            "     scf_cp_table: 0x%08X\n"
            "    scf_vcs_table: 0x%08X\n"
            "    scf_reduction: %s\n"
            "  scf_map.plane_a: 0x%08X (bank %i)\n"
            "  scf_map.plane_b: 0x%08X (bank %i)\n"
            "  scf_map.plane_c: 0x%08X (bank %i)\n"
            "  scf_map.plane_d: 0x%08X (bank %i)\n"
            "\n"
            "  priv_pnd_bitmap: 0x%02X\n",
            scroll_screen_names[log2_pow2(config->scf_scroll_screen)],
            cc_count_names[config->scf_cc_count],
            pnd_size_names[config->scf_pnd_size],
            config->scf_cp_table,
            config->scf_vcs_table,
            reduction_names[config->scf_reduction],
            config->scf_map.plane_a,
            VRAM_BANK_4MBIT(config->scf_map.plane_a),
            config->scf_map.plane_b,
            VRAM_BANK_4MBIT(config->scf_map.plane_b),
            config->scf_map.plane_c,
            VRAM_BANK_4MBIT(config->scf_map.plane_c),
            config->scf_map.plane_d,
            VRAM_BANK_4MBIT(config->scf_map.plane_d),
            config->priv_pnd_bitmap);

        return output_buffer;
}
#endif /* DEBUG */

#ifdef DEBUG
#endif /* DEBUG */
