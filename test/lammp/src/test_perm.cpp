#include <chrono>
#include "../include/test_short.hpp"

void test_perm() {
    size_t n = 1000000, r = n;
    size_t len = lmmp_nPr_size_(n, r);
    std::cout << "len = " << len << std::endl;

    mp_ptr a;
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    mp_ptr limb_vec = ALLOC_TYPE(n, mp_limb_t);
    for (size_t i = 0; i < n; i++) {
        limb_vec[i] = i + 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    mp_size_t an = lmmp_limb_elem_mul_(&a, limb_vec, n);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: (native)" << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nPr_(b, len, n, r);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: (queued)" << duration2 << " microseconds" << std::endl;

    if (bn != an)
        std::cout << "bn != an" << std::endl;
    else 
        std::cout << bn << " == " << an << std::endl;
    for (size_t i = 0; i < an; i++) {
        if (a[i] != b[i]) {
            std::cout << "i = " << i << " a[i] = " << a[i] << " b[i] = " << b[i] << std::endl;
            break;
        }
    }
    std::cout << "passed\n";
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(limb_vec);
}