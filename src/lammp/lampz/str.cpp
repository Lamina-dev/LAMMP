/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include "../../../include/lammp/lammp.hpp"
#include "../../../include/lammp/lampz.h"
#include "../../../include/lammp/numeral_table.h"
#include "math.h"
#include <stdexcept>
#include <cstring>

void str_reverse(char* s) {
    if (s == NULL || *s == '\0') { 
        return;
    }
    char* left = s;                   // 左指针（指向字符串开头）
    char* right = s + strlen(s) - 1;  // 右指针（指向字符串末尾，跳过'\0'）
    char temp;

    while (left < right) {
        temp = *left;
        *left = *right;
        *right = temp;
        left++;   // 左指针右移
        right--;  // 右指针左移
    }
}

lamp_sz lampz_to_str_len(const lampz_t z, lamp_sz base) {
    lamp_sz len = lampz_get_len(z);
    lamp_sz res = 0;
    if (base == 2) {
        res = len * LAMPUI_BITS;
    } else if (base == 16) {
        res = len * 8;
    } else {
        if (len <= 1) {
            res = GET_BASE_LEN(base);
        } else if (len == 2) {
            res = 2 * GET_BASE_LEN(base);
        } else {
            res = ceil((double)len * double(LAMPUI_BITS) / log2(base)) + GET_BASE_LEN(base);
        }
    }
    return res + 2; // 加上'\0'和'-'符号位
}

/*
 * @brief 字符串转大整数 z，z 需要的字长
 * @param str_len 输入：字符串长度
 * @param base 输入：进制，2-36
 * @return int 大整数需要的字长
 */
lamp_sz to_lampz_len(lamp_sz str_len, lamp_sz base) {
    lamp_sz bit = ceil(log2(base) * str_len) + 1;
    return (bit + LAMPUI_BITS - 1) / LAMPUI_BITS;
}

/**
 * @brief 小端序二进制字符串数组 -> 64位无符号整数
 * @param bin_array 输入：二进制字符数组（每个元素是'0'或'1'），小端序（索引0=数字低位）
 * @param len 输入：数组长度（≤64）
 * @return uint64_t 转换后的64位无符号整数；非法输入返回0
 */
lamp_ui bin_array_le_to_ui(const char bin_array[], lamp_sz len) {
    assert(bin_array != nullptr && len > 0 && len <= 64);
    lamp_ui result = 0;
    for (lamp_sz i = 0; i < len; ++i) {
        assert(bin_array[i] == '0' || bin_array[i] == '1');
        if (bin_array[i] == '1') {
            result |= (1ULL << i);  // 第i位设为1（1ULL确保无符号64位移位，避免溢出）
        }
    }
    return result;
}

/**
 * @brief 小端序16进制字符数组 -> 64位无符号整数
 * @param hex_array 输入：16进制字符数组（'0'-'9'/'a'-'f'/'A'-'F'），小端序（索引0=数字低位）
 * @param len 输入：数组长度（≤16）
 * @return uint64_t 转换后的64位无符号整数；非法输入返回0
 */
lamp_ui hex_array_le_to_ui(const char hex_array[], lamp_sz len) {
    assert(hex_array != nullptr && len > 0 && len <= 16);

    lamp_ui result = 0;
    for (lamp_sz i = 0; i < len; ++i) {
        char c = hex_array[i];
        lamp_ui hex_val;

        // 2. 16进制字符转数值（区分大小写）
        if (c >= '0' && c <= '9') {
            hex_val = c - '0';  // '0'-'9' → 0-9
        } else if (c >= 'a' && c <= 'f') {
            hex_val = 10 + (c - 'a');  // 'a'-'f' → 10-15
        } else if (c >= 'A' && c <= 'F') {
            hex_val = 10 + (c - 'A');  // 'A'-'F' → 10-15
        } else {
            assert(false && "hex_array_le_to_ui: invalid hex character");
            return 0;
        }
        result += hex_val * (1ULL << (4 * i));
    }
    return result;
}

/**
 * @brief 小端序16进制字符数组 -> 64位无符号整数
 * @param char_array 输入：base进制字符数组（'0'-'9'/'a'-'z'/'A'-'Z'），小端序（索引0=数字低位）
 * @param len 输入：数组长度（≤16）
 * @return uint64_t 转换后的64位无符号整数；非法输入返回0
 */
lamp_ui char_array_le_to_ui(const char char_array[], lamp_sz len, lamp_sz base) {
    assert(base >= 2 && base <= 36);
    assert(char_array != nullptr && len > 0 && len <= GET_BASE_LEN(base));
    lamp_ui result = 0;
    lamp_ui pow_i = 1;
    for (lamp_sz i = 0; i < len; ++i) {
        char c = char_array[i];
        lamp_ui val;
        if (c >= '0' && c <= '9') {
            val = c - '0';  // '0'-'9' → 0-9
        } else if (c >= 'a' && c <= 'a' + char(base - 10)) {
            val = 10 + (c - 'a');  // 'a'-'z' → 10-35
        } else if (c >= 'A' && c <= 'A' + char(base - 10)) {
            val = 10 + (c - 'A');  // 'A'-'Z' → 10-35
        } else {
            assert(false && "char_array_le_to_ui: invalid hex character");
            return 0;
        }
        result += val * pow_i;
        pow_i *= base;
    }
    return result;
}

/**
 * @brief 64位无符号整数 -> 小端序二进制字符数组
 * @param val 输入：要转换的64位无符号整数
 * @param bin_array 输出：二进制字符缓冲区（调用者提供，存储'0'/'1'），小端序（索引0=数字低位）
 * @param len 输入：缓冲区长度（≤64，需≥实际有效位长度，不足补'0'）
 */
void ui_to_bin_array_le(lamp_ui val, char bin_array[], lamp_sz len) {
    assert(bin_array != nullptr && len > 0 && len <= 64);
    for (lamp_sz i = 0; i < len; ++i) {
        bin_array[i] = (val & (1ULL << i)) ? '1' : '0';
    }
}

/**
 * @brief 64位无符号整数 -> 小端序16进制字符数组（小写）
 * @param val 输入：要转换的64位无符号整数
 * @param hex_array 输出：16进制字符缓冲区（调用者提供，存储'0'-'f'），小端序（索引0=数字低位）
 * @param len 输入：缓冲区长度（≤16，需≥实际有效位长度，不足补'0'）
 */
void ui_to_hex_array_le(lamp_ui val, char hex_array[], lamp_sz len) {
    assert(hex_array != nullptr && len > 0 && len <= 16);
    constexpr char hex_chars[] = "0123456789abcdef";
    for (lamp_sz i = 0; i < len; ++i) {
        uint8_t hex_val = (val >> (4 * i)) & lamp_ui(0x0F);
        hex_array[i] = hex_chars[hex_val];
    }
}

/**
 * @brief 64位无符号整数 -> 小端序通用进制字符数组（小写，base=2~36）
 * @param val 输入：要转换的64位无符号整数
 * @param char_array 输出：通用进制字符缓冲区，小端序（索引0=数字低位）
 * @param len 输入：缓冲区长度
 * @param base 输入：转换进制（2~36）
 */
void ui_to_char_array_le(lamp_ui val, char char_array[], lamp_sz len, lamp_sz base) {
    assert(base >= 2 && base <= 36);
    assert(char_array != nullptr && len > 0 && len <= GET_BASE_LEN(base));
    for (lamp_sz i = 0; i < len; ++i) {
        lamp_ui tmp = val % base;  
        char_array[i] = (tmp < 10) ? ('0' + char(tmp)) : ('a' + char(tmp - 10));
        val /= base;
    }
}

void set_bin_str(lampz_t z, const char* str, lamp_sz str_len) {
    assert(__lampz_get_capacity(z) >= (str_len + 63) / 64);
    for (lamp_sz i = 0; i < str_len; i += 64) {
        char const* p = str + i;
        const lamp_ui len = (str_len - i < 64) ? str_len - i : 64;
        const lamp_ui ui = bin_array_le_to_ui(p, len);
        z->begin[i / 64] = ui;
    }
    z->len = (str_len + 63) / 64;
}

void set_hex_str(lampz_t z, const char* str, lamp_sz str_len) {
    for (lamp_sz i = 0; i < str_len; i += 16) {
        char const* p = str + i;
        const lamp_sz len = (str_len - i < 16) ? str_len - i : 16;
        const lamp_ui ui = hex_array_le_to_ui(p, len);
        z->begin[i / 16] = ui;
    }
    z->len = (str_len + 15) / 16;
}

void set_base_str(lampz_t z, const char* str, lamp_sz str_len, lamp_sz base) {
    assert(__lampz_get_capacity(z) >= to_lampz_len(str_len, base));
    const lamp_sz base_len = GET_BASE_LEN(base);  // 进制位数
    for (lamp_sz i = 0; i < str_len; i += base_len) {
        char const* p = str + i;
        const lamp_ui len = (str_len - i < base_len) ? str_len - i : base_len;
        const lamp_ui ui = char_array_le_to_ui(p, len, base);
        z->begin[i / base_len] = ui;
    }
    z->len = (str_len + GET_BASE_LEN(base) - 1 ) / GET_BASE_LEN(base);
}

void get_base_str(lampz_t z, char* str, lamp_sz str_len, lamp_sz base) {
    lamp_sz len = lampz_get_len(z);
    const int base_len = GET_BASE_LEN(base);  // 进制位数
    assert(len * base_len <= str_len);
    for (lamp_sz i = 0; i < len; ++i) {
        const lamp_ui ui = z->begin[i];
        ui_to_char_array_le(ui, str + i * base_len, base_len, base);
    }
}

void get_hex_str(const lampz_t z, char* str, lamp_sz str_len) {
    lamp_ui len = lampz_get_len(z);
    assert(len * 16 <= str_len);
    for (lamp_sz i = 0; i < len; ++i) {
        const lamp_ui ui = z->begin[i];
        ui_to_hex_array_le(ui, str + i * 16, 16);
    }
}

void get_bin_str(const lampz_t z, char* str, lamp_sz str_len) {
    lamp_ui len = lampz_get_len(z);
    assert(len * 64 <= str_len);
    for (lamp_sz i = 0; i < len; ++i) {
        const lamp_ui ui = z->begin[i];
        ui_to_bin_array_le(ui, str + i * 64, 64);
    }
}

void lampz_set_str(lampz_t z, const char* str, lamp_sz str_len, lamp_sz base) {
    assert(base >= 2 && base <= 36 && "set_str: only support base 2-36");
    
    if (str == nullptr || *str == '\0') {
        z->len = 1;
        (z->begin)[0] = 0;
        return;
    }

    char const* endptr = str + str_len;
    --endptr;  // 指向字符串末尾，跳过'\0'
    while (*endptr == '0' && endptr != str) endptr--;  // 去除前导0

    str_len = (lamp_sz)(endptr - str + 1);

    if (str_len == 0) {
        z->len = 1;
        z->begin[0] = 0;
        return;
    }

    const lamp_sz req_word_len = to_lampz_len(str_len, base);  // 计算所需字长
    if (req_word_len > __lampz_get_capacity(z)) {
        __lampz_talloc(z, req_word_len);  // 重新分配内存
    }

    if (base == 2) {
        set_bin_str(z, str, str_len);
        return;
    } else if (base == 16) {
        set_hex_str(z, str, str_len);
        return;
    } else {
        lamp_ui __z_len = str_len / GET_BASE_LEN(base) + 1;
        lampz_t __z;
        __lampz_init(__z);
        __lampz_talloc(__z, __z_len);
        set_base_str(__z, str, str_len, base);
        z->len = lammp::Arithmetic::Numeral::base2binary(__z->begin, __z_len, base, z->begin);
        lampz_free(__z);  // 释放临时大整数
    }
    return;
}

lamp_sz lampz_to_str(char* str, const lamp_sz str_len, const lampz_t z, lamp_sz base) {
    if (base < 2 || base > 36) {
        throw std::invalid_argument("to_str: invalid base");
    }
    if (str == nullptr || str_len < lampz_to_str_len(z, base)) {
        throw std::invalid_argument("to_str: invalid str_len");
    }
    if (lampz_is_nan(z)) {
        return 0;
    }
    if (str_len <= 128) {
        std::fill(str, str + str_len, '0');
    } else {
        std::fill(str + str_len - 128, str + str_len, '0');
    }
    if (base == 2) {
        get_bin_str(z, str, str_len);
    } else if (base == 16) {
        get_hex_str(z, str, str_len);
    } else {
        lampz_t __z_copy;
        __lampz_init(__z_copy);
        lampz_copy(__z_copy, z);
        lamp_sz __z_copy_len = lampz_get_len(__z_copy);
        lamp_sz __out_len = lammp::Arithmetic::Numeral::get_buffer_size(__z_copy_len, GET_BASE_D(base));
        lampz_t __out;
        __lampz_init(__out);
        __lampz_talloc(__out, __out_len);
        __out->len = lammp::Arithmetic::Numeral::binary2base(__z_copy->begin, __z_copy_len, base, __out->begin);
        get_base_str(__out, str, str_len, base);
        lampz_free(__z_copy);  // 释放临时大整数
        lampz_free(__out);  // 释放临时大整数
    }
    lamp_sz __str_len = str_len;
    while (__str_len > 1 && str[__str_len - 1] == '0') __str_len--;  // 去除末尾0
    str[__str_len] = '\0';
    return __str_len;
}