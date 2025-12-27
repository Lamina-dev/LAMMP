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

#ifndef __LAMMP_VERSION_H__
#define __LAMMP_VERSION_H__

#ifdef __clang__
#define LAMMP_COMPILER "Clang C++"
#define LAMMP_COMPILER_VERSION 4
#elif defined(__GNUC__)
#define LAMMP_COMPILER "GNU C++"
#define LAMMP_COMPILER_VERSION 1
#elif defined(_MSC_VER)
#define LAMMP_COMPILER "Microsoft Visual C++"
#define LAMMP_COMPILER_VERSION 2
#elif defined(__INTEL_COMPILER)
#define LAMMP_COMPILER "Intel C++"
#define LAMMP_COMPILER_VERSION 3
#else
#define LAMMP_COMPILER "Unknown"
#define LAMMP_COMPILER_VERSION 0  
#endif

#if !defined(_WIN64) && !defined(__x86_64__) && !defined(__aarch64__) && !defined(__ppc64__)
#error "LAMMP only supports 64-bit architectures"
#endif

#define LAMMP_VERSION "0.0.1"
#define LAMMP_ALPHA_YEAR "2025"
#define LAMMP_DATE "12-26"
#define LAMMP_COPYRIGHT "Copyright (C) [2025] [HJimmyK/LAMINA]"

#include <stdio.h>

static inline int lammp_version() {
    if (LAMMP_COMPILER_VERSION == 0) {
        printf("Unknown compiler\n");
    } else {
        printf("Compiler: %s\n", LAMMP_COMPILER);
        printf("Compiler version: %d\n", LAMMP_COMPILER_VERSION);
    }
    printf("LAMMP version: %s\n", LAMMP_VERSION);
    printf("LAMMP alpha year: %s\n", LAMMP_ALPHA_YEAR);
    printf("LAMMP date: %s\n", LAMMP_DATE);
    printf("LAMMP copyright: %s\n", LAMMP_COPYRIGHT);
    printf("======================================================\n");
    printf("Hello, lammp!\n");
    return 0;
}

#endif /* __LAMMP_VERSION_H__ */