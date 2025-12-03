#ifndef LAMPZ_H
#define LAMPZ_H

#include "alloc.h"  // 引入跨平台内存分配头文件（LAMMP_ALLOC/LAMMP_FREE）

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>  // 用于 bool（C99+）
#include <stddef.h>   // 用于 size_t
#include <stdint.h>   // 用于 int64_t/uint64_t
#include <stdlib.h>   // 用于 llabs（C99+）


// 基础类型定义（大整数库专用，明确语义）
typedef int64_t lamp_si;           // 有符号整数：用于长度标记（正负表符号）、索引
typedef uint64_t lamp_ui;          // 无符号64位：大整数的单个“字”（word）类型
typedef lamp_ui* lamp_ptr;         // 大整数的内存指针：指向 lamp_ui 数组（核心修正）
typedef max_align_t lamp_align_t;  // 最大对齐类型（用于对齐检查）

// 配置宏（大整数库核心参数，可根据需求调整）
#define LAMPZ_ALIGN 8    // 内存对齐粒度（必须是 2 的幂，且 >= sizeof(lamp_ui)=8）
#define LAMPUI_BITS 64   // 单个字的位宽（与 lamp_ui 一致，64位）
#define LAMPSI_BITS 64   // 有符号长度的位宽（与 lamp_si 一致）
#define LAMPZ_MIN_LEN 1  // 大整数最小字长（至少1个64位字）

// 大整数核心结构体（指针类型 lampz_t 直接操作）
typedef struct lampz {
    lamp_ptr begin;  // 指向大整数的字数组（lamp_ui[]）起始地址
    lamp_ptr end;    // 指向字数组末尾的下一个位置（end = begin + 实际字长）
    lamp_si len;     // 长度标记：正负表符号（正=非负，负=负），绝对值=字数组长度（count of lamp_ui）
}* lampz_t;

// -----------------------------------------------------------------------------
// 核心辅助函数（内联，无额外开销）
// -----------------------------------------------------------------------------

/**
 * @brief 获取大整数的字数组指针（安全封装）
 * @param z 大整数对象
 * @return 成功返回 lamp_ui[] 指针，失败返回 NULL
 */
static inline lamp_ptr lampz_get_ptr(lampz_t z) { return (z != NULL && z->begin != NULL) ? z->begin : NULL; }

/**
 * @brief 获取大整数的字长（不含符号，仅数量）
 * @param z 大整数对象
 * @return 字长（count of lamp_ui），失败返回 0
 */
static inline lamp_ui lampz_get_len(const lampz_t z) { return z != NULL ? (lamp_ui)llabs(z->len) : 0; }

/**
 * @brief 获取大整数的符号
 * @param z 大整数对象
 * @return 1=非负，-1=负，0=对象无效
 */
static inline lamp_si lampz_get_sign(const lampz_t z) {
    if (z == NULL || z->begin == NULL)
        return 0;
    return z->len >= 0 ? 1 : -1;
}

/**
 * @brief 检查大整数的内存对齐是否合法
 * @param z 大整数对象
 * @return 0=对齐合法，-1=无效对象，1=对齐非法
 */
static inline int lampz_check_alignment(const lampz_t z) {
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
    return (z == NULL || z->begin == NULL || lampz_get_len(z) < LAMPZ_MIN_LEN);
}

/**
 * @brief 设置大整数的符号（不改变字长和数据）
 * @param z 大整数对象（必须有效）
 * @param sign 符号（1=非负，-1=负）
 */
static inline void lampz_set_sign(lampz_t z, lamp_si sign) {
    if (z != NULL && z->len != 0) {
        z->len = (sign >= 0) ? (lamp_si)llabs(z->len) : -(lamp_si)llabs(z->len);
    }
}

// -----------------------------------------------------------------------------
// 核心构造/析构函数（跨平台兼容，内存安全）
// -----------------------------------------------------------------------------

/**
 * @brief 创建一个新的大整数对象
 * @param bit 大整数所需的最小位宽（例如：要存储200位的数，bit=200）
 * @param sign 符号（true=非负，false=负）
 * @return 成功返回 lampz_t 对象，失败返回 NULL（内存分配失败）
 * @note 内部会自动向上对齐到 lamp_ui 字长，且内存按 LAMPZ_ALIGN 对齐
 */
static inline lampz_t lampz_malloc(lamp_ui bit, bool sign) {
    lamp_si req_len;
    if (bit == 0) {
        req_len = LAMPZ_MIN_LEN;  // 位宽为0时，默认1个word（存储0）
    } else {
        // 公式：ceil(bit / LAMPUI_BITS) = (bit + LAMPUI_BITS - 1) / LAMPUI_BITS
        req_len = (lamp_si)((bit + LAMPUI_BITS - 1) / LAMPUI_BITS);
    }
    if (req_len < LAMPZ_MIN_LEN) {
        req_len = LAMPZ_MIN_LEN;
    }

    const size_t alloc_bytes = (size_t)req_len * sizeof(lamp_ui);

    lamp_ptr word_ptr = (lamp_ptr)LAMMP_ALLOC(LAMPZ_ALIGN, alloc_bytes);
    if (word_ptr == NULL) {
        return NULL;  // 内存分配失败
    }

    lampz_t z = (lampz_t)LAMMP_ALLOC(LAMPZ_ALIGN, sizeof(struct lampz));
    if (z == NULL) {
        LAMMP_FREE(word_ptr);  // 结构体分配失败，释放已分配的字数组内存（避免泄漏）
        return NULL;
    }

    z->begin = word_ptr;
    z->end = word_ptr + req_len;        
    z->len = sign ? req_len : -req_len;  

    return z;
}

/**
 * @brief 释放对象
 * @param z 大整数对象（可以是 NULL，内部安全处理）
 * @note 必须调用此函数释放，不能直接 free(z)（会泄露数字数组内存）
 */
static inline void lampz_free(lampz_t z) {
    if (z != NULL) {
        LAMMP_FREE(z->begin);  // 先释放字数组内存
        LAMMP_FREE(z);         // 再释放结构体本身
    }
}

// -----------------------------------------------------------------------------
// 大整数运算函数声明（保持你的原始设计，语义明确）
// -----------------------------------------------------------------------------

/**
 * @brief 二元运算：z = x + y（z 的容量如果不够，会自动分配新内存）
 */
void a_add_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x - y（z 的容量如果不够，会自动分配新内存）
 */
void a_sub_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x * y（z 的容量如果不够，会自动分配新内存）
 */
void a_mul_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x / y（整数除法，向下取整，z 的容量如果不够，会自动分配新内存）
 */
void a_div_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：z = x % y（取余，结果符号与 x 一致，z 的容量如果不够，会自动分配新内存）
 */
void a_mod_xy(lampz_t z, const lampz_t x, const lampz_t y);

/**
 * @brief 二元运算：q = x / y，r = x % y（同时计算商和余数，q,r 的容量如果不够，会自动分配新内存）
 */
void ab_div_mod_xy(lampz_t q, lampz_t r, const lampz_t x, const lampz_t y);

/**
 * @brief 一元运算：z += x（z 自身累加 x，z 的容量如果不够，会自动分配新内存）
 */
void a_add_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z -= x（z 自身减去 x，z 的容量如果不够，会自动分配新内存）
 */
void a_sub_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z *= x（z 自身乘以 x，z 的容量如果不够，会自动分配新内存）
 */
void a_mul_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z = x * x（平方，效率高于普通乘法，z 的容量如果不够，会自动分配新内存）
 */
void a_sqr_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z /= x（z 自身除以 x，z 将会自动分配新内存）
 */
void a_div_x(lampz_t z, const lampz_t x);

/**
 * @brief 一元运算：z = z % x（z 自身取余 x，z 将会自动分配新内存）
 */
void a_mod_x(lampz_t z, const lampz_t x);

void set_ui(lampz_t z, lamp_ui value);
void set_si(lampz_t z, lamp_si value);
void set_str(lampz_t z, const char* str, int base);
void set_str(lampz_t z, const char* str, int strlen, int base);
int to_str_len(const lampz_t z, int base);
void to_str(char* str, const int str_len, const lampz_t z, int base);
void clean(lampz_t z);

void copy(lampz_t z1, const lampz_t z2);
void move(lampz_t z1, lampz_t z2);
void swap(lampz_t z1, lampz_t z2);

bool is_prime(const lampz_t n);
void factorial(lampz_t result, const lampz_t n);
void fibonacci(lampz_t result, const lampz_t n);
void gcd(lampz_t result, const lampz_t a, const lampz_t b);
void lcm(lampz_t result, const lampz_t a, const lampz_t b);
void pow(lampz_t result, const lampz_t base, const lampz_t exponent);
void sqrt(lampz_t result, const lampz_t x);
void find_root(lampz_t result, const lampz_t x, const lampz_t n);
void pow_mod(lampz_t result, const lampz_t base, const lampz_t exponent, const lampz_t modulus);


#ifdef __cplusplus
}
#endif

#endif  // LAMPZ_H