/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include "../../../../include/lammp/inter_buffer.hpp"
#include "../../../../include/lammp/lammp.hpp"
#include "../../../../include/lammp/uint192.hpp"


namespace lammp::Arithmetic {

lamp_ui barrett_2powN_recursive(lamp_ptr in, lamp_ui len, lamp_ptr out) {
    assert(in != nullptr && len > 0);

    if (len == 1) {
        assert(false && "This function should not be called with len == 1");
        _uint192 n(0, 0, 1);
        n.self_div_rem(in[0]);
        out[0] = n.low64();
        out[1] = n.mid64();
        out[2] = n.high64();
        return rlz(out, 3);
    } else if (len <= 16) {
        lamp_ui _2_powN_len = len * 2 + 1;
        _internal_buffer<0> _2_powN(_2_powN_len + 1, 0);
        lamp_si shift = lammp_clz(in[len - 1]);
        _2_powN.set(_2_powN_len - 1, (1ull << shift));
        _internal_buffer<0> _in(len + 1, 0); /* 此处加一没有用，主要因为lsift强制要求导致的 */
        lshift_in_word(in, len, _in.data(), shift);
        abs_div_knuth(_2_powN.data(), _2_powN_len, _in.data(), len, out, nullptr);
        lamp_ui out_len = rlz(out, len + 1);
        return out_len;
    }

    lamp_ui _len = len / 2 + 1;
    lamp_ui rem_len = len - _len;

    _internal_buffer<0> q_hat(_len + 2, 0); /* q_hat 减去了 rem_len，同时必须多分配一个 */
    lamp_ui q_hat_len = barrett_2powN_recursive(in + rem_len, _len, q_hat.data());

    lamp_ui q_hat_sqr_in_len = get_mul_len(q_hat_len * 2, len);
    _internal_buffer<0> q_hat_sqr_in(q_hat_sqr_in_len + 1, 0);
    abs_mul64(q_hat.data(), q_hat_len, q_hat.data(), q_hat_len, q_hat_sqr_in.data());
    abs_mul64(q_hat_sqr_in.data(), 2 * q_hat_len, in, len, q_hat_sqr_in.data());
    q_hat_sqr_in_len = rlz(q_hat_sqr_in.data(), q_hat_sqr_in_len);
    /*
    x = q * ( 2 * B^2N - q * x ) ./ B^2N
    x = 2 * q - q * q * x / B^2N
    */

    /*
    q_hat:

                   data         data + q_hat_len
    |----rem_len----|----_len + 1-----|
    +---------------+-----------------+
    | 0000000000000 |ttttttttttttttttt|
    +---------------+-----------------+

    q_hat_sqr_in:
                                    data              data + q_hat_sqr_in_len
    |----rem_len----|----rem_len----|-----2 * q_hat_len + len-----|
    +---------------+---------------+-----------------------------+
    | 0000000000000 | 0000000000000 |aaaaaaaaaaaaaaaaaaaaaaaaaaaaa|
    +---------------+---------------+----+------------+-----------+
    |                  delete            |    copy    |    diff   |
    +------------------------------------+------------+-----------+
                                         |            |
                            data + 2*N-2*rem_len  data + 2*N-rem_len

    out:  2 * q_hat - q_hat * q_hat * x / B^2N

    |---- rem_len ----|---- out_len - rem_len -----|
    +-----------------+----------------------------+
    |aaaaaaaaaaaaaaaaa|         2t - a             |
    +-----------------+----------------------------+

    */
    lamp_ui _2len = len << 1, diff_len = q_hat_sqr_in_len - _2len + rem_len;
    lshift_in_word(q_hat.data(), q_hat_len, q_hat.data(), 1);
    assert(q_hat_len + 1 <= q_hat.capacity());
    q_hat_len = rlz(q_hat.data(), q_hat_len + 1);
    std::copy(q_hat_sqr_in.data() + _2len - 2 * rem_len, q_hat_sqr_in.data() + _2len - rem_len, out);
    lamp_si suss =
        abs_sub_binary(q_hat.data(), q_hat_len, q_hat_sqr_in.data() + _2len - rem_len, diff_len, out + rem_len);
    assert(suss >= 0);
    lamp_ui out_len = rlz(out, len + 1);
    return out_len;
}

/*
 * @brief 计算 ceil(base^N / in)，使用牛顿迭代法
 * @param N 指数
 * @param in 被除数
 * @param len 被除数的长度
 * @param out 商的输出数组，长度至少 N + 1 - len + 1
 * @return 商的长度
 * @details
 * 该函数计算 base^N / in，其中 base 为 2^64，in 为一个大整数。
 * 主要调用 barrett_2powN_recursive 函数。
 */
lamp_ui barrett_2powN(lamp_ui N, lamp_ptr in, lamp_ui len, lamp_ptr out) {
    assert(in != nullptr && len > 0);
    assert(N >= 2 * len);

    lamp_ui carry_flag = in[len - 1] & (in[len - 1] - 1);
    for (lamp_ui i = 0; i < len - 1; i++) {
        carry_flag |= in[i];
    }
    if (carry_flag == 0) {
        /*
         * carry_flag == 0 表示 in 为二的幂
         */
        lamp_ui in_bits = len * 64 - lammp_clz(in[len - 1]);
        lamp_ui out_bits = 64 * N + 1 - in_bits;
        lamp_ui word_shr = out_bits / 64;
        lamp_ui bit_shr = out_bits % 64;
        std::fill(out, out + word_shr, 0);
        out[word_shr] = 1ull << bit_shr;
        return rlz(out, word_shr + 1);
    }

    lamp_ui offset = N - 2 * len;

    lamp_ui _in_len = len + 2 + offset;
    _internal_buffer<0> _in(_in_len + 1, 0);
    std::copy(in, in + len, _in.data() + 2 + offset);
    _internal_buffer<0> _out(_in_len + 1, 0);
    lamp_ui _out_len = barrett_2powN_recursive(_in.data(), _in_len, _out.data());
    lamp_ui one[1] = {1};
    std::copy(_out.data() + 2, _out.data() + _out_len, out);
    lamp_ui out_len = rlz(out, _out_len - 2);
    bool carry = abs_add_binary_half(out, out_len, one, 1, out);
    if (carry) {
        out[out_len] = carry;
    }
    // abs_add_binary(out, _out_len, one, 1, out);
    return rlz(out, out_len + 1);
}

}; // namespace lammp::Arithmetic