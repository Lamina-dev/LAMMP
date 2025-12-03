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

#ifndef __LAMMP_UINT128_HPP__
#define __LAMMP_UINT128_HPP__
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <cassert>
#include "base_cal.hpp"

namespace lammp {

class _uint128 {
   private:
    uint64_t low, high;

   public:
    constexpr _uint128(uint64_t l = 0, uint64_t h = 0) : low(l), high(h) {}
    constexpr _uint128(std::pair<uint64_t, uint64_t> p) : low(p.first), high(p.second) {}

    constexpr _uint128 operator+(_uint128 rhs) const {
        rhs.low += low;
        rhs.high += high + (rhs.low < low);
        return rhs;
    }
    constexpr _uint128 operator-(_uint128 rhs) const {
        rhs.low = low - rhs.low;
        rhs.high = high - rhs.high - (rhs.low > low);
        return rhs;
    }
    constexpr _uint128 operator+(uint64_t rhs) const {
        rhs = low + rhs;
        return _uint128(rhs, high + (rhs < low));
    }
    constexpr _uint128 operator-(uint64_t rhs) const {
        rhs = low - rhs;
        return _uint128(rhs, high - (rhs > low));
    }
    // Only compute the low * rhs.low
    _uint128 operator*(_uint128 rhs) const {
        _uint128 res;
        mul64x64to128(low, rhs.low, res.low, res.high);
        return res;
    }
    // Only compute the low * rhs
    _uint128 operator*(uint64_t rhs) const {
        _uint128 res;
        mul64x64to128(low, rhs, res.low, res.high);
        return res;
    }
    // Only compute the 128bit / 64 bit
    constexpr _uint128 operator/(_uint128 rhs) const { return *this / rhs.low; }
    // Only compute the 128bit % 64 bit
    constexpr _uint128 operator%(const _uint128& rhs) const { return *this % rhs.low; }
    // Only compute the 128bit / 64 bit
    constexpr _uint128 operator/(uint64_t rhs) const {
        _uint128 quot = *this;
        quot.selfDivRem(rhs);
        return quot;
    }
    // Only compute the 128bit % 64 bit
    constexpr _uint128 operator%(uint64_t rhs) const {
        _uint128 quot = *this;
        uint64_t rem = quot.selfDivRem(rhs);
        return _uint128(rem);
    }
    constexpr _uint128& operator+=(const _uint128& rhs) { return *this = *this + rhs; }
    constexpr _uint128& operator-=(const _uint128& rhs) { return *this = *this - rhs; }
    constexpr _uint128& operator+=(uint64_t rhs) { return *this = *this + rhs; }
    constexpr _uint128& operator-=(uint64_t rhs) { return *this = *this - rhs; }
    // Only compute the low * rhs.low
    constexpr _uint128& operator*=(const _uint128& rhs) {
        mul64x64to128_base(low, rhs.low, low, high);
        return *this;
    }
    constexpr _uint128& operator/=(const _uint128& rhs) { return *this = *this / rhs; }
    constexpr _uint128& operator%=(const _uint128& rhs) { return *this = *this % rhs; }
    // Return *this % divisor, *this /= divisor
    constexpr uint64_t selfDivRem(uint64_t divisor) {
        if ((divisor >> 32) == 0) {
            return div128by32_base(high, low, uint32_t(divisor));
        }
        uint64_t divid1 = high % divisor, divid0 = low;
        high /= divisor;
        low = div128by64to64_base(divid1, divid0, divisor);
        return divid0;
    }
    uint64_t self_div_rem(uint64_t divisor) {
        assert((divisor >> 32) > 0);
        uint64_t divid1 = high % divisor, divid0 = low;
        high /= divisor;
        low = div128by64to64(divid1, divid0, divisor);
        return divid0;
    }
    uint32_t self_div_rem(uint32_t divisor) { return div128by32_base(high, low, uint32_t(divisor)); }
    static constexpr _uint128 mul64x64(uint64_t a, uint64_t b) {
        _uint128 res;
        mul64x64to128_base(a, b, res.low, res.high);
        return res;
    }
    static _uint128 mul64x64_fast(uint64_t a, uint64_t b) {
        _uint128 res;
        mul64x64to128(a, b, res.low, res.high);
        return res;
    }
    constexpr bool operator<(const _uint128& rhs) const {
        if (high != rhs.high) {
            return high < rhs.high;
        }
        return low < rhs.low;
    }
    constexpr bool operator==(const _uint128& rhs) const { return high == rhs.high && low == rhs.low; }
    constexpr _uint128 operator<<(int shift) const {
        if (shift == 0) {
            return *this;
        }
        shift %= 128;
        shift = shift < 0 ? shift + 128 : shift;
        if (shift < 64) {
            return _uint128(low << shift, (high << shift) | (low >> (64 - shift)));
        }
        return _uint128(0, low << (shift - 64));
    }
    constexpr _uint128 operator>>(int shift) const {
        if (shift == 0) {
            return *this;
        }
        shift %= 128;
        shift = shift < 0 ? shift + 128 : shift;
        if (shift < 64) {
            return _uint128((low >> shift) | (high << (64 - shift)), high >> shift);
        }
        return _uint128(high >> (shift - 64), 0);
    }
    constexpr _uint128& operator<<=(int shift) { return *this = *this << shift; }
    constexpr _uint128& operator>>=(int shift) { return *this = *this >> shift; }
    constexpr uint64_t high64() const { return high; }
    constexpr uint64_t low64() const { return low; }
    constexpr operator uint64_t() const { return low64(); }
    std::string toStringBase10() const {
        if (high == 0) {
            return std::to_string(low);
        }
        constexpr uint64_t BASE(10000'0000'0000'0000);
        _uint128 copy(*this);
        std::string s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        return std::to_string(uint64_t(copy.selfDivRem(BASE))) + s;
    }
    void printDec() const { std::cout << std::dec << toStringBase10() << '\n'; }
    void printHex() const { std::cout << std::hex << "0x" << high << ' ' << low << std::dec << '\n'; }
};  // _uint128

};  // namespace lammp

#endif  // __LAMMP_UINT128_HPP__