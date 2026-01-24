#include <iostream>
#include "include/test_long.hpp"
#include "include/test_short.hpp"
#include <vector>

int main() {
    // pri_int tab;
    // uint n = 56000;
    // lmmp_prime_int_init_(&tab, n);
    
    // int tsu = test_pow(2, 55993);
    // if (tsu == 0) {
    //     std::cout << "passed";
    // } else {
    //     std::cout << "failed";
    // }
    // std::cout << std::endl;

    // std::vector<mp_limb_t> a_vec;
    // std::vector<mp_limb_t> e_vec;
    // a_vec.reserve(tab.prin);
    // e_vec.reserve(tab.prin);
    // for (int i = 0; i < tab.prin; ++i) {
    //     mp_limb_t a = tab.pri[i];
        
    //     ulong pn = n;
    //     mp_limb_t e = 0;
    //     while (pn > 0) {
    //         pn /= a;
    //         e += pn;
    //     }
    //     if (e < 50) continue;
        
    //     e_vec.push_back(e);
    //     a_vec.push_back(a);
    // }

    // for (int i = 0; i < a_vec.size(); ++i) {
    //     if (test_pow(a_vec[i], e_vec[i]) == 0) {
    //         std::cout << "passed" << std::endl;
    //     } else {
    //         std::cout << "failed" << std::endl;
    //     }
    // }
    // test_factorial();
    
    std::cout << "Testing pow(2, 100)... " << std::flush;
    if (test_pow(2, 100) == 0) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    std::cout << "Testing pow(3, 50)... " << std::flush;
    if (test_pow(3, 50) == 0) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    std::cout << "Testing pow(10, 200)... " << std::flush;
    if (test_pow(10, 200) == 0) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    std::cout << "Testing pow(151, 1439)... " << std::flush;
    if (test_pow(151, 1439) == 0) std::cout << "PASSED" << std::endl;
    else std::cout << "FAILED" << std::endl;

    return 0;
}