/**
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#ifndef __LAMMP_ALLOC_H__
#define __LAMMP_ALLOC_H__
#include "lampz.h"

/**
 * @brief  对齐内存分配
 * @param  size     ：需要分配的有效内存大小（字节）
 * @param  alignment：对齐大小（必须是2的幂次，且不小于sizeof(void*)）
 * @return 成功：返回对齐后的内存地址；失败：返回NULL
 * @note   1. Linux：基于 posix_memalign 实现
 *         2. Windows：基于 _aligned_malloc 实现
 *         3. 其他平台基于 malloc 实现。
 *         4. 内存分配失败时，返回 NULL。
 */
void* lammp_alloc(size_t elem_align, size_t elem_len);

/**
 * @brief  对齐内存释放（与 aligned_malloc 配套使用）
 * @param  ptr ：由 aligned_malloc 分配的内存地址（NULL安全，传入NULL无操作）
 * @note   1. Linux：基于 free 实现（posix_memalign 分配的内存可直接用 free 释放）
 *         2. Windows：基于 _aligned_free 实现
 *         3. 其他平台基于 free 实现。
 */
void lammp_free(void* ptr);

#include <stdlib.h>

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (sizeof(void*))
#endif

#if defined(_WIN32)
#include <malloc.h> 
#elif defined(__linux__)
#else
#warning "Unsupported platform, using generic aligned malloc implementation."
#include <stdint.h>
#endif

/**
 * @brief  对齐内存分配 + 全内存清零
 * @param  elem_align  单个元素的对齐大小（必须是大于0的2的幂次，如4、8、16、32等）
 * @param  elem_len    需要分配的元素总个数（大于0，总内存大小 = elem_len * elem_align）
 * @return 成功：返回满足 elem_align 对齐要求的内存地址（内存已全部清零为0）
 *         失败：返回 NULL（内存分配不足导致）
 * @note   1.  分配的内存必须使用 lammp_free 函数释放，不可使用普通 free/_aligned_free 单独释放
 *         2.  总分配内存大小为 elem_len * elem_align，确保能容纳 elem_len 个大小为 elem_align 的元素
 *         3.  与标准 calloc 区别：本函数按指定对齐大小分配，而非默认数据类型对齐
 */
void* lammp_calloc(size_t elem_align, size_t elem_len);

/**
 * @brief  对齐内存扩容（前部数据复制 + 尾部新增区域清零 + 原内存自动释放）
 * @param  ptr         输入输出参数：指向原对齐内存指针的指针，扩容后存储新内存地址
 * @param  buf_len     原内存的有效元素个数（对应原内存大小 = buf_len * elem_align）
 * @param  elem_align  单个元素的对齐大小（必须是大于0的2的幂次，与原内存对齐大小一致）
 * @param  elem_len    新内存的元素总个数（必须大于 buf_len，实现扩容，新内存大小 = elem_len * elem_align）
 * @return 无返回值（扩容失败时，*ptr 保持原内存地址不变，不会丢失原数据）
 * @note   1.  原内存由函数内部自动调用 lammp_free 释放，调用者无需手动释放
 *         2.  若 *ptr 为 NULL，本函数等价于 lammp_calloc（直接分配新对齐内存并清零）
 *         3.  新内存前部（buf_len 个元素）复制原内存数据，尾部（elem_len - buf_len 个元素）清零为0
 *         4.  必须保证 elem_align 与原内存的对齐大小一致，否则可能导致数据错乱
 */
void lammp_realloc(void** ptr, size_t buf_len, size_t elem_align, size_t elem_len);

/**
 * @brief  对齐内存分配 + 尾部指定区域清零
 * @param  elem_align   单个元素的对齐大小（必须是大于0的2的幂次，如4、8、16、32等）
 * @param  elem_len     需要分配的元素总个数（大于0，总内存大小 = elem_len * elem_align）
 * @param  tail_zero_len  尾部需要清零的元素个数（0 ≤ tail_zero_len ≤ elem_len）
 * @return 成功：返回满足 elem_align 对齐要求的内存地址
 *         失败：返回 NULL（内存分配不足导致）
 * @note   1.  分配的内存必须使用 lammp_free 函数释放，不可使用普通 free/_aligned_free 单独释放
 *         2.  当 tail_zero_len = 0 时，等价于 lammp_alloc（仅分配对齐内存，不执行清零操作）
 *         3.  当 tail_zero_len = elem_len 时，等价于 lammp_calloc（全内存清零）
 *         4.  清零范围：仅尾部 tail_zero_len 个元素对应的内存（前 elem_len - tail_zero_len 个元素为随机值）
 */
void* lammp_talloc(size_t elem_align, size_t elem_len, size_t tail_zero_len);

#endif  // __LAMMP_ALLOC_H__