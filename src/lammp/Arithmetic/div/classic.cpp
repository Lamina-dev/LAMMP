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

/*
 * @brief 将一个表示为64位块数组的大整数除以一个64位除数
 * @param in 表示要被除的大整数的输入数组。数组的每个元素都是该整数的一个64位块
 * @param length 输入数组中64位块的数量
 * @param out 存储除法结果的输出数组。除法后，out将表示商（*this / divisor）
 * @param divisor 用于除大整数的64位数字
 * @return 除法的余数（*this % divisor），为64位值
 * @details
 * 该函数对由多个64位块表示的大整数执行除法操作：
 * 1. 将`remainder_high64bit`初始化为0
 * 2. 对于大整数的每个块，从最高有效块（索引`length-1`）到最低有效块（索引0）：
 *    a. 组合当前块`in[length]`和当前`remainder_high64bit`以形成128位值
 *    b. 对这个128位值调用`selfDivRem(divisor)`：
 *       - 用64位`divisor`除以128位值
 *       - 用商更新`out[llength]`中的当前块
 *       - 用余数更新`remainder_high64bit`
 * 3. 处理完所有块后，`remainder_high64bit`的最终值就是整个除法的余数
 * 4. 返回`remainder_high64bit`
 */
lamp_ui abs_div_rem_num64(lamp_ptr in, lamp_ui length, lamp_ptr out, lamp_ui divisor) {
    lamp_ui remainder_high64bit = 0;
    assert(divisor != 0);
    if (divisor == 1) {
        std::copy(in, in + length, out);
        return 0;
    } else if (divisor > UINT32_MAX) {
        while (length > 0) {
            length--;
            _uint128 n(in[length], remainder_high64bit);
            remainder_high64bit = n.self_div_rem(divisor);
            out[length] = n;
        }
        return remainder_high64bit;
    } else {
        const uint32_t divisor32 = uint32_t(divisor);
        while (length > 0) {
            length--;
            _uint128 n(in[length], remainder_high64bit);
            remainder_high64bit = n.self_div_rem(divisor32);
            out[length] = n;
        }
        return remainder_high64bit;
    }
}

/*
 in 输入会被更改，且输入in的缓冲区长度应为len+1
 */
void abs_div_knuth(lamp_ptr in,
                   lamp_ui len,
                   lamp_ptr divisor,
                   lamp_ui divisor_len,
                   lamp_ptr out,       // 商的输出数组，长度为len-divisor_len+1
                   lamp_ptr remainder  // 余数的输出数组，长度为divisor_len
) {
    assert(in != nullptr && len > 0 && divisor != nullptr && divisor_len > 0);
    assert(divisor_len <= len);
    assert(divisor[divisor_len - 1] >= (1ull << 63));

    if (divisor_len == 1) {
        if (remainder != nullptr) {
            remainder[0] = abs_div_rem_num64(in, len, out, divisor[0]);
        } else {
            lamp_ui rem = abs_div_rem_num64(in, len, out, divisor[0]);
            std::fill(in, in + len, 0);
            in[0] = rem;
        }
        return;
    }
    if (len == divisor_len) {
        lamp_si sign = abs_compare(in, len, divisor, divisor_len);
        if (remainder != nullptr) {
            if (sign > 0) {
                out[0] = 1;
                abs_sub_binary(in, len, divisor, divisor_len, remainder);
            } else if (sign == 0) {
                out[0] = 1;
                remainder[0] = 0;
            } else {
                out[0] = 0;
                std::copy(in, in + len, remainder);
            }
        } else {
            if (sign > 0) {
                out[0] = 1;
                abs_sub_binary(in, len, divisor, divisor_len, in);
            } else if (sign == 0) {
                out[0] = 1;
                std::fill(in, in + len, 0);
            } else {
                out[0] = 0;
            }
        }
        return;
    }

    lamp_ui out_len = get_div_len(len, divisor_len);
    lamp_ui _dividend_len = len + 1;  // 由于kunth除法一定高估，所以需要多分配一个元素
    _internal_buffer<0> _dividend(_dividend_len, 0);

    for (lamp_ui i = out_len - 1; i-- != 0;) {
        if (len < divisor_len) {
            break;
        }
        _uint128 _dividend_i(in[len - 2], in[len - 1]);

        _dividend_i.self_div_rem(divisor[divisor_len - 1]);

        lamp_ui q_hat[2] = {_dividend_i.low64(), _dividend_i.high64()};
        lamp_ui q_hat_len = rlz(q_hat, 2);

        abs_mul64_classic(q_hat, q_hat_len, divisor, divisor_len, _dividend.data() + i, nullptr, nullptr);
        _dividend_len = rlz(_dividend.data(), i + get_mul_len(divisor_len, q_hat_len));
        lamp_si sign = abs_compare(_dividend.data(), _dividend_len, in, len);
        if (sign > 0) {
            _dividend_i -= 1;
            lamp_si sub_flag =
                abs_sub_binary(_dividend.data() + i, _dividend_len - i, divisor, divisor_len, _dividend.data() + i);
            assert(sub_flag == 0);
            _dividend_len = rlz(_dividend.data(), _dividend_len);
            sign = abs_compare(_dividend.data(), _dividend_len, in, len);
            if (sign > 0) {
                sub_flag =
                    abs_sub_binary(_dividend.data() + i, _dividend_len - i, divisor, divisor_len, _dividend.data() + i);
                assert(sub_flag == 0);
            }
        }
        lamp_si sus_flag = abs_sub_binary(in + i, len - i, _dividend.data() + i, _dividend_len - i, in + i);
        assert(sus_flag == 0);
        out[i + 1] = (_dividend_i.high64() == 0) ? out[i + 1] : _dividend_i.high64();
        out[i] = _dividend_i.low64();
        len = rlz(in, len);
        std::fill(_dividend.data() + i, _dividend.data() + _dividend_len, 0);
    }
    if (remainder != nullptr) {
        std::copy(in, in + len, remainder);
    }
}
};  // namespace lammp::Arithmetic