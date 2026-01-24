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

#define LAMMP_VERSION "2.0.0"
#define LAMMP_ALPHA_YEAR "2026"
#define LAMMP_DATE "01-10"
#define LAMMP_COPYRIGHT "Copyright (C) [2025] [HJimmyK/LAMINA]"

#endif /* __LAMMP_VERSION_H__ */