#include <chrono>
#include <random>

#include "../include/test_short.hpp"


void test_vec_ele_mul() {
    const mp_ssize_t n = 300;
    mp_ssize_t a[n];
    mp_ptr p[n];
    for (mp_size_t i = 0; i < n; i++) {
        a[i] = rand() % n;
        a[i] = a[i] == 0 ? 1 : a[i];
        p[i] = ALLOC_TYPE(a[i], mp_limb_t);
        a[i] = lmmp_random_(p[i], a[i]);
    }
    mp_ptr dst1 = ALLOC_TYPE(n * n, mp_limb_t);
    mp_ptr dst3 = ALLOC_TYPE(n * n, mp_limb_t);
    bool nd = true;
    dst1[0] = 1;
    mp_ssize_t dstn1 = 1;
    auto start = std::chrono::high_resolution_clock::now();
    for (mp_size_t i = 0; i < n; i++) {
        if (nd) {
            if (dstn1 > a[i])
                lmmp_mul_(dst3, dst1, dstn1, p[i], a[i]);
            else 
                lmmp_mul_(dst3, p[i], a[i], dst1, dstn1);
            dstn1 += a[i];
            dstn1 -= dst3[dstn1 - 1] == 0 ? 1 : 0;
            nd = false;
        } else {
            if (dstn1 > a[i])
                lmmp_mul_(dst1, dst3, dstn1, p[i], a[i]);
            else
                lmmp_mul_(dst1, p[i], a[i], dst3, dstn1);
            dstn1 += a[i];
            dstn1 -= dst1[dstn1 - 1] == 0 ? 1 : 0;
            nd = true;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "duration1: " << duration << " microseconds" << std::endl;

    if (nd == false)
        LMMP_SWAP(dst1, dst3, mp_ptr);

    mp_ptr dst2;
    lmmp_vecn_t vec;
    vec.n = n;
    vec.num = p;
    vec.len = a;
    auto start2 = std::chrono::high_resolution_clock::now();
    mp_ssize_t dstn2 = lmmp_vec_elem_mul_(&dst2, &vec);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "duration2: " << duration2 << " microseconds" << std::endl;

    if (dstn1 != dstn2) {
        std::cout << "dstn1 != dstn2" << std::endl;
    }
    for (mp_ssize_t i = 0; i < dstn1; i++) {
        if (dst1[i] != dst2[i]) {
            std::cout << "dst1[i] != dst2[i]" << std::endl;
            break;
        }
    }

    lmmp_free(dst1);
    lmmp_free(dst2);
    lmmp_free(dst3);
    for (mp_ssize_t i = 0; i < n; i++) {
        lmmp_free(p[i]);
    }
}