#include "../include/test_short.hpp"
#include <chrono>

int test_pow(uint a0, uint exp) {
    mp_limb_t a[1] = {a0};

    mp_size_t bn = lmmp_pow_size_(a, 1, exp);
    mp_ptr b = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    //std::cout << "1" << std::endl;
    mp_size_t n1 = lmmp_pow_1_(b, bn, a[0], exp);
    mp_size_t n2 = lmmp_pow_win3_(c, a, 1, exp);
    if (n1 != n2) {
        FREE(b);
        FREE(c);
        return -1;
    }
    for (size_t i = 0; i != n1 - 1; ++i) {
        if (b[i] != c[i]) {
            FREE(b);
            FREE(c);
            return -1;
        }
    }
    FREE(c);
    FREE(b);
    return 0;
}