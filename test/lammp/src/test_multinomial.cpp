#include <chrono>
#include "../include/test_short.hpp"


void test_multinomial() {
    size_t m = 4;
    uint r[4] = {1231, 31610, 12320, 21022};
    ulong n = 0;
    size_t len = lmmp_multinomial_size_(r, m, &n);
    std::cout << "len = " << len << std::endl;
    std::cout << "n = " << n << std::endl;

    mp_ptr a = ALLOC_TYPE(len, mp_limb_t);

    auto start2 = std::chrono::high_resolution_clock::now();
    mp_size_t bn = lmmp_multinomial_(a, len, n, r, m);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
    std::cout << "Time elapsed: (queued)" << duration2 << " microseconds" << std::endl;

    for (size_t i = bn - 1; i != (size_t)-1 ; --i) {
        std::cout << std::hex << a[i];
    }
    std::cout << std::endl;
    lmmp_free(a);
}