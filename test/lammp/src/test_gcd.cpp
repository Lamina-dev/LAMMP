#include "../include/test_short.hpp"

void test_gcd() {
    // 示例：计算GCD(0x1234567890ABCDEF1234567890ABCDEF, 0x0FEDCBA0987654321FEDCBA0987654321)
    // 两个数各2个limb（64位*2=128位）
    mp_size_t rn = lmmp_nCr_size_(10000, 600);
    mp_ptr dst = ALLOC_TYPE(rn - 4, mp_limb_t);
    rn = lmmp_nCr_(dst, rn, 10000, 600);

    lmmp_free(dst);
}