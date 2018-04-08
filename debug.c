#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "vdp2.h"

#include "debug.h"

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
        static const char *background_names[] __unused = {
                "NBG0",
                "NBG1",
                "NBG3",
                "NBG4",
                "RGB0",
                "RGB1",
                NULL
        };

        static const char *ccc_names[] __unused = {
                "16 (palette)",
                "256 (palette)",
                "2,048 (palette)",
                "32,768 (RGB)",
                "16,770,000 (RGB)",
                NULL,
        };

        char *output_buffer;

        /* Two lines plus extra newline and NUL byte */
        uint32_t bytes;
        bytes = 0 + 1 + 1;

        output_buffer = malloc(bytes);
        assert(output_buffer != NULL);
        memset(output_buffer, '\0', bytes);

        return output_buffer;
}
#endif /* DEBUG */

#ifdef DEBUG
#endif /* DEBUG */
