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
#ifndef __LAMMP_SPE_INT_HPP__
#define __LAMMP_SPE_INT_HPP__
#include <cstdint>
#include "uint128.hpp"

namespace lammp {

/*
 * ============================================================
 * 使用蒙哥马利域计算，消除模乘
 * 计算的编译期常量模数不可以超过 2^62，同时必须大于 2^32
 * ============================================================
 */
template <uint64_t MOD, typename Int128Type = _uint128>
class MontInt64Lazy {
   private:
    static_assert(MOD > UINT32_MAX, "Montgomery64 modulus must be greater than 2^32");
    static_assert(lammp_log2(MOD) < 62, "MOD can't be larger than 62 bits");
    uint64_t data;

   public:
    using IntType = uint64_t;

    constexpr MontInt64Lazy() : data(0) {}
    constexpr MontInt64Lazy(uint64_t n) : data(mulMontCompileTime(n, rSquare())) {}

    constexpr MontInt64Lazy operator+(MontInt64Lazy rhs) const {
        rhs.data = data + rhs.data;
        rhs.data = rhs.data < mod2() ? rhs.data : rhs.data - mod2();
        return rhs;
    }
    constexpr MontInt64Lazy operator-(MontInt64Lazy rhs) const {
        rhs.data = data - rhs.data;
        rhs.data = rhs.data > data ? rhs.data + mod2() : rhs.data;
        return rhs;
    }
    MontInt64Lazy operator*(MontInt64Lazy rhs) const {
        rhs.data = mulMontRunTimeLazy(data, rhs.data);
        return rhs;
    }
    constexpr MontInt64Lazy& operator+=(const MontInt64Lazy& rhs) { return *this = *this + rhs; }
    constexpr MontInt64Lazy& operator-=(const MontInt64Lazy& rhs) { return *this = *this - rhs; }
    constexpr MontInt64Lazy& operator*=(const MontInt64Lazy& rhs) {
        data = mulMontCompileTime(data, rhs.data);
        return *this;
    }
    constexpr MontInt64Lazy largeNorm2() const {
        MontInt64Lazy res;
        res.data = data >= mod2() ? data - mod2() : data;
        return res;
    }
    constexpr MontInt64Lazy rawAdd(MontInt64Lazy rhs) const {
        rhs.data = data + rhs.data;
        return rhs;
    }
    constexpr MontInt64Lazy rawSub(MontInt64Lazy rhs) const {
        rhs.data = data - rhs.data + mod2();
        return rhs;
    }
    constexpr operator uint64_t() const { return toInt(data); }

    static constexpr uint64_t mod() { return MOD; }
    static constexpr uint64_t mod2() { return MOD * 2; }
    static constexpr uint64_t modInv() {
        constexpr uint64_t mod_inv = inv_mod2pow(mod(), 64);  //(mod_inv * mod)%(2^64) = 1
        return mod_inv;
    }
    static constexpr uint64_t modInvNeg() {
        constexpr uint64_t mod_inv_neg = uint64_t(0 - modInv());  //(mod_inv_neg + mod_inv)%(2^64) = 0
        return mod_inv_neg;
    }
    static constexpr uint64_t rSquare() {
        constexpr Int128Type r = (Int128Type(1) << 64) % Int128Type(mod());  // R % mod
        constexpr uint64_t r2 = uint64_t(qpow(r, 2, Int128Type(mod())));     // R^2 % mod
        return r2;
    }
    static_assert((mod() * modInv()) == 1, "mod_inv not correct");

    static constexpr uint64_t toMont(uint64_t n) { return mulMontCompileTime(n, rSquare()); }
    static constexpr uint64_t toInt(uint64_t n) { return redc(Int128Type(n)); }

    static uint64_t redcFastLazy(const Int128Type& input) {
        Int128Type n = uint64_t(input) * modInvNeg();
        n = n * mod();
        n += input;
        return high64(n);
    }
    static uint64_t redcFast(const Int128Type& input) {
        uint64_t n = redcFastLazy(input);
        return n < mod() ? n : n - mod();
    }
    static constexpr uint64_t redc(const Int128Type& input) {
        Int128Type n = uint64_t(input) * modInvNeg();
        n *= Int128Type(mod());
        n += input;
        uint64_t m = high64(n);
        return m < mod() ? m : m - mod();
    }
    static uint64_t mulMontRunTime(uint64_t a, uint64_t b) { return redcFast(Int128Type(a) * b); }
    static uint64_t mulMontRunTimeLazy(uint64_t a, uint64_t b) { return redcFastLazy(Int128Type(a) * b); }
    static constexpr uint64_t mulMontCompileTime(uint64_t a, uint64_t b) {
        Int128Type prod(a);
        prod *= Int128Type(b);
        return redc(prod);
    }
};  // class MontInt64Lazy

};  // namespace lammp

#endif  // __LAMMP_SPE_INT_HPP__