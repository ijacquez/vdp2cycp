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

#endif /* !DEBUG_H_ */
