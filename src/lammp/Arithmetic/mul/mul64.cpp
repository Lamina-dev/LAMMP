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

/*
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky & HJimmyK

https://github.com/Twilight-Dream-Of-Magic/
https://github.com/With-Sky
https://github.com/HJimmyK

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

namespace lammp::Arithmetic {

void abs_mul64_balanced(lamp_ptr in1,
                               lamp_ui len1,
                               lamp_ptr in2,
                               lamp_ui len2,
                               lamp_ptr out,
                               lamp_ptr work_begin,
                               lamp_ptr work_end) {
    if (len1 < len2) {
        std::swap(in1, in2);
        std::swap(len1, len2);
    }
    if (len2 <= KARATSUBA_MIN_THRESHOLD) {
        abs_mul64_classic(in1, len1, in2, len2, out, work_begin, work_end);
    } else if (len2 <= KARATSUBA_MAX_THRESHOLD) {
        abs_mul64_karatsuba_buffered(in1, len1, in2, len2, out, work_begin, work_end);
    } else {
        abs_mul64_ntt(in1, len1, in2, len2, out);
    }
}

void abs_mul64(lamp_ptr in1,
                      lamp_ui len1,
                      lamp_ptr in2,
                      lamp_ui len2,
                      lamp_ptr out,
                      lamp_ptr work_begin,
                      lamp_ptr work_end) {
    if (len1 < len2) {
        std::swap(in1, in2);
        std::swap(len1, len2);
    }
    if (len2 <= KARATSUBA_MIN_THRESHOLD) {
        abs_mul64_classic(in1, len1, in2, len2, out, work_begin, work_end);
        return;
    } else if (len2 >= KARATSUBA_MAX_THRESHOLD) {
        lamp_ui M = len1 / len2;
        if (M >= 3 && M <= 9) {
            abs_mul64_ntt_unbalanced(in1, len1, in2, len2, 0, out);
            return;
        } else if (M > 9) {
            M = std::sqrt(len1 / len2);
            abs_mul64_ntt_unbalanced(in1, len1, in2, len2, M, out);
            return;
        } else {
            abs_mul64_ntt(in1, len1, in2, len2, out);
            return;
        }
    }
    // Get enough work memory
    _internal_buffer<0> work_mem(0);
    const lamp_ui work_size = len2 * 3 + len1;  // len1 + len2 + len2 * 2,存放结果以及平衡乘积
    if (work_begin + work_size > work_end) {
        work_mem.resize(work_size + len2 * 6);  // 为karatsuba做准备
        work_begin = work_mem.data();
        work_end = work_begin + work_mem.capacity();
    } else {
        std::fill_n(work_begin, work_size, lamp_ui(0));
    }
    auto balance_prod = work_begin, total_prod = balance_prod + len2 * 2;
    lamp_ui rem = len1 % len2, i = len2;
    abs_mul64_balanced(in2, len2, in1, len2, total_prod, work_begin + work_size, work_end);
    for (; i < len1 - rem; i += len2) {
        abs_mul64_balanced(in2, len2, in1 + i, len2, balance_prod, work_begin + work_size, work_end);
        abs_add_binary_half(balance_prod, len2 * 2, total_prod + i, len2, total_prod + i);
    }
    if (rem > 0) {
        abs_mul64(in2, len2, in1 + i, rem, balance_prod, work_begin + work_size, work_end);
        // 注意这两个加数不可调换
        abs_add_binary_half(total_prod + i, len2, balance_prod, len2 + rem, total_prod + i);
    }
    std::copy(total_prod, total_prod + len1 + len2, out);
}
}; // namespace lammp::Arithmetic
