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
#ifndef __LAMMP_RAND_STATE_H__
#define __LAMMP_RAND_STATE_H__

#include "../lmmp.h"
#include "../impl/u128_u192.h"

typedef struct {
    mp_limb_t state[2];
    mp_limb_t inc[2];  // 必须为奇数
} pcg64_128_state;

typedef struct {
    mp_limb_t s[4];  // 256位状态，必须初始化为非零值
} xoshiro256pp_state;

#define PCG128_DEFAULT_MULTIPLIER_HI 0x2360ED051FC65DA4ULL
#define PCG128_DEFAULT_MULTIPLIER_LO 0x4385DF649FCCF645ULL

#ifndef INLINE_
#define INLINE_ static inline
#endif

/**
 * @brief 种子生成器
 * @param seed 低熵种子
 * @return 高熵种子
 */
INLINE_ mp_limb_t lmmp_seed_generator(mp_limb_t seed) {
    mp_limb_t z = (seed += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

INLINE_ void pcg64_128_action(mp_limb_t state[2], const mp_limb_t inc[2]) {
    // state = (state * PCG64_MULT + inc) mod 2^128
    mp_limb_t tmp[2] = {0, 0};
    _umul64to128_(state[0], PCG128_DEFAULT_MULTIPLIER_LO, tmp, tmp + 1);
    tmp[1] += state[1] * PCG128_DEFAULT_MULTIPLIER_LO;
    tmp[1] += state[0] * PCG128_DEFAULT_MULTIPLIER_HI;
    _u128add(state, tmp, inc);
}

INLINE_ void lmmp_pcg64_128_srandom(pcg64_128_state* rng, mp_limb_t seed) {
    lmmp_param_assert(rng != NULL);

    rng->state[0] = lmmp_seed_generator(seed);
    rng->state[1] = lmmp_seed_generator(seed << 17);
    rng->inc[0] = lmmp_seed_generator(seed << 7); 
    rng->inc[0] |= 1ull;               
    rng->inc[1] = lmmp_seed_generator(seed << 21);

    // warm up
    pcg64_128_action(rng->state, rng->inc);
    pcg64_128_action(rng->state, rng->inc);
}

// （PCG-XSL-RR-128/64）
INLINE_ mp_limb_t lmmp_pcg64_128_random(pcg64_128_state* rng) {
    lmmp_param_assert(rng != NULL);
    mp_limb_t oldstate[2] = {rng->state[0], rng->state[1]};
    pcg64_128_action(rng->state, rng->inc);

    // XSL-RR
    mp_limb_t xsl = ((oldstate[1]) ^ oldstate[0]);
    
    mp_byte_t rot = (mp_byte_t)(oldstate[1] >> 58); 
    return (xsl >> rot) | (xsl << ((-rot) & 63));
}

INLINE_ mp_limb_t rotl(const mp_limb_t x, int k) {
    const int shift = k & 63;
    return (x << shift) | (x >> (64 - shift));
}

INLINE_ mp_limb_t lmmp_xoshiro256pp_random(xoshiro256pp_state* rng) {
    lmmp_param_assert(rng != NULL);
    const mp_limb_t r = rotl(rng->s[0] + rng->s[3], 23) + rng->s[0];
    const mp_limb_t t = rng->s[1] << 17;

    rng->s[2] ^= rng->s[0];
    rng->s[3] ^= rng->s[1];
    rng->s[1] ^= rng->s[2];
    rng->s[0] ^= rng->s[3];
    rng->s[2] ^= t;
    rng->s[3] = rotl(rng->s[3], 45);

    return r;
}

INLINE_ void lmmp_xoshiro256pp_srandom(xoshiro256pp_state* rng, mp_limb_t seed) {
    lmmp_param_assert(rng != NULL);

    rng->s[0] = lmmp_seed_generator(seed);
    rng->s[1] = lmmp_seed_generator(seed << 17);
    rng->s[2] = lmmp_seed_generator(0x9e37b97f8a5a7c19ULL ^ seed);
    rng->s[3] = lmmp_seed_generator(0x8a07e6c7f6b9c92eULL ^ seed);

    // assert that the initial state is non-zero
    if (rng->s[0] == 0 && rng->s[1] == 0 && rng->s[2] == 0 && rng->s[3] == 0) {
        rng->s[0] = 0xa12ef3383a3d2eefULL;
        rng->s[1] = 0xcdfabe82ecd412ceULL;
        rng->s[2] = 0x90b5ec55c9235815ULL;
        rng->s[3] = 0xcfb28093ca79a3a7ULL;
    }
    // warm up
    lmmp_xoshiro256pp_random(rng);
    lmmp_xoshiro256pp_random(rng);
    lmmp_xoshiro256pp_random(rng);
}

#define PCG64_LE_MULTIPLIER 6364136223846793005ULL 
#define PCG64_LE_INCREMENT 1442695040888963407ULL 

typedef struct {
    mp_size_t k;
    mp_limb_t* restrict state;
} pcg64_le_seq_t;

INLINE_ void pcg64_le_seq_init(pcg64_le_seq_t* rng, mp_limb_t seed) {
    lmmp_param_assert(rng != NULL);
    lmmp_param_assert(rng->k > 0);
    lmmp_param_assert(rng->state != NULL);

    mp_size_t i;
    for (i = 0; i < rng->k; i += 4) {
        mp_limb_t s0 = lmmp_seed_generator(seed + i + 0);
        mp_limb_t s1 = lmmp_seed_generator(seed + i + 1);
        mp_limb_t s2 = lmmp_seed_generator(seed + i + 2);
        mp_limb_t s3 = lmmp_seed_generator(seed + i + 3);
        s0 ^= (i + 0) ^ ((i + 0) << 32);
        s1 ^= (i + 1) ^ ((i + 1) << 32);
        s2 ^= (i + 2) ^ ((i + 2) << 32);
        s3 ^= (i + 3) ^ ((i + 3) << 32);
        rng->state[i + 0] = lmmp_seed_generator(s0 ^ rotl(s0, 17));
        rng->state[i + 1] = lmmp_seed_generator(s1 ^ rotl(s1, 21));
        rng->state[i + 2] = lmmp_seed_generator(s2 ^ rotl(s2, 13));
        rng->state[i + 3] = lmmp_seed_generator(s3 ^ rotl(s3, 33));
    }
    for (; i < rng->k; i++) {
        mp_limb_t s = lmmp_seed_generator(seed + i);
        s ^= (i + 0) ^ ((i + 0) << 32);
        rng->state[i] = lmmp_seed_generator(s ^ rotl(s, 45));
    }
}

INLINE_ mp_limb_t pcg64_le_action(mp_limb_t* state) {
    mp_limb_t old_state = *state;
    *state = old_state * PCG64_LE_MULTIPLIER + PCG64_LE_INCREMENT;

    // RXS-M-XS
    mp_limb_t x = old_state;
    int count = x >> 59;
    x ^= x >> (5 + count);
    x *= 12605985483714917081ULL;
    x ^= x >> 43;

    return x;
}

INLINE_ void pcg64_le_seq_next(mp_ptr dst, pcg64_le_seq_t* rng) {
    lmmp_param_assert(dst != NULL);
    lmmp_param_assert(rng != NULL);
    lmmp_param_assert(rng->state != NULL);
    mp_size_t i;

    for (i = 0; i < rng->k; i += 4) {
        dst[i + 0] = pcg64_le_action(&rng->state[i + 0]);
        dst[i + 1] = pcg64_le_action(&rng->state[i + 1]);
        dst[i + 2] = pcg64_le_action(&rng->state[i + 2]);
        dst[i + 3] = pcg64_le_action(&rng->state[i + 3]);
    }
    for (; i < rng->k; i++) {
        dst[i] = pcg64_le_action(&rng->state[i]);
    }
}

#undef INLINE_

#endif // __LAMMP_RAND_STATE_H__