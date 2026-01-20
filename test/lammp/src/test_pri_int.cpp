#include "../include/test_short.hpp"

void test_pri_int() {
    pri_int tab;
    lmmp_prime_int_init_(&tab, 256);
    for (size_t i = 0; i < tab.prin; i++) {
        std::cout << tab.pri[i] << ",";
    }
    std::cout << std::endl;
    lmmp_prime_int_free_(&tab);
}