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

#ifndef __LAMMP_ALLOC_HPP__
#define __LAMMP_ALLOC_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
// Windows 平台：用 _aligned_malloc（参数顺序：size, alignment）
#include <malloc.h>
#define LAMMP_ALLOC(align, size) _aligned_malloc((size), (align))

#elif defined(__cplusplus) && __cplusplus >= 201703L
// C++17+ 平台（Linux/macOS/MinGW）：用 posix_memalign（兼容所有 C++ 编译器）
#include <cstdlib>
#define LAMMP_ALLOC(align, size)                                                       \
    ({                                                                                 \
        void* _ptr = NULL;                                                             \
        (posix_memalign(&_ptr, (align), (size)) == 0) ? _ptr : (errno = ENOMEM, NULL); \
    })

#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
// C11+ 平台（Linux/macOS）：优先用 aligned_alloc，失败降级 posix_memalign
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdlib.h>
#if defined(__STDC_LIB_EXT1__)
#define LAMMP_ALLOC(align, size) aligned_alloc((align), (size))
#else
#define LAMMP_ALLOC(align, size)                                                       \
    ({                                                                                 \
        void* _ptr = NULL;                                                             \
        (posix_memalign(&_ptr, (align), (size)) == 0) ? _ptr : (errno = ENOMEM, NULL); \
    })
#endif

#else
// 其他平台（低版本 C/C++）：用 posix_memalign（兼容多数编译器）
#include <stdlib.h>
#define LAMMP_ALLOC(align, size)                                                       \
    ({                                                                                 \
        void* _ptr = NULL;                                                             \
        (posix_memalign(&_ptr, (align), (size)) == 0) ? _ptr : (errno = ENOMEM, NULL); \
    })
#endif

// 配套：跨平台对齐内存释放宏（统一调用方式：LAMMP_FREE(指针)）
#if defined(_WIN32)
// Windows _aligned_malloc 分配的内存，需用 _aligned_free 释放（不能用 free！）
#define LAMMP_FREE(ptr) _aligned_free((ptr))
#else
// Linux/macOS/MinGW：posix_memalign/aligned_alloc 分配的内存，用 free 释放
#define LAMMP_FREE(ptr) free((ptr))
#endif

#ifdef __cplusplus
}
#endif

#endif  // __LAMMP_ALLOC_HPP__