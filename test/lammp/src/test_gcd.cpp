#include "../include/test_short.hpp"

void test_gcd() {
    TEMP_DECL;

    mp_size_t an = 10, bn = 10, cn = 10;

    mp_ptr a = TALLOC_TYPE(an, mp_limb_t);
    mp_ptr b = TALLOC_TYPE(bn, mp_limb_t);
    mp_ptr _a = TALLOC_TYPE(an + cn, mp_limb_t);
    mp_ptr _b = TALLOC_TYPE(bn + cn, mp_limb_t);
    mp_ptr c = TALLOC_TYPE(cn, mp_limb_t);

    lmmp_global_rng_init_(20, 0);
    lmmp_random_(a, an);
    lmmp_random_(b, bn);    
    lmmp_random_(c, cn);

    lmmp_mul_(_a, a, an, c, cn);
    an += cn;
    an -= _a[an - 1] == 0 ? 1 : 0;

    lmmp_mul_(_b, b, bn, c, cn);
    bn += cn;
    bn -= _b[bn - 1] == 0 ? 1 : 0;

    mp_size_t rn = LMMP_MIN(an, bn);
    mp_ptr dst = TALLOC_TYPE(rn, mp_limb_t);
    rn = lmmp_gcd_basecase_(dst, _a, an, _b, bn);

    for (mp_size_t i = 0; i < rn; i++) {
        std::cout << dst[i] << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;

    for (mp_size_t i = 0; i < cn; i++) {
        std::cout << c[i] << " ";
    }
    std::cout << std::endl;

    TEMP_FREE;
}