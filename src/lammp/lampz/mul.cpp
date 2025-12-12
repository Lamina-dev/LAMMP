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

void lampz_mul_xy(lampz_t &z, const lampz_t x, const lampz_t y) {
    lamp_ui len_x = lampz_get_len(x);
    lamp_ui len_y = lampz_get_len(y);
    if (len_x == 0 || len_y == 0) {
        lampz_free(z);
        return;
    }
    lamp_ui z_cap = __lampz_get_capacity(z);
    lamp_ui len_z = lammp::Arithmetic::get_mul_len(len_x, len_y);
    if (len_z > z_cap) {
        lampz_free(z);
        z = __lampz_talloc(len_z);
    }
    lammp::Arithmetic::abs_mul64(x->begin, len_x, y->begin, len_y, z->begin);
    z->len = lammp::Arithmetic::rlz(z->begin, len_z);
    bool sign = (lampz_get_sign(x) ^ lampz_get_sign(y));
    z->len = sign ? -z->len : z->len;
    return;
}

void lampz_mul_x(lampz_t& z, const lampz_t x) {
    lamp_ui len_x = lampz_get_len(x);
    lamp_ui len_z = lampz_get_len(z);
    if (len_x == 0) {
        lampz_free(z);
        return;
    }
    lamp_ui z_cap = __lampz_get_capacity(z);
    lamp_ui z_len = lammp::Arithmetic::get_mul_len(len_z, len_x);
    if (z_len > z_cap) {
        lampz_free(z);
        z = __lampz_talloc(z_len);
    }
    lammp::Arithmetic::abs_mul64(x->begin, len_x, z->begin, len_z, z->begin);
    z->len = lammp::Arithmetic::rlz(z->begin, z_len);
    bool sign = (lampz_get_sign(x) ^ lampz_get_sign(z));
    z->len = sign ? -z->len : z->len;
    return;
}