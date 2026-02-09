#include "../../../include/lammp/mprand.h"
#include "../../../include/lammp/impl/rand_state.h"

typedef struct {
    xorshift256pp_state state;
    int seed_type;
#define INIT 1
    int is_init; /* 1 if initialized, 0 otherwise */
} lmmp_global_rng_t;

static lmmp_global_rng_t lmmp_global_rng;

void lmmp_global_rng_init_(int seed, int seed_type) {
    lmmp_xorshift_srandom(&lmmp_global_rng.state, seed);
    lmmp_global_rng.seed_type = seed_type % 2;
    lmmp_global_rng.is_init = INIT;
}

mp_size_t lmmp_seed_random_(mp_ptr dst, mp_size_t n, mp_limb_t seed, int seed_type) {
    seed_type %= 2;
    if (seed_type == 0) {
        pcg64_128_state rng;
        lmmp_pcg64_128_srandom(&rng, seed);
        mp_size_t i = 0;
        for (; i + 3 < n; i += 4) {
            dst[i + 0] = lmmp_pcg64_128_random(&rng);
            dst[i + 1] = lmmp_pcg64_128_random(&rng);
            dst[i + 2] = lmmp_pcg64_128_random(&rng);
            dst[i + 3] = lmmp_pcg64_128_random(&rng);
        }
        for (; i < n; ++i) {
            dst[i] = lmmp_pcg64_128_random(&rng);
        }
    } else {
        xorshift256pp_state rng;
        lmmp_xorshift_srandom(&rng, seed);
        mp_size_t i = 0;
        for (; i + 3 < n; i += 4) {
            dst[i + 0] = lmmp_xorshift_random(&rng);
            dst[i + 1] = lmmp_xorshift_random(&rng);
            dst[i + 2] = lmmp_xorshift_random(&rng);
            dst[i + 3] = lmmp_xorshift_random(&rng);
        }
        for (; i < n; ++i) {
            dst[i] = lmmp_xorshift_random(&rng);
        }
    }
    while (n > 0 && dst[n - 1] == 0) {
        --n;
    }
    return n;
}

mp_size_t lmmp_random_(mp_ptr dst, mp_size_t n) {
    if (lmmp_global_rng.is_init == INIT) {
        mp_limb_t seed = lmmp_xorshift_random(&lmmp_global_rng.state);
        return lmmp_seed_random_(dst, n, seed, lmmp_global_rng.seed_type);
    } else {
        lmmp_global_rng_init_(0, 1);
        mp_limb_t seed = lmmp_xorshift_random(&lmmp_global_rng.state);
        return lmmp_seed_random_(dst, n, seed, lmmp_global_rng.seed_type);
    }
}
