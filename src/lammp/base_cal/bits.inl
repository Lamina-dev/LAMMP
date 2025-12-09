/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is a part of the LAMMP package.
 * you can see more details about LAMMP at:
 * <https://github.com/Lamina-dev/LAMMP>
 */

#ifndef __LAMMP_BITS_BITS_INL__
#define __LAMMP_BITS_BITS_INL__

#include <cstdint>

namespace lammp {
// bits of 1, equals to 2^bits - 1
template <typename T>
constexpr T all_one(int bits) {
    T temp = T(1) << (bits - 1);
    return temp - 1 + temp;
}

// Leading zeros
template <typename IntTy>
constexpr int lammp_clz(IntTy x) {
    constexpr uint32_t MASK32 = uint32_t(0xFFFF) << 16;
    int res = sizeof(IntTy) * CHAR_BIT;
    if (x & MASK32) {
        res -= 16;
        x >>= 16;
    }
    if (x & (MASK32 >> 8)) {
        res -= 8;
        x >>= 8;
    }
    if (x & (MASK32 >> 12)) {
        res -= 4;
        x >>= 4;
    }
    if (x & (MASK32 >> 14)) {
        res -= 2;
        x >>= 2;
    }
    if (x & (MASK32 >> 15)) {
        res -= 1;
        x >>= 1;
    }
    return res - x;
}

// 32位无符号整数的前导零计数（x=0 时结果未定义）
static inline constexpr int lammp_clz(uint32_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    // _BitScanReverse 找到最高位1的位置，前导零 = 31 - 位置
    _BitScanReverse(&idx, x);
    return 31 - (int)idx;
#elif defined(_LAMMP_GCC)
    return __builtin_clz(x);
#endif
}

// 64位无符号整数的前导零计数（x=0 时结果未定义）
static inline constexpr int lammp_clz(uint64_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    _BitScanReverse64(&idx, x);  // 64位版本的位扫描
    return 63 - (int)idx;
#elif defined(_LAMMP_GCC)
    return __builtin_clzll(x);
#endif
}

// 32位无符号整数的尾随零计数（x=0 时结果未定义）
static inline constexpr int lammp_ctz(uint32_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    // _BitScanForward 找到最低位1的位置，即尾随零的数量
    _BitScanForward(&idx, x);
    return (int)idx;
#elif defined(_LAMMP_GCC)
    return __builtin_ctz(x);
#endif
}

// 64位无符号整数的尾随零计数（x=0 时结果未定义）
static inline constexpr int lammp_ctz(uint64_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    _BitScanForward64(&idx, x);  // 64位版本的位扫描
    return (int)idx;
#else
    return __builtin_ctzll(x);
#endif
}

// 32位无符号整数的位计数
static inline constexpr int lammp_cnt(uint32_t x) {
#if defined(_LAMMP_MSVC)
    return _mm_popcnt_u32(x);
#elif defined(_LAMMP_GCC)
    return __builtin_popcount(x);
#endif
}

// 64位无符号整数的位计数
static inline constexpr int lammp_cnt(uint64_t x) {
#if defined(_LAMMP_MSVC)
    return _mm_popcnt_u64(x);
#elif defined(_LAMMP_GCC)
    return __builtin_popcountll(x);
#endif
}

// 32位无符号整数的位长度（bit length）
static inline constexpr int lammp_bit_length(uint32_t x) { return (x == 0) ? 0 : (32 - lammp_clz(x)); }

// 64位无符号整数的位长度（bit length）
static inline constexpr int lammp_bit_length(uint64_t x) { return (x == 0) ? 0 : (64 - lammp_clz(x)); }

// 32位无符号整数的log2（返回最高位1的位置索引）
static inline constexpr int lammp_log2(uint32_t x) { return (x == 0) ? -1 : (31 - lammp_clz(x)); }

// 64位无符号整数的log2（返回最高位1的位置索引）
static inline constexpr int lammp_log2(uint64_t x) { return (x == 0) ? -1 : (63 - lammp_clz(x)); }

};  // namespace lammp
#endif  // __LAMMP_BITS_BITS_INL__