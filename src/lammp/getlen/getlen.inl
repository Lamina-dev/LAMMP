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

#ifndef __LAMMP_GETLEN_INL__
#define __LAMMP_GETLEN_INL__
namespace lammp::Arithmetic {
// remove leading zeros and return new length
constexpr lamp_ui rlz(const lamp_ptr array, lamp_ui length) {
    if (array == nullptr) {
        return 0;
    }
    while (length > 0 && array[length - 1] == 0) {
        length--;
    }
    return length;
}

inline lamp_ui get_add_len(lamp_ui l_len, lamp_ui r_len) { return ((l_len < r_len) ? r_len : l_len) + 1; }

inline lamp_ui get_sub_len(lamp_ui l_len, lamp_ui r_len) { return (l_len < r_len) ? r_len : l_len; }

inline lamp_ui get_mul_len(lamp_ui l_len, lamp_ui r_len) {
    if (l_len == 0 || r_len == 0) {
        return 0;
    }
    return l_len + r_len;
}

inline lamp_ui get_div_len(lamp_ui l_len, lamp_ui r_len) { return l_len - r_len + 1; }
};  // namespace lammp::Arithmetic
#endif  // __LAMMP_GETLEN_INL__
