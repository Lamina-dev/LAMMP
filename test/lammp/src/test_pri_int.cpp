#include "../include/test_short.hpp"

void test_pri_int() {
    pri_int tab;
    lmmp_prime_int_init_(&tab, 92753);
    std::cout << tab.prin << std::endl;
    for (size_t i = tab.prin - 100; i < tab.prin; i++) {
        std::cout << tab.pri[i] << ",";
    }
    std::cout << std::endl;
    lmmp_prime_int_free_(&tab);
}