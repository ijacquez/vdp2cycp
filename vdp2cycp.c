#include <sys/cdefs.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>

#include "vdp2.h"
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

/*
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

#define VRAM_CTL_CYCP_TIMING_BIT(x)     (((x) & 0x7) << 2)
#define VRAM_CTL_CYCP_TIMING_MASK(x)    ((0x0000000F) << VRAM_CTL_CYCP_TIMING_BIT(x))

#define VRAM_CTL_CYCP_TIMING_VALUE(pv, x)                                      \
        (((uint32_t)(pv) & VRAM_CTL_CYCP_TIMING_MASK(x)) >> VRAM_CTL_CYCP_TIMING_BIT(x))

typedef union {
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
} vram_cycp;

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
