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

#ifndef __LAMMP_HPP__
#define __LAMMP_HPP__

#include <math.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

#include "inter_buffer.hpp"
#include "base_cal.hpp"

namespace lammp {
namespace Arithmetic {
typedef uint64_t lamp_ui;
typedef uint64_t* lamp_ptr;
typedef int64_t lamp_si;

constexpr lamp_ui rlz(const lamp_ptr array, lamp_ui length);
inline lamp_ui get_add_len(lamp_ui l_len, lamp_ui r_len);
inline lamp_ui get_sub_len(lamp_ui l_len, lamp_ui r_len);
inline lamp_ui get_mul_len(lamp_ui l_len, lamp_ui r_len);
inline lamp_ui get_div_len(lamp_ui l_len, lamp_ui r_len);

inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui bit_pos, bool value = true);
inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui word_pos, lamp_ui bit_pos, bool value = true);
inline bool get_bit(const lamp_ptr in, lamp_ui len, lamp_ui bit_pos);
inline lamp_ui bit_length(lamp_ptr in, lamp_ui len);

lamp_ui lshift_in_word_half(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift);
void lshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift);
void rshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift);
void rshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift);
void lshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift);

bool abs_add_binary_half(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum);
bool abs_add_half_base(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum, const lamp_ui base_num);
void abs_add_binary(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum);
void abs_add_base(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum, lamp_ui base_num);
bool abs_sub_binary(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr diff, bool assign_borow = false);
bool abs_sub_binary_num(lamp_ptr a, lamp_ui len_a, lamp_ui num, lamp_ptr diff);

[[nodiscard]] auto abs_compare(const lamp_ptr in1, const lamp_ptr in2, lamp_ui len);
[[nodiscard]] int abs_compare(const lamp_ptr in1, lamp_ui len1, const lamp_ptr in2, lamp_ui len2);
[[nodiscard]] lamp_si abs_difference_binary(lamp_ptr a, lamp_ui len1, lamp_ptr b, lamp_ui len2, lamp_ptr diff);

lamp_ui abs_mul_add_num64_half(const lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui num_add, lamp_ui num_mul);

void abs_mul_add_num64(const lamp_ptr in, lamp_ui length, lamp_ptr out, lamp_ui num_add, lamp_ui num_mul);

void mul64_sub_proc(const lamp_ptr in, lamp_ui len, lamp_ptr in_out, lamp_ui num_mul);

constexpr size_t KARATSUBA_MIN_THRESHOLD = 24;
constexpr size_t KARATSUBA_MAX_THRESHOLD = 1536;

void abs_mul64_classic(lamp_ptr in1,
                       lamp_ui len1,
                       lamp_ptr in2,
                       lamp_ui len2,
                       lamp_ptr out,
                       lamp_ptr work_begin,
                       lamp_ptr work_end);

void abs_mul64_karatsuba_buffered(lamp_ptr in1,
                                  lamp_ui len1,
                                  lamp_ptr in2,
                                  lamp_ui len2,
                                  lamp_ptr out,
                                  lamp_ptr buffer_begin,
                                  lamp_ptr buffer_end);
void abs_mul64_karatsuba(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out);
void abs_sqr64_ntt(lamp_ptr in, lamp_ui len, lamp_ptr out);
void abs_mul64_ntt(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out);
void abs_mul64_ntt_unbalanced(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ui M, lamp_ptr out);
void abs_sqr64_ntt_base(lamp_ptr in, lamp_ui len, lamp_ptr out, const lamp_ui base_num);
void abs_mul64_ntt_base(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out, const lamp_ui base_num);
void abs_mul64_balanced(lamp_ptr in1,
                        lamp_ui len1,
                        lamp_ptr in2,
                        lamp_ui len2,
                        lamp_ptr out,
                        lamp_ptr work_begin = nullptr,
                        lamp_ptr work_end = nullptr);
void abs_mul64(lamp_ptr in1,
               lamp_ui len1,
               lamp_ptr in2,
               lamp_ui len2,
               lamp_ptr out,
               lamp_ptr work_begin = nullptr,
               lamp_ptr work_end = nullptr);

#include "../../src/lammp/Arithmetic/div/div_supp.inl"

lamp_ui abs_div_rem_num64(lamp_ptr in, lamp_ui length, lamp_ptr out, lamp_ui divisor);

void abs_div_knuth(lamp_ptr in,
                   lamp_ui len,
                   lamp_ptr divisor,
                   lamp_ui divisor_len,
                   lamp_ptr out,
                   lamp_ptr remainder = nullptr);

lamp_ui barrett_2powN_recursive(lamp_ptr in, lamp_ui len, lamp_ptr out);

lamp_ui barrett_2powN(lamp_ui N, lamp_ptr in, lamp_ui len, lamp_ptr out);

namespace Numeral {


lamp_ui num2base_classic(lamp_ptr in, lamp_ui len, const lamp_ui base_num, lamp_ptr res);

lamp_ui base2num_classic(lamp_ptr in, lamp_ui len, const lamp_ui base_num, lamp_ptr res);

lamp_ui _2_64power_index_classic(const lamp_ui base_num, const lamp_ui index, lamp_ptr res);

lamp_ui base_power_index_classic(const lamp_ui base_num, const lamp_ui index, lamp_ptr res);

lamp_ui get_buffer_size(lamp_ui len, double base_d);

void abs_mul2pow64_base(lamp_ptr in, lamp_ui len, const lamp_ui base_num);

void abs_mul_base(lamp_ptr in, lamp_ui len, const lamp_ui base_num);

lamp_ui _2_64_power_index(const lamp_ui base_num, const lamp_ui index, lamp_ptr res, lamp_ui res_len);

lamp_ui base_power_index(const lamp_ui base_num, const lamp_ui index, lamp_ptr res, lamp_ui res_len);

typedef struct base_index_node {
    lamp_ui index;
    lamp_ui length;
    base_index_node* front;
    base_index_node* back;
    _internal_buffer<0> base_index;
    base_index_node(lamp_ui _index, double base_d) {
        index = _index;
        length = get_buffer_size(_index, base_d);
        base_index.resize(length);
        front = nullptr;
        back = nullptr;
    }
}* _2pow64_index_list;

typedef struct base_index_node* _base_index_list;

constexpr size_t MIN_LEN = 64ull;

lamp_ui num_base_recursive_core(lamp_ptr in,
                                lamp_ui len,
                                const lamp_ui base_num,
                                const double base_d,
                                lamp_ptr out,
                                const _2pow64_index_list list);

_2pow64_index_list create_2pow64_index_list(lamp_ui max_index, const lamp_ui base_num, const double base_d);

lamp_ui base_num_recursive_core(lamp_ptr in,
                                lamp_ui len,
                                const lamp_ui base_num,
                                const double base_d,
                                lamp_ptr out,
                                const _base_index_list list);

_base_index_list create_base_index_list(lamp_ui max_index, const double base_d, const lamp_ui base_num);

_2pow64_index_list find_head(_2pow64_index_list head, lamp_ui index);

lamp_ui binary2base(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

lamp_ui base2binary(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

};  // namespace Numeral
};  // namespace Arithmetic
};  // namespace lammp

#include "../../src/lammp/Arithmetic/bit/bit.inl"
#include "../../src/lammp/getlen/getlen.inl"


#endif  // __LAMMP_HPP__