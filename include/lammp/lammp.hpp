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
#include "alloc.hpp"
#include "uint192.hpp"
#include "spe_int.hpp"
#include "base_cal.hpp"
#include "num_theo.hpp"

namespace lammp {
namespace Arithmetic {
typedef uint64_t lamp_ui;
typedef uint64_t* lamp_ptr;
typedef int64_t lamp_si;

class lampz {
   protected:
    lamp_si _len;                   /* 绝对值表示大整数的非前导零长度，负值即代表为负数 */
    _internal_buffer<0> _lamp_data; /*数组缓冲区*/
   public:
    lamp_ptr get_ptr() { return _lamp_data.data(); }
    lamp_ui get_len() const { return std::abs(_len); }
    lamp_si get_sign() const { return _len < 0 ? -1 : 1; }
};  // class lampz

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
bool abs_add_half_base(lamp_ptr a,
                                 lamp_ui len_a,
                                 lamp_ptr b,
                                 lamp_ui len_b,
                                 lamp_ptr sum,
                                 const lamp_ui base_num);
void abs_add_binary(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum);
void abs_add_base(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum, lamp_ui base_num);
bool abs_sub_binary(lamp_ptr a,
                              lamp_ui len_a,
                              lamp_ptr b,
                              lamp_ui len_b,
                              lamp_ptr diff,
                              bool assign_borow = false);
bool abs_sub_binary_num(lamp_ptr a, lamp_ui len_a, lamp_ui num, lamp_ptr diff);

[[nodiscard]] auto abs_compare(const lamp_ptr in1, const lamp_ptr in2, lamp_ui len);
[[nodiscard]] int abs_compare(const lamp_ptr in1, lamp_ui len1, const lamp_ptr in2, lamp_ui len2);
[[nodiscard]] lamp_si abs_difference_binary(lamp_ptr a,
                                                      lamp_ui len1,
                                                      lamp_ptr b,
                                                      lamp_ui len2,
                                                      lamp_ptr diff);


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
void abs_mul64_ntt_base(lamp_ptr in1,
                               lamp_ui len1,
                               lamp_ptr in2,
                               lamp_ui len2,
                               lamp_ptr out,
                               const lamp_ui base_num);
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
namespace BaseTable {
constexpr lamp_ui table1[35][2] = {
    {9223372036854775808ull, 63ull},  {12157665459056928801ull, 40ull}, {4611686018427387904ull, 31ull},
    {7450580596923828125ull, 27ull},  {4738381338321616896ull, 24ull},  {3909821048582988049ull, 22ull},
    {9223372036854775808ull, 21ull},  {12157665459056928801ull, 20ull}, {10000000000000000000ull, 19ull},
    {5559917313492231481ull, 18ull},  {2218611106740436992ull, 17ull},  {8650415919381337933ull, 17ull},
    {2177953337809371136ull, 16ull},  {6568408355712890625ull, 16ull},  {1152921504606846976ull, 15ull},
    {2862423051509815793ull, 15ull},  {6746640616477458432ull, 15ull},  {15181127029874798299ull, 15ull},
    {1638400000000000000ull, 14ull},  {3243919932521508681ull, 14ull},  {6221821273427820544ull, 14ull},
    {11592836324538749809ull, 14ull}, {876488338465357824ull, 13ull},   {1490116119384765625ull, 13ull},
    {2481152873203736576ull, 13ull},  {4052555153018976267ull, 13ull},  {6502111422497947648ull, 13ull},
    {10260628712958602189ull, 13ull}, {15943230000000000000ull, 13ull}, {787662783788549761ull, 12ull},
    {1152921504606846976ull, 12ull},  {1667889514952984961ull, 12ull},  {2386420683693101056ull, 12ull},
    {3379220508056640625ull, 12ull},  {4738381338321616896ull, 12ull}};
constexpr double table2[35] = {
    1.015873015873016e+00, 1.009487605714332e+00, 1.032258064516129e+00, 1.020862952470265e+00, 1.031607485958778e+00,
    1.036239089768792e+00, 1.015873015873016e+00, 1.009487605714332e+00, 1.013995774868147e+00, 1.027786049130268e+00,
    1.050138148333665e+00, 1.017367169608733e+00, 1.050598140148774e+00, 1.023832099239262e+00, 1.066666666666667e+00,
    1.043842313037765e+00, 1.023199857357361e+00, 1.004411363697657e+00, 1.057728974444613e+00, 1.040778279757500e+00,
    1.025114624994631e+00, 1.010581620377160e+00, 1.073744206698002e+00, 1.060126912180660e+00, 1.047365186724249e+00,
    1.039781450100194e+00, 1.031607485958778e+00};

};  // namespace BaseTable 
std::string to_string_base(lamp_ui num, const lamp_ui base, const lamp_ui base_len);

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

lamp_ui num2base(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

lamp_ui base2num(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

};  // namespace Numeral
};  // namespace Arithmetic
};  // namespace lammp

#include "../../src/lammp/getlen/getlen.inl"
#include "../../src/lammp/Arithmetic/bit/bit.inl"

#endif  // __LAMMP_HPP__