#include "../include/test_short.hpp"

void test_umul64to128() {
    uint64_t a = 0x123456789ABCDEF0;
    uint64_t b = 0xFEDCBA9876543210;
    uint64_t re[2] = {0, 0};
    _umul64to128_(a, b, re, re + 1);
    if (lmmp_mulh_(a, b) == re[1]) {
        fprintf(stdout, "test_umul64to128 passed\n");
    } else {
        fprintf(stderr, "test_umul64to128 failed\n");
    }
    return;
}
