#include "../include/test_short.hpp"
#include "../../../include/lammp/lampz.h"
#include <iostream>
#include <cassert>
#include <string>
#include <algorithm>
#include <vector>

namespace test_short {

static std::string to_le_str(std::string s) {
    std::reverse(s.begin(), s.end());
    return s;
}

void test_div() {
    std::cout << "Testing division..." << std::endl;
    
    lampz_t a, b, q, r;
    __lampz_init(a);
    __lampz_init(b);
    __lampz_init(q);
    __lampz_init(r);

    // --- Case 1: 100 / 3 ---
    lampz_set_ui(a, 100);
    lampz_set_ui(b, 3);
    
    lampz_div_mod_xy(q, r, a, b);
    
    assert(lampz_to_ui(q) == 33);
    assert(lampz_to_ui(r) == 1);
    std::cout << "Pass: 100 / 3 = 33 ... 1" << std::endl;

    // --- Case 2: Big number division ---
    // 10^20 / 10^10 = 10^10
    std::string s_a_norm = "100000000000000000000"; // 21 digits
    std::string s_b_norm = "10000000000"; // 11 digits
    
    std::string s_a = to_le_str(s_a_norm);
    std::string s_b = to_le_str(s_b_norm);
    
    lampz_set_str(a, s_a.c_str(), s_a.length(), 10);
    lampz_set_str(b, s_b.c_str(), s_b.length(), 10);
    
    lampz_div_mod_xy(q, r, a, b);
    
    // Check remainder is 0
    // assert(lampz_is_zero(r)); // r might be empty or 0 length
    assert(lampz_to_ui(r) == 0);
    
    // Check quotient is 10^10
    assert(lampz_to_ui(q) == 10000000000ULL);
    std::cout << "Pass: 10^20 / 10^10 = 10^10" << std::endl;

    // --- Case 3: Division satisfying q*b + r == a ---
    // a = 12345678, b = 123
    lampz_set_ui(a, 12345678);
    lampz_set_ui(b, 123);
    lampz_div_mod_xy(q, r, a, b);
    
    // q * b + r should be a
    lampz_t temp;
    __lampz_init(temp);
    lampz_mul_xy(temp, q, b);
    lampz_add_xy(temp, temp, r);
    
    // Compare temp and a
    // Since we don't have cmp(a,b), we rely on subtraction being 0
    lampz_sub_xy(temp, temp, a);
    assert(lampz_is_zero(temp));
    
    std::cout << "Pass: q * b + r == a check" << std::endl;

    // --- Case 4: Division by larger number ---
    // 10 / 20 = 0 ... 10
    lampz_set_ui(a, 10);
    lampz_set_ui(b, 20);
    lampz_div_mod_xy(q, r, a, b);
    assert(lampz_to_ui(q) == 0);
    assert(lampz_to_ui(r) == 10);
    std::cout << "Pass: 10 / 20 = 0 ... 10" << std::endl;
    
    // --- Case 5: Negative numbers ---
    // 10 / -3 = -3 ... 1  (Truncated division usually: 10 = (-3)*(-3) + 1)
    
    lampz_set_ui(a, 10);
    lampz_set_ui(b, 3);
    lampz_set_sign(b, -1); // b = -3
    
    lampz_div_mod_xy(q, r, a, b);
    // q should be -3
    assert(lampz_to_si(q) == -3);
    // r should be 1
    assert(lampz_to_si(r) == 1);
    std::cout << "Pass: 10 / -3 = -3 ... 1" << std::endl;

    lampz_set_ui(a, 10);
    lampz_set_sign(a, -1); // a = -10
    lampz_set_ui(b, 3);    // b = 3
    
    lampz_div_mod_xy(q, r, a, b);
    // q should be -3
    std::cout << "Debug case 5b: q=" << lampz_to_si(q) << " r=" << lampz_to_si(r) << std::endl;
    assert(lampz_to_si(q) == -3);
    // r should be -1
    assert(lampz_to_si(r) == -1);
    std::cout << "Pass: -10 / 3 = -3 ... -1" << std::endl;

    lampz_free(a);
    lampz_free(b);
    lampz_free(q);
    lampz_free(r);
    lampz_free(temp);
}

} // namespace test_short
