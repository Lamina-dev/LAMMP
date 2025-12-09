/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is a part of the LAMMP package.
 * you can see more details about LAMMP at:
 * <https://github.com/Lamina-dev/LAMMP>
 */

#include "../../../include/lammp/lammp.hpp"
#include "../../../include/lammp/lampz.h"
#include "../../../include/lammp/numeral_table.h"

#define GET_BASE_LEN(base) lammp::Arithmetic::Numeral::BaseTable::table1[base - 2][1]
#define GET_BASE_D(base) lammp::Arithmetic::Numeral::BaseTable::table2[base - 2]

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

/*
 * @brief 大整数 z 转字符串，字符串需要的长度
 * @param z 输入：大整数
 * @param base 输入：进制，2-36
 * @return int 字符串长度
 */
lamp_ui lampz_to_str_len(const lampz_t z, size_t base) {
    lamp_ui len = lampz_get_len(z);
    lamp_ui res = 0;
    if (base == 2) {
        res = len * LAMPUI_BITS;
    } else if (base == 16) {
        res = len * 8;
    } else {
        if (len <= 1) {
            res = GET_BASE_LEN(base);
        } else if (len <= 2) {
            res = 2 * GET_BASE_LEN(base);
        } else {
            res = ceil((double)len * double(LAMPUI_BITS) / log2(base)) + GET_BASE_LEN(base);
        }
    }
    return res;
}

/*
 * @brief 字符串转大整数 z，z 需要的字长
 * @param str_len 输入：字符串长度
 * @param base 输入：进制，2-36
 * @return int 大整数需要的字长
 */
lamp_ui to_lampz_len(size_t str_len, size_t base) {
    lamp_ui bit = ceil(log2(base) * str_len) + 1;
    return (bit + LAMPUI_BITS - 1) / LAMPUI_BITS;
}

/**
 * @brief 小端序二进制字符串数组 -> 64位无符号整数
 * @param bin_array 输入：二进制字符数组（每个元素是'0'或'1'），小端序（索引0=数字低位）
 * @param len 输入：数组长度（≤64）
 * @return uint64_t 转换后的64位无符号整数；非法输入返回0
 */
lamp_ui bin_array_le_to_ui(const char bin_array[], size_t len) {
    assert(bin_array != nullptr && len > 0 && len <= 64);
    lamp_ui result = 0;
    for (size_t i = 0; i < len; ++i) {
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
lamp_ui hex_array_le_to_ui(const char hex_array[], size_t len) {
    assert(hex_array != nullptr && len > 0 && len <= 16);

    lamp_ui result = 0;
    for (size_t i = 0; i < len; ++i) {
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
lamp_ui char_array_le_to_ui(const char char_array[], size_t len, size_t base) {
    assert(base >= 2 && base <= 36);
    assert(char_array != nullptr && len > 0 && len <= GET_BASE_LEN(base));
    lamp_ui result = 0;
    lamp_ui pow_i = 1;
    for (size_t i = 0; i < len; ++i) {
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
void ui_to_bin_array_le(lamp_ui val, char bin_array[], size_t len) {
    assert(bin_array != nullptr && len > 0 && len <= 64);
    for (size_t i = 0; i < len; ++i) {
        bin_array[i] = (val & (1ULL << i)) ? '1' : '0';
    }
}

/**
 * @brief 64位无符号整数 -> 小端序16进制字符数组（小写）
 * @param val 输入：要转换的64位无符号整数
 * @param hex_array 输出：16进制字符缓冲区（调用者提供，存储'0'-'f'），小端序（索引0=数字低位）
 * @param len 输入：缓冲区长度（≤16，需≥实际有效位长度，不足补'0'）
 */
void ui_to_hex_array_le(lamp_ui val, char hex_array[], size_t len) {
    assert(hex_array != nullptr && len > 0 && len <= 16);
    constexpr char hex_chars[] = "0123456789abcdef";

    for (size_t i = 0; i < len; ++i) {
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
void ui_to_char_array_le(lamp_ui val, char char_array[], lamp_ui len, lamp_ui base) {
    assert(base >= 2 && base <= 36);
    assert(char_array != nullptr && len > 0 && len <= GET_BASE_LEN(base));
    for (size_t i = 0; i < len; ++i) {
        lamp_ui tmp = val % base;  
        char_array[i] = (tmp < 10) ? ('0' + char(tmp)) : ('a' + char(tmp - 10));
        val /= base;
    }
}

void set_bin_str(lampz_t z, const char* str, lamp_ui str_len) {
    assert(__lampz_get_capacity(z) >= (str_len + 63) / 64);
    for (lamp_ui i = 0; i < str_len; i += 64) {
        char const* p = str + i;
        const lamp_ui len = (str_len - i < 64) ? str_len - i : 64;
        const lamp_ui ui = bin_array_le_to_ui(p, len);
        z->begin[i / 64] = ui;
    }
    z->len = (str_len + 63) / 64;
}

void set_hex_str(lampz_t z, const char* str, lamp_ui str_len) {
    for (lamp_ui i = 0; i < str_len; i += 16) {
        char const* p = str + i;
        const lamp_ui len = (str_len - i < 16) ? str_len - i : 16;
        const lamp_ui ui = hex_array_le_to_ui(p, len);
        z->begin[i / 16] = ui;
    }
    z->len = (str_len + 15) / 16;
}

void set_base_str(lampz_t z, const char* str, lamp_ui str_len, lamp_ui base) {
    assert(__lampz_get_capacity(z) >= to_lampz_len(str_len, base));
    const lamp_ui base_len = GET_BASE_LEN(base);  // 进制位数
    for (lamp_ui i = 0; i < str_len; i += base_len) {
        char const* p = str + i;
        const lamp_ui len = (str_len - i < base_len) ? str_len - i : base_len;
        const lamp_ui ui = char_array_le_to_ui(p, len, base);
        z->begin[i / base_len] = ui;
    }
    z->len = (str_len + GET_BASE_LEN(base) - 1 ) / GET_BASE_LEN(base);
}

void get_base_str(lampz_t z, char* str, lamp_ui str_len, lamp_ui base) {
    lamp_ui len = lampz_get_len(z);
    const int base_len = GET_BASE_LEN(base);  // 进制位数
    assert(len * base_len <= str_len);
    for (lamp_ui i = 0; i < len; ++i) {
        const lamp_ui ui = z->begin[i];
        ui_to_char_array_le(ui, str + i * base_len, base_len, base);
    }
}

void get_hex_str(lampz_t z, char* str, size_t str_len) {
    lamp_ui len = lampz_get_len(z);
    assert(len * 16 <= str_len);
    for (size_t i = 0; i < len; ++i) {
        const lamp_ui ui = z->begin[i];
        ui_to_hex_array_le(ui, str + i * 16, 16);
    }
}

void get_bin_str(lampz_t z, char* str, size_t str_len) {
    lamp_ui len = lampz_get_len(z);
    assert(len * 64 <= str_len);
    for (size_t i = 0; i < len; ++i) {
        const lamp_ui ui = z->begin[i];
        ui_to_bin_array_le(ui, str + i * 64, 64);
    }
}

/**
 * @brief 将二进制字符串赋值给大整数（64位小端数组存储）
 * @param z 目标大整数（必须已通过 lampz_new 创建，且字长足够）
 * @param str 二进制字符串（可保留前导零，默认小端序）
 * @param base 必须为2（二进制）
 * @note 1. 字符串无效/空时，z 设为0；2. 自动处理符号（z的符号由原有 sign 字段决定，字符串仅含数值位）
 */
void lampz_set_str(lampz_t& z, const char* str, lamp_ui base) {
    assert(base >= 2 && base <= 36 && "set_str: only support base 2-36");
    
    if (str == nullptr || *str == '\0') {
        z->len = 1;
        *z->begin = 0;
        return;
    }

    char const* endptr = str + strlen(str);
    --endptr;  // 指向字符串末尾，跳过'\0'
    while (*endptr == '0' && endptr != str) endptr--;  // 去除前导0

    const lamp_ui str_len = (lamp_ui)(endptr - str + 1);

    if (str_len == 0) {
        z->len = 1;
        *z->begin = 0;
        return;
    }

    const lamp_ui req_word_len = to_lampz_len(str_len, base);  // 计算所需字长（含符号位）
    if (req_word_len > __lampz_get_capacity(z)) {
        lampz_free(z);  // 字长不足，释放并返回
        z = __lampz_talloc(req_word_len);  // 重新分配内存
    }

    if (base == 2) {
        set_bin_str(z, str, str_len);
        return;
    } else if (base == 16) {
        set_hex_str(z, str, str_len);
        return;
    } else {
        lamp_ui __z_len = str_len / GET_BASE_LEN(base) + 1;
        lampz_t __z = __lampz_talloc(__z_len);
        set_base_str(__z, str, str_len, base);
        z->len = lammp::Arithmetic::Numeral::base2binary(__z->begin, __z_len, base, z->begin);
        lampz_free(__z);  // 释放临时大整数
    }
    return;
}

lamp_ui lampz_to_str(char* str, const lamp_ui str_len, const lampz_t z, lamp_ui base) {
    assert(base >= 2 && base <= 36 && "to_str: only support base 2-36");
    if (str == nullptr || str_len < lampz_to_str_len(z, base)) {
        throw std::invalid_argument("to_str: invalid str_len");
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
        lampz_t __z_copy = nullptr;
        lampz_copy(__z_copy, z);
        lamp_ui __z_copy_len = lampz_get_len(__z_copy);
        lamp_ui __out_len = lammp::Arithmetic::Numeral::get_buffer_size(__z_copy_len, GET_BASE_D(base));
        lampz_t __out = __lampz_talloc(__out_len);
        __out->len = lammp::Arithmetic::Numeral::binary2base(__z_copy->begin, __z_copy_len, base, __out->begin);
        get_base_str(__out, str, str_len, base);
        lampz_free(__z_copy);  // 释放临时大整数
        lampz_free(__out);  // 释放临时大整数
    }
    lamp_ui __str_len = str_len;
    while (__str_len > 1 && str[__str_len - 1] == '0') __str_len--;  // 去除末尾0
    str[__str_len] = '\0';
    return __str_len;
}