
#ifndef __LAMMP_INTEGER_HPP__
#define __LAMMP_INTEGER_HPP__

#include <cstdint>
#include <string>
#include <iostream>
#include "lammp.hpp"

namespace lammp {

/*
 *=======================================================================
 * 128位无符号整数类（并非完整功能，请勿轻易代替__uint128_t）
 *=======================================================================
 */
class _uint128 {
   private:
    uint64_t low, high;

   public:
    constexpr _uint128(uint64_t l = 0, uint64_t h = 0);
    constexpr _uint128(std::pair<uint64_t, uint64_t> p);

    constexpr _uint128 operator+(_uint128 rhs) const;
    constexpr _uint128 operator-(_uint128 rhs) const;
    constexpr _uint128 operator+(uint64_t rhs) const;
    constexpr _uint128 operator-(uint64_t rhs) const;
    _uint128 operator*(_uint128 rhs) const;
    _uint128 operator*(uint64_t rhs) const;
    constexpr _uint128 operator/(_uint128 rhs) const;
    constexpr _uint128 operator%(const _uint128& rhs) const;
    constexpr _uint128 operator/(uint64_t rhs) const;
    constexpr _uint128 operator%(uint64_t rhs) const;
    constexpr _uint128& operator+=(const _uint128& rhs);
    constexpr _uint128& operator-=(const _uint128& rhs);
    constexpr _uint128& operator+=(uint64_t rhs);
    constexpr _uint128& operator-=(uint64_t rhs);
    constexpr _uint128& operator*=(const _uint128& rhs);
    constexpr _uint128& operator/=(const _uint128& rhs);
    constexpr _uint128& operator%=(const _uint128& rhs);
    constexpr uint64_t selfDivRem(uint64_t divisor);
    uint64_t self_div_rem(uint64_t divisor);
    uint32_t self_div_rem(uint32_t divisor);
    static constexpr _uint128 mul64x64(uint64_t a, uint64_t b);
    static _uint128 mul64x64_fast(uint64_t a, uint64_t b);
    constexpr bool operator<(const _uint128& rhs) const;
    constexpr bool operator==(const _uint128& rhs) const;
    constexpr _uint128 operator<<(int shift) const;
    constexpr _uint128 operator>>(int shift) const;
    constexpr _uint128& operator<<=(int shift);
    constexpr _uint128& operator>>=(int shift);
    constexpr uint64_t high64() const;
    constexpr uint64_t low64() const;
    constexpr operator uint64_t() const;
    std::string toStringBase10() const;
    void printDec() const;
    void printHex() const;
};

/*
 *=======================================================================
 * 192位无符号整数类
 *=======================================================================
 */
class _uint192 {
   private:
    uint64_t low, mid, high;

   public:
    constexpr _uint192();
    constexpr _uint192(uint64_t low, uint64_t mi = 0, uint64_t high = 0);
    constexpr _uint192(_uint128 n);
    constexpr _uint192 operator+(_uint192 rhs) const;
    constexpr _uint192 operator-(_uint192 rhs) const;
    constexpr _uint192 operator/(uint64_t rhs) const;
    constexpr _uint192 operator%(uint64_t rhs) const;
    constexpr _uint192& operator+=(const _uint192& rhs);
    constexpr _uint192& operator-=(const _uint192& rhs);
    constexpr _uint192& operator/=(const _uint192& rhs);
    constexpr _uint192& operator%=(const _uint192& rhs);
    constexpr _uint192 operator<<(int shift) const;
    friend constexpr bool operator<(const _uint192& lhs, const _uint192& rhs);
    friend constexpr bool operator<=(const _uint192& lhs, const _uint192& rhs);
    friend constexpr bool operator>(const _uint192& lhs, const _uint128& rhs);
    friend constexpr bool operator>=(const _uint192& lhs, const _uint192& rhs);
    friend constexpr bool operator==(const _uint192& lhs, const _uint192& rhs);
    friend constexpr bool operator!=(const _uint192& lhs, const _uint192& rhs);
    static constexpr _uint192 mul128x64(_uint128 a, uint64_t b);
    static constexpr _uint192 mul64x64x64(uint64_t a, uint64_t b, uint64_t c);
    static _uint192 mul128x64_fast(_uint128 a, uint64_t b);
    static _uint192 mul64x64x64_fast(uint64_t a, uint64_t b, uint64_t c);
    constexpr uint64_t selfDivRem(uint64_t divisor);
    uint64_t self_div_rem(uint64_t divisor);
    constexpr _uint192 rShift64() const;
    constexpr _uint192 lShift64() const;
    constexpr void set_low(uint64_t low);
    constexpr void set_mid(uint64_t mid);
    constexpr void set_high(uint64_t high);
    constexpr operator uint64_t() const;
    std::string toStringBase10() const;
    constexpr uint64_t low64() const;
    constexpr uint64_t mid64() const;
    constexpr uint64_t high64() const;
    void printDec() const;
    void printHex() const;
};

template <uint64_t MOD, typename Int128Type = _uint128>
class MontInt64Lazy {
   private:
    static_assert(MOD > UINT32_MAX, "Montgomery64 modulus must be greater than 2^32");
    static_assert(lammp_log2(MOD) < 62, "MOD can't be larger than 62 bits");
    uint64_t data;

   public:
    using IntType = uint64_t;

    constexpr MontInt64Lazy();
    constexpr MontInt64Lazy(uint64_t n);

    constexpr MontInt64Lazy operator+(MontInt64Lazy rhs) const;
    constexpr MontInt64Lazy operator-(MontInt64Lazy rhs) const;
    MontInt64Lazy operator*(MontInt64Lazy rhs) const;
    constexpr MontInt64Lazy& operator+=(const MontInt64Lazy& rhs);
    constexpr MontInt64Lazy& operator-=(const MontInt64Lazy& rhs);
    constexpr MontInt64Lazy& operator*=(const MontInt64Lazy& rhs);
    constexpr MontInt64Lazy largeNorm2() const;
    constexpr MontInt64Lazy rawAdd(MontInt64Lazy rhs) const;
    constexpr MontInt64Lazy rawSub(MontInt64Lazy rhs) const;
    constexpr operator uint64_t() const;

    static constexpr uint64_t mod();
    static constexpr uint64_t mod2();
    static constexpr uint64_t modInv();
    static constexpr uint64_t modInvNeg();
    static constexpr uint64_t rSquare();
    static_assert((mod() * modInv()) == 1, "mod_inv not correct");

    static constexpr uint64_t toMont(uint64_t n);
    static constexpr uint64_t toInt(uint64_t n);
    static uint64_t redcFastLazy(const Int128Type& input);
    static uint64_t redcFast(const Int128Type& input);
    static constexpr uint64_t redc(const Int128Type& input);
    static uint64_t mulMontRunTime(uint64_t a, uint64_t b);
    static uint64_t mulMontRunTimeLazy(uint64_t a, uint64_t b);
    static constexpr uint64_t mulMontCompileTime(uint64_t a, uint64_t b);
};

};  // namespace lammp

#include "../../src/lammp/integer/uint128.inl"
#include "../../src/lammp/integer/uint192.inl"
#include "../../src/lammp/integer/montint.inl"

#endif  // __LAMMP_INTEGER_HPP__