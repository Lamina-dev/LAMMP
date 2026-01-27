#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/mprand.h"

/**
 * algorithm: xorshift256++
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
 * length= 1 gigabyte (2^30 bytes), time= 5.3 seconds
 *   no anomalies in 243 test result(s)
 */

static inline mp_limb_t rotl(const mp_limb_t x, int k) {
    const int shift = k & 63;
    return (x << shift) | (x >> (64 - shift));
}

mp_limb_t lmmp_xorshift_random(xorshift256pp_state* rng) {
    lmmp_debug_assert(rng != NULL);
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

void lmmp_xorshift_srandom(xorshift256pp_state* rng, mp_limb_t seed) {
    lmmp_debug_assert(rng != NULL);

    // assert that the seed is non-zero
    mp_limb_t seed0 = (seed == 0) ? 0x123456789abcdefULL : seed;
    mp_limb_t seed1 = (seed == 0) ? 0xfedcba987654321ULL : seed;

    rng->s[0] = seed0;
    rng->s[1] = seed1;
    rng->s[2] = 0x9e3779b97f4a7c15ULL ^ seed0;  
    rng->s[3] = 0x6a09e667f3bcc909ULL ^ seed1; 

    // assert that the initial state is non-zero
    if (rng->s[0] == 0 && rng->s[1] == 0 && rng->s[2] == 0 && rng->s[3] == 0) {
        rng->s[0] = 0xdeadbeefdeadbeefULL;
        rng->s[1] = 0xfeedfacefeedfaceULL;
        rng->s[2] = 0x2005555555555555ULL;
        rng->s[3] = 0xecfb23809a79a3a7ULL;
    }
    // warm up
    lmmp_xorshift_random(rng);
    lmmp_xorshift_random(rng);
    lmmp_xorshift_random(rng);
}