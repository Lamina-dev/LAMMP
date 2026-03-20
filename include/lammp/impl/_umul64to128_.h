/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __UMUL64TO128_H__
#define __UMUL64TO128_H__

#include <stdint.h>

static inline void _umul64to128_(uint64_t a, uint64_t b, uint64_t *low, uint64_t *high) {
#if (defined(__GNUC__) || defined(__clang__))
#if defined(USE_ASM) && (defined(__x86_64__))
    __asm__("mul %[b]" 
            : "=a"(*low),
              "=d"(*high)         
            : "a"(a), [b] "r"(b)  
            :                    
    );
#else
    __uint128_t prod = (__uint128_t)a * b;
    *low = (uint64_t)prod;
    *high = (uint64_t)(prod >> 64);
#endif
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    *low = _umul128(a, b, high);
#else
    uint64_t ah = a >> 32, bh = b >> 32;
    a = (uint32_t)a, b = (uint32_t)b;
    uint64_t r0 = a * b, r1 = a * bh, r2 = ah * b, r3 = ah * bh;
    r3 += (r1 >> 32) + (r2 >> 32);
    r1 = (uint32_t)r1, r2 = (uint32_t)r2;
    r1 += r2;
    r1 += (r0 >> 32);
    *high = r3 + (r1 >> 32);
    *low = (r1 << 32) | (uint32_t)r0;
#endif
}

#endif // __UMUL64TO128_H__