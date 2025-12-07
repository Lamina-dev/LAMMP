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

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
// Windows 平台：用 _aligned_malloc（参数顺序：size, alignment）
#include <malloc.h>
#define LAMMP_ALLOC(align, size) _aligned_malloc((size), (align))

#elif defined(__cplusplus) && __cplusplus >= 201703L
// C++17+ 平台（Linux/macOS/MinGW）：用 posix_memalign
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
// 其他平台（低版本 C/C++）：用 posix_memalign
#include <stdlib.h>
#define LAMMP_ALLOC(align, size)                                                       \
    ({                                                                                 \
        void* _ptr = NULL;                                                             \
        (posix_memalign(&_ptr, (align), (size)) == 0) ? _ptr : (errno = ENOMEM, NULL); \
    })
#endif

#if defined(_WIN32)
// Windows _aligned_malloc 分配的内存，需用 _aligned_free 释放
#define LAMMP_FREE(ptr) _aligned_free((ptr))
#else
// Linux/macOS/MinGW：posix_memalign/aligned_alloc 分配的内存，用 free 释放
#define LAMMP_FREE(ptr) free((ptr))
#endif

#include <memory.h>
#include <stdbool.h> 
#include <stddef.h> 
#include <stdint.h> 
#include <stdlib.h> 

typedef int64_t lamp_si;           // 有符号整数：用于长度标记（正负表符号）、索引
typedef uint64_t lamp_ui;          // 无符号64位：大整数的单个“字”（word）类型
typedef lamp_ui* lamp_ptr;         // 大整数的内存指针：指向 lamp_ui 数组
typedef max_align_t lamp_align_t;  // 最大对齐类型（用于对齐检查）

// 配置宏
#define LAMPZ_ALIGN 8      // 内存对齐粒度（必须是 2 的幂，且 >= sizeof(lamp_ui)=8）
#define LAMPUI_BITS 64     // 单个字的位宽（与 lamp_ui 一致，64位）
#define LAMPSI_BITS 64     // 有符号长度的位宽（与 lamp_si 一致）
#define LAMPZ_MIN_LEN 1    // 大整数最小字长（至少1个64位字）
#define LAMMP_TALLOC_LEN 4 // 尾部赋值缓冲区长度（用于尾部赋值）

// 大整数核心结构体（指针类型 lampz_t 直接操作）
typedef struct lampz {
    lamp_ptr begin;  // 指向大整数的字数组（lamp_ui[]）起始地址
    lamp_ptr end;    // 指向字数组末尾的下一个位置（end = begin + 实际字长）
    lamp_si len;     // 长度标记：正负表符号（正=非负，负=负），绝对值=字数组长度（count of lamp_ui）
}* lampz_t;

/**
 * @brief 获取大整数的字数组指针（安全封装）
 * @param z 大整数对象
 * @return 成功返回 lamp_ui[] 指针，失败返回 NULL
 */
static inline lamp_ptr __lampz_get_ptr(lampz_t z) { return (z != nullptr && z->begin != nullptr) ? z->begin : nullptr; }

/**
 * @brief 获取大整数的缓冲区长度
 * @param z 大整数对象
 * @return 字长（count of lamp_ui），失败返回 0
 */
static inline lamp_ui __lampz_get_capacity(const lampz_t z) { return z != nullptr ? (lamp_ui)(z->end - z->begin + 1) : 0; }

/** 
 * @brief 获取大整数的长度（字长）
 * @param z 大整数对象
 * @return 字长（count of lamp_ui），失败返回 0
 */
static inline lamp_ui lampz_get_len(const lampz_t z) { return z != nullptr ? (lamp_ui)llabs(z->len) : 0; }

/**
 * @brief 获取大整数的符号
 * @param z 大整数对象
 * @return 1=非负，-1=负，0=对象无效
 */
static inline lamp_si lampz_get_sign(const lampz_t z) {
    if (z == nullptr || z->begin == nullptr)
        return 0;
    return z->len >= 0 ? 1 : -1;
}

/**
 * @brief 检查大整数的内存对齐是否合法
 * @param z 大整数对象
 * @return 0=对齐合法，-1=无效对象，1=对齐非法
 */
static inline lamp_si __lampz_check_alignment(const lampz_t z) {
    if (!z || !z->begin)
        return -1;
    // 检查 begin 地址是否按 LAMPZ_ALIGN 对齐（大整数内存必须对齐，否则影响运算效率/正确性）
    return ((size_t)z->begin % LAMPZ_ALIGN == 0) ? 0 : 1;
}

/**
 * @brief 判空大整数（是否为有效对象）
 * @param z 大整数对象
 * @return true=无效（NULL或无内存），false=有效
 */
static inline bool lampz_is_null(const lampz_t z) {
    return (z == nullptr || z->begin == nullptr || __lampz_get_capacity(z) < LAMPZ_MIN_LEN);
}

/**
 * @brief 设置大整数的符号（不改变字长和数据）
 * @param z 大整数对象（必须有效）
 * @param sign 符号（1=非负，-1=负）
 */
static inline void lampz_set_sign(lampz_t z, lamp_si sign) {
    if (z != nullptr && z->len != 0) {
        z->len = (sign >= 0) ? (lamp_si)llabs(z->len) : -(lamp_si)llabs(z->len);
    }
}

/**
 * @brief 创建一个新的大整数对象
 * @param bit 大整数所需的最小二进制位宽（例如：要存储200位的数，bit=200）
 * @return 成功返回 lampz_t 对象，失败返回 NULL（内存分配失败）
 * @note 内部会自动向上对齐到 lamp_ui 字长，且内存按 LAMPZ_ALIGN 对齐
 */
static inline lampz_t lampz_new(lamp_ui bit = 0) {
    lamp_si req_len;
    if (bit == 0) {
        req_len = LAMPZ_MIN_LEN;  // 位宽为0时，默认1个word（存储0）
    } else {
        // ceil(bit / LAMPUI_BITS) = (bit + LAMPUI_BITS - 1) / LAMPUI_BITS
        req_len = (lamp_si)((bit + LAMPUI_BITS - 1) / LAMPUI_BITS);
    }
    if (req_len < LAMPZ_MIN_LEN) {
        req_len = LAMPZ_MIN_LEN;
    }

    const size_t alloc_bytes = (size_t)req_len * sizeof(lamp_ui);

    lamp_ptr word_ptr = (lamp_ptr)LAMMP_ALLOC(LAMPZ_ALIGN, alloc_bytes);
    if (word_ptr == nullptr) {
        return nullptr;  // 内存分配失败
    }

    lampz_t z = (lampz_t)malloc(sizeof(lampz));
    if (z == nullptr) {
        LAMMP_FREE(word_ptr);  // 结构体分配失败，释放已分配的字数组内存
        return nullptr;
    }

    z->begin = word_ptr;
    z->end = word_ptr + req_len;        
    z->len = 0;  

    memset(z->begin, 0, alloc_bytes);  // 字数组内存初始化为0
    return z;
}

static inline lampz_t __lampz_malloc(lamp_ui word_len) {
    if (word_len < LAMPZ_MIN_LEN) {
        word_len = LAMPZ_MIN_LEN;
    }
    const size_t alloc_bytes = (size_t)word_len * sizeof(lamp_ui);
    lamp_ptr word_ptr = (lamp_ptr)LAMMP_ALLOC(LAMPZ_ALIGN, alloc_bytes);
    if (word_ptr == nullptr) {
        return nullptr;  // 内存分配失败
    }
    lampz_t z = (lampz_t)malloc(sizeof(lampz));
    if (z == nullptr) {
        LAMMP_FREE(word_ptr);  // 结构体分配失败，释放已分配的字数组内存
        return nullptr;
    }
    z->begin = word_ptr;
    z->end = word_ptr + word_len;
    z->len = 0;
    return z;
}

static inline lampz_t __lampz_calloc(lamp_ui word_len) {
    if (word_len < LAMPZ_MIN_LEN) {
        word_len = LAMPZ_MIN_LEN;
    }
    const size_t alloc_bytes = (size_t)word_len * sizeof(lamp_ui);
    lamp_ptr word_ptr = (lamp_ptr)LAMMP_ALLOC(LAMPZ_ALIGN, alloc_bytes);
    if (word_ptr == nullptr) {
        return nullptr;  // 内存分配失败
    }
    lampz_t z = (lampz_t)malloc(sizeof(lampz));
    if (z == nullptr) {
        LAMMP_FREE(word_ptr);  // 结构体分配失败，释放已分配的字数组内存
        return nullptr;
    }
    z->begin = word_ptr;
    z->end = word_ptr + word_len;
    z->len = 0;
    memset(z->begin, 0, alloc_bytes);  // 字数组内存初始化为0
    return z;
}

/** 
 * @brief 创建一个新的大整数对象（尾部赋值缓冲区）
 * @param word_len 大整数所需的字长
 * @param talloc_len 尾部赋值缓冲区长度（默认 LAMMP_TALLOC_LEN）
 * @return 成功返回 lampz_t 对象，失败返回 NULL（内存分配失败）
 * @note 内部会自动向上对齐到 lamp_ui 字长，且内存按 LAMPZ_ALIGN 对齐
 * @note 尾部赋值缓冲区初始化，用于减少赋零初始化的开销操作
 * @warning 不建议外部使用，除非你知道自己在做什么
 */
static inline lampz_t __lampz_talloc(lamp_ui word_len, lamp_ui talloc_len = LAMMP_TALLOC_LEN) {
    if (word_len < LAMPZ_MIN_LEN) {
        word_len = LAMPZ_MIN_LEN;
    }
    const size_t alloc_bytes = (size_t)word_len * sizeof(lamp_ui);
    lamp_ptr word_ptr = (lamp_ptr)LAMMP_ALLOC(LAMPZ_ALIGN, alloc_bytes);
    if (word_ptr == nullptr) {
        return nullptr;  // 内存分配失败
    }
    lampz_t z = (lampz_t)malloc(sizeof(lampz));
    if (z == nullptr) {
        LAMMP_FREE(word_ptr);  // 结构体分配失败，释放已分配的字数组内存（避免泄漏）
        return nullptr;
    }
    z->begin = word_ptr;
    z->end = word_ptr + word_len;
    z->len = 0;
    if (word_len < talloc_len) {
        memset(z->begin, 0, alloc_bytes);  // 字数组内存初始化为0
    } else {
        memset(z->end - talloc_len, 0, talloc_len * sizeof(lamp_ui));
    }
    return z;
}

/**
 * @brief 释放对象
 * @param z 大整数对象（可以是 NULL，内部安全处理）
 * @note 必须调用此函数释放，不能直接 free(z)（会泄露数字数组内存）
 */
static inline void lampz_free(lampz_t& z) {
    if (z != nullptr) {
        LAMMP_FREE(z->begin);  // 先释放字数组内存
        free(z);               // 再释放结构体本身
    }
    z = nullptr;
}

// -----------------------------------------------------------------------------
// 大整数运算函数声明
// -----------------------------------------------------------------------------

/**
 * @brief 二元运算：z = x + y（z 的容量如果不够，会自动分配新内存）
 */
void lampz_add_xy(lampz_t& z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x - y（z 的容量如果不够，会自动分配新内存）
 */
void lampz_sub_xy(lampz_t& z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x * y（z 的容量如果不够，会自动分配新内存）
 * @note 如果 x 和 y 的指针相同，将自动调用 lampz_sqr_x 函数
 */
void lampz_mul_xy(lampz_t& z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x / y（整数除法，向下取整，z 的容量如果不够，会自动分配新内存）
 */
void lampz_div_xy(lampz_t& z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x % y（取余，结果符号与 x 一致，z 的容量如果不够，会自动分配新内存）
 */
void lampz_mod_xy(lampz_t& z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：q = x / y，r = x % y（同时计算商和余数，q,r 的容量如果不够，会自动分配新内存）
 */
void lampz_div_mod_xy(lampz_t& q, lampz_t& r, const lampz_t x, const lampz_t y);

/**
 * @brief 一元运算：z += x（z 自身累加 x，z 的容量如果不够，会自动分配新内存）
 */
void lampz_add_x(lampz_t& z, const lampz_t x);

/**
 * @brief 一元运算：z -= x（z 自身减去 x，z 的容量如果不够，会自动分配新内存）
 */
void lampz_sub_x(lampz_t& z, const lampz_t x);

/**
 * @brief 一元运算：z *= x（z 自身乘以 x，z 的容量如果不够，会自动分配新内存）
 */
void lampz_mul_x(lampz_t& z, const lampz_t x);

/**
 * @brief 一元运算：z = x * x（平方，效率高于普通乘法，z 的容量如果不够，会自动分配新内存）
 */
void lampz_sqr_x(lampz_t& z, const lampz_t x);

/**
 * @brief 一元运算：z /= x（z 自身除以 x，z 将会自动分配新内存）
 */
void lampz_div_x(lampz_t& z, const lampz_t x);

/**
 * @brief 一元运算：z = z % x（z 自身取余 x，z 将会自动分配新内存）
 */
void lampz_mod_x(lampz_t& z, const lampz_t x);

lamp_si lampz_is_zero(const lampz_t z);

void lampz_set_ui(lampz_t& z, lamp_ui value);
void lampz_set_si(lampz_t& z, lamp_si value);
void lampz_set_str(lampz_t& z, const char* str, lamp_ui base);

lamp_ui lampz_to_str_len(const lampz_t z, lamp_ui base);
lamp_ui lampz_to_str(char* str, const lamp_ui str_len, const lampz_t z, lamp_ui base);
void lampz_clean(lampz_t& z);

void lampz_copy(lampz_t& z1, const lampz_t z2);
void lampz_move(lampz_t& z1, lampz_t& z2);
void lampz_swap(lampz_t& z1, lampz_t& z2);

/*
bool lampz_is_prime(const lampz_t n);
void lampz_factorial(lampz_t& result, const lampz_t n);
void lampz_fibonacci(lampz_t& result, const lampz_t n);
void lampz_gcd(lampz_t& result, const lampz_t a, const lampz_t b);
void lampz_lcm(lampz_t& result, const lampz_t a, const lampz_t b);
void lampz_pow(lampz_t& result, const lampz_t base, const lampz_t exponent);
void lampz_sqrt(lampz_t& result, const lampz_t x);
void lampz_find_root(lampz_t& result, const lampz_t x, const lampz_t n);
void lampz_pow_mod(lampz_t& result, const lampz_t base, const lampz_t exponent, const lampz_t modulus);
*/

#ifdef __cplusplus
}
#endif

#endif  // LAMPZ_H