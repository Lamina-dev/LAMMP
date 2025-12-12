/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is a part of the LAMMP package.
 * you can see more details about LAMMP at:
 * <https://github.com/Lamina-dev/LAMMP>
 */

/*
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky & HJimmyK

https://github.com/Twilight-Dream-Of-Magic/Easy-BigInteger

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
#include <algorithm>
namespace lammp::Arithmetic {
// Binary absolute addtion a+b=sum, return the carry
bool abs_add_binary_half(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum) {
    bool carry = false;
    lamp_ui i = 0, min_len = std::min(len_a, len_b);
    for (; i < min_len; i++) {
        sum[i] = add_carry(a[i], b[i], carry);
    }
    for (; i < len_a; i++) {
        sum[i] = add_half(a[i], lamp_ui(carry), carry);
    }
    for (; i < len_b; i++) {
        sum[i] = add_half(b[i], lamp_ui(carry), carry);
    }
    return carry;
}

bool abs_add_half_base(lamp_ptr a,
                                 lamp_ui len_a,
                                 lamp_ptr b,
                                 lamp_ui len_b,
                                 lamp_ptr sum,
                                 const lamp_ui base_num) {
    bool carry = false;
    lamp_ui i = 0, min_len = std::min(len_a, len_b);
    for (; i < min_len; i++) {
        sum[i] = add_carry_base(a[i], b[i], carry, base_num);
    }
    for (; i < len_a; i++) {
        sum[i] = add_half_base(a[i], lamp_ui(carry), carry, base_num);
    }
    for (; i < len_b; i++) {
        sum[i] = add_half_base(b[i], lamp_ui(carry), carry, base_num);
    }
    return carry;
}
// Binary absolute addtion a+b=sum, return the carry
void abs_add_binary(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum) {
    bool carry = abs_add_binary_half(a, len_a, b, len_b, sum);
    sum[std::max(len_a, len_b)] = (lamp_ui)carry;
}

void abs_add_base(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum, lamp_ui base_num) {
    lamp_ui carry = abs_add_half_base(a, len_a, b, len_b, sum, base_num);
    sum[std::max(len_a, len_b)] = carry;
}

// Binary absolute subtraction a-b=diff, return the borrow
bool abs_sub_binary(lamp_ptr a,
                              lamp_ui len_a,
                              lamp_ptr b,
                              lamp_ui len_b,
                              lamp_ptr diff,
                              bool assign_borow) {
    bool borrow = false;
    lamp_ui i = 0, min_len = std::min(len_a, len_b);
    for (; i < min_len; i++) {
        diff[i] = sub_borrow(a[i], b[i], borrow);
    }
    for (; i < len_a; i++) {
        diff[i] = sub_half(a[i], lamp_ui(borrow), borrow);
    }
    for (; i < len_b; i++) {
        diff[i] = sub_half(lamp_ui(0) - lamp_ui(borrow), b[i], borrow);
    }
    if (assign_borow) {
        diff[i] = lamp_ui(borrow);  // 借位
    }
    return borrow;
}

// a - num
bool abs_sub_binary_num(lamp_ptr a, lamp_ui len_a, lamp_ui num, lamp_ptr diff) {
    assert(len_a > 0);
    bool borrow = false;
    lamp_ui i = 1;
    diff[0] = sub_half(a[0], num, borrow);
    for (; i < len_a; i++) {
        diff[i] = sub_half(a[i], lamp_ui(borrow), borrow);
    }
    return borrow;
}

// Absolute compare, return 1 if a > b, -1 if a < b, 0 if a == b
// Return the diffence length if a != b
[[nodiscard]] auto abs_compare(const lamp_ptr in1, const lamp_ptr in2, lamp_ui len) {
    struct CompareResult {
        lamp_ui diff_len;
        int cmp = 0;
    };
    while (len > 0) {
        len--;
        if (in1[len] != in2[len]) {
            CompareResult result{len + 1, 0};
            result.cmp = in1[len] > in2[len] ? 1 : -1;
            return result;
        }
    }
    return CompareResult{0, 0};
}

// Absolute compare, return 1 if a > b, -1 if a < b, 0 if a == b
[[nodiscard]] int abs_compare(const lamp_ptr in1, lamp_ui len1, const lamp_ptr in2, lamp_ui len2) {
    if (len1 != len2) {
        return len1 > len2 ? 1 : -1;
    }
    return abs_compare(in1, in2, len1).cmp;
}

/**
 * @brief 计算两个二进制表示的大整数的绝对值差，并返回符号
 *
 * 该函数用于处理以数组形式存储的大整数（每个元素为 lamp_ui 类型的数字片段），
 * 计算两者的绝对值差并存储到输出数组中，同时通过返回值指示原始两个数的大小关系。
 * 内部确保用大数减去小数，避免负数结果，简化差值计算逻辑。
 *
 * @tparam lamp_ui 数组元素类型，必须为无符号整数类型
 * @param[in] a 第一个大整数的二进制表示数组
 * @param[in] len1 a 数组的长度（元素个数）
 * @param[in] b 第二个大整数的二进制表示数组
 * @param[in] len2 b 数组的长度（元素个数）
 * @param[out] diff 输出参数，用于存储 a 和 b 的绝对值差结果（需提前分配足够空间）
 * @return int 符号：
 *          1 表示 a > b（diff 存储 a - b）
 *          -1 表示 a < b（diff 存储 b - a）
 *          0 表示 a == b（diff 存储全 0）
 * @warning diff 数组的长度需至少为两个输入数组中的最大长度，将不会进行越界检查。
 */
[[nodiscard]] lamp_si abs_difference_binary(lamp_ptr a,
                                                      lamp_ui len1,
                                                      lamp_ptr b,
                                                      lamp_ui len2,
                                                      lamp_ptr diff) {
    int sign = 1;
    if (len1 == len2) {
        auto cmp = abs_compare(a, b, len1);
        sign = cmp.cmp;
        std::fill(diff + cmp.diff_len, diff + len1, lamp_ui(0));
        len1 = len2 = cmp.diff_len;
        if (sign < 0) {
            std::swap(a, b);
        }
    } else if (len1 < len2) {
        std::swap(a, b);
        std::swap(len1, len2);
        sign = -1;
    }
    abs_sub_binary(a, len1, b, len2, diff);
    return sign;
}
};  // namespace lammp::Arithmetic