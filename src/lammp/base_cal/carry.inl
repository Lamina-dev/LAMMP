/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is a part of the LAMMP package.
 * you can see more details about LAMMP at:
 * <https://github.com/Lamina-dev/LAMMP>
 */

#ifndef __LAMMP_BITS_CARRY_INL__
#define __LAMMP_BITS_CARRY_INL__

namespace lammp{

// Fast power
template <typename T, typename T1>
constexpr T qpow(T m, T1 n) {
    T result = 1;
    while (n > 0) {
        if ((n & 1) != 0) {
            result *= m;
        }
        m *= m;
        n >>= 1;
    }
    return result;
}

// Fast power with mod
template <typename T, typename T1>
constexpr T qpow(T m, T1 n, T mod) {
    T result = 1;
    while (n > 0) {
        if ((n & 1) != 0) {
            result *= m;
            result %= mod;
        }
        m *= m;
        m %= mod;
        n >>= 1;
    }
    return result;
}

// Get cloest power of 2 that not larger than n
template <typename T>
constexpr T int_floor2(T n) {
    constexpr int bits = sizeof(n) * CHAR_BIT;
    for (int i = 1; i < bits; i *= 2) {
        n |= (n >> i);
    }
    return (n >> 1) + 1;
}

// Get cloest power of 2 that not smaller than n
template <typename T>
constexpr T int_ceil2(T n) {
    constexpr int bits = sizeof(n) * CHAR_BIT;
    n--;
    for (int i = 1; i < bits; i *= 2) {
        n |= (n >> i);
    }
    return n + 1;
}

// x + y = sum with carry
template <typename UintTy>
constexpr UintTy add_half(UintTy x, UintTy y, bool& cf) {
    x = x + y;
    cf = (x < y);
    return x;
}

// x - y = diff with borrow
template <typename UintTy>
constexpr UintTy sub_half(UintTy x, UintTy y, bool& bf) {
    y = x - y;
    bf = (y > x);
    return y;
}

// x + y + cf = sum with carry
template <typename UintTy>
constexpr UintTy add_carry(UintTy x, UintTy y, bool& cf) {
    UintTy sum = x + cf;
    cf = (sum < x);
    sum += y;              // carry
    cf = cf || (sum < y);  // carry
    return sum;
}

// carry = (x + y) / base_num, return (x + y) mod base_num
template <typename UintTy>
constexpr UintTy add_half_base(UintTy x, UintTy y, bool& carry, const UintTy& base_num) {
    assert(x < base_num && y < base_num);
    // assert(base_num < (all_one<UintTy>(sizeof(UintTy) * CHAR_BIT - 1) + 1));
    UintTy sum = x + y;
    carry = (sum >= base_num) || (sum < x);
    return (carry) ? (sum - base_num) : (sum % base_num);
}

// carry = (x + y + carry) / base_num, return (x + y + carry) mod base_num
template <typename UintTy>
constexpr UintTy add_carry_base(UintTy x, UintTy y, bool& carry, const UintTy& base_num) {
    assert(x < base_num && y < base_num);
    // assert(base_num < (all_one<UintTy>(sizeof(UintTy) * CHAR_BIT - 1) + 1));
    UintTy sum = x + carry;
    carry = (sum < x);
    sum += y;                                         // carry
    carry = carry || (sum >= base_num) || (sum < y);  // carry
    return (carry) ? (sum - base_num) : (sum % base_num);
}

// x - y - bf = diff with borrow
template <typename UintTy>
constexpr UintTy sub_borrow(UintTy x, UintTy y, bool& bf) {
    UintTy diff = x - bf;
    bf = (diff > x);
    y = diff - y;           // borrow
    bf = bf || (y > diff);  // borrow
    return y;
}

// a * x + b * y = gcd(a,b)
template <typename IntTy>
constexpr IntTy exgcd(IntTy a, IntTy b, IntTy& x, IntTy& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    IntTy k = a / b;
    IntTy g = exgcd(b, a - k * b, y, x);
    y -= k * x;
    return g;
}

// return n^-1 mod mod
template <typename IntTy>
constexpr IntTy mod_inv(IntTy n, IntTy mod) {
    n %= mod;
    IntTy x = 0, y = 0;
    exgcd(n, mod, x, y);
    if (x < 0) {
        x += mod;
    } else if (x >= mod) {
        x -= mod;
    }
    return x;
}

// return n^-1 mod 2^pow, Newton iteration
constexpr uint64_t inv_mod2pow(uint64_t n, int pow) {
    const uint64_t mask = all_one<uint64_t>(pow);
    uint64_t xn = 1, t = n & mask;
    while (t != 1) {
        xn = (xn * (2 - t));
        t = (xn * n) & mask;
    }
    return xn & mask;
}
};  // namespace lammp

#endif // __LAMMP_BITS_CARRY_INL__