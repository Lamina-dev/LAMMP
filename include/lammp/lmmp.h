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

#ifndef LAMMP_LMMP_H
#define LAMMP_LMMP_H

#include <stddef.h> 
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LAMMP_DEFAULT_STACK_SIZE 128*1024 // 默认全局栈大小，单位为字节

/* LAMMP 调试宏，定义为1时，会开启相应的调试功能，共有三个开销等级：低、中、高。 */

// 开启时，将会检查栈溢出；开销：低
#define LAMMP_DEBUG_DEFAULT_STACK_OVERFLOW_CHECK 1

// 开启时，将会进行debug_assert的检查；开销：中
#define LAMMP_DEBUG_ASSERT_CHECK 0

// 开启时，将会进行参数检查；开销：中
#define LAMMP_DEBUG_PARAM_ASSERT_CHECK 0

// 开启时，将会进行堆内存的检查；开销：高
#define LAMMP_DEBUG_MEMORY_CHECK 0

// 开启时，会增加内存分配和释放次数的统计功能
// 需要手动调用检查宏
#define LAMMP_DEBUG_ALLOC_FREE_COUNT 0

/*
 LAMMP 内存分配函数指针类型：
 1. heap_alloc : 堆内存分配器
 2. heap_free : 堆内存释放器
 3. realloc : 堆内存重新分配器
 4. stack_get_top : 获取当前栈顶指针
 5. stack_set_top : 设置当前栈顶指针

 前三个函数默认使用 malloc、free、realloc 实现，
 而 stack_get_top 和 stack_set_top 函数默认实现为：
  首次调用时分配一块大小为LAMMP_DEFAULT_STACK_SIZE的堆内存
  （通过调用 heap_alloc 函数），通过维护此堆内存来模拟栈。
  请注意，我们默认栈是向上增长的，即从低地址到高地址。
 
 如果使用自定义栈，请进行手动内存管理和处理溢出。默认栈是全局的，
 同时默认栈的大小是可调的，可以通过函数 lmmp_default_stack_reset 来调整
 */

typedef void* (*lmmp_heap_alloc_fn)(size_t size);
typedef void (*lmmp_heap_free_fn)(void* ptr);
typedef void* (*lmmp_realloc_fn)(void* ptr, size_t size);
typedef void* (*lmmp_stack_get_top_fn)(void);
typedef void (*lmmp_stack_set_top_fn)(void* top);

lmmp_heap_alloc_fn lmmp_set_heap_alloc_fn(lmmp_heap_alloc_fn func);
lmmp_heap_free_fn lmmp_set_heap_free_fn(lmmp_heap_free_fn func);
lmmp_realloc_fn lmmp_set_realloc_fn(lmmp_realloc_fn func);
lmmp_stack_get_top_fn lmmp_set_stack_get_top_fn(lmmp_stack_get_top_fn func);
lmmp_stack_set_top_fn lmmp_set_stack_set_top_fn(lmmp_stack_set_top_fn func);

/**
 * @brief LAMMP 全局默认栈重置函数
 * @param size 新的默认栈大小，单位为字节
 * @warning 请注意，此函数会释放掉当前的默认栈，并重新分配一块新的堆内存作为默认栈。
 *          因此，调用此函数后，访问之前的分配的栈空间将会导致未定义行为。
 * @note 当 size 为 0 时，将会释放调用默认栈，如果此后再使用栈内存，将会重新申请一块大小为
 *        LAMMP_DEFAULT_STACK_SIZE 的堆内存作为默认栈。
 */
void lmmp_default_stack_reset(size_t size);

typedef enum {
    LAMMP_ERROR_ASSERT_FAILURE = 1,
    LAMMP_ERROR_DEBUG_ASSERT_FAILURE = 2,
    LAMMP_ERROR_PARAM_ASSERT_FAILURE = 3,
    LAMMP_ERROR_MEMORY_ALLOC_FAILURE = 4,
    LAMMP_ERROR_MEMORY_FREE_FAILURE = 5,
    LAMMP_ERROR_OUT_OF_BOUNDS = 6,
    LAMMP_ERROR_UNEXPECTED_ERROR = 7
} lmmp_error_t;

/**
 * @brief LAMMP 全局退出函数指针类型
 * @param type 退出类型（可以查看lmmp_abort函数对此参数的说明，这里不再重复）
 * @param msg 退出信息，取决于type
 * @param file 退出处的文件名
 * @param line 退出处的行号
 */
typedef void (*lmmp_abort_fn)(lmmp_error_t type, const char* msg, const char* file, int line);

/**
 * @brief 设置 LAMMP 全局退出函数
 * @param func 退出函数指针，可以为NULL
 * @return 返回之前的退出函数指针，若原指针为NULL，则返回NULL。
 * @warning 请注意，我们将不会对 func 的调用做任何保护，因此请不要在 func 里做任何危险的操作，
 *          本库的开发者不对 func 函数的调用产生的影响做任何保证。
 */
lmmp_abort_fn lmmp_set_abort_fn(lmmp_abort_fn func);

/**
 * @brief LAMMP 全局退出函数，内部错误或断言失败时调用，若设置了全局退出函数，则会调用该函数，否则会调用默认的退出函数。
 * @param msg 退出信息，assert类型的错误信息通常仅包含断言内容，其他类型的错误则因类型不同而不同。
 * @param file 退出处的文件名
 * @param line 退出处的行号
 * @param type 退出类型。有以下几个类型：
 * 
 *        1. ASSERT_FAILURE （枚举值为1）为lmmp_assert触发的退出，lmmp_assert触发的普通退出几乎不可能发生，
 *             其通常代表不可能发生的计算错误，可能表明程序其他部分的计算错误。比如预期无进位的加法产生了进位。
 *             此类错误不可接受，会导致计算无法继续进行，导致程序崩溃。
 *
 *        2. DEBUG_ASSERT_FAILURE （枚举值为2）为lmmp_debug_assert触发的退出，其通常表明预期之外的错误，
 *             大部分情况下，可能是调用者未按照规定使用函数，导致函数入参检查失败，在函数开头通常有lmmp_debug_assert宏
 *             来检查部分参数的输入，不排除其他地方出现的错误。此类型只会在定义了 LAMMP_DEBUG_ASSERT_CHECK 宏为 1 的
 *             情况下才会触发，
 *         
 *        3. PARAM_ASSERT_FAILURE （枚举值为3）为参数检查失败导致的退出，其通常表明调用者传入了无效的参数，
 *             导致函数的行为不符合预期。此类错误不可接受，会导致计算无法继续进行，导致程序崩溃。此类型的错误只有在
 *             定义了 LAMMP_DEBUG_PARAM_ASSERT_CHECK 宏为 1 的情况下才会触发。
 *
 *        4. MEMORY_ALLOC_FAILURE （枚举值为4）为内存分配失败退出，这可能有两种情况：一种情况为分配了堆内存不足，
 *             导致程序崩溃；另一种情况为默认栈溢出（若是自定义栈，也不会有此情况），其中，情况一是会永远进行的，而情况二
 *             只有在定义了 LAMMP_DEBUG_DEFAULT_STACK_OVERFLOW_CHECK 宏为 1 的情况下才会触发。
 *
 *        5. MEMORY_FREE_FAILURE （枚举值为5）为内存释放错误，此错误只有一种触发可能，那就是使用默认栈时，调整的栈帧小于
 *             栈底指针，导致栈下溢，只有在定义了 LAMMP_DEBUG_DEFAULT_STACK_OVERFLOW_CHECK 宏为 1 的情况下才会触发。
 * 
 *        6. OUT_OF_BOUNDS （枚举值为6）为数组越界访问导致的退出，通常表明未按规定分配空间，或者计算内部变量超
 *             出范围。此类型只会在定义了 MEMORY_CHECK 宏为 1 的情况下才会触发，Release 模式下通常为 0 。
 *
 *        7. UNEXPECTED_ERROR （枚举值为7）为其他未知错误导致的退出。目前暂未使用，为预留作用。
 *
 * @note 相应的错误检查开关宏可以自行查阅上面的说明。
 *
 * @warning LAMMP内部中断都将会调用此函数，如果全局退出函数为NULL，则使用默认的退出函数，会打印出全部错误信息，并调用 
 *          abort 函数中断程序。设置全局退出函数请通过 lmmp_set_abort_func 函数进行设置。请不要在全局退出函数里做任
 *          何危险的操作，本库的开发者不对其调用产生的影响做任何保证。
 */
void lmmp_abort(lmmp_error_t type, const char* msg, const char* file, int line);


typedef uint8_t mp_byte_t;           // 字节类型 (8位无符号整数)
typedef uint64_t mp_limb_t;          // 基本运算单元(limb)类型 (64位无符号整数)
typedef uint64_t mp_size_t;          // 表示limb数量的无符号整数类型
typedef int64_t mp_slimb_t;          // 有符号limb类型 (64位有符号整数)
typedef int64_t mp_ssize_t;          // 表示limb数量的有符号整数类型
typedef mp_limb_t* mp_ptr;           // 指向limb类型的指针
typedef const mp_limb_t* mp_srcptr;  // 指向const limb类型的指针（源操作数指针）

#define LIMB_BITS 64
#define LIMB_BYTES 8
#define LOG2_LIMB_BITS 6
#define LIMB_MAX (~(mp_limb_t)0)

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define STATIC_ASSERT _Static_assert
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define STATIC_ASSERT static_assert
#else
// C99/C89 fallback (no message)
#define STATIC_ASSERT(cond, msg) typedef char static_assert_##__LINE__[(cond) ? 1 : -1]
#endif

STATIC_ASSERT(sizeof(void*) == 8, "64-bit architecture required");

#undef STATIC_ASSERT

#if MEMORY_CHECK == 1
void* lmmp_alloc(size_t size, const char* file, int line);
#define lmmp_alloc(size) lmmp_alloc(size, __FILE__, __LINE__)
#else
/**
 * @brief 内存分配函数（调用lmmp_heap_alloc_fn）
 * @param size 要分配的内存字节数
 * @return 成功返回指向分配内存的指针，失败返回NULL
 * @note 是标准malloc的安全封装版本
 */
void* lmmp_alloc(size_t size);
#endif 

#if MEMORY_CHECK == 1
void* lmmp_realloc(void* ptr, size_t size, const char* file, int line);
#define lmmp_realloc(ptr, size) lmmp_realloc(ptr, size, __FILE__, __LINE__)
#else
/**
 * @brief 内存重分配函数（调用lmmp_realloc_fn）
 * @param ptr 已分配的内存指针
 * @param size 新的内存大小（字节）
 * @return 成功返回指向新内存区域的指针，失败返回NULL
 * @note 是标准realloc的安全封装版本
 */
void* lmmp_realloc(void* ptr, size_t size);
#endif 


#if MEMORY_CHECK == 1
void lmmp_free(void* ptr, const char* file, int line);
#define lmmp_free(ptr) lmmp_free(ptr, __FILE__, __LINE__)
#else
/**
 * @brief 内存释放函数（调用lmmp_heap_free_fn）
 * @param ptr 要释放的内存指针
 * @note 是标准free的安全封装版本，确保空指针释放安全
 */
void lmmp_free(void*);
#endif

#if ALLOC_FREE_COUNT == 1
/**
 * @brief 获取当前分配的内存数量
 * @param cnt 如果cnt不为负数，则返回当前分配的内存数量（旧值），并将当前计数器值设置为cnt。
 *            若为负数，则仅返回当前分配的内存数量（新值）。
 * @return 当前分配的内存数量（旧值）
 */
int lmmp_alloc_count(int cnt);
#endif

// 计算整数的绝对值
#define LMMP_ABS(x) ((x) >= 0 ? (x) : -(x))
// 返回两个数中的较小值
#define LMMP_MIN(l, o) ((l) < (o) ? (l) : (o))
// 返回两个数中的较大值
#define LMMP_MAX(h, i) ((h) > (i) ? (h) : (i))
// 交换两个同类型变量的值
#define LMMP_SWAP(x, y, type) \
    do {                      \
        type _swap_ = (x);    \
        (x) = (y);            \
        (y) = _swap_;         \
    } while (0)
// 检查n是否为2的整数次幂
#define LMMP_POW2_Q(n) (((n) & ((n) - 1)) == 0)
// 将a向上取整为m的整数倍
#define LMMP_ROUND_UP_MULTIPLE(a, m) ((a) + (m) - 1 - ((a) + (m) - 1) % (m))

/**
 * @brief 临时堆内存分配函数
 * @param pmarker 标记
 * @param size 要分配的内存字节数
 */
void* lmmp_temp_heap_alloc_(void** pmarker, size_t size);

/**
 * @brief 临时栈内存分配函数
 * @param pmarker 标记
 * @param size 要分配的内存字节数
 */
void* lmmp_temp_stack_alloc_(void** pmarker, size_t size);

/**
 * @brief 临时堆内存释放函数
 * @param marker 要释放的临时内存标记
 */
void lmmp_temp_heap_free_(void* marker);

/**
 * @brief 临时栈内存释放函数
 * @param marker 要释放的临时内存标记
 */
void lmmp_temp_stack_free_(void* marker);

// 临时内存标记声明：用于跟踪临时内存分配
#define TEMP_DECL void *lmmp_temp_alloc_marker_ = NULL, *lmmp_temp_stack_marker_ = NULL

// 栈内存分配：使用alloca在栈上分配n字节内存（小内存）
#define TEMP_SALLOC(n) lmmp_temp_stack_alloc_(&lmmp_temp_stack_marker_, (n))
// 堆内存分配：使用lmmp_temp_alloc_在堆上分配n字节内存（大内存）
#define TEMP_BALLOC(n) lmmp_temp_heap_alloc_(&lmmp_temp_alloc_marker_, (n))
// 临时内存分配：小内存用栈，大内存用堆
#define TEMP_TALLOC(n) ((n) <= 0x7f00 ? TEMP_SALLOC(n) : TEMP_BALLOC(n))
// 类型化栈内存分配：分配n个type类型的栈内存
#define SALLOC_TYPE(n, type) ((type*)TEMP_SALLOC((n) * sizeof(type)))
// 类型化堆内存分配：分配n个type类型的堆内存
#define BALLOC_TYPE(n, type) ((type*)TEMP_BALLOC((n) * sizeof(type)))
// 类型化临时内存分配：智能选择栈/堆分配n个type类型内存
#define TALLOC_TYPE(n, type) ((type*)TEMP_TALLOC((n) * sizeof(type)))
// 临时内存释放：释放所有通过TEMP_XALLOC系列函数分配的临时内存
#define TEMP_FREE                                           \
    do {                                                    \
        if (lmmp_temp_alloc_marker_)                        \
            lmmp_temp_heap_free_(lmmp_temp_alloc_marker_);  \
        if (lmmp_temp_stack_marker_)                        \
            lmmp_temp_stack_free_(lmmp_temp_stack_marker_); \
    } while (0)

// 类型化内存分配：分配n个type类型的内存（堆）
#define ALLOC_TYPE(n, type) ((type*)lmmp_alloc((size_t)(n) * sizeof(type)))
// 类型化内存重分配：将p指向的内存重分配为new_size个type类型
#define REALLOC_TYPE(p, new_size, type) ((type*)lmmp_realloc((p), (new_size) * sizeof(type)))

// 内存拷贝宏：拷贝n个limb（每个8字节），使用memmove保证重叠安全
#define lmmp_copy(dst, src, n) memmove(dst, src, (n) << 3)
// 内存置零宏：将n个limb置零（每个8字节）
#define lmmp_zero(dst, n) memset(dst, 0, (n) << 3)
// 内存设置宏：将n个limb的值设置为val（每个8字节）
#define lmmp_set(dst, val, n) memset(dst, val, (n) << 3)

// 断言宏：检查条件x是否成立，不成立则触发段错误（严格的错误检查）
// RELEASE 版本也会检查
#define lmmp_assert(x)                                                      \
    do {                                                                    \
        if (!(x)) {                                                         \
            lmmp_abort(LAMMP_ERROR_ASSERT_FAILURE, #x, __FILE__, __LINE__); \
        }                                                                   \
    } while (0)

#if LAMMP_DEBUG == 1
// 调试断言宏：检查条件x是否成立，不成立则触发段错误（调试版本）
#define lmmp_debug_assert(x)                                                \
    do {                                                                    \
        if (!(x)) {                                                         \
            lmmp_abort(LAMMP_DEBUG_ASSERT_FAILURE, #x, __FILE__, __LINE__); \
        }                                                                   \
    } while (0)
#else
// 调试断言宏：检查条件x是否成立，不成立则触发段错误（调试版本）
#define lmmp_debug_assert(x) ((void)0)
#endif

#if ALLOC_FREE_COUNT == 1
#define ALLOC_FREE_COUNT_CHECK lmmp_assert(lmmp_alloc_count(-1) == 0 && "Memory leak detected")
#else
#define ALLOC_FREE_COUNT_CHECK ((void)0)
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // LAMMP_LMMP_H