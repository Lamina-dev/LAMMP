/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is a part of the LAMMP package.
 * you can see more details about LAMMP at:
 * <https://github.com/Lamina-dev/LAMMP>
 */

#ifndef __LAMMP_HPP__
#define __LAMMP_HPP__

#include <cstdint>
#include <cassert>
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

inline lamp_ui get_buffer_size(lamp_ui len, double base_d) { return static_cast<lamp_ui>(base_d * len) + 2; }

constexpr lamp_ui MIN_LEN = 64;

lamp_ui binary2base(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

lamp_ui base2binary(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

};  // namespace Numeral
};  // namespace Arithmetic
};  // namespace lammp

#include "../../src/lammp/Arithmetic/bit/bit.inl"
#include "../../src/lammp/getlen/getlen.inl"


#endif  // __LAMMP_HPP__