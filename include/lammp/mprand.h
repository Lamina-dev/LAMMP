#ifndef LAMMP_MPRAND_H
#define LAMMP_MPRAND_H

#include "lmmp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    mp_limb_t state[2];
    mp_limb_t inc[2];  // 必须为奇数
} pcg64_128_state;

typedef struct {
    mp_limb_t s[4];  // 256位状态，必须初始化为非零值
} xorshift256pp_state;

/**
 * @brief 初始化全局随机数生成器
 * @param seed 种子
 * @param seed_type 随机数发生器类型（0：pcg64_128，1：xorshift256++，2：mt19937-64）
 * @warning seed_type 必须在0到2之间，如果不是，则使用 seed_type%3 作为seed_type
 */
void lmmp_global_rng_init_(uint64_t seed, int seed_type);

/**
 * @brief 生成随机大整数（0 - B^n-1 均匀分布）
 * @param dst 随机数存储位置
 * @param n dst的 limb 长度
 * @param seed 种子
 * @param seed_type 随机数发生器类型（0：pcg64_128，1：xorshift256++，2：mt19937-64）
 * @warning seed_type 必须在0到2之间，如果不是，则使用 seed_type%3 作为seed_type
 * @return 随机数的 limb 长度（由于可能存在生成随机数为0的情况，所以返回值可能小于n，但不会大于n）
 */
mp_size_t lmmp_random_(mp_ptr dst, mp_size_t n, uint64_t seed, int seed_type);

/**
 * @brief 生成随机大整数（0 - B^n-1 均匀分布）
 * @param dst 随机数存储位置
 * @param n dst的 limb 长度
 * @warning 种子由 lmmp_global_rng_init() 设置，发生器类型由 lmmp_global_rng_init() 设置
 *          如果没有进行全局初始化，则使用默认种子（默认为0）和默认发生器类型（默认为xorshift256++）
 * @return 随机数的 limb 长度（由于可能存在生成随机数为0的情况，所以返回值可能小于n，但不会大于n）
 */
mp_size_t lmmp_random_default_(mp_ptr dst, mp_size_t n);

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


#ifdef __cplusplus
}
#endif

#endif // LAMMP_MPRAND_H
