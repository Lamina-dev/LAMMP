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

#include "../../../../include/lammp/3ntt_crt_kernal.h"
#include "../../../../include/lammp/lammp.hpp"
#include "../../../../include/lammp/base_cal.hpp"
#include "../../../../include/lammp/inter_buffer.hpp"
#include <assert.h>

uint64_t self_div_rem(u192 in, uint64_t divisor) {
    uint64_t divid1 = in[2] % divisor, divid0 = in[1];
    in[2] /= divisor;
    in[1] = lammp::div128by64to64(divid1, divid0, divisor);
    divid1 = divid0, divid0 = in[0];
    in[0] = lammp::div128by64to64(divid1, divid0, divisor);
    return divid0;
}

namespace lammp::Arithmetic {

void abs_mul64_ntt(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out) {
    assert(in1 != NULL && in2 != NULL && out != NULL);
    if (in1 == in2) {
        abs_sqr64_ntt(in1, len1, out);
        return;
    }
    uint64_t out_len = len1 + len2, conv_len = out_len - 1;
    uint64_t ntt_len = int_ceil2(conv_len);

    mont64 *buf1_mont, *buf2_mont, *buf3_mont, *tmp_mont;
    
    _internal_buffer<0, MONT64BIT> buf1(ntt_len), buf2(ntt_len), buf3(ntt_len), tmp(ntt_len);
    buf1_mont = buf1.data();
    buf2_mont = buf2.data();
    buf3_mont = buf3.data();
    tmp_mont = tmp.data();

    ntt_short table;
    table.ntt_len = (ntt_len < long_threshold) ? ntt_len : long_threshold;
    table.log_len = log2_64(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf1(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf2(table.ntt_len);
    table.omega = table_buf1.data();
    table.iomega = table_buf2.data();

    for (size_t ii = len1; ii < ntt_len; ii++) {
        buf1_mont[ii] = 0;
        buf2_mont[ii] = 0;
        buf3_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len1; ii++) {
        buf1_mont[ii] = in1[ii];
        buf2_mont[ii] = in1[ii];
        buf3_mont[ii] = in1[ii];
        _mont64_tomont_func(buf1_mont[ii], 1);
        _mont64_tomont_func(buf2_mont[ii], 2);
        _mont64_tomont_func(buf3_mont[ii], 3);
    }

    for (size_t ii = len2; ii < ntt_len; ii++) {
        tmp_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len2; ii++) {
        tmp_mont[ii] = in2[ii];
        _mont64_tomont_func(tmp_mont[ii], 1);
    }

    cover_nttshort_func(table.log_len, &table, 1);

    conv_rec_func(buf1_mont, tmp_mont, buf1_mont, &table, ntt_len, 1);

    for (size_t ii = len2; ii < ntt_len; ii++) {
        tmp_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len2; ii++) {
        tmp_mont[ii] = in2[ii];
        _mont64_tomont_func(tmp_mont[ii], 2);
    }
    cover_nttshort_func(table.log_len, &table, 2);


    conv_rec_func(buf2_mont, tmp_mont, buf2_mont, &table, ntt_len, 2);

    for (size_t ii = len2; ii < ntt_len; ii++) {
        tmp_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len2; ii++) {
        tmp_mont[ii] = in2[ii];
        _mont64_tomont_func(tmp_mont[ii], 3);
    }
    cover_nttshort_func(table.log_len, &table, 3);

    conv_rec_func(buf3_mont, tmp_mont, buf3_mont, &table, ntt_len, 3);


    u192 carry = {0, 0, 0};
    for (size_t ii = 0; ii < conv_len; ii++) {
        u192 temp = {0, 0, 0};
        crt3(buf1_mont[ii], buf2_mont[ii], buf3_mont[ii], temp);
        _u192add(carry, temp);
        out[ii] = (*(carry));
        carry[0] = carry[1];
        carry[1] = carry[2];
        carry[2] = 0;
    }

    out[conv_len] = carry[0];
}

void abs_sqr64_ntt(lamp_ptr in1, lamp_ui len1, lamp_ptr out) {
    assert(in1 != NULL && out != NULL);
    uint64_t out_len = len1 * 2, conv_len = out_len - 1;
    uint64_t ntt_len = int_ceil2(conv_len);

    mont64 *buf1_mont, *buf2_mont, *buf3_mont;
    _internal_buffer<0, MONT64BIT> buf1(ntt_len), buf2(ntt_len), buf3(ntt_len);
    buf1_mont = buf1.data();
    buf2_mont = buf2.data();
    buf3_mont = buf3.data();


    ntt_short table;
    table.ntt_len = (ntt_len < long_threshold) ? ntt_len : long_threshold;
    table.log_len = log2_64(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf1(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf2(table.ntt_len);
    table.omega = table_buf1.data();
    table.iomega = table_buf2.data();

    for (size_t ii = len1; ii < ntt_len; ii++) {
        buf1_mont[ii] = 0;
        buf2_mont[ii] = 0;
        buf3_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len1; ii++) {
        buf1_mont[ii] = in1[ii];
        buf2_mont[ii] = in1[ii];
        buf3_mont[ii] = in1[ii];
        _mont64_tomont_func(buf1_mont[ii], 1);
        _mont64_tomont_func(buf2_mont[ii], 2);
        _mont64_tomont_func(buf3_mont[ii], 3);
    }

    cover_nttshort_func(table.log_len, &table, 1);
    conv_sqr_func(buf1_mont, buf1_mont, &table, ntt_len, 1);

    cover_nttshort_func(table.log_len, &table, 2);
    conv_sqr_func(buf2_mont, buf2_mont, &table, ntt_len, 2);

    cover_nttshort_func(table.log_len, &table, 3);
    conv_sqr_func(buf3_mont, buf3_mont, &table, ntt_len, 3);

    u192 carry = {0, 0, 0};
    for (size_t ii = 0; ii < conv_len; ii++) {
        u192 temp = {0, 0, 0};
        crt3(buf1_mont[ii], buf2_mont[ii], buf3_mont[ii], temp);
        _u192add(carry, temp);
        out[ii] = (*(carry));
        carry[0] = carry[1];
        carry[1] = carry[2];
        carry[2] = 0;
    }
    out[conv_len] = carry[0];
}

void abs_mul64_ntt_base(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out, const lamp_ui base_num) {
    assert(in1 != NULL && in2 != NULL && out != NULL);
    if (in1 == in2) {
        abs_sqr64_ntt_base(in1, len1, out, base_num);
        return;
    }
    uint64_t out_len = len1 + len2, conv_len = out_len - 1;
    uint64_t ntt_len = int_ceil2(conv_len);

    mont64 *buf1_mont, *buf2_mont, *buf3_mont, *tmp_mont;

    _internal_buffer<0, MONT64BIT> buf1(ntt_len), buf2(ntt_len), buf3(ntt_len), tmp(ntt_len);
    buf1_mont = buf1.data();
    buf2_mont = buf2.data();
    buf3_mont = buf3.data();
    tmp_mont = tmp.data();

    ntt_short table;
    table.ntt_len = (ntt_len < long_threshold) ? ntt_len : long_threshold;
    table.log_len = log2_64(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf1(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf2(table.ntt_len);
    table.omega = table_buf1.data();
    table.iomega = table_buf2.data();

    for (size_t ii = len1; ii < ntt_len; ii++) {
        buf1_mont[ii] = 0;
        buf2_mont[ii] = 0;
        buf3_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len1; ii++) {
        buf1_mont[ii] = in1[ii];
        buf2_mont[ii] = in1[ii];
        buf3_mont[ii] = in1[ii];
        _mont64_tomont_func(buf1_mont[ii], 1);
        _mont64_tomont_func(buf2_mont[ii], 2);
        _mont64_tomont_func(buf3_mont[ii], 3);
    }

    for (size_t ii = len2; ii < ntt_len; ii++) {
        tmp_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len2; ii++) {
        tmp_mont[ii] = in2[ii];
        _mont64_tomont_func(tmp_mont[ii], 1);
    }

    cover_nttshort_func(table.log_len, &table, 1);

    conv_rec_func(buf1_mont, tmp_mont, buf1_mont, &table, ntt_len, 1);

    for (size_t ii = len2; ii < ntt_len; ii++) {
        tmp_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len2; ii++) {
        tmp_mont[ii] = in2[ii];
        _mont64_tomont_func(tmp_mont[ii], 2);
    }
    cover_nttshort_func(table.log_len, &table, 2);

    conv_rec_func(buf2_mont, tmp_mont, buf2_mont, &table, ntt_len, 2);

    for (size_t ii = len2; ii < ntt_len; ii++) {
        tmp_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len2; ii++) {
        tmp_mont[ii] = in2[ii];
        _mont64_tomont_func(tmp_mont[ii], 3);
    }
    cover_nttshort_func(table.log_len, &table, 3);

    conv_rec_func(buf3_mont, tmp_mont, buf3_mont, &table, ntt_len, 3);

    u192 carry = {0, 0, 0};
    for (size_t ii = 0; ii < conv_len; ii++) {
        u192 temp = {0, 0, 0};
        crt3(buf1_mont[ii], buf2_mont[ii], buf3_mont[ii], temp);
        _u192add(carry, temp);
        out[ii] = self_div_rem(carry, base_num);
    }
    out[conv_len] = self_div_rem(carry, base_num);
}

void abs_sqr64_ntt_base(lamp_ptr in1, lamp_ui len1, lamp_ptr out, const lamp_ui base_num) {
    assert(in1 != NULL && out != NULL);
    uint64_t out_len = len1 * 2, conv_len = out_len - 1;
    uint64_t ntt_len = int_ceil2(conv_len);

    mont64 *buf1_mont, *buf2_mont, *buf3_mont;
    _internal_buffer<0, MONT64BIT> buf1(ntt_len), buf2(ntt_len), buf3(ntt_len);
    buf1_mont = buf1.data();
    buf2_mont = buf2.data();
    buf3_mont = buf3.data();

    ntt_short table;
    table.ntt_len = (ntt_len < long_threshold) ? ntt_len : long_threshold;
    table.log_len = log2_64(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf1(table.ntt_len);
    _internal_buffer<0, MONT64BIT> table_buf2(table.ntt_len);
    table.omega = table_buf1.data();
    table.iomega = table_buf2.data();

    for (size_t ii = len1; ii < ntt_len; ii++) {
        buf1_mont[ii] = 0;
        buf2_mont[ii] = 0;
        buf3_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < len1; ii++) {
        buf1_mont[ii] = in1[ii];
        buf2_mont[ii] = in1[ii];
        buf3_mont[ii] = in1[ii];
        _mont64_tomont_func(buf1_mont[ii], 1);
        _mont64_tomont_func(buf2_mont[ii], 2);
        _mont64_tomont_func(buf3_mont[ii], 3);
    }

    cover_nttshort_func(table.log_len, &table, 1);
    conv_sqr_func(buf1_mont, buf1_mont, &table, ntt_len, 1);

    cover_nttshort_func(table.log_len, &table, 2);
    conv_sqr_func(buf2_mont, buf2_mont, &table, ntt_len, 2);

    cover_nttshort_func(table.log_len, &table, 3);
    conv_sqr_func(buf3_mont, buf3_mont, &table, ntt_len, 3);

    u192 carry = {0, 0, 0};
    for (size_t ii = 0; ii < conv_len; ii++) {
        u192 temp = {0, 0, 0};
        crt3(buf1_mont[ii], buf2_mont[ii], buf3_mont[ii], temp);
        _u192add(carry, temp);
        out[ii] = self_div_rem(carry, base_num);
    }
    out[conv_len] = self_div_rem(carry, base_num);
}

void abs_mul64_ntt_unbalanced(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ui M, lamp_ptr out) {
    assert(in1 != NULL && in2 != NULL && out != NULL);
    assert(in1 != in2 && len1 > len2);

    lamp_ui min_sum = len2 + std::max(len2, M);

    min_sum -= ((min_sum & (min_sum - 1)) == 0) ? len1 : 0;

    int highest_bit = 63 - lammp_clz(min_sum);
    uint64_t next_power = 1ULL << (highest_bit + 1);

    lamp_ui balance_len = next_power, conv_len = balance_len - 1, single_len = balance_len - len2;
    assert(single_len <= len1 && "please use balanced version");

    lamp_ui ntt_len = int_ceil2(conv_len), rem = len1 % single_len;


    /* 预处理表 */
    ntt_short table1, table2, table3;
    table1.ntt_len = (ntt_len < long_threshold) ? ntt_len : long_threshold;
    table1.log_len = log2_64(table1.ntt_len);
    table2.ntt_len = table1.ntt_len;
    table2.log_len = table1.log_len;
    table3.ntt_len = table1.ntt_len;
    table3.log_len = table1.log_len;
    _internal_buffer<0, MONT64BIT> table1_buf1(table1.ntt_len);
    _internal_buffer<0, MONT64BIT> table1_buf2(table1.ntt_len);
    table1.omega = table1_buf1.data();
    table1.iomega = table1_buf2.data();
    _internal_buffer<0, MONT64BIT> table2_buf1(table2.ntt_len);
    _internal_buffer<0, MONT64BIT> table2_buf2(table2.ntt_len);
    table2.omega = table2_buf1.data();
    table2.iomega = table2_buf2.data();
    _internal_buffer<0, MONT64BIT> table3_buf1(table3.ntt_len);
    _internal_buffer<0, MONT64BIT> table3_buf2(table3.ntt_len);
    table3.omega = table3_buf1.data();
    table3.iomega = table3_buf2.data();

    cover_nttshort_func(table1.log_len, &table1, 1);
    cover_nttshort_func(table2.log_len, &table2, 2);
    cover_nttshort_func(table3.log_len, &table3, 3);


    mont64 *buf1_mont, *buf2_mont, *buf3_mont, *buf4_mont, *buf5_mont, *buf6_mont;
    _internal_buffer<0, MONT64BIT> buf1(ntt_len), buf2(ntt_len), buf3(ntt_len), buf4(ntt_len), buf5(ntt_len), buf6(ntt_len);
    buf1_mont = buf1.data();
    buf2_mont = buf2.data();
    buf3_mont = buf3.data();
    buf4_mont = buf4.data();
    buf5_mont = buf5.data();
    buf6_mont = buf6.data();

    for (size_t ii = len2; ii < ntt_len; ii++) {
        buf2_mont[ii] = 0;
        buf4_mont[ii] = 0;
        buf6_mont[ii] = 0;
    } 
    for (size_t ii = 0; ii < len2; ii++) {
        buf2_mont[ii] = in2[ii];
        buf4_mont[ii] = in2[ii];
        buf6_mont[ii] = in2[ii];
        _mont64_tomont_func(buf2_mont[ii], 1);
        _mont64_tomont_func(buf4_mont[ii], 2);
        _mont64_tomont_func(buf6_mont[ii], 3);
    }

    for (size_t ii = single_len; ii < ntt_len; ii++) {
        buf1_mont[ii] = 0;
        buf3_mont[ii] = 0;
        buf5_mont[ii] = 0;
    }
    for (size_t ii = 0; ii < single_len; ii++) {
        buf1_mont[ii] = in1[ii];
        buf3_mont[ii] = in1[ii];
        buf5_mont[ii] = in1[ii];
        _mont64_tomont_func(buf1_mont[ii], 1);
        _mont64_tomont_func(buf3_mont[ii], 2);
        _mont64_tomont_func(buf5_mont[ii], 3);
    }

    conv_rec_func(buf1_mont, buf2_mont, buf1_mont, &table1, ntt_len, 1);
    conv_rec_func(buf3_mont, buf4_mont, buf3_mont, &table2, ntt_len, 2);
    conv_rec_func(buf5_mont, buf6_mont, buf5_mont, &table3, ntt_len, 3);

    _internal_buffer<0> balance_prod(balance_len);

    u192 carry = {0, 0, 0};
    for (lamp_ui i = 0; i < conv_len; i++) {
        u192 temp = {0, 0, 0};
        crt3(buf1_mont[i], buf3_mont[i], buf5_mont[i], temp);
        _u192add(carry, temp);
        out[i] = (*(carry));
        carry[0] = carry[1];
        carry[1] = carry[2];
        carry[2] = 0;
    }
    out[conv_len] = carry[0];

    //
    //             len2 = 2
    // balance_prod_len = 4
    // +---+---+---+---+
    // | 1 | 2 | 3 | 4 |
    // +---+---+---+---+
    //         |              prod
    //         +---+---+---+---+
    //         | 2 | 3 | 4 | 5 |
    //         +---+---+---+---+
    //                 |   out+len2    out
    //                 +---+---+---+---+
    //                 | 3 | 4 | 5 | 6 |
    //                 +---+---+---+---+
    lamp_ui len = single_len;
    auto in1_p = in1;
    for (; len < len1 - rem; len += single_len) {
        in1_p += single_len;
        for (size_t ii = single_len; ii < ntt_len; ii++) {
            buf1_mont[ii] = 0;
            buf3_mont[ii] = 0;
            buf5_mont[ii] = 0;
        }
        for (size_t ii = 0; ii < single_len; ii++) {
            buf1_mont[ii] = in1_p[ii];
            buf3_mont[ii] = in1_p[ii];
            buf5_mont[ii] = in1_p[ii];
            _mont64_tomont_func(buf1_mont[ii], 1);
            _mont64_tomont_func(buf3_mont[ii], 2);
            _mont64_tomont_func(buf5_mont[ii], 3);
        }
        conv_single_func(buf2_mont, buf1_mont, buf1_mont, &table1, ntt_len, 1);
        conv_single_func(buf4_mont, buf3_mont, buf3_mont, &table2, ntt_len, 2);
        conv_single_func(buf6_mont, buf5_mont, buf5_mont, &table3, ntt_len, 3);
        carry[0] = 0;
        carry[1] = 0;
        carry[2] = 0;
        for (lamp_ui i = 0; i < conv_len; i++) {
            u192 temp = {0, 0, 0};
            crt3(buf1[i], buf3[i], buf5[i], temp);
            _u192add(carry, temp);
            balance_prod.set(i, *(carry));
            carry[0] = carry[1];
            carry[1] = carry[2];
            carry[2] = 0;
        }
        balance_prod.set(conv_len, *(carry));
        abs_add_binary_half(balance_prod.data(), balance_len, out + len, len2, out + len);
    }
    if (rem > 0) {
        in1_p = in1 + len;
        for (size_t ii = single_len; ii < ntt_len; ii++) {
            buf1_mont[ii] = 0;
            buf3_mont[ii] = 0;
            buf5_mont[ii] = 0;
        }
        for (size_t ii = 0; ii < single_len; ii++) {
            buf1_mont[ii] = in1_p[ii];
            buf3_mont[ii] = in1_p[ii];
            buf5_mont[ii] = in1_p[ii];
            _mont64_tomont_func(buf1_mont[ii], 1);
            _mont64_tomont_func(buf3_mont[ii], 2);
            _mont64_tomont_func(buf5_mont[ii], 3);
        }
        conv_single_func(buf2_mont, buf1_mont, buf1_mont, &table1, ntt_len, 1);
        conv_single_func(buf4_mont, buf3_mont, buf3_mont, &table2, ntt_len, 2);
        conv_single_func(buf6_mont, buf5_mont, buf5_mont, &table3, ntt_len, 3);
        carry[0] = 0;
        carry[1] = 0;
        carry[2] = 0;
        for (lamp_ui i = 0; i < conv_len; i++) {
            u192 temp = {0, 0, 0};
            crt3(buf1[i], buf3[i], buf5[i], temp);
            _u192add(carry, temp);
            balance_prod.set(i, *(carry));
            carry[0] = carry[1];
            carry[1] = carry[2];
            carry[2] = 0;
        }
        balance_prod.set(conv_len, *(carry));
        // 注意这两个加数不可调换，否则越界
        abs_add_binary_half(out + len, len2, balance_prod.data(), len2 + rem, out + len);
    }
}

};   // namespace lammp::Arithmetic