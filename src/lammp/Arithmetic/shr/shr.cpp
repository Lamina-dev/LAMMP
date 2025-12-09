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

namespace lammp::Arithmetic {
/*
 * @brief 左移大整数的每个部分（半移位器）
 *
 * 该函数将一个以64位无符号整数数组表示的大整数左移指定的位数，并将结果存储到另一个数组中。
 * 如果左移操作导致高位溢出，溢出的部分会被返回。
 *
 * @param in 指向输入数组的指针，存储需要左移的整数。
 * @param len 输入数组的长度，即需要左移的64位无符号整数个数。
 * @param out 指向输出数组的指针，存储左移后的整数。
 * @param shift 左移的位数。
 * @return lamp_ui 左移操作后被丢弃的高位部分。
 * @warning shift 必须在0到63（对于64位整数）之间。
 */
lamp_ui lshift_in_word_half(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift) {
    constexpr int WORD_BITS = sizeof(lamp_ui) * CHAR_BIT;
    assert(shift >= 0 && shift < WORD_BITS);
    if (0 == len) {
        return 0;
    }
    if (0 == shift) {
        std::copy(in, in + len, out);
        return 0;
    }
    // [n,last] -> [?,n >> shift_rem | last << shift]
    lamp_ui last = in[len - 1], ret = last;
    const int shift_rem = WORD_BITS - shift;
    lamp_ui i = len - 1;
    while (i > 0) {
        i--;
        lamp_ui n = in[i];
        out[i + 1] = (last << shift) | (n >> shift_rem);
        last = n;
    }
    out[0] = last << shift;
    return ret >> shift_rem;
}
/*
 * @brief 左移大整数的每个部分（全移位器）
 *
 * 该函数将一个以64位无符号整数数组表示的大整数左移指定的位数，并将结果存储到另一个数组中。
 * 如果左移操作导致高位溢出，溢出的部分会被赋值给最后一个输出元素。
 *
 * @param in 指向输入数组的指针，存储需要左移的整数。
 * @param len 输入数组的长度，即需要左移的64位无符号整数个数。
 * @param out 指向输出数组的指针，存储左移后的整数。
 * @param shift 左移的位数。
 * @warning shift 必须在0到63（对于64位整数）之间。且输出数组必须至少有 len+1 个元素。
 */
void lshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift) {
    if (0 == len) {
        return;
    }
    assert(shift >= 0 && lamp_ui(shift) < sizeof(lamp_ui) * CHAR_BIT);
    lamp_ui last = lshift_in_word_half(in, len, out, shift);
    out[len] = last;
}

/*
 * @brief 右移大整数的每个部分（全移位器）
 *
 * 该函数将一个以64位无符号整数数组表示的大整数右移指定的位数，并将结果存储到另一个数组中。
 * 如果右移操作导致低位溢出，溢出的部分会被舍弃。
 *
 * @param in 指向输入数组的指针，存储需要右移的整数。
 * @param len 输入数组的长度，即需要右移的64位无符号整数个数。
 * @param out 指向输出数组的指针，存储右移后的整数。
 * @param shift 右移的位数。
 * @warning shift 必须在0到63（对于64位整数）之间。且输出数组必须至少有 len+1 个元素。
 */
void rshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift) {
    constexpr int WORD_BITS = sizeof(lamp_ui) * CHAR_BIT;
    if (0 == len) {
        return;
    }
    if (0 == shift) {
        std::copy(in, in + len, out);
        return;
    }
    assert(shift >= 0 && lamp_ui(shift) < sizeof(lamp_ui) * CHAR_BIT);
    lamp_ui last = in[0];
    const int shift_rem = WORD_BITS - shift;
    for (lamp_ui i = 1; i < len; i++) {
        lamp_ui n = in[i];
        out[i - 1] = (last >> shift) | (n << shift_rem);
        last = n;
    }
    out[len - 1] = last >> shift;
}

void rshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift) {
    lamp_ui shr_word = shift / 64;
    lamp_ui shr_bits = shift % 64;
    assert(shr_word < len);
    if (shr_bits == 0) {
        std::copy(in + shr_word, in + len, out);
    } else {
        rshift_in_word(in + shr_word, len - shr_word, out, shr_bits);
    }
}

void lshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift) {
    lamp_ui shr_word = shift / 64;
    lamp_ui shr_bits = shift % 64;
    assert(shr_word < len);
    if (shr_bits == 0) {
        std::copy(in + shr_word, in + len, out);
    } else {
        rshift_in_word(in + shr_word, len - shr_word, out, shr_bits);
    }
}

};  // namespace lammp::Arithmetic