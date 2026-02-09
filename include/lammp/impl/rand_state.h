#ifndef __LAMMP_RAND_STATE_H__
#define __LAMMP_RAND_STATE_H__

#include "../lmmp.h"

typedef struct {
    mp_limb_t state[2];
    mp_limb_t inc[2];  // 必须为奇数
} pcg64_128_state;

typedef struct {
    mp_limb_t s[4];  // 256位状态，必须初始化为非零值
} xorshift256pp_state;

/**
 * @brief 初始化pcg64_128随机数生成器
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @param seed 种子
 */
void lmmp_pcg64_128_srandom(pcg64_128_state* rng, mp_limb_t seed);

/**
 * @brief 生成一个随机数
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @return 随机数
 */
mp_limb_t lmmp_pcg64_128_random(pcg64_128_state* rng);

/**
 * @brief 初始化xorshift256++随机数生成器
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @param seed 种子
 */
void lmmp_xorshift_srandom(xorshift256pp_state* rng, mp_limb_t seed);

/**
 * @brief 生成一个随机数
 * @param rng 随机数生成器状态
 * @warning rng != NULL
 * @return 随机数
 */
mp_limb_t lmmp_xorshift_random(xorshift256pp_state* rng);

#endif // __LAMMP_RAND_STATE_H__