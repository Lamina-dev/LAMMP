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

#ifndef __LAMMP_NUM_THEO_HPP__
#define __LAMMP_NUM_THEO_HPP__

#include <array>
#include "uint192.hpp"
#include "spe_int.hpp"

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

template <typename IntType>
constexpr bool check_inv(uint64_t n, uint64_t n_inv, uint64_t mod) {
    n %= mod;
    n_inv %= mod;
    IntType m(n);
    m *= IntType(n_inv);
    m %= IntType(mod);
    return m == IntType(1);
}

// 3 modulars Chinese Remainder Theorem (CRT) with 192 bit result.
template <typename ModInt1, typename ModInt2, typename ModInt3>
inline _uint192 crt3(ModInt1 n1, ModInt2 n2, ModInt3 n3) {
    constexpr uint64_t MOD1 = ModInt1::mod(), MOD2 = ModInt2::mod(), MOD3 = ModInt3::mod();
    constexpr _uint192 MOD123 = _uint192::mul64x64x64(MOD1, MOD2, MOD3);  // MOD1*MOD2*MOD3
    constexpr _uint128 MOD12 = _uint128::mul64x64(MOD1, MOD2);            // MOD1*MOD2
    constexpr _uint128 MOD23 = _uint128::mul64x64(MOD2, MOD3);            // MOD2*MOD3
    constexpr _uint128 MOD13 = _uint128::mul64x64(MOD1, MOD3);            // MOD1*MOD3
    constexpr uint64_t MOD23_M1 =
        _uint128::mul64x64(MOD2 % MOD1, MOD3 % MOD1) % _uint128(MOD1);  // (MOD2*MOD3)  mod MOD1
    constexpr uint64_t MOD13_M2 =
        _uint128::mul64x64(MOD1 % MOD2, MOD3 % MOD2) % _uint128(MOD2);  // (MOD1*MOD3)  mod MOD2
    constexpr uint64_t MOD12_M3 =
        _uint128::mul64x64(MOD1 % MOD3, MOD2 % MOD3) % _uint128(MOD3);  // (MOD1*MOD2)  mod MOD3
    constexpr ModInt1 MOD23_INV1 = mod_inv<int64_t>(MOD23_M1, MOD1);    // (MOD2*MOD3)^-1 mod MOD1
    constexpr ModInt2 MOD13_INV2 = mod_inv<int64_t>(MOD13_M2, MOD2);    // (MOD1*MOD3)^-1 mod MOD2
    constexpr ModInt3 MOD12_INV3 = mod_inv<int64_t>(MOD12_M3, MOD3);    // (MOD1*MOD2)^-1 mod MOD3
    static_assert(check_inv<_uint128>(MOD23_INV1, MOD23_M1, MOD1), "INV1 error");
    static_assert(check_inv<_uint128>(MOD13_INV2, MOD13_M2, MOD2), "INV2 error");
    static_assert(check_inv<_uint128>(MOD12_INV3, MOD12_M3, MOD3), "INV3 error");
    n1 = n1 * MOD23_INV1;
    n2 = n2 * MOD13_INV2;
    n3 = n3 * MOD12_INV3;
    _uint192 result = _uint192::mul128x64_fast(MOD23, uint64_t(n1));
    result += _uint192::mul128x64_fast(MOD13, uint64_t(n2));
    result += _uint192::mul128x64_fast(MOD12, uint64_t(n3));
    result = result < MOD123 ? result : result - MOD123;
    return result < MOD123 ? result : result - MOD123;
}

/*
 * =====================================================================
 *  Split-Radix NTT  模板
 * =====================================================================
 */
namespace SplitRadix {

template <typename T>
inline void transform2(T& sum, T& diff) {
    T temp0 = sum, temp1 = diff;
    sum = temp0 + temp1;
    diff = temp0 - temp1;
}

template <uint64_t ROOT, typename ModIntType>
inline ModIntType mul_w41(ModIntType n) {
    constexpr ModIntType W_4_1 = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / 4);
    return n * W_4_1;
}

// in: in_out0<4p, in_ou1<4p; in_out2<2p, in_ou3<2p
// out: in_out0<4p, in_ou1<4p; in_out2<4p, in_ou3<4p
template <uint64_t ROOT, typename ModIntType>
inline void dit_butterfly244(ModIntType& in_out0, ModIntType& in_out1, ModIntType& in_out2, ModIntType& in_out3) {
    ModIntType temp0, temp1, temp2, temp3;
    temp0 = in_out0.largeNorm2();
    temp1 = in_out1.largeNorm2();
    temp2 = in_out2 + in_out3;
    temp3 = in_out2.rawSub(in_out3);
    temp3 = mul_w41<ROOT>(temp3);
    in_out0 = temp0.rawAdd(temp2);
    in_out2 = temp0.rawSub(temp2);
    in_out1 = temp1.rawAdd(temp3);
    in_out3 = temp1.rawSub(temp3);
}

// in: in_out0<2p, in_ou1<2p; in_out2<2p, in_ou3<2p
// out: in_out0<2p, in_ou1<2p; in_out2<4p, in_ou3<4p
template <uint64_t ROOT, typename ModIntType>
inline void dif_butterfly244(ModIntType& in_out0, ModIntType& in_out1, ModIntType& in_out2, ModIntType& in_out3) {
    ModIntType temp0, temp1, temp2, temp3;
    temp0 = in_out0.rawAdd(in_out2);
    temp2 = in_out0 - in_out2;
    temp1 = in_out1.rawAdd(in_out3);
    temp3 = in_out1.rawSub(in_out3);
    temp3 = mul_w41<ROOT>(temp3);
    in_out0 = temp0.largeNorm2();
    in_out1 = temp1.largeNorm2();
    in_out2 = temp2.rawAdd(temp3);
    in_out3 = temp2.rawSub(temp3);
}

// in: in_out0<4p, in_ou1<4p
// out: in_out0<4p, in_ou1<4p
template <typename ModIntType>
inline void dit_butterfly2(ModIntType& in_out0, ModIntType& in_out1, const ModIntType& omega) {
    auto x = in_out0.largeNorm2();
    auto y = in_out1 * omega;
    in_out0 = x.rawAdd(y);
    in_out1 = x.rawSub(y);
}

// in: in_out0<2p, in_ou1<2p
// out: in_out0<2p, in_ou1<2p
template <typename ModIntType>
inline void dif_butterfly2(ModIntType& in_out0, ModIntType& in_out1, const ModIntType& omega) {
    auto x = in_out0 + in_out1;
    auto y = in_out0.rawSub(in_out1);
    in_out0 = x;
    in_out1 = y * omega;
}

template <size_t MAX_LEN, uint64_t ROOT, typename ModIntType>
struct NTTShort {
    static constexpr size_t NTT_LEN = MAX_LEN;
    static constexpr int LOG_LEN = lammp_log2(NTT_LEN);
    struct TableType {
        std::array<ModIntType, NTT_LEN> omega_table;
        // Compute in compile time if need.
        /*constexpr*/ TableType() {
            for (int omega_log_len = 0; omega_log_len <= LOG_LEN; omega_log_len++) {
                size_t omega_len = size_t(1) << omega_log_len, omega_count = omega_len / 2;
                auto it = &omega_table[omega_len / 2];
                ModIntType root = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / omega_len);
                ModIntType omega(1);
                for (size_t i = 0; i < omega_count; i++) {
                    it[i] = omega;
                    omega *= root;
                }
            }
        }
        constexpr ModIntType& operator[](size_t i) { return omega_table[i]; }
        constexpr const ModIntType& operator[](size_t i) const { return omega_table[i]; }
        constexpr const ModIntType* getOmegaIt(size_t len) const { return &omega_table[len / 2]; }
    };

    static TableType table;

    static void dit(ModIntType in_out[], size_t len) {
        len = std::min(NTT_LEN, len);
        size_t rank = len;
        if (lammp_log2(len) % 2 == 0) {
            NTTShort<4, ROOT, ModIntType>::dit(in_out, len);
            for (size_t i = 4; i < len; i += 4) {
                NTTShort<4, ROOT, ModIntType>::dit(in_out + i);
            }
            rank = 16;
        } else {
            NTTShort<8, ROOT, ModIntType>::dit(in_out, len);
            for (size_t i = 8; i < len; i += 8) {
                NTTShort<8, ROOT, ModIntType>::dit(in_out + i);
            }
            rank = 32;
        }
        for (; rank <= len; rank *= 4) {
            size_t gap = rank / 4;
            auto omega_it = table.getOmegaIt(rank), last_omega_it = table.getOmegaIt(rank / 2);
            auto it0 = in_out, it1 = in_out + gap, it2 = in_out + gap * 2, it3 = in_out + gap * 3;
            for (size_t j = 0; j < len; j += rank) {
                for (size_t i = 0; i < gap; i++) {
                    auto temp0 = it0[j + i], temp1 = it1[j + i], temp2 = it2[j + i], temp3 = it3[j + i],
                         omega = last_omega_it[i];
                    dit_butterfly2(temp0, temp1, omega);
                    dit_butterfly2(temp2, temp3, omega);
                    dit_butterfly2(temp0, temp2, omega_it[i]);
                    dit_butterfly2(temp1, temp3, omega_it[gap + i]);
                    it0[j + i] = temp0, it1[j + i] = temp1, it2[j + i] = temp2, it3[j + i] = temp3;
                }
            }
        }
    }
    static void dif(ModIntType in_out[], size_t len) {
        len = std::min(NTT_LEN, len);
        size_t rank = len;
        for (; rank >= 16; rank /= 4) {
            size_t gap = rank / 4;
            auto omega_it = table.getOmegaIt(rank), last_omega_it = table.getOmegaIt(rank / 2);
            auto it0 = in_out, it1 = in_out + gap, it2 = in_out + gap * 2, it3 = in_out + gap * 3;
            for (size_t j = 0; j < len; j += rank) {
                for (size_t i = 0; i < gap; i++) {
                    auto temp0 = it0[j + i], temp1 = it1[j + i], temp2 = it2[j + i], temp3 = it3[j + i],
                         omega = last_omega_it[i];
                    dif_butterfly2(temp0, temp2, omega_it[i]);
                    dif_butterfly2(temp1, temp3, omega_it[gap + i]);
                    dif_butterfly2(temp0, temp1, omega);
                    dif_butterfly2(temp2, temp3, omega);
                    it0[j + i] = temp0, it1[j + i] = temp1, it2[j + i] = temp2, it3[j + i] = temp3;
                }
            }
        }
        if (lammp_log2(rank) % 2 == 0) {
            NTTShort<4, ROOT, ModIntType>::dif(in_out, len);
            for (size_t i = 4; i < len; i += 4) {
                NTTShort<4, ROOT, ModIntType>::dif(in_out + i);
            }
        } else {
            NTTShort<8, ROOT, ModIntType>::dif(in_out, len);
            for (size_t i = 8; i < len; i += 8) {
                NTTShort<8, ROOT, ModIntType>::dif(in_out + i);
            }
        }
    }
};
template <size_t LEN, uint64_t ROOT, typename ModIntType>
typename NTTShort<LEN, ROOT, ModIntType>::TableType NTTShort<LEN, ROOT, ModIntType>::table;
template <size_t LEN, uint64_t ROOT, typename ModIntType>
constexpr size_t NTTShort<LEN, ROOT, ModIntType>::NTT_LEN;
template <size_t LEN, uint64_t ROOT, typename ModIntType>
constexpr int NTTShort<LEN, ROOT, ModIntType>::LOG_LEN;

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<0, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {}
    static void dif(ModIntType in_out[]) {}
    static void dit(ModIntType in_out[], size_t len) {}
    static void dif(ModIntType in_out[], size_t len) {}
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<1, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {}
    static void dif(ModIntType in_out[]) {}
    static void dit(ModIntType in_out[], size_t len) {}
    static void dif(ModIntType in_out[], size_t len) {}
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<2, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) { transform2(in_out[0], in_out[1]); }
    static void dif(ModIntType in_out[]) { transform2(in_out[0], in_out[1]); }
    static void dit(ModIntType in_out[], size_t len) {
        if (len < 2) {
            return;
        }
        dit(in_out);
    }
    static void dif(ModIntType in_out[], size_t len) {
        if (len < 2) {
            return;
        }
        dif(in_out);
    }
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<4, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];

        transform2(temp0, temp1);
        transform2(temp2, temp3);
        temp3 = mul_w41<ROOT>(temp3);

        in_out[0] = temp0 + temp2;
        in_out[1] = temp1 + temp3;
        in_out[2] = temp0 - temp2;
        in_out[3] = temp1 - temp3;
    }
    static void dif(ModIntType in_out[]) {
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];

        transform2(temp0, temp2);
        transform2(temp1, temp3);
        temp3 = mul_w41<ROOT>(temp3);

        in_out[0] = temp0 + temp1;
        in_out[1] = temp0 - temp1;
        in_out[2] = temp2 + temp3;
        in_out[3] = temp2 - temp3;
    }
    static void dit(ModIntType in_out[], size_t len) {
        if (len < 4) {
            NTTShort<2, ROOT, ModIntType>::dit(in_out, len);
            return;
        }
        dit(in_out);
    }
    static void dif(ModIntType in_out[], size_t len) {
        if (len < 4) {
            NTTShort<2, ROOT, ModIntType>::dif(in_out, len);
            return;
        }
        dif(in_out);
    }
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<8, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {
        static constexpr ModIntType w1 = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / 8);
        static constexpr ModIntType w2 = qpow(w1, 2);
        static constexpr ModIntType w3 = qpow(w1, 3);
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];
        auto temp4 = in_out[4];
        auto temp5 = in_out[5];
        auto temp6 = in_out[6];
        auto temp7 = in_out[7];

        transform2(temp0, temp1);
        transform2(temp2, temp3);
        transform2(temp4, temp5);
        transform2(temp6, temp7);
        temp3 = mul_w41<ROOT>(temp3);
        temp7 = mul_w41<ROOT>(temp7);

        transform2(temp0, temp2);
        transform2(temp1, temp3);
        transform2(temp4, temp6);
        transform2(temp5, temp7);
        temp5 = temp5 * w1;
        temp6 = temp6 * w2;
        temp7 = temp7 * w3;

        in_out[0] = temp0 + temp4;
        in_out[1] = temp1 + temp5;
        in_out[2] = temp2 + temp6;
        in_out[3] = temp3 + temp7;
        in_out[4] = temp0 - temp4;
        in_out[5] = temp1 - temp5;
        in_out[6] = temp2 - temp6;
        in_out[7] = temp3 - temp7;
    }
    static void dif(ModIntType in_out[]) {
        static constexpr ModIntType w1 = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / 8);
        static constexpr ModIntType w2 = qpow(w1, 2);
        static constexpr ModIntType w3 = qpow(w1, 3);
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];
        auto temp4 = in_out[4];
        auto temp5 = in_out[5];
        auto temp6 = in_out[6];
        auto temp7 = in_out[7];

        transform2(temp0, temp4);
        transform2(temp1, temp5);
        transform2(temp2, temp6);
        transform2(temp3, temp7);
        temp5 = temp5 * w1;
        temp6 = temp6 * w2;
        temp7 = temp7 * w3;

        transform2(temp0, temp2);
        transform2(temp1, temp3);
        transform2(temp4, temp6);
        transform2(temp5, temp7);
        temp3 = mul_w41<ROOT>(temp3);
        temp7 = mul_w41<ROOT>(temp7);

        in_out[0] = temp0 + temp1;
        in_out[1] = temp0 - temp1;
        in_out[2] = temp2 + temp3;
        in_out[3] = temp2 - temp3;
        in_out[4] = temp4 + temp5;
        in_out[5] = temp4 - temp5;
        in_out[6] = temp6 + temp7;
        in_out[7] = temp6 - temp7;
    }
    static void dit(ModIntType in_out[], size_t len) {
        if (len < 8) {
            NTTShort<4, ROOT, ModIntType>::dit(in_out, len);
            return;
        }
        dit(in_out);
    }
    static void dif(ModIntType in_out[], size_t len) {
        if (len < 8) {
            NTTShort<4, ROOT, ModIntType>::dif(in_out, len);
            return;
        }
        dif(in_out);
    }
};

#ifdef UINT128T
using uint128_default = __uint128_t;
#else
using uint128_default = _uint128;
#endif  // UINT128T

template <uint64_t MOD, uint64_t ROOT, typename Int128Type = uint128_default>
struct NTT {
    static constexpr uint64_t mod() { return MOD; }
    static constexpr uint64_t root() { return ROOT; }
    static constexpr uint64_t rootInv() {
        constexpr uint64_t IROOT = mod_inv<int64_t>(ROOT, MOD);
        return IROOT;
    }

    static_assert(root() < mod(), "ROOT must be smaller than MOD");
    static_assert(check_inv<Int128Type>(root(), rootInv(), mod()), "IROOT * ROOT % MOD must be 1");
    static constexpr int MOD_BITS = lammp_log2(mod()) + 1;
    static constexpr int MAX_LOG_LEN = lammp_ctz(mod() - 1);

    static constexpr size_t getMaxLen() {
        if constexpr (MAX_LOG_LEN < sizeof(size_t) * CHAR_BIT) {
            return size_t(1) << MAX_LOG_LEN;
        }
        return size_t(1) << (sizeof(size_t) * CHAR_BIT - 1);
    }
    static constexpr size_t NTT_MAX_LEN = getMaxLen();

    using INTT = NTT<mod(), rootInv(), Int128Type>;
    using ModInt64Type = MontInt64Lazy<MOD, Int128Type>;
    using ModIntType = ModInt64Type;
    using IntType = typename ModIntType::IntType;

    static constexpr size_t L2_BYTE = size_t(1) << 20;  // 1MB L2 cache size, change this
                                                        // if you know your cache size.
    static constexpr size_t LONG_THRESHOLD = std::min(L2_BYTE / sizeof(ModIntType), NTT_MAX_LEN);
    using NTTTemplate = NTTShort<LONG_THRESHOLD, root(), ModIntType>;

    static void dit244(ModIntType in_out[], size_t ntt_len) {
        ntt_len = std::min(int_floor2(ntt_len), NTT_MAX_LEN);
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dit(in_out, ntt_len);
            return;
        }
        size_t quarter_len = ntt_len / 4;
        dit244(in_out + quarter_len * 3, ntt_len / 4);
        dit244(in_out + quarter_len * 2, ntt_len / 4);
        dit244(in_out, ntt_len / 2);
        const ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        const ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        auto it0 = in_out, it1 = in_out + quarter_len, it2 = in_out + quarter_len * 2, it3 = in_out + quarter_len * 3;
        for (size_t i = 0; i < quarter_len; i++) {
            ModIntType temp0 = it0[i], temp1 = it1[i], temp2 = it2[i] * omega1, temp3 = it3[i] * omega3;
            dit_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
            it0[i] = temp0, it1[i] = temp1, it2[i] = temp2, it3[i] = temp3;
            omega1 = omega1 * unit_omega1;
            omega3 = omega3 * unit_omega3;
        }
    }
    static void dif244(ModIntType in_out[], size_t ntt_len) {
        ntt_len = std::min(int_floor2(ntt_len), NTT_MAX_LEN);
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dif(in_out, ntt_len);
            return;
        }
        size_t quarter_len = ntt_len / 4;
        const ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        const ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        auto it0 = in_out, it1 = in_out + quarter_len, it2 = in_out + quarter_len * 2, it3 = in_out + quarter_len * 3;
        for (size_t i = 0; i < quarter_len; i++) {
            ModIntType temp0 = it0[i], temp1 = it1[i], temp2 = it2[i], temp3 = it3[i];
            dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
            it0[i] = temp0, it1[i] = temp1, it2[i] = temp2 * omega1, it3[i] = temp3 * omega3;
            omega1 = omega1 * unit_omega1;
            omega3 = omega3 * unit_omega3;
        }
        dif244(in_out, ntt_len / 2);
        dif244(in_out + quarter_len * 3, ntt_len / 4);
        dif244(in_out + quarter_len * 2, ntt_len / 4);
    }
    static void convolution(ModIntType in1[],
                            ModIntType in2[],
                            ModIntType out[],
                            size_t ntt_len,
                            bool normlize = true) {
        dif244(in1, ntt_len);
        dif244(in2, ntt_len);
        if (normlize) {
            const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
            for (size_t i = 0; i < ntt_len; i++) {
                out[i] = in1[i] * in2[i] * inv_len;
            }
        } else {
            for (size_t i = 0; i < ntt_len; i++) {
                out[i] = in1[i] * in2[i];
            }
        }
        INTT::dit244(out, ntt_len);
    }
    static void convolutionRecursion(ModIntType in1[],
                                     ModIntType in2[],
                                     ModIntType out[],
                                     size_t ntt_len,
                                     bool normlize = true) {
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dif(in1, ntt_len);
            if (in1 != in2) {
                NTTTemplate::dif(in2, ntt_len);
            }
            if (normlize) {
                const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i] * inv_len;
                }
            } else {
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i];
                }
            }
            INTT::NTTTemplate::dit(out, ntt_len);
            return;
        }
        const size_t quarter_len = ntt_len / 4;
        ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        if (in1 != in2) {
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = in1[i], temp1 = in1[quarter_len + i], temp2 = in1[quarter_len * 2 + i],
                           temp3 = in1[quarter_len * 3 + i];
                dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
                in1[i] = temp0, in1[quarter_len + i] = temp1, in1[quarter_len * 2 + i] = temp2 * omega1,
                in1[quarter_len * 3 + i] = temp3 * omega3;

                temp0 = in2[i], temp1 = in2[quarter_len + i], temp2 = in2[quarter_len * 2 + i],
                temp3 = in2[quarter_len * 3 + i];
                dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
                in2[i] = temp0, in2[quarter_len + i] = temp1, in2[quarter_len * 2 + i] = temp2 * omega1,
                in2[quarter_len * 3 + i] = temp3 * omega3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        } else {
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = in1[i], temp1 = in1[quarter_len + i], temp2 = in1[quarter_len * 2 + i],
                           temp3 = in1[quarter_len * 3 + i];
                dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
                in1[i] = temp0, in1[quarter_len + i] = temp1, in1[quarter_len * 2 + i] = temp2 * omega1,
                in1[quarter_len * 3 + i] = temp3 * omega3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        }

        convolutionRecursion(in1, in2, out, ntt_len / 2, false);
        convolutionRecursion(in1 + quarter_len * 2, in2 + quarter_len * 2, out + quarter_len * 2, ntt_len / 4, false);
        convolutionRecursion(in1 + quarter_len * 3, in2 + quarter_len * 3, out + quarter_len * 3, ntt_len / 4, false);

        unit_omega1 = qpow(ModIntType(rootInv()), (mod() - 1) / ntt_len);
        unit_omega3 = qpow(unit_omega1, 3);
        if (normlize) {
            const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
            omega1 = inv_len, omega3 = inv_len;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i] * inv_len, temp1 = out[quarter_len + i] * inv_len,
                           temp2 = out[quarter_len * 2 + i] * omega1, temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        } else {
            omega1 = 1, omega3 = 1;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i], temp1 = out[quarter_len + i], temp2 = out[quarter_len * 2 + i] * omega1,
                           temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        }
    }
    // in1 has been transformed
    static void convolutionRecursion_rep(const ModIntType in1[],
                                         ModIntType in2[],
                                         ModIntType out[],
                                         size_t ntt_len,
                                         bool normlize = true) {
        assert(in1 != in2);
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dif(in2, ntt_len);
            if (normlize) {
                const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i] * inv_len;
                }
            } else {
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i];
                }
            }
            INTT::NTTTemplate::dit(out, ntt_len);
            return;
        }
        const size_t quarter_len = ntt_len / 4;
        ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        for (size_t i = 0; i < quarter_len; i++) {
            ModIntType temp0 = in2[i], temp1 = in2[quarter_len + i], temp2 = in2[quarter_len * 2 + i],
                       temp3 = in2[quarter_len * 3 + i];
            dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
            in2[i] = temp0, in2[quarter_len + i] = temp1, in2[quarter_len * 2 + i] = temp2 * omega1,
            in2[quarter_len * 3 + i] = temp3 * omega3;

            omega1 = omega1 * unit_omega1;
            omega3 = omega3 * unit_omega3;
        }

        convolutionRecursion_rep(in1, in2, out, ntt_len / 2, false);
        convolutionRecursion_rep(in1 + quarter_len * 2, in2 + quarter_len * 2, out + quarter_len * 2, ntt_len / 4,
                                 false);
        convolutionRecursion_rep(in1 + quarter_len * 3, in2 + quarter_len * 3, out + quarter_len * 3, ntt_len / 4,
                                 false);

        unit_omega1 = qpow(ModIntType(rootInv()), (mod() - 1) / ntt_len);
        unit_omega3 = qpow(unit_omega1, 3);
        if (normlize) {
            const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
            omega1 = inv_len, omega3 = inv_len;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i] * inv_len, temp1 = out[quarter_len + i] * inv_len,
                           temp2 = out[quarter_len * 2 + i] * omega1, temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        } else {
            omega1 = 1, omega3 = 1;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i], temp1 = out[quarter_len + i], temp2 = out[quarter_len * 2 + i] * omega1,
                           temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        }
    }
};
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr int NTT<MOD, ROOT, Int128Type>::MOD_BITS;
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr int NTT<MOD, ROOT, Int128Type>::MAX_LOG_LEN;
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr size_t NTT<MOD, ROOT, Int128Type>::NTT_MAX_LEN;
}  // namespace SplitRadix

using NTT0 = SplitRadix::NTT<MOD0, ROOT0>;  // using 64bit integer, Montgomery speed up
using NTT1 = SplitRadix::NTT<MOD1, ROOT1>;  // using 64bit integer, Montgomery speed up
using NTT2 = SplitRadix::NTT<MOD2, ROOT2>;  // using 64bit integer, Montgomery speed up
};  // namespace number_theory
};  // namespace Transform
};  // namespace lammp

#endif  // __LAMMP_NUM_THEO_HPP__