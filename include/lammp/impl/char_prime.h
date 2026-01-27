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

#ifndef __LAMMP_CHAR_PRIME_H__
#define __LAMMP_CHAR_PRIME_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHAR_PRIME_size 54

const uint8_t pri_char_table[54] = {2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,
                                    47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107,
                                    109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
                                    191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251};

#ifdef __cplusplus
}
#endif

#endif // __LAMMP_CHAR_PRIME_H__