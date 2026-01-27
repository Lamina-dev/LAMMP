#include "../include/test_short.hpp"

void test_powmod() {
    ulong a = 9213;
    ulong b = 0;
    ulong m = 81007;
    ulong res = lmmp_powmod_ulong_(a, b, m);
    std::cout << "a^b mod m = " << res << std::endl;
}