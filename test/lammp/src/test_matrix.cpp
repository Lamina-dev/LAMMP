/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <chrono>

#include "../include/test_short.hpp"

#define ALLOC_TYPE(n, type) (type*)lmmp_alloc((n) * sizeof(type))

void test_matrix() {
    lmmp_mat22_t A1, A2, B, C;
    lmmp_global_rng_init_(123823, 0);
    const mp_size_t Bn = 55, Cn = 60;
    B.n00 = Bn;
    B.n01 = Bn;
    B.n10 = Bn;
    B.n11 = Bn;
    C.n00 = Cn;
    C.n01 = Cn;
    C.n10 = Cn;
    C.n11 = Cn;
    B.a00 = ALLOC_TYPE(B.n00, mp_limb_t);
    B.a01 = ALLOC_TYPE(B.n01, mp_limb_t);
    B.a10 = ALLOC_TYPE(B.n10, mp_limb_t);
    B.a11 = ALLOC_TYPE(B.n11, mp_limb_t);
    C.a00 = ALLOC_TYPE(C.n00, mp_limb_t);
    C.a01 = ALLOC_TYPE(C.n01, mp_limb_t);
    C.a10 = ALLOC_TYPE(C.n10, mp_limb_t);
    C.a11 = ALLOC_TYPE(C.n11, mp_limb_t);
    B.n00 = lmmp_random_(B.a00, B.n00);
    B.n01 = lmmp_random_(B.a01, B.n01);
    B.n10 = lmmp_random_(B.a10, B.n10);
    B.n11 = lmmp_random_(B.a11, B.n11);
    C.n00 = lmmp_random_(C.a00, C.n00);
    C.n01 = lmmp_random_(C.a01, C.n01);
    C.n10 = lmmp_random_(C.a10, C.n10);
    C.n11 = lmmp_random_(C.a11, C.n11);
    B.n00 = -B.n00;
    C.n00 = -C.n00;
    C.n11 = -C.n11;

    const mp_size_t an = Bn + Bn + 2;
    A1.a00 = ALLOC_TYPE(an, mp_limb_t);
    A1.a01 = ALLOC_TYPE(an, mp_limb_t);
    A1.a10 = ALLOC_TYPE(an, mp_limb_t);
    A1.a11 = ALLOC_TYPE(an, mp_limb_t);
    A2.a00 = ALLOC_TYPE(an, mp_limb_t);
    A2.a01 = ALLOC_TYPE(an, mp_limb_t);
    A2.a10 = ALLOC_TYPE(an, mp_limb_t);
    A2.a11 = ALLOC_TYPE(an, mp_limb_t);

    const mp_size_t tn = an;
    mp_ptr tp = ALLOC_TYPE(tn * 2, mp_limb_t);
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_mat22_mul_basecase_(&A1, &B, &B, tp, tn);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    printf("basecase time: %lld us\n", duration);

    start = std::chrono::high_resolution_clock::now();
    lmmp_mat22_mul_strassen_(&A2, &B, &B, NULL, tn, Bn + 1);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    printf("strassen time: %lld us\n", duration);

    if (A1.n00 != A2.n00 || A1.n01 != A2.n01 || A1.n10 != A2.n10 || A1.n11 != A2.n11) {
        printf("Error: matrix multiplication result is different\n");
        std::cout << "A1: " << A1.n00 << " " << A1.n01 << " " << A1.n10 << " " << A1.n11 << std::endl;
        std::cout << "A2: " << A2.n00 << " " << A2.n01 << " " << A2.n10 << " " << A2.n11 << std::endl;
    } else {
        if (lmmp_cmp_(A1.a00, A2.a00, LMMP_ABS(A1.n00)) == 0) {
            printf("a00 equals\n");
        }
        if (lmmp_cmp_(A1.a01, A2.a01, LMMP_ABS(A1.n01)) == 0) {
            printf("a01 equals\n");
        }
        if (lmmp_cmp_(A1.a10, A2.a10, LMMP_ABS(A1.n10)) == 0) {
            printf("a10 equals\n");
        }
        if (lmmp_cmp_(A1.a11, A2.a11, LMMP_ABS(A1.n11)) == 0) {
            printf("a11 equals\n");
        }
    }

    lmmp_free(A1.a00);
    lmmp_free(A1.a01);
    lmmp_free(A1.a10);
    lmmp_free(A1.a11);
    lmmp_free(A2.a00);
    lmmp_free(A2.a01);
    lmmp_free(A2.a10);
    lmmp_free(A2.a11);
    lmmp_free(B.a00);
    lmmp_free(B.a01);
    lmmp_free(B.a10);
    lmmp_free(B.a11);
    lmmp_free(C.a00);
    lmmp_free(C.a01);
    lmmp_free(C.a10);
    lmmp_free(C.a11);
    lmmp_free(tp);
}