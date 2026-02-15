/*
===============================================================================
1. LGPL v2.1 LICENSED CODE (LAMMP Project)
Copyright (c) 2025-2026 HJimmyK/LAMINA
This file is part of LAMMP (LGPL v2.1 License)
Full LGPL v2.1 License Text: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
LAMMP Repository: https://github.com/Lamina-dev/LAMMP

Modification Note: This file contains modifications to the original MIT-licensed code to adapt to LAMMP's LGPL v2.1
environment.

===============================================================================
2. MIT LICENSED CODE (Original Source)
MIT License

Copyright (c) 2024-2050 HJimmyK
Project URL: https://github.com/HJimmyK/fast_mul_3ntt_crt_CC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __LAMMP_3NTT_CRT_CONST_H__
#define __LAMMP_3NTT_CRT_CONST_H__
#include <stdint.h>

typedef uint64_t mont64;
#define MONT64BIT 64
/* 全局静态变量获取 */

/* global const 原根 */
#define g_root(i) (global_ROOT##i)
/* global const 原根逆 */
#define g_rootinv(i) (global_root_inv##i)
/* global const 模数 */
#define g_mod(i) (global_mod##i)
/* global const R^2对模数的逆 */
#define g_r2(i) (global_r2##i)
/* global const 模数的平方 */
#define g_mod2(i) (global_mod2##i)
/* global const 模数关于R的逆的负数 */
#define g_modInvNeg(i) (global_modInvNeg##i)
/* global const G in menten form */
#define g_mont_root(i) (global_mont_ROOT##i)
/* global const Ginv in menten form */
#define g_mont_rootinv(i) (global_mont_ROOT_inv##i)
/* global const 1 in menten form */
#define g_one(i) (global_one##i)

/* internal global const */
#define g_w41(i) (global_w41_##i)
/* internal global const */
#define g_w41inv(i) (global_w41_inv##i)
/* internal global const */
#define g_w1(i) (global_w1_##i)
/* internal global const */
#define g_w2(i) (global_w2_##i)
/* internal global const */
#define g_w3(i) (global_w3_##i)
/* internal global const */
#define g_w1inv(i) (global_w1_inv##i)
/* internal global const */
#define g_w2inv(i) (global_w2_inv##i)
/* internal global const */
#define g_w3inv(i) (global_w3_inv##i)

/* R = 2^64 */

/* 原根 */
extern const uint32_t global_ROOT1;
extern const uint32_t global_ROOT2;
extern const uint32_t global_ROOT3;

/* 原根关于对应模数的逆 */
extern const uint64_t global_root_inv1;
extern const uint64_t global_root_inv2;
extern const uint64_t global_root_inv3;

/* 模数 */
extern const uint64_t global_mod1;
extern const uint64_t global_mod2;
extern const uint64_t global_mod3;

/* R^2 mod 模数 */
extern const uint64_t global_r21;
extern const uint64_t global_r22;
extern const uint64_t global_r23;

/* 模数的平方 */
extern const uint64_t global_mod21;
extern const uint64_t global_mod22;
extern const uint64_t global_mod23;

/* mont64(ROOT) */
extern const mont64 global_mont_ROOT1;
extern const mont64 global_mont_ROOT2;
extern const mont64 global_mont_ROOT3;

/* ROOTinv = ROOT^-1 % mod */
/* mont64(ROOTinv)         */
extern const mont64 global_mont_ROOT_inv1;
extern const mont64 global_mont_ROOT_inv2;
extern const mont64 global_mont_ROOT_inv3;

/* (mod_inv * mod) % R = 1         */
/* (mod_inv_neg + mod_inv) % R = 0 */
extern const uint64_t global_modInvNeg1;
extern const uint64_t global_modInvNeg2;
extern const uint64_t global_modInvNeg3;

/*  W_4_1 = qpow(mont64(ROOT), (mod - 1) / 4);  */
extern const mont64 global_w41_1;
extern const mont64 global_w41_2;
extern const mont64 global_w41_3;

/*  W_4_1 = qpow(mont64(ROOTinv), (mod - 1) / 4);  */
extern const mont64 global_w41_inv1;
extern const mont64 global_w41_inv2;
extern const mont64 global_w41_inv3;

/*
 mont64 w1 = qpow(mont64(ROOT), (mod() - 1) / 8);
 mont64 w2 = qpow(w1, 2);
 mont64 w3 = qpow(w1, 3);
*/
extern const mont64 global_w1_1;
extern const mont64 global_w2_1;
extern const mont64 global_w3_1;

extern const mont64 global_w1_2;
extern const mont64 global_w2_2;
extern const mont64 global_w3_2;

extern const mont64 global_w1_3;
extern const mont64 global_w2_3;
extern const mont64 global_w3_3;

/*
 mont64 w1 = qpow(mont64(ROOTinv), (mod() - 1) / 8);
 mont64 w2 = qpow(w1, 2);
 mont64 w3 = qpow(w1, 3);
*/
extern const mont64 global_w1_inv1;
extern const mont64 global_w2_inv1;
extern const mont64 global_w3_inv1;

extern const mont64 global_w1_inv2;
extern const mont64 global_w2_inv2;
extern const mont64 global_w3_inv2;

extern const mont64 global_w1_inv3;
extern const mont64 global_w2_inv3;
extern const mont64 global_w3_inv3;

/* mont64(1) */
extern const mont64 global_one1;
extern const mont64 global_one2;
extern const mont64 global_one3;

#endif /* __LAMMP_3NTT_CRT_CONST_H__ */