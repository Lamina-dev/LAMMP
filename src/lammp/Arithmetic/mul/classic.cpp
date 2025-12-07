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
#include "../../../../include/lammp/uint128.hpp"

namespace lammp::Arithmetic {

lamp_ui abs_mul_add_num64_half(const lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui num_add, lamp_ui num_mul) {
    lamp_ui i = 0;
    lamp_ui prod_lo, prod_hi;
    for (const lamp_ui rem_len = len - len % 4; i < rem_len; i += 4) {
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);

        mul64x64to128(in[i + 1], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i + 1] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);

        mul64x64to128(in[i + 2], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i + 2] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);

        mul64x64to128(in[i + 3], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i + 3] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);
    }
    for (; i < len; i++) {
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);
    }
    return num_add;
}

/// @brief 2^64 base long integer multiply 64bit number, add another 64bit number to product.
/// @param in Input long integer.
/// @param length Number of 64-bit blocks in the input array.
/// @param out Output long integer, equals to input * num_mul + num_add
/// @param num_add The 64 bit number to add.
/// @param num_mul The 64 bit number to multiply.
/// @details
/// The function performs multiplication and addition on a large integer represented by multiple 64-bit blocks:
/// 1. For each block of the large integer from index 0 to `length-1`:
///    a. Multiply the current block `in[i]` by `num_mul`.
///    b. Add the current value of `num_add` to the product.
///    c. Store the lower 64 bits of the result in `out[i]`.
///    d. Update `num_add` with the higher 64 bits of the product (carry-over to the next block).
/// 2. After processing all blocks, store the final value of `num_add` (the carry-over) in `out[length]`.
void abs_mul_add_num64(const lamp_ptr in, lamp_ui length, lamp_ptr out, lamp_ui num_add, lamp_ui num_mul) {
    for (lamp_ui i = 0; i < length; i++) {
        _uint128 product = _uint128(in[i]) * num_mul + num_add;
        out[i] = lamp_ui(product);
        num_add = product.high64();
    }
    out[length] = num_add;
}

// in * num_mul + in_out -> in_out
void mul64_sub_proc(const lamp_ptr in, lamp_ui len, lamp_ptr in_out, lamp_ui num_mul) {
    lamp_ui carry = 0;
    lamp_ui i = 0;
    for (const lamp_ui rem_len = len - len % 4; i < rem_len; i += 4) {
        bool cf;
        lamp_ui prod_lo, prod_hi;
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i], cf);
        prod_hi += cf;
        in_out[i] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;

        mul64x64to128(in[i + 1], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i + 1], cf);
        prod_hi += cf;
        in_out[i + 1] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;

        mul64x64to128(in[i + 2], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i + 2], cf);
        prod_hi += cf;
        in_out[i + 2] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;

        mul64x64to128(in[i + 3], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i + 3], cf);
        prod_hi += cf;
        in_out[i + 3] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;
    }
    for (; i < len; i++) {
        bool cf;
        uint64_t prod_lo, prod_hi;
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i], cf);
        prod_hi += cf;
        in_out[i] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;
    }
    in_out[len] = carry;
}


// 朴素乘法
void abs_mul64_classic(lamp_ptr in1,
                              lamp_ui len1,
                              lamp_ptr in2,
                              lamp_ui len2,
                              lamp_ptr out,
                              lamp_ptr work_begin,
                              lamp_ptr work_end) {
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
    if (1 == len2) {
        abs_mul_add_num64(in1, len1, out, 0, in2[0]);
        return;
    }
    // Get enough work memory
    _internal_buffer<0> work_mem(0);
    const lamp_ui work_size = get_mul_len(len1, len2);
    if (work_begin + work_size > work_end) {
        work_mem.resize(work_size);
        work_begin = work_mem.data();
        work_end = work_begin + work_mem.capacity();
    } else {
        // Clear work_mem that may used
        std::fill_n(work_begin, work_size, lamp_ui(0));
    }
    auto out_temp = work_begin;
    for (lamp_ui i = 0; i < len1; i++) {
        mul64_sub_proc(in2, len2, out_temp + i, in1[i]);
    }
    std::copy(out_temp, out_temp + work_size, out);
    std::fill(out + work_size, out + out_len, lamp_ui(0));
}
}; // namespace lammp::Arithmetic