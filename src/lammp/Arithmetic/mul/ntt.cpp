/*
===============================================================================
【1. LGPL v2.1 LICENSED CODE (LAMMP Project)】
Copyright (c) 2025-2026 HJimmyK/LAMINA
This file is part of LAMMP (LGPL v2.1 License)
Full LGPL v2.1 License Text: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
LAMMP Repository: https://github.com/Lamina-dev/LAMMP

Modification Note: This file contains modifications to the original MIT-licensed code to adapt to LAMMP's LGPL v2.1
environment.

===============================================================================
【2. MIT LICENSED CODE (Original Source)】
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky & HJimmyK
Project URL: https://github.com/Twilight-Dream-Of-Magic/Easy-BigInteger

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

#include "../../../../include/lammp/lammp.hpp"
#include "../../../../include/lammp/uint128.hpp"
#include "../../../../include/lammp/num_theo.hpp"
#include "../../../../include/lammp/mont64_div64.hpp"
#include "../../../../include/lammp/inter_buffer.hpp"
#include <vector>

namespace lammp::Arithmetic {

// NTT square
void abs_sqr64_ntt(lamp_ptr in, lamp_ui len, lamp_ptr out) {
    using namespace lammp::Transform::number_theory;
    if (0 == len || in == nullptr) {
        return;
    }
    lamp_ui out_len = len * 2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::copy(in, in + len, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer1.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer2(ntt_len);
    {
        std::copy(in, in + len, buffer2.begin());
        NTT1::convolutionRecursion(buffer2.data(), buffer2.data(), buffer2.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer3(ntt_len);
    {
        std::copy(in, in + len, buffer3.begin());
        NTT2::convolutionRecursion(buffer3.data(), buffer3.data(), buffer3.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer2[i], buffer3[i]);
        out[i] = lamp_ui(carry);
        carry = carry.rShift64();
    }
    out[conv_len] = lamp_ui(carry);
}

// NTT multiplication
void abs_mul64_ntt(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out) {
    if (0 == len1 || 0 == len2 || in1 == nullptr || in2 == nullptr) {
        return;
    }
    if (in1 == in2) {
        abs_sqr64_ntt(in1, len1, out);  // Square
        return;
    }
    using namespace lammp::Transform::number_theory;
    lamp_ui out_len = len1 + len2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::vector<NTT0::ModIntType> buffer2(ntt_len);
        std::copy(in2, in2 + len2, buffer2.begin());
        std::copy(in1, in1 + len1, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer2.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer3(ntt_len);
    {
        std::vector<NTT1::ModIntType> buffer4(ntt_len);
        std::copy(in2, in2 + len2, buffer4.begin());
        std::copy(in1, in1 + len1, buffer3.begin());
        NTT1::convolutionRecursion(buffer3.data(), buffer4.data(), buffer3.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer5(ntt_len);
    {
        std::vector<NTT2::ModIntType> buffer6(ntt_len);
        std::copy(in2, in2 + len2, buffer6.begin());
        std::copy(in1, in1 + len1, buffer5.begin());
        NTT2::convolutionRecursion(buffer5.data(), buffer6.data(), buffer5.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
        out[i] = lamp_ui(carry);
        carry = carry.rShift64();
    }
    out[conv_len] = lamp_ui(carry);
}

void abs_mul64_ntt_unbalanced(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ui M, lamp_ptr out) {
    assert(in1 != in2 && len1 > len2);
    using namespace lammp::Transform::number_theory;
    lamp_ui min_sum = len2 + std::max(len2, M);

    min_sum -= ((min_sum & (min_sum - 1)) == 0) ? len1 : 0;

    int highest_bit = 63 - lammp_clz(min_sum);
    uint64_t next_power = 1ULL << (highest_bit + 1);

    lamp_ui balance_len = next_power, conv_len = balance_len - 1, single_len = balance_len - len2;
    assert(single_len <= len1 && "please use balanced version");

    lamp_ui ntt_len = int_ceil2(conv_len), rem = len1 % single_len;

    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    std::vector<NTT0::ModIntType> buffer2(ntt_len);
    std::copy(in2, in2 + len2, buffer2.begin());
    std::copy(in1, in1 + single_len, buffer1.begin());
    NTT0::convolutionRecursion(buffer1.data(), buffer2.data(), buffer1.data(), ntt_len);

    std::vector<NTT1::ModIntType> buffer3(ntt_len);
    std::vector<NTT1::ModIntType> buffer4(ntt_len);
    std::copy(in2, in2 + len2, buffer4.begin());
    std::copy(in1, in1 + single_len, buffer3.begin());
    NTT1::convolutionRecursion(buffer3.data(), buffer4.data(), buffer3.data(), ntt_len);

    std::vector<NTT2::ModIntType> buffer5(ntt_len);
    std::vector<NTT2::ModIntType> buffer6(ntt_len);
    std::copy(in2, in2 + len2, buffer6.begin());
    std::copy(in1, in1 + single_len, buffer5.begin());
    NTT2::convolutionRecursion(buffer5.data(), buffer6.data(), buffer5.data(), ntt_len);

    _internal_buffer<0> balance_prod(balance_len);

    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
        out[i] = lamp_ui(carry);
        carry = carry.rShift64();
    }
    out[conv_len] = lamp_ui(carry);
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
        std::fill(buffer1.begin() + single_len, buffer1.begin() + ntt_len, NTT0::ModIntType(0));
        std::fill(buffer3.begin() + single_len, buffer3.begin() + ntt_len, NTT1::ModIntType(0));
        std::fill(buffer5.begin() + single_len, buffer5.begin() + ntt_len, NTT2::ModIntType(0));

        std::copy(in1_p, in1_p + single_len, buffer1.begin());
        NTT0::convolutionRecursion_rep(buffer2.data(), buffer1.data(), buffer1.data(), ntt_len);
        std::copy(in1_p, in1_p + single_len, buffer3.begin());
        NTT1::convolutionRecursion_rep(buffer4.data(), buffer3.data(), buffer3.data(), ntt_len);
        std::copy(in1_p, in1_p + single_len, buffer5.begin());
        NTT2::convolutionRecursion_rep(buffer6.data(), buffer5.data(), buffer5.data(), ntt_len);
        carry = 0;
        for (lamp_ui i = 0; i < conv_len; i++) {
            carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
            balance_prod.set(i, lamp_ui(carry));
            carry = carry.rShift64();
        }
        balance_prod.set(conv_len, lamp_ui(carry));
        abs_add_binary_half(balance_prod.data(), balance_len, out + len, len2, out + len);
    }
    if (rem > 0) {
        in1_p = in1 + len;
        std::fill(buffer1.begin() + rem, buffer1.begin() + ntt_len, NTT0::ModIntType(0));
        std::fill(buffer3.begin() + rem, buffer3.begin() + ntt_len, NTT1::ModIntType(0));
        std::fill(buffer5.begin() + rem, buffer5.begin() + ntt_len, NTT2::ModIntType(0));
        std::copy(in1_p, in1_p + rem, buffer1.begin());
        NTT0::convolutionRecursion_rep(buffer2.data(), buffer1.data(), buffer1.data(), ntt_len);
        std::copy(in1_p, in1_p + rem, buffer3.begin());
        NTT1::convolutionRecursion_rep(buffer4.data(), buffer3.data(), buffer3.data(), ntt_len);
        std::copy(in1_p, in1_p + rem, buffer5.begin());
        NTT2::convolutionRecursion_rep(buffer6.data(), buffer5.data(), buffer5.data(), ntt_len);
        carry = 0;
        for (lamp_ui i = 0; i < conv_len; i++) {
            carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
            balance_prod.set(i, lamp_ui(carry));
            carry = carry.rShift64();
        }
        balance_prod.set(conv_len, lamp_ui(carry));
        // 注意这两个加数不可调换，否则越界
        abs_add_binary_half(out + len, len2, balance_prod.data(), len2 + rem, out + len);
    }
}

// NTT square 在 base_num进制下
void abs_sqr64_ntt_base(lamp_ptr in, lamp_ui len, lamp_ptr out, const lamp_ui base_num) {
    using namespace lammp::Transform::number_theory;
    if (0 == len || in == nullptr) {
        return;
    }
    lamp_ui out_len = len * 2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::copy(in, in + len, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer1.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer2(ntt_len);
    {
        std::copy(in, in + len, buffer2.begin());
        NTT1::convolutionRecursion(buffer2.data(), buffer2.data(), buffer2.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer3(ntt_len);
    {
        std::copy(in, in + len, buffer3.begin());
        NTT2::convolutionRecursion(buffer3.data(), buffer3.data(), buffer3.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer2[i], buffer3[i]);
        out[i] = carry.self_div_rem(base_num);
    }
    out[conv_len] = carry.self_div_rem(base_num);
}

// NTT multiplication 在 base_num进制下
void abs_mul64_ntt_base(lamp_ptr in1,
                               lamp_ui len1,
                               lamp_ptr in2,
                               lamp_ui len2,
                               lamp_ptr out,
                               const lamp_ui base_num) {
    if (0 == len1 || 0 == len2 || in1 == nullptr || in2 == nullptr) {
        return;
    }
    if (in1 == in2) {
        abs_sqr64_ntt_base(in1, len1, out, base_num);  // Square
        return;
    }
    using namespace lammp::Transform::number_theory;
    lamp_ui out_len = len1 + len2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::vector<NTT0::ModIntType> buffer2(ntt_len);
        std::copy(in2, in2 + len2, buffer2.begin());
        std::copy(in1, in1 + len1, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer2.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer3(ntt_len);
    {
        std::vector<NTT1::ModIntType> buffer4(ntt_len);
        std::copy(in2, in2 + len2, buffer4.begin());
        std::copy(in1, in1 + len1, buffer3.begin());
        NTT1::convolutionRecursion(buffer3.data(), buffer4.data(), buffer3.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer5(ntt_len);
    {
        std::vector<NTT2::ModIntType> buffer6(ntt_len);
        std::copy(in2, in2 + len2, buffer6.begin());
        std::copy(in1, in1 + len1, buffer5.begin());
        NTT2::convolutionRecursion(buffer5.data(), buffer6.data(), buffer5.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
        out[i] = carry.self_div_rem(base_num);
    }
    out[conv_len] = carry.self_div_rem(base_num);
}
}; // namespace lammp::Arithmetic
