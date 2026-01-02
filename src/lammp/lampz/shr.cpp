/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include "../../../include/lammp/lammp.hpp"
#include "../../../include/lammp/lampz.h"


void lampz_lshift_x(lampz_t z, const lampz_t x, const lamp_sz shift) {
    if (lampz_is_nan(x)) {
        lampz_free(z);
        return;
    }
    lamp_sz shift_word = shift / 64;
    lamp_sz shift_bits = shift % 64;
    lamp_sz len = lampz_get_len(x);
    lamp_si sign = lampz_get_sign(x);
    lamp_sz new_len = len + shift_word + 1;
    if (new_len > __lampz_get_capacity(z)) {
        __lampz_talloc(z, new_len);
    }
    if (shift_bits == 0) {
        std::copy(x->begin, x->begin + len, z->begin + shift_word);
    } else {
        lammp::Arithmetic::lshift_in_word(x->begin, len, z->begin + shift_word, shift_bits);
    }
    std::fill(z->begin, z->begin + shift_word, 0);
    z->len = lammp::Arithmetic::rlz(z->begin, new_len);
    z->len *= sign;
    return;
}

void lampz_rshift_x(lampz_t z, const lampz_t x, const lamp_sz shift) {
    if (lampz_is_nan(x)) {
        lampz_free(z);
        return;
    }
    lamp_sz shift_word = shift / 64;
    lamp_sz shift_bits = shift % 64;
    lamp_sz len = lampz_get_len(x);
    lamp_si sign = lampz_get_sign(x);
    if (shift_bits == 0) {
        std::copy(x->begin + shift_word, x->begin + len, z->begin);
    } else {
        lammp::Arithmetic::rshift_in_word(x->begin + shift_word, len - shift_word, z->begin, shift_bits);
    }
    z->len = lammp::Arithmetic::rlz(z->begin, len - shift_word);
    z->len *= sign;
    return;
}

void lampz_lshift(lampz_t z, const lamp_sz shift) {
    if (lampz_is_nan(z)) {
        return;
    }
    lamp_sz shift_word = shift / 64;
    lamp_sz shift_bits = shift % 64;
    lamp_sz len = lampz_get_len(z);
    lamp_si sign = lampz_get_sign(z);
    lamp_sz new_len = len + shift_word + 1;
    if (new_len > __lampz_get_capacity(z)) {
        __lampz_talloc(z, new_len);
    }
    if (shift_bits == 0) {
        std::copy(z->begin, z->begin + len, z->begin + shift_word);
    } else {
        lammp::Arithmetic::lshift_in_word(z->begin, len, z->begin + shift_word, shift_bits);
    }
    std::fill(z->begin, z->begin + shift_word, 0);
    z->len = lammp::Arithmetic::rlz(z->begin, new_len);
    z->len *= sign;
    return;
}

void lampz_rshift(lampz_t z, const lamp_sz shift) {
    if (lampz_is_nan(z)) {
        return;
    }
    lamp_sz shift_word = shift / 64;
    lamp_sz shift_bits = shift % 64;
    lamp_sz len = lampz_get_len(z);
    lamp_si sign = lampz_get_sign(z);
    if (shift_bits == 0) {
        std::copy(z->begin + shift_word, z->begin + len, z->begin);
    } else {
        lammp::Arithmetic::rshift_in_word(z->begin + shift_word, len - shift_word, z->begin, shift_bits);
    }
    z->len = lammp::Arithmetic::rlz(z->begin, len - shift_word);
    z->len *= sign;
    return;
}