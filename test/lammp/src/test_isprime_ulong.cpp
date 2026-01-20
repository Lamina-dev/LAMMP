#include "../include/test_short.hpp"

void test_isprime_ulong() {
    ulong n = 2305843009213693951ull;
    if (lmmp_is_prime_ulong_(n)) {
        std::cout << n << " is a prime number" << std::endl;
    }
    else {
        std::cout << n << " is not a prime number" << std::endl;
    }
}