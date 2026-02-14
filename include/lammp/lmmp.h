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

/*************************************************************
        符号说明:

        B           基数，固定为 2^64

        [p,n,b]     表示指针p指向的、以b为基数的n位数
                    p[i-1] 代表其第i位最低有效位 (0<i<=n)
                    如果省略b，则默认基数为B。通常情况下，
                    用此符号表示函数参数时，一般暗指高位不存在0，
                    而用此符号表示函数返回值时，表示写入的位数，
                    即使可能写入为0。

        sep         指针指向的内存区域完全分离

        eqsep       完全相同的内存区域或者完全分离

                    备注：我们都假定内存是向上增长的，dst <= num+1
                    的内存布局可以这样表示
                            dst ──┐
                   num ──┐        |00000000|00000000|
                         |********|********|

        MSB(x)      x的最高有效位，比如最高有效位为1，大部分语境下
                    代表 x >= B / 2

        [x|y]       x或y，用于表示参数或返回值的取值范围
****************************************************************/

#include <stddef.h> 
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        LAMMP_ASSERT_FAILURE = 1,
        LAMMP_DEBUG_ASSERT_FAILURE = 2,
        LAMMP_MEMORY_ALLOC_FAILURE = 3,
        LAMMP_OUT_OF_BOUNDS = 4,
        LAMMP_UNEXPECTED_ERROR = 5
} lmmp_error_t;

/**
 * LAMMP 全局退出函数指针类型
 * @param type 退出类型（可以查看lmmp_abort函数对此参数的说明，这里不再重复）
 * @param msg 退出信息，取决于type，并不一定很详细，大部分情况下为断言错误直接转换为字符串
 * @param file 退出处的文件名
 * @param line 退出处的行号
 */
typedef void (*lmmp_abort_func_t)(lmmp_error_t type, const char* msg, const char* file, int line);

/**
 * 设置 LAMMP 全局退出函数
 * @param func 退出函数指针，可以为NULL
 * @return 返回之前的退出函数指针，若原指针为NULL，则返回NULL。
 * @warning 请注意，我们将不会对 func 的调用做任何保护，因此请不要在 func 里做任何危险的操作，
 *          本库的开发者不对 func 函数的调用产生的影响做任何保证。
 */
lmmp_abort_func_t lmmp_set_abort_func(lmmp_abort_func_t func);

/**
 * LAMMP 全局退出函数，内部错误或断言失败时调用，若设置了全局退出函数，则会调用该函数，否则会调用默认的退出函数。
 * @param msg 退出信息，大部分情况下，为断言错误直接转换为字符串。若type为LAMMP_OUT_OF_BOUNDS，则会包含较多的信息，
 *         详细说明越界的指针、何处分配、何处销毁。详细信息可以查看 impl/safe_memory.h 中的相关函数实现。
 * @param file 退出处的文件名
 * @param line 退出处的行号
 * @param type 退出类型。有以下几个类型：
 * 
 *        1. LAMMP_ASSERT_FAILURE （枚举值为1）为lmmp_assert触发的退出，lmmp_assert触发的普通退出几乎不可能发生，
 *             其通常代表不可能发生的计算错误，可能表明程序其他部分的计算错误。比如预期无进位的加法产生了进位。
 *             此类错误不可接受，会导致计算无法继续进行，导致程序崩溃。
 *
 *        2. LAMMP_DEBUG_ASSERT_FAILURE （枚举值为2）为lmmp_debug_assert触发的退出，其通常表明预期之外的错误，
 *             大部分情况下，可能是调用者未按照规定使用函数，导致函数入参检查失败，在函数开头通常有lmmp_debug_assert宏
 *             来检查部分参数的输入，不排除其他地方出现的错误。此类型只会在定义了 LAMMP_DEBUG 宏为 1 的情况下才会触发，
 *             Release 模式下通常为 0 。
 *
 *        3. LAMMP_MEMORY_ALLOC_FAILURE （枚举值为3）为内存分配失败退出，这通常源于隐蔽的内存越界导致堆损坏，
 *             或者分配过大的系统内存。
 *
 *        4. LAMMP_OUT_OF_BOUNDS （枚举值为4）为数组越界访问导致的退出，通常表明未按规定分配空间，或者计算内部变量超
 *             出范围。此类型只会在定义了 MEMORY_CHECK 宏为 1 的情况下才会触发，Release 模式下通常为 0 。
 *
 *        5. LAMMP_UNEXPECTED_ERROR （枚举值为5）为其他未知错误导致的退出。目前暂未使用，为预留作用。
 *
 * @note + 调用此函数会导致本程序退出。需要注意的是，出于对性能的考量，在未定义LAMMP_DEBUG宏为 1（RELEASE模式下，
 *       其通常为0）的情况下，lmmp_debug_assert不会产生任何作用，也就是不会触发全局退出函数。在未定义 MEMORY_CHECK
 *       宏为 1（在RELEASE模式下， 其通常为0）的情况下，不会检查内存有无越界情况，也不会触发全局退出函数，
 *       不会产生 LAMMP_OUT_OF_BOUNDS 宏的退出。而 lmmp_assert 和 LAMMP_MEMORY_ALLOC_FAILURE 在何种情况下
 *       都会触发全局退出函数。
 *
 *       + 如果调用者希望在Release模式下开启 LAMMP_DEBUG_ASSERT_FAILURE 和 LAMMP_OUT_OF_BOUNDS ，需要在编译时
 *       定义相应的宏，无法在运行时动态设置。
 *
 * @warning LAMMP内部中断都将会调用此函数，如果全局退出函数为NULL，则使用默认的退出函数，会打印出全部错误信息，并调用 
 *          abort 函数中断程序。设置全局退出函数请通过 lmmp_set_abort_func 函数进行设置。请不要在全局退出函数里做任
 *          何危险的操作，本库的开发者不对其调用产生的影响做任何保证。
 */
void lmmp_abort(lmmp_error_t type, const char* msg, const char* file, int line);

// 此宏为1时，会增加lmmp_debug_assert的检查，包括入参检查和中间结果检查。
// 开启此宏可能会带来一定的性能开销
#define LAMMP_DEBUG 0
// 此宏为1时，会增加内存越界检查的功能。
// 开启此宏会带来较多的性能开销
#define MEMORY_CHECK 0

typedef uint8_t mp_byte_t;           // 字节类型 (8位无符号整数)
typedef uint64_t mp_limb_t;          // 基本运算单元(limb)类型 (64位无符号整数)
typedef uint64_t mp_size_t;          // 表示limb数量的无符号整数类型
typedef int64_t mp_slimb_t;          // 有符号limb类型 (64位有符号整数)
typedef int64_t mp_ssize_t;          // 表示limb数量的有符号整数类型
typedef mp_limb_t* mp_ptr;           // 指向limb类型的指针
typedef const mp_limb_t* mp_srcptr;  // 指向const limb类型的指针（源操作数指针）

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
 * 内存分配函数
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
 * 内存重分配函数
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
 * 内存释放函数
 * @param ptr 要释放的内存指针
 * @note 是标准free的安全封装版本，确保空指针释放安全
 */
void lmmp_free(void*);
#endif

// ===================== lmmp_ 底层不安全运算函数 =====================
/**
 * @brief 带进位的n位加法 [dst,n] = [numa,n] + [numb,n] + c
 * @warning c=[0|1], n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n limb长度
 * @param c 初始进位值 [0|1]
 * @return 运算后的最终进位值 [0|1]
 */
mp_limb_t lmmp_add_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * @brief 无进位的n位加法 [dst,n] = [numa,n] + [numb,n]
 * @warning n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n limb长度
 * @return 运算后的最终进位值 [0|1]
 */
mp_limb_t lmmp_add_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * @brief 带借位的n位减法 [dst,n] = [numa,n] - [numb,n] - c
 * @warning c=[0|1], n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n limb长度
 * @param c 初始借位值 [0|1]
 * @return 运算后的最终借位值 [0|1]
 */
mp_limb_t lmmp_sub_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * @brief 无借位的n位减法 [dst,n] = [numa,n] - [numb,n]
 * @warning n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n limb长度
 * @return 运算后的最终借位值 [0|1]
 */
mp_limb_t lmmp_sub_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * @brief 同时执行n位加法和减法 ([dsta,n],[dstb,n]) = ([numa,n]+[numb,n],[numa,n]-[numb,n])
 * @warning n>0, eqsep(dsta,[numa|numb]), eqsep(dstb,[numa|numb])
 * @param dsta 加法结果输出指针
 * @param dstb 减法结果输出指针
 * @param numa 第一个操作数指针（被加数/被减数）
 * @param numb 第二个操作数指针（加数/减数）
 * @param n limb长度
 * @return 组合返回值 cb = 2*c + b (c为加法进位, b为减法借位)
 *         返回值范围: 0(无进位无借位),1(无进位有借位),2(有进位无借位),3(有进位有借位)
 */
mp_limb_t lmmp_add_n_sub_n_(mp_ptr dsta, mp_ptr dstb, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * @brief 加法后右移1位 [dst,n] = ([numa,n] + [numb,n]) >> 1
 * @warning n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n limb长度
 * @return 右移操作产生的进位值 [0|1]
 */
mp_limb_t lmmp_shr1add_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * @brief 带进位加法后右移1位 [dst,n] = ([numa,n] + [numb,n] + c) >> 1
 * @warning n>0, c=[0|1], eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n limb长度
 * @param c 初始进位值 [0|1]
 * @return 右移操作产生的进位值 [0|1]
 */
mp_limb_t lmmp_shr1add_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * @brief 减法后右移1位 [dst,n] = ([numa,n] - [numb,n]) >> 1
 * @warning n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n 操作数的位数（limb数量）
 * @return 右移操作产生的进位值 (0或1)
 */
mp_limb_t lmmp_shr1sub_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * @brief 带借位减法后右移1位 [dst,n] = ([numa,n] - [numb,n] - c) >> 1
 * @warning n>0, c=[0|1], eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n limb长度
 * @param c 初始借位值 [0|1]
 * @return 右移操作产生的进位值 [0|1]
 */
mp_limb_t lmmp_shr1sub_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * @brief 大数右移操作 [dst,na] = [numa,na] >> shr，dst的高shr位填充0
 * @warning na>0, 0<=shr<64, eqsep(dst,numa)
 *          允许dst指针地址小于numa（即支持原地长移位操作）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na limb长度
 * @param shr 右移的位数 (0~63)
 * @return 其最高shr个比特位填充[numa,na]被移出的shr个最低位，其余比特位为0
 */
mp_limb_t lmmp_shr_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shr);

/**
 * @brief 带进位的大数右移操作 [dst,na] = [numa,na] >> shr，dst的高shr位填充c的高shr位
 * @warning na>0, 0<=shr<64, eqsep(dst,numa)
 *          c的低(64-shr)位必须为0
 *          允许dst指针地址小于numa（即支持原地长移位操作）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na limb长度
 * @param shr 右移的位数 (0~63)
 * @param c 进位值（其低(64-shr)位必须为0）
 * @return 其最高shr个比特位填充[numa,na]被移出的shr个最低位，其余比特位为0
 */
mp_limb_t lmmp_shr_c_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shr, mp_limb_t c);

/**
 * @brief 大数左移操作 [dst,na] = [numa,na] << shl，dst的低shl位填充0
 * @warning na>0, 0<=shl<64, eqsep(dst,numa)
 *         允许dst指针地址大于numa（即支持原地长移位操作）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na limb长度
 * @param shl 左移的位数 (0~63)
 * @return 其最低shl个比特位填充[numa,na]被移出的shl个最高位，其余比特位为0
 */
mp_limb_t lmmp_shl_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shl);

/**
 * @brief 带进位的大数左移操作 [dst,na] = [numa,na] << shl，dst的低shl位填充c的低shl位
 * @warning na>0, 0<=shl<64, eqsep(dst,numa)
 *          c的高(64-shl)位必须为0
 *          允许dst指针地址大于numa（即支持原地长移位操作）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na limb长度
 * @param shl 左移的位数 (0~63)
 * @param c 进位值（其高(64-shl)位必须为0）
 * @return 其最低shl个比特位填充[numa,na]被移出的shl个最高位，其余比特位为0
 */
mp_limb_t lmmp_shl_c_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shl, mp_limb_t c);

/**
 * @brief 大数按位取反操作 [dst,na] = ~[numa,na] (对每个limb执行按位非操作)
 * @warning na>0, eqsep(dst,numa)
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na limb长度
 */
void lmmp_not_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * @brief 左移后按位取反操作 [dst,na] = ~([numa,na] << shl)，dst的低shl位填充1
 * @warning na>0, 0<=shl<64, eqsep(dst,numa)
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na limb长度
 * @param shl 左移的位数 (0~63)
 * @return 其最低shl个比特位填充[numa,na]被移出的shl个最高位，其余比特位为0
 */
mp_limb_t lmmp_shlnot_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shl);

/**
 * @brief 加法结合左移1位操作 [dst,n] = [numa,n] + ([numb,n] << 1)
 * @warning n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 被加数指针
 * @param numb 加数指针（先左移1位）
 * @param n limb长度
 * @return 运算后的进位值 [0|1|2]
 */
mp_limb_t lmmp_addshl1_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * @brief 减法结合左移1位操作 [dst,n] = [numa,n] - ([numb,n] << 1)
 * @warning n>0, eqsep(dst,[numa|numb])
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针（先左移1位）
 * @param n limb长度
 * @return 运算后的借位值 [0|1|2]
 */
mp_limb_t lmmp_subshl1_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * @brief 大数乘以单limb并累加操作 [numa,n] += [numb,n] * b
 * @warning n>0, eqsep(numa,numb))
 * @param numa 被加数指针（结果也存储在此）
 * @param numb 乘数指针
 * @param n limb长度
 * @param b 乘数
 * @return 运算后的进位limb值
 */
mp_limb_t lmmp_addmul_1_(mp_ptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t b);

/**
 * @brief 大数乘以单limb并累减操作 [numa,n] -= [numb,n] * b
 * @warning n>0, eqsep(numa,numb))
 * @param numa 被减数指针（结果也存储在此）
 * @param numb 乘数指针
 * @param n limb长度
 * @param b 乘数
 * @return 运算后的借位limb值
 */
mp_limb_t lmmp_submul_1_(mp_ptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t b);

/**
 * @brief 大数乘以单limb操作 [dst,na] = [numa,na] * x
 * @warning na>0, eqsep(dst,numa)
 *       支持 dst<=numa+1 的内存布局
 * @param dst 结果输出指针
 * @param numa 被乘数指针
 * @param na 操作数的位数（limb数量）
 * @param x 单个limb乘数
 * @return 运算后的进位limb值
 */
mp_limb_t lmmp_mul_1_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_limb_t x);

/**
 * @brief 不等长大数乘法操作 [dst,na+nb] = [numa,na] * [numb,nb]
 * @warning 0<nb<=na, sep(dst,[numa|numb])
 *      特殊情况:  nb==1时dst<=numa+1是允许的
 *                nb==2时dst<=numa是允许的
 * @param dst 乘积结果输出指针（需要 na+nb 的 limb 长度）
 * @param numa 第一个乘数指针（较长的操作数）
 * @param na 第一个操作数的 limb 长度
 * @param numb 第二个乘数指针（较短的操作数）
 * @param nb 第二个操作数的 limb 长度
 */
void lmmp_mul_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief 大数除法和取模操作
 * @note 如果dstq不为NULL: [dstq,na-nb+1] = [numa,na] / [numb,nb] (商)
 *       如果dstr不为NULL: [dstr,nb] = [numa,na] mod [numb,nb] (余数)
 * @warning 0<nb<=na, numb[nb-1]!=0, sep(dstq,[numa|numb]), eqsep(dstr,[numa|numb]))
 *          特殊情况: nb==1时, dstq>=numa-1 是允许的
 *                   nb==2时, dstq>=numa 是允许的
 * @param dstq 商结果输出指针（NULL表示不计算商）
 * @param dstr 余数结果输出指针（NULL表示不计算余数）
 * @param numa 被除数指针
 * @param na 被除数的 limb 长度
 * @param numb 除数指针
 * @param nb 除数的 limb 长度
 */
void lmmp_div_(mp_ptr dstq, mp_ptr dstr, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * @brief 大数平方根和取余操作
 * @note 如果dstr不为NULL: [dsts,nf+na/2+1], [dstr,nf+na/2+1] = sqrtrem([numa,na]*B^(2*nf))
 *                         也即 [numa,na] × B^(2×nf) = [dsts,nf+na/2+1]^2 + [dstr,nf+na/2+1]
 *                         且 0 <= [dstr,nf+na/2+1] < 2 * [dsts,nf+na/2+1] + 1
 *        如果dstr为NULL:   [dsts,nf+na/2+1] = [round|floor](sqrt([numa,na]*B^(2*nf)))
 * @warning na>0, numa[na-1]!=0, eqsep(dsts,numa), eqsep(dstr,numa)
 * @param dsts 平方根结果输出指针
 * @param dstr 余数结果输出指针（NULL表示不计算余数）
 * @param numa 源操作数指针
 * @param na 操作数的 limb 长度
 * @param nf 精度因子
 */
void lmmp_sqrt_(mp_ptr dsts, mp_ptr dstr, mp_srcptr numa, mp_size_t na, mp_size_t nf);

/**
 * @brief 大数求逆操作 [dst,na+nf+1] = (B^(2*(na+nf)) - 1) / ([numa,na]*B^nf) + [0|-1]
 * @warning na>0, numa[na-1]!=0, eqsep(dst,numa)
 * @param dst 逆元结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的 limb 长度
 * @param nf 精度因子
 */
void lmmp_inv_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t nf);

/**
 * @brief 字符串转大数操作 [src,len,base] to [dst,return value,B]
 * @warning len>=0, 2<=base<=256
 * @param dst 大数结果输出指针
 * @param src 字符串源指针
 * @param len 字符串长度
 * @param base 字符串的进制基数
 * @return 转换后的大数 limb 长度
 */
mp_size_t lmmp_from_str_(mp_ptr dst, const mp_byte_t* src, mp_size_t len, int base);

/**
 * @brief 大数转字符串操作 [numa,na,B] to [dst,return value,base]
 * @warning na>=0, 2<=base<=256
 * @param dst 字符串结果输出指针
 * @param numa 大数源指针
 * @param na 大数的 limb 长度
 * @param base 目标字符串的进制基数
 * @return 转换后的字符串长度
 */
mp_size_t lmmp_to_str_(mp_byte_t* dst, mp_srcptr numa, mp_size_t na, int base);

/**
 * @brief 提取高位指定位数，并返回低位bits位数
 * @param num 待提取的大数指针
 * @param n num的 limb 长度
 * @param bits 待提取的位数(1-64)
 * @param ext 提取结果输出指针
 * @warning n>0, 1<=bits<=64, ext!=NULL
 * @note 如果bits大于num的实际位数，则不会保证ext有效位数为bits位；
 *       如果bits小于等于num的实际位数，则ext将会有bits位有效位数。
 * @return 剩余的低位bits数量
 */
mp_size_t lmmp_extract_bits_(mp_srcptr num, mp_size_t n, mp_limb_t* ext, int bits);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // LAMMP_LMMP_H