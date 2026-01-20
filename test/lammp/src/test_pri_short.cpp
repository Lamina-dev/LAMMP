#include "../include/test_short.hpp"

void test_pri_short() {
    pri_short tab;
    lmmp_prime_short_init_(&tab, 65535);
    for (size_t i = 0; i < tab.prin; i++) {
        std::cout << tab.pri[i] << " ";
    }
    std::cout << std::endl;
    lmmp_prime_short_free_(&tab);
}