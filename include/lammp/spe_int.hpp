/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is a part of the LAMMP package.
 * you can see more details about LAMMP at:
 * <https://github.com/Lamina-dev/LAMMP>
 */

/*
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky & HJimmyK

https://github.com/Twilight-Dream-Of-Magic/Easy-BigInteger

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