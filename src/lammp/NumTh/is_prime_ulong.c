/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/is_prime_table.h"
#include "../../../include/lammp/impl/mpdef.h"
#include "../../../include/lammp/impl/prime_table.h"
#include "../../../include/lammp/impl/u128_u192.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"

/*
mont63 只能用于小于 2^63 的数，否则会溢出导致计算结果不正确
mont64 可以用于任意大小的数，但由于考虑了溢出的情况，所以速度理论上会慢一些（并未详细测试）
*/
#define MONT63_MAX ((ulong)(0x7fffffffffffffff))

static inline ulong mont64_reduce(u128 t, ulong m, ulong m_inv) {
    ulong k = t[0] * m_inv;
    u192 tmp;
    _u128mul(tmp, k, m);
    tmp[0] += t[0];
    ulong c = tmp[0] < t[0];
    tmp[1] += c;
    c = tmp[1] == 0;
    tmp[1] += t[1];
    c = tmp[1] < t[1];
    tmp[2] = c;
    if (!c) {
        ulong res = tmp[1] >= m ? tmp[1] - m : tmp[1];
        return res;
    } else {
        _u128sub64(tmp + 1, tmp + 1, m);
        return tmp[1];
    }
}

static inline ulong mont64_R2(ulong m) {
    u192 r = {0, 0, 1};
    u128 q;
    lmmp_div_1_s_(q, r, 3, m);
    return r[0];
}

static inline ulong to_mont64(ulong x, ulong R2, ulong m, ulong m_inv) {
    u128 t;
    _u128mul(t, x, R2);
    return mont64_reduce(t, m, m_inv);
}

static inline ulong from_mont64(ulong x, ulong m, ulong m_inv) {
    u128 t = {x, 0};
    return mont64_reduce(t, m, m_inv);
}

static inline ulong mont64_mul(ulong a, ulong b, ulong m, ulong m_inv) {
    u128 t;
    _u128mul(t, a, b);
    return mont64_reduce(t, m, m_inv);
}

static inline ulong mont63_reduce(u128 t, ulong m, ulong m_inv) {
    ulong k = t[0] * m_inv;
    u128 tmp;
    _u128mul(tmp, k, m);
    _u128add(tmp, tmp, t);
    ulong res = tmp[1] >= m ? tmp[1] - m : tmp[1];
    return res;
}

static inline ulong mont63_R2(ulong m) {
    int shift = lmmp_leading_zeros_(m);
    u192 r = {0, 0, 1ull << shift};
    u128 q;
    lmmp_div_1_s_(q, r, 3, m << shift);
    return r[0] >> shift;
}

static inline ulong to_mont63(ulong x, ulong R2, ulong m, ulong m_inv) {
    u128 t;
    _u128mul(t, x, R2);
    return mont63_reduce(t, m, m_inv);
}

static inline ulong from_mont63(ulong x, ulong m, ulong m_inv) {
    u128 t = {x, 0};
    return mont63_reduce(t, m, m_inv);
}

static inline ulong mont63_mul(ulong a, ulong b, ulong m, ulong m_inv) {
    u128 t;
    _u128mul(t, a, b);
    return mont63_reduce(t, m, m_inv);
}

static inline uint lmmp_powmod_uint_(ulong base, ulong exp, uint mod) {
    ulong dst = 1;
    while (1) {
        if (exp & 1)
            dst = dst * base % mod;
        exp >>= 1;
        if (exp == 0)
            break;
        base = base * base % mod;
    }
    return dst;
}

ulong lmmp_powmod_ulong_(ulong base, ulong exp, ulong mod) {
    if (mod <= MP_UINT_MAX)
        return lmmp_powmod_uint_(base, exp, mod);
    else if (mod <= MONT63_MAX) {
        ulong R2 = mont63_R2(mod);
        ulong m_inv = lmmp_binvert_ulong_(mod);
        m_inv = -m_inv;
        ulong dst = to_mont63(1, R2, mod, m_inv);
        base = to_mont63(base, R2, mod, m_inv);
        while (1) {
            if (exp & 1)
                dst = mont63_mul(dst, base, mod, m_inv);
            exp >>= 1;
            if (exp == 0)
                break;
            base = mont63_mul(base, base, mod, m_inv);
        }
        return from_mont63(dst, mod, m_inv);
    } else {
        ulong R2 = mont64_R2(mod);
        ulong m_inv = lmmp_binvert_ulong_(mod);
        m_inv = -m_inv;
        ulong dst = to_mont64(1, R2, mod, m_inv);
        base = to_mont64(base, R2, mod, m_inv);
        while (1) {
            if (exp & 1)
                dst = mont64_mul(dst, base, mod, m_inv);
            exp >>= 1;
            if (exp == 0)
                break;
            base = mont64_mul(base, base, mod, m_inv);
        }
        return from_mont64(dst, mod, m_inv);
    }
}

static inline int miller_rabin_32(ulong a, ulong t, ulong u, uint m) {
    ulong v;
    v = lmmp_powmod_uint_(a, u, m);
    if (v == 1 || v == m - 1)
        return 1;
    for (ulong j = 1; j < t; ++j) {
        v = v * v % m;
        if (v == m - 1)
            return 1;
        if (v == 1)
            return 0;
    }
    return 0;
}

static inline int miller_rabin_63(ulong a, ulong t, ulong u, ulong m, ulong m_inv, ulong one, ulong m_1) {
    ulong v = one;
    ulong base = a;
    while (1) {
        if (u & 1)
            v = mont63_mul(v, base, m, m_inv);
        u >>= 1;
        if (u == 0)
            break;
        base = mont63_mul(base, base, m, m_inv);
    }
    if (v == one || v == m_1)
        return 1;

    for (ulong j = 1; j < t; ++j) {
        v = mont63_mul(v, v, m, m_inv);
        if (v == m_1)
            return 1;
        if (v == one)
            return 0;
    }
    return 0;
}

static inline int miller_rabin_64(ulong a, ulong t, ulong u, ulong m, ulong m_inv, ulong one, ulong m_1) {
    ulong v = one;
    ulong base = a;
    while (1) {
        if (u & 1)
            v = mont64_mul(v, base, m, m_inv);
        u >>= 1;
        if (u == 0)
            break;
        base = mont64_mul(base, base, m, m_inv);
    }
    if (v == one || v == m_1)
        return 1;
    for (ulong j = 1; j < t; ++j) {
        v = mont64_mul(v, v, m, m_inv);
        if (v == m_1)
            return 1;
        if (v == one)
            return 0;
    }
    return 0;
}

/*******************************************************************************
 * from http://probableprime.org/download/example-primality.c
 * Deterministic Miller-Rabin tests for 64-bit.
 * Hashed 2-bases for n < 684630005672341 (slightly more than 2^49)
 * Hashed 3-bases for n < 2^64
 *
 * Based on Steve Worley's 2^32 example:
 *    http://www.mersenneforum.org/showthread.php?t=12209
 * With a 3-base encoding idea from Bradley Berg.
 *
 * Copyright 2014, Dana Jacobsen <dana@acm.org>
 *******************************************************************************/

bool lmmp_is_prime_ulong_(ulong n) {
    if (n < PRIME_SHORT_TABLE_N) {
        return lmmp_is_prime_table_(n);
    }
    if (n % 2 == 0 || n % 3 == 0 || n % 5 == 0 || n % 7 == 0) 
        return false;
    if (n < 684630005672341) {
        ushort bases[2];
        bases[0] = 2;
        bases[1] = dj_base49[((0x3AC69A35UL * n) & 0xFFFFFFFFUL) >> 21] + 3;
        if (n % bases[0] == 0)
            return false;
        if (n % bases[1] == 0)
            return false;

        ulong one = 1;
        ulong m_1 = n - 1;
        ulong m_inv = lmmp_binvert_ulong_(n);
        m_inv = -m_inv;
        ulong R2 = mont63_R2(n);
        one = to_mont63(one, R2, n, m_inv);
        m_1 = to_mont63(m_1, R2, n, m_inv);

        ulong u = n - 1, t = 0;
        while (u % 2 == 0) u /= 2, ++t;

        if (miller_rabin_63(bases[0], t, u, n, m_inv, one, m_1))
            if (miller_rabin_63(bases[1], t, u, n, m_inv, one, m_1))
                return true;
            else
                return false;
        else
            return false;
    } else {
        ushort bases[3];
        ulong bbmask = dj_base64[((0x3AC69A35UL * n) & 0xFFFFFFFFUL) >> 18];
        bases[0] = 2;
        bases[1] = (bbmask & 0x8000) ? 26460 : 9375;
        bases[2] = (bbmask & 0x7FFF) + 3;

        if (n % bases[0] == 0)
            return false;
        if (n % bases[1] == 0)
            return false;
        if (n % bases[2] == 0)
            return false;

        ulong one = 1;
        ulong m_1 = n - 1;
        ulong m_inv = lmmp_binvert_ulong_(n);
        m_inv = -m_inv;

        if (n <= MONT63_MAX) {
            ulong R2 = mont63_R2(n);
            one = to_mont63(one, R2, n, m_inv);
            m_1 = to_mont63(m_1, R2, n, m_inv);

            ulong u = n - 1, t = 0;
            while (u % 2 == 0) u /= 2, ++t;

            if (miller_rabin_63(bases[0], t, u, n, m_inv, one, m_1))
                if (miller_rabin_63(bases[1], t, u, n, m_inv, one, m_1))
                    if (miller_rabin_63(bases[2], t, u, n, m_inv, one, m_1))
                        return true;
                    else
                        return false;
                else
                    return false;
            else
                return false;
        } else {
            ulong R2 = mont64_R2(n);
            one = to_mont64(one, R2, n, m_inv);
            m_1 = to_mont64(m_1, R2, n, m_inv);

            ulong u = n - 1, t = 0;
            while (u % 2 == 0) u /= 2, ++t;

            if (miller_rabin_64(bases[0], t, u, n, m_inv, one, m_1))
                if (miller_rabin_64(bases[1], t, u, n, m_inv, one, m_1))
                    if (miller_rabin_64(bases[2], t, u, n, m_inv, one, m_1))
                        return true;
                    else
                        return false;
                else
                    return false;
            else
                return false;
        }
    }
}