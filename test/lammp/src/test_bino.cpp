#include <chrono>
#include "../include/test_short.hpp"

mp_size_t bino_native(mp_ptr dst, mp_size_t rn, ulong n, ulong r) {
    TEMP_DECL;

    mp_size_t tn = lmmp_nPr_size_(n, r);
    mp_ptr t = TALLOC_TYPE(tn, mp_limb_t);

    tn = lmmp_nPr_(t, tn, n, r);

    mp_size_t prn = lmmp_nPr_size_(r, r);
    mp_ptr pr = TALLOC_TYPE(prn, mp_limb_t);

    prn = lmmp_factorial_(pr, prn, r);

    lmmp_div_(dst, NULL, t, tn, pr, prn);
    TEMP_FREE;
    rn = tn - prn + 1;
    while (dst[rn - 1] == 0) {
        --rn;
    }
    return rn;
}

void test_bino() {
    size_t n = 0x9fff, r = n / 2;
    size_t len = lmmp_nCr_size_(n, r);
    std::cout << "len = " << len << std::endl;

    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);
    mp_ptr b = ALLOC_TYPE(len, mp_limb_t);

    a[0] = 1;
    mp_size_t an = 1;
    auto start = std::chrono::high_resolution_clock::now();
    an = bino_native(a, len, n, r);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Time elapsed: (native)" << duration << " microseconds" << std::endl;

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_nCr_short_(b, len, n, r);
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
}