/*
 * [LAMMP]
 * Copyright (C) [2025-2026] [HJimmyK(Jericho Knox)]
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

#ifndef __LAMMP_PRINT_HEX_H__
#define __LAMMP_PRINT_HEX_H__

#include "../lmmp.h"
#include <stdio.h>

static inline void lmmp_print_hex(mp_srcptr num, mp_ssize_t n) {
    if (n < 0) {
        printf("-");
        n = -n;
    }
    printf("0x");
    for (mp_ssize_t i = n - 1; i >= 0; --i) {
        printf("%016llX", num[i]);
    }
    printf("\n");
}

#endif // __LAMMP_PRINT_HEX_H__