#include "../include/test_short.hpp"
#include <chrono>

void test_gcd() {
    TEMP_DECL;

    mp_size_t an = 1, bn = 1, cn = 1;

    mp_ptr a = TALLOC_TYPE(an, mp_limb_t);
    mp_ptr b = TALLOC_TYPE(bn, mp_limb_t);
    mp_ptr _a = TALLOC_TYPE(an + cn, mp_limb_t);
    mp_ptr _b = TALLOC_TYPE(bn + cn, mp_limb_t);
    mp_ptr c = TALLOC_TYPE(cn, mp_limb_t);

    lmmp_global_rng_init_(22212313, 1);
    lmmp_random_(a, an);
    lmmp_random_(b, bn);    
    lmmp_random_(c, cn);

    lmmp_mul_(_a, a, an, c, cn);
    an += cn;
    an -= _a[an - 1] == 0 ? 1 : 0;

    lmmp_mul_(_b, b, bn, c, cn);
    bn += cn;
    bn -= _b[bn - 1] == 0 ? 1 : 0;

    mp_size_t rn1 = LMMP_MIN(an, bn);
    mp_ptr dst1 = TALLOC_TYPE(rn1, mp_limb_t);
    auto start = std::chrono::high_resolution_clock::now();
    rn1 = lmmp_gcd_basecase_(dst1, _a, an, _b, bn);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "gcd_basecase time: " << duration.count() << " microseconds" << std::endl;

    mp_size_t rn2 = LMMP_MIN(an, bn);
    mp_ptr dst2 = TALLOC_TYPE(rn2, mp_limb_t);
    start = std::chrono::high_resolution_clock::now();
    rn2 = lmmp_gcd_lehmer_(dst2, _a, an, _b, bn);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "gcd_lehmer time: " << duration.count() << " microseconds" << std::endl;

    if (rn1 != rn2) {
        std::cout << "Error: gcd_basecase and gcd_lehmer return different results" << std::endl;
        std::cout << "rn1: " << rn1 << " rn2: " << rn2 << std::endl;
    }

    for (mp_size_t i = 0; i < rn1; i++) {
        if (dst1[i] != dst2[i]) {
            std::cout << "Error: gcd_basecase and gcd_lehmer return different results" << std::endl;
            break;
        }
    }

    TEMP_FREE;
}