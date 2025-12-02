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

#ifndef __LAMMP_HPP__
#define __LAMMP_HPP__

#include <math.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include "alloc.hpp"
#include "integer.hpp"
#include "base_cal.hpp"


namespace lammp {


namespace Transform {
namespace number_theory {

constexpr uint64_t MOD0 = 2485986994308513793ull, ROOT0 = 5ull;
constexpr uint64_t MOD1 = 1945555039024054273ull, ROOT1 = 5ull;
constexpr uint64_t MOD2 = 4179340454199820289ull, ROOT2 = 3ull;
constexpr uint64_t MOD3 = 754974721ull, ROOT3 = 11ull;
constexpr uint64_t MOD4 = 469762049ull, ROOT4 = 3ull;
constexpr uint64_t MOD5 = 3489660929ull, ROOT5 = 3ull;
constexpr uint64_t MOD6 = 3221225473ull, ROOT6 = 5ull;


template <typename Int128Type>
constexpr uint64_t high64(const Int128Type& n) {
    return n >> 64;
}
constexpr uint64_t high64(const _uint128& n) { return n.high64(); }

#ifdef UINT128T
using uint128_default = __uint128_t;
#else
using uint128_default = _uint128;
#endif


template <typename IntType>
constexpr bool check_inv(uint64_t n, uint64_t n_inv, uint64_t mod);

template <typename ModInt1, typename ModInt2, typename ModInt3>
inline _uint192 crt3(ModInt1 n1, ModInt2 n2, ModInt3 n3);

namespace SplitRadix {

template <typename T>
inline void transform2(T& sum, T& diff);

template <uint64_t ROOT, typename ModIntType>
inline ModIntType mul_w41(ModIntType n);

template <uint64_t ROOT, typename ModIntType>
inline void dit_butterfly244(ModIntType& in_out0, ModIntType& in_out1, ModIntType& in_out2, ModIntType& in_out3);

template <uint64_t ROOT, typename ModIntType>
inline void dif_butterfly244(ModIntType& in_out0, ModIntType& in_out1, ModIntType& in_out2, ModIntType& in_out3);

template <typename ModIntType>
inline void dit_butterfly2(ModIntType& in_out0, ModIntType& in_out1, const ModIntType& omega);

template <typename ModIntType>
inline void dif_butterfly2(ModIntType& in_out0, ModIntType& in_out1, const ModIntType& omega);

template <size_t MAX_LEN, uint64_t ROOT, typename ModIntType>
struct NTTShort {
    static constexpr size_t NTT_LEN = MAX_LEN;
    static constexpr int LOG_LEN = lammp_log2(NTT_LEN);
    struct TableType {
        std::array<ModIntType, NTT_LEN> omega_table;
        TableType();
        constexpr ModIntType& operator[](size_t i);
        constexpr const ModIntType& operator[](size_t i) const;
        constexpr const ModIntType* getOmegaIt(size_t len) const;
    };

    static TableType table;

    static void dit(ModIntType in_out[], size_t len);
    static void dif(ModIntType in_out[], size_t len);
};

template <size_t LEN, uint64_t ROOT, typename ModIntType>
typename NTTShort<LEN, ROOT, ModIntType>::TableType NTTShort<LEN, ROOT, ModIntType>::table;
template <size_t LEN, uint64_t ROOT, typename ModIntType>
constexpr size_t NTTShort<LEN, ROOT, ModIntType>::NTT_LEN;
template <size_t LEN, uint64_t ROOT, typename ModIntType>
constexpr int NTTShort<LEN, ROOT, ModIntType>::LOG_LEN;

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<0, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]);
    static void dif(ModIntType in_out[]);
    static void dit(ModIntType in_out[], size_t len);
    static void dif(ModIntType in_out[], size_t len);
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<1, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]);
    static void dif(ModIntType in_out[]);
    static void dit(ModIntType in_out[], size_t len);
    static void dif(ModIntType in_out[], size_t len);
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<2, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]);
    static void dif(ModIntType in_out[]);
    static void dit(ModIntType in_out[], size_t len);
    static void dif(ModIntType in_out[], size_t len);
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<4, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]);
    static void dif(ModIntType in_out[]);
    static void dit(ModIntType in_out[], size_t len);
    static void dif(ModIntType in_out[], size_t len);
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<8, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]);
    static void dif(ModIntType in_out[]);
    static void dit(ModIntType in_out[], size_t len);
    static void dif(ModIntType in_out[], size_t len);
};

template <uint64_t MOD, uint64_t ROOT, typename Int128Type = uint128_default>
struct NTT {
    static constexpr uint64_t mod();
    static constexpr uint64_t root();
    static constexpr uint64_t rootInv();
    static_assert(root() < mod(), "ROOT must be smaller than MOD");
    static_assert(check_inv<Int128Type>(root(), rootInv(), mod()), "IROOT * ROOT % MOD must be 1");
    static constexpr int MOD_BITS = lammp_log2(mod()) + 1;
    static constexpr int MAX_LOG_LEN = lammp_ctz(mod() - 1);
    static constexpr size_t getMaxLen();
    static constexpr size_t NTT_MAX_LEN = getMaxLen();

    using INTT = NTT<mod(), rootInv(), Int128Type>;
    using ModInt64Type = MontInt64Lazy<MOD, Int128Type>;
    using ModIntType = ModInt64Type;
    using IntType = typename ModIntType::IntType;

    static constexpr size_t L2_BYTE = size_t(1) << 20;
    static constexpr size_t LONG_THRESHOLD = std::min(L2_BYTE / sizeof(ModIntType), NTT_MAX_LEN);
    using NTTTemplate = NTTShort<LONG_THRESHOLD, root(), ModIntType>;

    static void dit244(ModIntType in_out[], size_t ntt_len);
    static void dif244(ModIntType in_out[], size_t ntt_len);
    static void convolution(ModIntType in1[], ModIntType in2[], ModIntType out[], size_t ntt_len, bool normlize = true);

    static void convolutionRecursion(ModIntType in1[],
                                     ModIntType in2[],
                                     ModIntType out[],
                                     size_t ntt_len,
                                     bool normlize = true);
    static void convolutionRecursion_rep(const ModIntType in1[],
                                         ModIntType in2[],
                                         ModIntType out[],
                                         size_t ntt_len,
                                         bool normlize = true);
};

template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr int NTT<MOD, ROOT, Int128Type>::MOD_BITS;
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr int NTT<MOD, ROOT, Int128Type>::MAX_LOG_LEN;
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr size_t NTT<MOD, ROOT, Int128Type>::NTT_MAX_LEN;
};  // namespace SplitRadix

using NTT0 = SplitRadix::NTT<MOD0, ROOT0>;
using NTT1 = SplitRadix::NTT<MOD1, ROOT1>;
using NTT2 = SplitRadix::NTT<MOD2, ROOT2>;

};  // namespace number_theory
};  // namespace Transform

namespace Arithmetic {
typedef uint64_t lamp_ui;
typedef uint64_t* lamp_ptr;
typedef int64_t lamp_si;

class lampz {
   protected:
    lamp_si _len;
    _internal_buffer<0> _lamp_data;

   public:
    lamp_ptr get_ptr();
    lamp_ui get_len() const;
    lamp_si get_sign() const;
};

constexpr lamp_ui rlz(const lamp_ptr array, lamp_ui length);
inline lamp_ui get_add_len(lamp_ui l_len, lamp_ui r_len);
inline lamp_ui get_sub_len(lamp_ui l_len, lamp_ui r_len);
inline lamp_ui get_mul_len(lamp_ui l_len, lamp_ui r_len);
inline lamp_ui get_div_len(lamp_ui l_len, lamp_ui r_len);
inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui bit_pos, bool value = true);
inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui word_pos, lamp_ui bit_pos, bool value = true);
inline bool get_bit(const lamp_ptr in, lamp_ui len, lamp_ui bit_pos);
inline lamp_ui bit_length(lamp_ptr in, lamp_ui len);

constexpr lamp_ui lshift_in_word_half(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift);
constexpr void lshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift);
constexpr void rshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift);
constexpr void rshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift);
constexpr void lshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift);

constexpr bool abs_add_binary_half(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum);
constexpr bool abs_add_half_base(lamp_ptr a,
                                 lamp_ui len_a,
                                 lamp_ptr b,
                                 lamp_ui len_b,
                                 lamp_ptr sum,
                                 const lamp_ui base_num);
constexpr void abs_add_binary(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum);
constexpr void abs_add_base(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum, lamp_ui base_num);
constexpr bool abs_sub_binary(lamp_ptr a,
                              lamp_ui len_a,
                              lamp_ptr b,
                              lamp_ui len_b,
                              lamp_ptr diff,
                              bool assign_borow = false);
constexpr bool abs_sub_binary_num(lamp_ptr a, lamp_ui len_a, lamp_ui num, lamp_ptr diff);

[[nodiscard]] constexpr auto abs_compare(const lamp_ptr in1, const lamp_ptr in2, lamp_ui len);
[[nodiscard]] constexpr int abs_compare(const lamp_ptr in1, lamp_ui len1, const lamp_ptr in2, lamp_ui len2);
[[nodiscard]] constexpr lamp_si abs_difference_binary(lamp_ptr a,
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

template <typename NumTy, typename ProdTy>
class DivSupporter {
   private:
    NumTy divisor = 0;
    NumTy inv = 0;
    int shift = 0, shift1 = 0, shift2 = 0;
    enum : int { NUM_BITS = sizeof(NumTy) * CHAR_BIT };

   public:
    constexpr DivSupporter(NumTy divisor_in);
    NumTy divMod(ProdTy& dividend) const;
    void prodDivMod(NumTy a, NumTy b, NumTy& quot, NumTy& rem) const;
    NumTy div(ProdTy dividend) const;
    NumTy mod(ProdTy dividend) const;
    static constexpr NumTy getInv(NumTy divisor, int& leading_zero);
};

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

struct BaseInfo {
    lamp_ui base_num;
    lamp_ui base_len;
    double base_d;
    BaseInfo(lamp_ui base);
    void base_d_inv();
};

template <lamp_ui Base>
struct ShortBaseInfo {
    static_assert(Base >= 2 && Base <= 36, "Base must be in [2, 36]");
    static constexpr lamp_ui index = Base - 2;
    static constexpr lamp_ui base_num = table1[index][0];
    static constexpr lamp_ui base_len = table1[index][1];
    static constexpr double base_d = table2[index];
    static constexpr double base_d_inv();
};
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

    base_index_node(lamp_ui _index, double base_d);
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

_base_index_list create_base_index_list(lamp_ui max_index, const lamp_ui base_num, const double base_d);

_2pow64_index_list find_head(_2pow64_index_list head, lamp_ui index);

lamp_ui num2base(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

lamp_ui base2num(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res);

};  // namespace Numeral
};  // namespace Arithmetic
};  // namespace lammp


#endif  // __LAMMP_HPP__