#include "../include/benchmark.hpp"

void ger_random_numbers(int len, mp_ptr dst, mp_size_t min, mp_size_t max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<mp_limb_t> dis(min, max);
    for (int i = 0; i < len; i++) {
        dst[i] = dis(gen);
    }
}