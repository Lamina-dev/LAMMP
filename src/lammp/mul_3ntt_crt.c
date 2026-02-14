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
#include "../../include/lammp/impl/3ntt_crt_kernel.h"
#include <math.h>

static inline double find_unbalanced_root(double m, double n) {
#define func_f(x) (n * log(m) - m / x - n * log(x) - n)
#define func_f_deriv(x) (m / (x * x) - n / x)
    /*
    事实上，这个初始值的估计几乎已经足够准确了。一步牛顿迭代意义并不大，
    但此函数也仅仅调用一次，省去一次牛顿迭代法也仅带来十几ns的开销，因
    此不值得过多纠结。
     */
    double x_prev = m / (n * log(n)) * 0.8728;
    double x_curr;
    double f_val = func_f(x_prev);
    double f_deriv_val = func_f_deriv(x_prev);

    x_curr = x_prev - f_val / f_deriv_val;
    return x_curr;
#undef func_f
#undef func_f_deriv
}

static inline int int_ceil2(mp_limb_t n) {
    int bits = sizeof(n) * CHAR_BIT;
    n--;
    for (int i = 1; i < bits; i *= 2) {
        n |= (n >> i);
    }
    return n + 1;
}

void lmmp_mul_ntt_unbal_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb, mp_size_t M) {
    lmmp_debug_assert(numa != NULL && numb != NULL && dst != NULL);
    lmmp_debug_assert(numa != numb && na > nb);

    TEMP_DECL;
    
    if (M == 0) M = find_unbalanced_root(na, nb);

    mp_limb_t min_sum = nb + na / LMMP_MAX(3, M);

    min_sum -= ((min_sum & (min_sum - 1)) == 0) ? na : 0;

    int highest_bit = 63 - lmmp_leading_zeros_(min_sum);
    mp_limb_t next_power = 1ULL << (highest_bit + 1);

    mp_size_t balance_len = next_power, conv_len = balance_len - 1, single_len = balance_len - nb;
    lmmp_assert(single_len <= na && "please use balanced version");

    mp_size_t ntt_len = int_ceil2(conv_len);
    mp_size_t rem = na % single_len;

    /* 预处理表 */
    _3ntt_short table0;
    table0.ntt_len = LMMP_MIN(ntt_len, long_threshold);
    table0.log_len = log2_64(table0.ntt_len);
    table0.omega1 = BALLOC_TYPE(table0.ntt_len, mont64);
    table0.iomega1 = BALLOC_TYPE(table0.ntt_len, mont64);
    table0.omega2 = BALLOC_TYPE(table0.ntt_len, mont64);
    table0.iomega2 = BALLOC_TYPE(table0.ntt_len, mont64);
    table0.omega3 = BALLOC_TYPE(table0.ntt_len, mont64);
    table0.iomega3 = BALLOC_TYPE(table0.ntt_len, mont64);

    cover_nttshort(table0.log_len, &table0);

    mont64 *buf1_mont, *buf2_mont, *buf3_mont, *buf4_mont, *buf5_mont, *buf6_mont;
    buf1_mont = BALLOC_TYPE(ntt_len, mont64);
    buf2_mont = BALLOC_TYPE(ntt_len, mont64);
    buf3_mont = BALLOC_TYPE(ntt_len, mont64);
    buf4_mont = BALLOC_TYPE(ntt_len, mont64);
    buf5_mont = BALLOC_TYPE(ntt_len, mont64);
    buf6_mont = BALLOC_TYPE(ntt_len, mont64);

    for (mp_size_t ii = nb; ii < ntt_len; ii++) {
        buf2_mont[ii] = 0;
        buf4_mont[ii] = 0;
        buf6_mont[ii] = 0;
    }
    for (mp_size_t ii = 0; ii < nb; ii++) {
        buf2_mont[ii] = numb[ii];
        buf4_mont[ii] = numb[ii];
        buf6_mont[ii] = numb[ii];
        _mont64_tomont_func(buf2_mont[ii], 1);
        _mont64_tomont_func(buf4_mont[ii], 2);
        _mont64_tomont_func(buf6_mont[ii], 3);
    }

    for (mp_size_t ii = single_len; ii < ntt_len; ii++) {
        buf1_mont[ii] = 0;
        buf3_mont[ii] = 0;
        buf5_mont[ii] = 0;
    }
    for (mp_size_t ii = 0; ii < single_len; ii++) {
        buf1_mont[ii] = numa[ii];
        buf3_mont[ii] = numa[ii];
        buf5_mont[ii] = numa[ii];
        _mont64_tomont_func(buf1_mont[ii], 1);
        _mont64_tomont_func(buf3_mont[ii], 2);
        _mont64_tomont_func(buf5_mont[ii], 3);
    }

    conv_rec_func(buf1_mont, buf2_mont, buf1_mont, &table0, ntt_len, 1);
    conv_rec_func(buf3_mont, buf4_mont, buf3_mont, &table0, ntt_len, 2);
    conv_rec_func(buf5_mont, buf6_mont, buf5_mont, &table0, ntt_len, 3);


    mp_ptr balance_prod = BALLOC_TYPE(balance_len, mp_limb_t);

    u192 carry = {0, 0, 0};
    for (mp_size_t i = 0; i < conv_len; i++) {
        u192 temp = {0, 0, 0};
        crt3(buf1_mont[i], buf3_mont[i], buf5_mont[i], temp);
        _u192add(carry, temp);
        dst[i] = (*(carry));
        carry[0] = carry[1];
        carry[1] = carry[2];
        carry[2] = 0;
    }
    dst[conv_len] = carry[0];

    //
    //             nb = 2
    // balance_prod_len = 4
    // +---+---+---+---+
    // | 1 | 2 | 3 | 4 |
    // +---+---+---+---+
    //         |              prod
    //         +---+---+---+---+
    //         | 2 | 3 | 4 | 5 |
    //         +---+---+---+---+
    //                 |   dst+nb    dst
    //                 +---+---+---+---+
    //                 | 3 | 4 | 5 | 6 |
    //                 +---+---+---+---+
    mp_size_t len = single_len;
    mp_srcptr numa_p = numa;
    for (; len < na - rem; len += single_len) {
        numa_p += single_len;
        for (mp_size_t ii = single_len; ii < ntt_len; ii++) {
            buf1_mont[ii] = 0;
            buf3_mont[ii] = 0;
            buf5_mont[ii] = 0;
        }
        for (mp_size_t ii = 0; ii < single_len; ii++) {
            buf1_mont[ii] = numa_p[ii];
            buf3_mont[ii] = numa_p[ii];
            buf5_mont[ii] = numa_p[ii];
            _mont64_tomont_func(buf1_mont[ii], 1);
            _mont64_tomont_func(buf3_mont[ii], 2);
            _mont64_tomont_func(buf5_mont[ii], 3);
        }
        conv_single_func(buf2_mont, buf1_mont, buf1_mont, &table0, ntt_len, 1);
        conv_single_func(buf4_mont, buf3_mont, buf3_mont, &table0, ntt_len, 2);
        conv_single_func(buf6_mont, buf5_mont, buf5_mont, &table0, ntt_len, 3);

        carry[0] = 0;
        carry[1] = 0;
        carry[2] = 0;
        for (mp_size_t i = 0; i < conv_len; i++) {
            u192 temp = {0, 0, 0};
            crt3(buf1_mont[i], buf3_mont[i], buf5_mont[i], temp);
            _u192add(carry, temp);
            balance_prod[i] = *(carry);
            carry[0] = carry[1];
            carry[1] = carry[2];
            carry[2] = 0;
        }
        balance_prod[conv_len] = *(carry);
        lmmp_add_(dst + len, balance_prod, balance_len, dst + len, nb);
    }
    if (rem > 0) {
        numa_p = numa + len;
        for (mp_size_t ii = rem; ii < ntt_len; ii++) {
            buf1_mont[ii] = 0;
            buf3_mont[ii] = 0;
            buf5_mont[ii] = 0;
        }
        for (mp_size_t ii = 0; ii < rem; ii++) {
            buf1_mont[ii] = numa_p[ii];
            buf3_mont[ii] = numa_p[ii];
            buf5_mont[ii] = numa_p[ii];
            _mont64_tomont_func(buf1_mont[ii], 1);
            _mont64_tomont_func(buf3_mont[ii], 2);
            _mont64_tomont_func(buf5_mont[ii], 3);
        }
        conv_single_func(buf2_mont, buf1_mont, buf1_mont, &table0, ntt_len, 1);
        conv_single_func(buf4_mont, buf3_mont, buf3_mont, &table0, ntt_len, 2);
        conv_single_func(buf6_mont, buf5_mont, buf5_mont, &table0, ntt_len, 3);
        carry[0] = 0;
        carry[1] = 0;
        carry[2] = 0;
        for (mp_size_t i = 0; i < conv_len; i++) {
            u192 temp = {0, 0, 0};
            crt3(buf1_mont[i], buf3_mont[i], buf5_mont[i], temp);
            _u192add(carry, temp);
            balance_prod[i] = *(carry);
            carry[0] = carry[1];
            carry[1] = carry[2];
            carry[2] = 0;
        }
        balance_prod[conv_len] = *(carry);
        lmmp_add_(dst + len, balance_prod, nb + rem, dst + len, nb);
    }

    TEMP_FREE;
}