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

#include "../../src/lammp/bits/bits.inl"
#include "../../src/lammp/bits/carry.inl"
#include "../../src/lammp/bits/u128.inl"

#endif // __LAMMP_BASE_CAL_HPP__