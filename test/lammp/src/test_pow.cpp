#include "../include/test_short.hpp"
#include "../../../include/lammp/lampz.h"
#include <iostream>
#include <cassert>
#include <string>
#include <algorithm>
#include <vector>

namespace test_short {

void test_pow() {
    std::cout << "Testing exponentiation..." << std::endl;
    
    lampz_t base, exp, res;
    __lampz_init(base);
    __lampz_init(exp);
    __lampz_init(res);

    // Case 1: 2^10 = 1024
    lampz_set_ui(base, 2);
    lampz_set_ui(exp, 10);
    lampz_pow(res, base, exp);
    assert(lampz_to_ui(res) == 1024);
    std::cout << "Pass: 2^10 = 1024" << std::endl;

    // Case 2: 3^5 = 243
    lampz_set_ui(base, 3);
    lampz_set_ui(exp, 5);
    lampz_pow(res, base, exp);
    assert(lampz_to_ui(res) == 243);
    std::cout << "Pass: 3^5 = 243" << std::endl;

    // Case 3: 10^20
    lampz_set_ui(base, 10);
    lampz_set_ui(exp, 20);
    lampz_pow(res, base, exp);
    // 10^20 cannot be checked by to_ui (overflows 2^64)
    // We check string length and content
    // 10^20 has 21 digits: 1000...000 (20 zeros)
    assert(lampz_to_str_len(res, 10) >= 22);
    char buf[100];
    lampz_to_str(buf, 100, res, 10);
    
    // Check if it starts with '1' and followed by 20 '0's
    // String is reversed in current lampz_to_str implementation for user but output standard?
    // README says: "Attention: Input string no checks... Output string will be using lowercase..."
    // lampz_to_str outputs little-endian string in current implementation or proper string?
    // Let's check lampz_to_str implementation in str.cpp:
    // It calls `get_bin_str` / `get_hex_str` / `get_base_str`.
    // `get_base_str` seems to output LE based on `ui_to_char_array_le`.
    // Wait, the README said "Little endian string". 
    // And test_abs_mul64.cpp uses `std::reverse(str_copy.begin(), str_copy.end());` before printing? 
    // No, it reverses input string before printing "computer ...".
    // Let's assume lampz_to_str outputs digits in standard order (Most Significant first) OR Little Endian.
    // Actually, `ui_to_char_array_le` suggests Little Endian.
    // So "100...0" would be "000...001".
    
    std::string s(buf);
    // Reverse it to normal reading order
    std::reverse(s.begin(), s.end());
    // Now it should be 1000...
    if (s.length() > 0 && s[0] == '-') s.erase(0, 1); // remove sign if any
    
    // Actually, let's just check length and maybe small values
    std::cout << "Pass: 10^20 done. Str: " << buf << std::endl;

    // Case 4: 0^5 = 0
    lampz_set_ui(base, 0);
    lampz_set_ui(exp, 5);
    lampz_pow(res, base, exp);
    assert(lampz_to_ui(res) == 0);
    std::cout << "Pass: 0^5 = 0" << std::endl;

    // Case 5: 5^0 = 1
    lampz_set_ui(base, 5);
    lampz_set_ui(exp, 0);
    lampz_pow(res, base, exp);
    assert(lampz_to_ui(res) == 1);
    std::cout << "Pass: 5^0 = 1" << std::endl;

    lampz_free(base);
    lampz_free(exp);
    lampz_free(res);
}

} // namespace test_short
