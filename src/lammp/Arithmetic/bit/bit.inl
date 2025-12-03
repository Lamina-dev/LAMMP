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


#ifndef __LAMMP_BIT_BIT_INL__
#define __LAMMP_BIT_BIT_INL__
namespace lammp::Arithmetic {
inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui bit_pos, bool value) {
    assert(bit_pos < len * 64);
    const lamp_ui word_pos = bit_pos / 64;
    const lamp_ui bit_in_word_pos = bit_pos % 64;
    in_out[word_pos] |= (value ? 1ull : 0ull) << bit_in_word_pos;
}

inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui word_pos, lamp_ui bit_pos, bool value) {
    assert(word_pos * 64 + bit_pos < len * 64);
    in_out[word_pos] |= (value ? 1ull : 0ull) << bit_pos;
}

inline bool get_bit(const lamp_ptr in, lamp_ui len, lamp_ui bit_pos) {
    assert(bit_pos < len * 64);
    const lamp_ui word_pos = bit_pos / 64;
    const lamp_ui bit_in_word_pos = bit_pos % 64;
    return (in[word_pos] >> bit_in_word_pos) & 1;
}

inline lamp_ui bit_length(lamp_ptr in, lamp_ui len) {
    constexpr lamp_ui WORD_BITS = sizeof(lamp_ui) * CHAR_BIT;
    assert(in != nullptr);
    len = rlz(in, len);
    return len * WORD_BITS - lammp_clz(in[len - 1]);
}
};  // namespace lammp::Arithmetic

#endif  // __LAMMP_BIT_BIT_INL__