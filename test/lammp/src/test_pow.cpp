#include "../include/test_short.hpp"
#include <chrono>

int test_pow() {
    mp_limb_t base[3] = {0, 0, 0xf123323};
    ulong exp = 251871;

    mp_size_t bn = lmmp_pow_size_(base, 3, exp);
    mp_ptr b = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(bn * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t n1 = lmmp_pow_win2_(b, bn, base, 3, exp);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "2_pow_1_ time: " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    mp_size_t n2 = lmmp_pow_(c, bn, base, 3, exp);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "pow_1_ time: " << duration << " microseconds" << std::endl;

    if (n1 != n2) {
        std::cout << "n1 != n2\n";
        std::cout << n1 << " " << n2 << std::endl;
        goto end;
    }
    for (size_t i = 0; i < n1; ++i){
        if (b[i] != c[i]) {
            std::cout << "error\n";
            std::cout << "i = " << i << "\n";
            std::cout << b[i] << " " << c[i] << std::endl;
            goto end;
        }
    }

    end:
    lmmp_free(c);
    lmmp_free(b);
    return 0;
}