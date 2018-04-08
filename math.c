#include <stdint.h>

#include "math.h"

/*-
 * Return the log base 2 of a 32-bit value V, assuming V is strictly a
 * power of 2.
 */ 
uint32_t
log2_pow2(uint32_t v)
{
        static const unsigned int b[] = {
                0xAAAAAAAA,
                0xCCCCCCCC,
                0xF0F0F0F0,
                0xFF00FF00,
                0xFFFF0000
        };

        register uint32_t r;
        r = (v & b[0]) != 0;

        r |= ((v & b[4]) != 0) << 4;
        r |= ((v & b[3]) != 0) << 3;
        r |= ((v & b[2]) != 0) << 2;
        r |= ((v & b[1]) != 0) << 1;

        return r;
}
