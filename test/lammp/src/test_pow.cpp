#include "../include/test_short.hpp"
#include <chrono>

int test_pow(uint len, uint exp) {
    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);
    a[0] = 0xffcff;
    for (size_t i = 1; i != len; ++i) {
        a[i] = (mp_limb_t)rand() * rand();
    }

    mp_size_t bn = lmmp_pow_size_(a, len, exp);
    mp_ptr b = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    //std::cout << "1" << std::endl;
    mp_size_t n1 = lmmp_pow_basecase_(b, bn, a, len, exp);
    mp_size_t n2 = lmmp_pow_win2_(c, bn, a, len, exp);
    if (n1 != n2) {
        lmmp_free(b);
        lmmp_free(c);
        return 1;
    }
    for (size_t i = 0; i != n1 - 1; ++i) {
        if (b[i] != c[i]) {
            lmmp_free(b);
            lmmp_free(c);
            return 1;
        }
    }
    lmmp_free(c);
    lmmp_free(b);
    return 0;
}