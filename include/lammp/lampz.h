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

#ifndef LAMPZ_H
#define LAMPZ_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// 申请内存的函数指针类型定义 
typedef void* (*_lammp_alloc_func)(size_t elem_align, size_t elem_len);
// 释放函数指针的定义
typedef void (*_lammp_free_func)(void* ptr);

static _lammp_alloc_func g_alloc_func = NULL;
static _lammp_free_func g_free_func = NULL;

// 全局注册接口（用户侧调用，无需重新编译动态库，运行时生效）
static inline void __lammp_set_alloc_func(_lammp_alloc_func custom_alloc) {
    g_alloc_func = custom_alloc;
}
// 设置自定义释放函数
static inline void __lammp_set_free_func(_lammp_free_func custom_free) {
    g_free_func = custom_free;
}

typedef int64_t lamp_si;           // 有符号整数：用于长度标记（正负表符号）、索引
typedef uint64_t lamp_ui;          // 无符号64位：大整数的单个“字”（word）类型
typedef lamp_ui* lamp_ptr;         // 大整数的内存指针：指向 lamp_ui 数组
typedef max_align_t lamp_align_t;  // 最大对齐类型（用于对齐检查）

// 配置宏
#define LAMPZ_ALIGN 8                                  // 内存对齐粒度（必须是 2 的幂，且 >= sizeof(lamp_ui)=8）
#define LAMPUI_BITS 64                                 // 单个字的位宽（与 lamp_ui 一致，64位）
#define LAMPSI_BITS 64                                 // 有符号长度的位宽（与 lamp_si 一致）
#define LAMPUI_SIZE 8                                  // 单个字的字节宽（与 lamp_ui 一致）
#define LAMPSI_SIZE 8                                  // 有符号长度的字节宽（与 lamp_si 一致）
#define LAMPZ_MIN_LEN 1                                // 大整数最小字长（至少1个64位字）
#define LAMMP_TALLOC_LEN 4                             // 尾部赋值缓冲区长度（用于尾部赋值）
#define LAMMP_UI_MAX 0xffffffffffffffffull             // 最大无符号64位整数值
#define LAMMP_UI_MIN 0x0ull                            // 最小无符号64位整数值
#define LAMMP_SI_MAX 0x7fffffffffffffffll              // 最大有符号64位整数值
#define LAMMP_SI_MIN (int64_t(0x8000000000000000ull))  // 最小有符号64位整数值

// 大整数结构体
typedef struct __lampz_struct {
    lamp_ptr begin;  // 指向大整数的字数组（lamp_ui[]）起始地址
    lamp_ptr end;    // 指向字数组末尾的下一个位置（end = begin + 实际字长）
    lamp_si len;     // 长度标记：正负表符号（正=非负，负=负），绝对值=字数组长度（count
                     // of lamp_ui）
}* __lampz_struct;

typedef struct __lampz_struct lampz_t[1];  // 大整数对象类型

/**
 * @brief 获取大整数的字数组指针（安全封装）
 * @param z 大整数对象
 * @return 成功返回 lamp_ui[] 指针，失败返回 NULL
 */
static inline lamp_ptr __lampz_get_ptr(lampz_t z);

/**
 * @brief 获取大整数的缓冲区长度
 * @param z 大整数对象
 * @return 字长（count of lamp_ui），失败返回 0
 */
static inline lamp_ui __lampz_get_capacity(const lampz_t z);

/**
 * @brief 获取大整数的长度（字长）
 * @param z 大整数对象
 * @return 字长（count of lamp_ui），失败返回 0
 */
static inline lamp_ui lampz_get_len(const lampz_t z);

/**
 * @brief 获取大整数的符号
 * @param z 大整数对象
 * @return 1=非负，-1=负，0=对象无效
 * @note 为零时，返回1，表示非负
 */
static inline lamp_si lampz_get_sign(const lampz_t z);

/**
 * @brief 检查大整数的内存对齐是否合法
 * @param z 大整数对象
 * @return 0=对齐合法，-1=无效对象，1=对齐非法
 */
static inline lamp_si __lampz_check_alignment(const lampz_t z);

/**
 * @brief 判空大整数（是否为有效对象）
 * @param z 大整数对象
 * @return true=无效（NULL或无内存），false=有效
 */
static inline bool lampz_is_null(const lampz_t z);

/**
 * @brief 设置大整数的符号（不改变字长和数据）
 * @param z 大整数对象（必须有效）
 * @param sign 符号（1=非负，-1=负）
 */
static inline void lampz_set_sign(lampz_t z, lamp_si sign);

/**
 * @brief 创建一个新的大整数对象
 * @param bit 大整数所需的最小二进制位宽（例如：要存储200位的数，bit=200）
 * @return 成功返回 lampz_t 对象，失败返回 NULL（内存分配失败）
 * @note 内部会自动向上对齐到 lamp_ui 字长，且内存按 LAMPZ_ALIGN 对齐
 */
static inline void lampz_init(lampz_t z, lamp_ui bit);

/**
 * @brief 释放对象
 * @param z 大整数对象（可以是 NULL，内部安全处理）
 * @note 必须调用此函数释放，不能直接 free(z)（会泄露数字数组内存）
 */
static inline void lampz_free(lampz_t z);

// -----------------------------------------------------------------------------
// 大整数运算函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 二元运算：z = x + y（z 的容量如果不够，会自动分配新内存）
 */
void lampz_add_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x - y（z 的容量如果不够，会自动分配新内存）
 */
void lampz_sub_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x * y（z 的容量如果不够，会自动分配新内存）
 * @note 如果 x 和 y 的指针相同，将自动调用 lampz_sqr_x 函数
 */
void lampz_mul_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x / y（整数除法，向下取整，z
 * 的容量如果不够，会自动分配新内存）
 */
void lampz_div_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x % y（取余，结果符号与 x 一致，z
 * 的容量如果不够，会自动分配新内存）
 */
void lampz_mod_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：q = x / y，r = x % y（同时计算商和余数，q,r
 * 的容量如果不够，会自动分配新内存）
 */
void lampz_div_mod_xy(lampz_t q, lampz_t r, const lampz_t x, const lampz_t y);

/**
 * @brief 一元运算：z += x（z 自身累加 x，z 的容量如果不够，会自动分配新内存）
 */
void lampz_add_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z -= x（z 自身减去 x，z 的容量如果不够，会自动分配新内存）
 */
void lampz_sub_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z *= x（z 自身乘以 x，z 的容量如果不够，会自动分配新内存）
 */
void lampz_mul_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z = x * x（平方，效率高于普通乘法，z
 * 的容量如果不够，会自动分配新内存）
 */
void lampz_sqr_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z /= x（z 自身除以 x，z 将会自动分配新内存）
 */
void lampz_div_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z = z % x（z 自身取余 x，z 将会自动分配新内存）
 */
void lampz_mod_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：判断 z 是否为 0
 * @return 1=非零，0=零
 * @note 为 nullptr 时，返回 1
 */
lamp_si lampz_is_zero(const lampz_t z);

/**
 * @brief 赋值：z = value（z 的容量如果不够，会自动分配新内存）
 */
void lampz_set_ui(lampz_t z, lamp_ui value);

/**
 * @brief 赋值：z = value（z 的容量如果不够，会自动分配新内存）
 */
void lampz_set_si(lampz_t z, lamp_si value);

/**
 * @brief 将二进制字符串赋值给大整数（64位小端数组存储）
 * @param z 目标大整数（若为容量不够，则自动分配内存）
 * @param str 二进制字符串（可保留前导零，默认小端序）
 * @param str_len 字符串长度
 * @param base 必须为2（二进制）
 * @note 1. 字符串无效/空时，z 设为0；2. 强制设为非负数符号（z的符号将会被设为非负）
 * @warning 若 z 原有内存分配足够，且进行了赋值将可能导致计算错误。
 * @warning 若 str 含有非法字符，则行为未定义；若 str 实际长度不足，将导致溢出。
 */
void lampz_set_str(lampz_t z, const char* str, lamp_ui str_len, lamp_ui base);

/**
 * @brief 大整数 z 转字符串，字符串需要的长度
 * @param z 输入：大整数
 * @param base 输入：进制，2-36
 * @return 字符串长度
 */
lamp_ui lampz_to_str_len(const lampz_t z, lamp_ui base);

/**
 * @brief 大整数 z 转字符串
 * @param z 目标大整数
 * @param str 二进制字符串（请通过 lampz_to_str_len 获得长度）
 * @param str_len 字符串长度（通过 lampz_to_str_len 获得）
 * @param base 进制 2-36
 * @note 字符串将会以小端序存储，且为绝对值，即改变 z 的符号将不影响输出结果
 * @warning 若 str 未分配足够内存，将导致溢出。
 */
lamp_ui lampz_to_str(char* str, const lamp_ui str_len, const lampz_t z, lamp_ui base);

/**
 * @brief 大整数 z 转整数
 * @param z 目标大整数
 * @return 整数值
 * @note 若 z 大于 LAMMP_SI_MAX 或为 nullptr，则返回 LAMMP_SI_MAX
 * @note 若 z 为小于 LAMMP_SI_MIN，则返回 LAMP_SI_MIN
 */
lamp_si lampz_to_si(const lampz_t z);

/**
 * @brief 大整数 z 转无符号整数
 * @param z 目标大整数
 * @return 无符号整数值
 * @note 若 z 大于 LAMMP_UI_MAX 或为 nullptr，则返回 LAMMP_UI_MAX
 * @warning 若 z 为负数，将会与 z 的符号无关，即返回值仅取决于 z 的绝对值
 */
lamp_ui lampz_to_ui(const lampz_t z);

/**
 * @brief 复制赋值：z1 = z2（z1 的容量如果不够，会自动分配新内存）
 * @note z2 不会被释放，而是被复制到 z1 中
 */
void lampz_copy(lampz_t z1, const lampz_t z2);

/**
 * @brief 移动赋值：z1 = z2（z1 的原有内存会被释放，并指向 z2 的内存）
 */
void lampz_move(lampz_t z1, lampz_t z2);

/**
 * @brief 交换赋值：z1 <-> z2（z1 和 z2 的内存地址交换）
 * @note 等价于 std::swap(z1, z2)
 */
void lampz_swap(lampz_t z1, lampz_t z2);

/*
bool lampz_is_prime(const lampz_t n);
void lampz_factorial(lampz_t& result, const lampz_t n);
void lampz_fibonacci(lampz_t& result, const lampz_t n);
void lampz_gcd(lampz_t& result, const lampz_t a, const lampz_t b);
void lampz_lcm(lampz_t& result, const lampz_t a, const lampz_t b);
void lampz_pow(lampz_t& result, const lampz_t base, const lampz_t exponent);
void lampz_sqrt(lampz_t& result, const lampz_t x);
void lampz_find_root(lampz_t& result, const lampz_t x, const lampz_t n);
void lampz_pow_mod(lampz_t& result, const lampz_t base, const lampz_t exponent,
const lampz_t modulus);
*/

#endif  // LAMPZ_H