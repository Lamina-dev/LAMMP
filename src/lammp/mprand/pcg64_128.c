#include "../../../include/lammp/mprand.h"
#include "../../../include/lammp/impl/u128_u192.h"
#include "../../../include/lammp/impl/_umul64to128_.h"

/**
 * algorithm: PCG-XSL-RR-128/64
 * editor: HJimmyK(https://github.com/HJimmyK)
 * date: 2026-01-24
 * test result:
 * RNG_test using PractRand version 0.94
 * RNG = RNG_stdin64, seed = unknown
 * test set = core, folding = standard (64 bit)
 *
 * rng=RNG_stdin64, seed=unknown
 * length= 512 megabytes (2^29 bytes), time= 2.6 seconds
 *   no anomalies in 226 test result(s)
 *
 * rng=RNG_stdin64, seed=unknown
 * length= 1 gigabyte (2^30 bytes), time= 5.4 seconds
 *   no anomalies in 243 test result(s)
 */

#define PCG128_DEFAULT_MULTIPLIER_HI 0x2360ED051FC65DA4ULL
#define PCG128_DEFAULT_MULTIPLIER_LO 0x4385DF649FCCF645ULL

static inline void pcg64_128_action(mp_limb_t state[2], const mp_limb_t inc[2]) {
    // state = (state * PCG64_MULT + inc) mod 2^128
    mp_limb_t tmp[2] = {0, 0};
    _umul64to128_(state[0], PCG128_DEFAULT_MULTIPLIER_LO, tmp, tmp + 1);
    tmp[1] += state[1] * PCG128_DEFAULT_MULTIPLIER_LO;
    tmp[1] += state[0] * PCG128_DEFAULT_MULTIPLIER_HI;
    _u128add(state, tmp, inc);
}

void lmmp_pcg64_128_srandom(pcg64_128_state* rng, mp_limb_t seed) {
    lmmp_debug_assert(rng != NULL);

    if (seed == 0) seed = 0x1234567890abcdefULL;

    rng->state[0] = seed;
    rng->state[1] = seed;
    rng->inc[0] = seed ^ (seed >> 18); /* no use may enhanced obfuscation */
    rng->inc[0] |= 1ull;               /* force the lowest bit to 1       */
    rng->inc[1] = seed;
    pcg64_128_action(rng->state, rng->inc);

    rng->state[0] += seed; 
    rng->state[1] ^= (seed >> 11); /* no use may enhanced obfuscation */
    pcg64_128_action(rng->state, rng->inc);
}

// （PCG-XSL-RR-128/64）
mp_limb_t lmmp_pcg64_128_random(pcg64_128_state* rng) {
    lmmp_debug_assert(rng != NULL);
    mp_limb_t oldstate[2] = {rng->state[0], rng->state[1]};
    pcg64_128_action(rng->state, rng->inc);

    // XSL-RR
    mp_limb_t xsl = ((oldstate[1]) ^ oldstate[0]);
    
    mp_byte_t rot = (mp_byte_t)(oldstate[1] >> 58); 
    return (xsl >> rot) | (xsl << ((-rot) & 63));
}
