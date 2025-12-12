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

#include "../../../../include/lammp/inter_buffer.hpp"
#include "../../../../include/lammp/lammp.hpp"
#include "../../../../include/lammp/uint128.hpp"

namespace lammp::Arithmetic {
// Karatsuba 乘法
void abs_mul64_karatsuba_buffered(lamp_ptr in1,
                                  lamp_ui len1,
                                  lamp_ptr in2,
                                  lamp_ui len2,
                                  lamp_ptr out,
                                  lamp_ptr buffer_begin,
                                  lamp_ptr buffer_end) {
    const lamp_ui out_len = get_mul_len(len1, len2);
    len1 = rlz(in1, len1);
    len2 = rlz(in2, len2);
    if (len1 < len2) {
        std::swap(in1, in2);
        std::swap(len1, len2);  // Let in1 be the loonger one
    }
    if (0 == len2 || nullptr == in1 || nullptr == in2) {
        std::fill_n(out, out_len, lamp_ui(0));
        return;
    }
    if (len2 < KARATSUBA_MIN_THRESHOLD) {
        abs_mul64_classic(in1, len1, in2, len2, out, buffer_begin, buffer_end);
        std::fill(out + len1 + len2, out + out_len, lamp_ui(0));
        return;
    }
    // Split A * B -> (AH * BASE + AL) * (BH * BASE + BL)
    // (AH * BASE + AL) * (BH * BASE + BL) = AH * BH * BASE^2 + (AH * BL + AL * BH) * BASE + AL * BL
    // Let M  = AL * BL,
    //     N  = AH * BH,
    //     K1 = (AH - AL),
    //     K2 = (BH - BL),
    //     K  = K1 * K2
    //        = AH * BH - (AH * BL + AL * BH) + AL * BL
    //
    // A * B = N * BASE^2 + (M + N - K) * BASE + M
    const lamp_ui base_len = (len1 + 1) / 2;
    lamp_ui len1_low = base_len, len1_high = len1 - base_len;
    lamp_ui len2_low = base_len, len2_high = len2 - base_len;
    if (len2 <= base_len) {
        len2_low = len2;
        len2_high = 0;
    }
    // Get length of every part
    lamp_ui m_len = get_mul_len(len1_low, len2_low);
    lamp_ui n_len = get_mul_len(len1_high, len2_high);

    // Get enough buffer
    _internal_buffer<0> buffer(0);
    const lamp_ui buffer_size = m_len + n_len + get_mul_len(len1_low, len2_low);
    if (buffer_begin + buffer_size > buffer_end) {
        buffer.resize(buffer_size * 2 + 1);
        buffer_begin = buffer.data();
        buffer_end = buffer_begin + buffer.capacity();
    }
    // Set pointer of every part
    auto m = buffer_begin, n = m + m_len, k1 = n + n_len, k2 = k1 + len1_low, k = k1;

    // Compute M,N
    abs_mul64_karatsuba_buffered(in1, len1_low, in2, len2_low, m, buffer_begin + buffer_size, buffer_end);
    abs_mul64_karatsuba_buffered(in1 + base_len, len1_high, in2 + base_len, len2_high, n, buffer_begin + buffer_size,
                                 buffer_end);

    // Compute K1,K2
    len1_low = rlz(in1, len1_low);
    len2_low = rlz(in2, len2_low);
    int cmp1 = abs_difference_binary(in1, len1_low, in1 + base_len, len1_high, k1);
    int cmp2 = abs_difference_binary(in2, len2_low, in2 + base_len, len2_high, k2);
    lamp_ui k1_len = rlz(k1, get_sub_len(len1_low, len1_high));
    lamp_ui k2_len = rlz(k2, get_sub_len(len2_low, len2_high));

    // Compute K1*K2 = K
    abs_mul64_karatsuba_buffered(k1, k1_len, k2, k2_len, k, buffer_begin + buffer_size, buffer_end);
    lamp_ui k_len = rlz(k, get_mul_len(k1_len, k2_len));

    // Combine the result
    {
        // out = M + N * BASE ^ 2 + (M + N) ^ BASE
        std::fill(out + m_len, out + base_len * 2, lamp_ui(0));
        std::fill(out + base_len * 2 + n_len, out + out_len, lamp_ui(0));
        std::copy(m, m + m_len, out);
        std::copy(n, n + n_len, out + base_len * 2);
        m_len = std::min(m_len, out_len - base_len);
        n_len = std::min(n_len, out_len - base_len);
        {
            if (m_len < n_len) {
                std::swap(m_len, n_len);
                std::swap(m, n);
            }
            uint8_t carry = 0;
            lamp_ui i = 0;
            auto out_p = out + base_len;
            for (; i < n_len; i++) {
                bool cf;
                lamp_ui sum = add_half(m[i], lamp_ui(carry), cf);
                carry = cf;
                sum = add_half(n[i], sum, cf);
                carry += cf;
                out_p[i] = add_half(out_p[i], sum, cf);
                carry += cf;
            }
            for (; i < m_len; i++) {
                bool cf;
                lamp_ui sum = add_half(m[i], lamp_ui(carry), cf);
                carry = cf;
                out_p[i] = add_half(out_p[i], sum, cf);
                carry += cf;
            }
            for (; i < out_len - base_len; i++) {
                bool cf;
                out_p[i] = add_half(out_p[i], lamp_ui(carry), cf);
                carry = cf;
            }
        }

        // out = M + N * BASE ^ 2 + (M + N - K) ^ BASE
        k_len = std::min(k_len, out_len - base_len);
        if (cmp1 * cmp2 > 0) {
            abs_sub_binary(out + base_len, out_len - base_len, k, k_len, out + base_len);
        } else {
            abs_add_binary_half(out + base_len, out_len - base_len, k, k_len, out + base_len);
        }
    }
}

void abs_mul64_karatsuba(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out) {
    abs_mul64_karatsuba_buffered(in1, len1, in2, len2, out, nullptr, nullptr);
}

};  // namespace lammp::Arithmetic
