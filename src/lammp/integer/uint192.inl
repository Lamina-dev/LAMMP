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

namespace lammp{

/*
 *=======================================================================
 * 192位无符号整数类
 *=======================================================================
 */
class _uint192 {
    friend _uint128;

   private:
    uint64_t low, mid, high;

   public:
    constexpr _uint192() : low(0), mid(0), high(0) {}
    constexpr _uint192(uint64_t low, uint64_t mi = 0, uint64_t high = 0) : low(low), mid(mi), high(high) {}
    constexpr _uint192(_uint128 n) : low(n.low64()), mid(n.high64()), high(0) {}
    constexpr _uint192 operator+(_uint192 rhs) const {
        bool cf = false;
        rhs.low = add_half(low, rhs.low, cf);
        rhs.mid = add_carry(mid, rhs.mid, cf);
        rhs.high = high + rhs.high + cf;
        return rhs;
    }
    constexpr _uint192 operator-(_uint192 rhs) const {
        bool bf = false;
        rhs.low = sub_half(low, rhs.low, bf);
        rhs.mid = sub_borrow(mid, rhs.mid, bf);
        rhs.high = high - rhs.high - bf;
        return rhs;
    }
    constexpr _uint192 operator/(uint64_t rhs) const {
        _uint192 result(*this);
        result.selfDivRem(rhs);
        return result;
    }
    constexpr _uint192 operator%(uint64_t rhs) const {
        _uint192 result(*this);
        return result.selfDivRem(rhs);
    }
    constexpr _uint192& operator+=(const _uint192& rhs) { return *this = *this + rhs; }
    constexpr _uint192& operator-=(const _uint192& rhs) { return *this = *this - rhs; }
    constexpr _uint192& operator/=(const _uint192& rhs) { return *this = *this / rhs; }
    constexpr _uint192& operator%=(const _uint192& rhs) { return *this = *this % rhs; }
    constexpr _uint192 operator<<(int shift) const {
        if (shift == 0) {
            return *this;
        }
        shift %= 192;
        shift = shift < 0 ? shift + 192 : shift;
        if (shift < 64) {
            return _uint192(low << shift, (mid << shift) | (low >> (64 - shift)),
                            (high << shift) | (mid >> (64 - shift)));
        } else if (shift < 128) {
            shift -= 64;
            return _uint192(0, low << shift, (mid << shift) | (low >> (64 - shift)));
        }
        return _uint192(0, 0, low << (shift - 128));
    }
    friend constexpr bool operator<(const _uint192& lhs, const _uint192& rhs) {
        if (lhs.high != rhs.high) {
            return lhs.high < rhs.high;
        }
        if (lhs.mid != rhs.mid) {
            return lhs.mid < rhs.mid;
        }
        return lhs.low < rhs.low;
    }
    friend constexpr bool operator<=(const _uint192& lhs, const _uint192& rhs) { return !(rhs > lhs); }
    friend constexpr bool operator>(const _uint192& lhs, const _uint128& rhs) { return rhs < lhs; }
    friend constexpr bool operator>=(const _uint192& lhs, const _uint192& rhs) { return !(lhs < rhs); }
    friend constexpr bool operator==(const _uint192& lhs, const _uint192& rhs) {
        return lhs.low == rhs.low && lhs.mid == rhs.mid && lhs.high == rhs.high;
    }
    friend constexpr bool operator!=(const _uint192& lhs, const _uint192& rhs) { return !(lhs == rhs); }

    static constexpr _uint192 mul128x64(_uint128 a, uint64_t b) {
        auto prod1 = _uint128::mul64x64(b, a.low64());
        auto prod2 = _uint128::mul64x64(b, a.high64());
        _uint192 result;
        result.low = prod1.low64();
        result.mid = prod1.high64() + prod2.low64();
        result.high = prod2.high64() + (result.mid < prod1.high64());
        return result;
    }
    static constexpr _uint192 mul64x64x64(uint64_t a, uint64_t b, uint64_t c) {
        return mul128x64(_uint128::mul64x64(a, b), c);
    }

    static _uint192 mul128x64_fast(_uint128 a, uint64_t b) {
        _uint192 res;
        mul64x64to128(a.low64(), b, res.low, res.mid);
        uint64_t tmp;
        mul64x64to128(a.high64(), b, tmp, res.high);
        res.mid += tmp;
        res.high += (res.mid < tmp) ? 1 : 0;
        return res;
    }
    static _uint192 mul64x64x64_fast(uint64_t a, uint64_t b, uint64_t c) {
        // 默认实现（保持原逻辑）
        uint64_t p1, p2, p3;
        _uint192 res;
        mul64x64to128(a, b, p1, p2);
        mul64x64to128(p1, c, res.low, res.mid);
        mul64x64to128(p2, c, p1, p3);
        res.mid += p1;
        res.high = p3 + (res.mid < p1);
        return res;
    }
    // 编译期计算，返回余数
    constexpr uint64_t selfDivRem(uint64_t divisor) {
        uint64_t divid1 = high % divisor, divid0 = mid;
        high /= divisor;
        mid = div128by64to64_base(divid1, divid0, divisor);
        divid1 = divid0, divid0 = low;
        low = div128by64to64_base(divid1, divid0, divisor);
        return divid0;
    }
    // 运行期计算（同时更快，返回余数）
    uint64_t self_div_rem(uint64_t divisor) {
        uint64_t divid1 = high % divisor, divid0 = mid;
        high /= divisor;
        mid = div128by64to64(divid1, divid0, divisor);
        divid1 = divid0, divid0 = low;
        low = div128by64to64(divid1, divid0, divisor);
        return divid0;
    }
    constexpr _uint192 rShift64() const { return _uint192(mid, high, 0); }
    constexpr _uint192 lShift64() const { return _uint192(0, low, high); }
    constexpr void set_low(uint64_t low) { this->low = low; }
    constexpr void set_mid(uint64_t mid) { this->mid = mid; }
    constexpr void set_high(uint64_t high) { this->high = high; }

    constexpr operator uint64_t() const { return low; }

    std::string toStringBase10() const {
        if (high == 0) {
            return _uint128(mid, low).toStringBase10();
        }
        constexpr uint64_t BASE(10000'0000'0000'0000);
        _uint192 copy(*this);
        std::string s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        return std::to_string(uint64_t(copy.selfDivRem(BASE))) + s;
    }
    constexpr uint64_t low64() const { return low; }
    constexpr uint64_t mid64() const { return mid; }
    constexpr uint64_t high64() const { return high; }

    void printDec() const { std::cout << std::dec << toStringBase10() << '\n'; }
    void printHex() const { std::cout << std::hex << "0x" << high << ' ' << mid << ' ' << low << std::dec << '\n'; }
};
};