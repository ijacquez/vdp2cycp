#include <sys/cdefs.h>

#include <assert.h>
#include <stdint.h>
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
        return 0;
}

/*-
 * Calculate VDP2 VRAM cycle patterns given selected VDP2 screens SCRNS.
 *
 * If successful, 0 is returned. Otherwise a negative value is returned
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
