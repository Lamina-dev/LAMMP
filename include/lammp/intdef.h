/**
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#ifndef __LAMMP_INTDEF_H__
#define __LAMMP_INTDEF_H__
#include <stddef.h>
#include <stdint.h>

/**
 * @brief   获取两个值中的较大值
 * @param   a   第一个比较值（任意可比较的数值类型：int/float/指针等）
 * @param   b   第二个比较值（需与a类型一致）
 * @return  返回a和b中较大的那个值
 * @note    1. 参数若为表达式（如i++）会被求值两次，可能引发副作用；
 *          2. 所有参数加括号，避免运算符优先级问题。
 */
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * @brief   获取两个值中的较小值
 * @param   a   第一个比较值（任意可比较的数值类型：int/float/指针等）
 * @param   b   第二个比较值（需与a类型一致）
 * @return  返回a和b中较小的那个值
 * @note    1. 参数若为表达式（如i++）会被求值两次，可能引发副作用；
 *          2. 所有参数加括号，避免运算符优先级问题。
 */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief   获取数值的绝对值
 * @param   a   待计算绝对值的数值（整型/浮点型均可）
 * @return  返回a的绝对值
 * @note    1. 对无符号类型使用时无意义（结果仍为原值）；
 *          2. 参数若为表达式（如i++）会被求值两次，可能引发副作用。
 */
#define ABS(a) ((a) < 0 ? -(a) : (a))

/**
 * @brief   交换两个变量的值
 * @param   a   第一个待交换的变量（任意可赋值的类型：int/char/结构体等）
 * @param   b   第二个待交换的变量（需与a类型完全一致）
 * @note    1. 使用typeof(a)自动推导变量类型，无需手动指定；
 *          2. 不支持数组/堆内存交换（仅交换变量本身，指针类型仅交换地址）；
 *          3. 禁止传入带副作用的表达式（如i++），会导致编译错误。
 */
#define SWAP(a, b)         \
    do {                   \
        typeof(a) t = (a); \
        (a) = (b);         \
        (b) = (t);         \
    } while (0)


typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;
typedef size_t lamp_size_t;
typedef ptrdiff_t lamp_ptrdiff_t; /* 不推荐使用，仅用于兼容性 */

#endif /* __LAMMP_INTDEF_H__ */