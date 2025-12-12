/*
===============================================================================
1. LGPL v2.1 LICENSED CODE (LAMMP Project)
Copyright (c) 2025-2026 HJimmyK/LAMINA
This file is part of LAMMP (LGPL v2.1 License)
Full LGPL v2.1 License Text: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
LAMMP Repository: https://github.com/Lamina-dev/LAMMP

Modification Note: This file contains modifications to the original MIT-licensed code to adapt to LAMMP's LGPL v2.1
environment.

===============================================================================
2. MIT LICENSED CODE (Original Source)
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky & HJimmyK
Project URL: https://github.com/Twilight-Dream-Of-Magic/Easy-BigInteger

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

#ifndef __LAMMP_BASE_CAL_HPP__
#define __LAMMP_BASE_CAL_HPP__
#include <cstdint>
#include <string>
#if defined(_WIN64)
#include <intrin.h>
#define UMUL128
#endif

#if defined(__SIZEOF_INT128__)
#define UINT128T
#endif

#if defined(_MSC_VER)
#include <nmmintrin.h>
#define _LAMMP_MSVC
#elif defined(__GNUC__)
#define _LAMMP_GCC
#endif

namespace lammp {

template <typename T>
constexpr T all_one(int bits);

template <typename IntTy>
constexpr int lammp_clz(IntTy x);

static inline constexpr int lammp_clz(uint32_t x);
static inline constexpr int lammp_clz(uint64_t x);
static inline constexpr int lammp_ctz(uint32_t x);
static inline constexpr int lammp_ctz(uint64_t x);
static inline constexpr int lammp_cnt(uint32_t x);
static inline constexpr int lammp_cnt(uint64_t x);
static inline constexpr int lammp_bit_length(uint32_t x);
static inline constexpr int lammp_bit_length(uint64_t x);
static inline constexpr int lammp_log2(uint32_t x);
static inline constexpr int lammp_log2(uint64_t x);

template <typename T, typename T1>
constexpr T qpow(T m, T1 n);

template <typename T, typename T1>
constexpr T qpow(T m, T1 n, T mod);

template <typename T>
constexpr T int_floor2(T n);

template <typename T>
constexpr T int_ceil2(T n);

template <typename UintTy>
constexpr UintTy add_half(UintTy x, UintTy y, bool& cf);

template <typename UintTy>
constexpr UintTy sub_half(UintTy x, UintTy y, bool& bf);

template <typename UintTy>
constexpr UintTy add_carry(UintTy x, UintTy y, bool& cf);

template <typename UintTy>
constexpr UintTy add_half_base(UintTy x, UintTy y, bool& carry, const UintTy& base_num);

template <typename UintTy>
constexpr UintTy add_carry_base(UintTy x, UintTy y, bool& carry, const UintTy& base_num);

template <typename UintTy>
constexpr UintTy sub_borrow(UintTy x, UintTy y, bool& bf);

template <typename IntTy>
constexpr IntTy exgcd(IntTy a, IntTy b, IntTy& x, IntTy& y);

template <typename IntTy>
constexpr IntTy mod_inv(IntTy n, IntTy mod);

constexpr uint64_t inv_mod2pow(uint64_t n, int pow);

constexpr void mul64x64to128_base(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high);
static void mul64x64to128(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high);

constexpr uint32_t div128by32_base(uint64_t& dividend_hi64, uint64_t& dividend_lo64, uint32_t divisor);
constexpr uint32_t div96by64to32_base(uint32_t dividend_hi32, uint64_t& dividend_lo64, uint64_t divisor);
constexpr uint64_t div128by64to64_base(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor);
static inline uint64_t div128by64to64(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor);

inline std::string ui64to_string_base10(uint64_t input, uint8_t digits);
static inline void umul128_to_256(uint64_t a_high, uint64_t a_low, uint64_t b_high, uint64_t b_low, uint64_t res[4]);

};  // namespace lammp

#include "../../src/lammp/base_cal/bits.inl"
#include "../../src/lammp/base_cal/carry.inl"
#include "../../src/lammp/base_cal/u128.inl"

#endif // __LAMMP_BASE_CAL_HPP__