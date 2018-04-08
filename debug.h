/*
 * Copyright (c) 2012-2016 Israel Jacquez
 * See LICENSE for details.
 *
 * Israel Jacquez <mrkotfw@gmail.com>
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEBUG_PRINTF(fmt, ...) do {                                            \
        (void)fprintf(stderr, "%s():L%i:" " " fmt, __FUNCTION__, __LINE__,     \
            ##__VA_ARGS__);                                                    \
        fflush(stderr);                                                        \
} while(false)
#else
#define DEBUG_PRINTF(x...)
#endif /* DEBUG */

#ifdef DEBUG
#define DEBUG_PATTERN(pv) do {                                                 \
        char *debug_print_pattern(uint32_t);                                   \
        char *_output_buffer;                                                  \
        _output_buffer = debug_print_pattern(pv);                              \
        DEBUG_PRINTF("%s", _output_buffer);                                    \
        free(_output_buffer);                                                  \
} while (false)
#else
#define DEBUG_PATTERN(...)
#endif /* DEBUG */

#endif /* !DEBUG_H_ */
