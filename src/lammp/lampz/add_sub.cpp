/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include "../../../include/lammp/lampz.h"
#include "../../../include/lammp/lammp.hpp"

void lampz_add_xy(lampz_t& z, const lampz_t x, const lampz_t y) {
    bool x_sign = x->len > 0;
    bool y_sign = y->len > 0;
    lamp_ui len_x = lampz_get_len(x);
    lamp_ui len_y = lampz_get_len(y);
    if (len_x == 0 || len_y == 0) {
        lampz_free(z);
        return;
    }
    lamp_ui z_cap = __lampz_get_capacity(z);
    lamp_ui z_len = lammp::Arithmetic::get_add_len(len_x, len_y);
    if (z_cap < z_len) {
        lampz_free(z);
        z = __lampz_talloc(z_len);
    }
    if (x_sign == y_sign) {
        lammp::Arithmetic::abs_add_binary(x->begin, len_x, y->begin, len_y, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = x_sign ? z->len : -z->len;
    } else {
        lamp_si sign = lammp::Arithmetic::abs_difference_binary(x->begin, len_x, y->begin, len_y, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = sign > 0 ? z->len : -z->len;
    }
    return;
}

void lampz_sub_xy(lampz_t& z, const lampz_t x, const lampz_t y) {
    bool x_sign = x->len > 0;
    bool y_sign = y->len > 0;
    lamp_ui len_x = lampz_get_len(x);
    lamp_ui len_y = lampz_get_len(y);
    if (len_x == 0 || len_y == 0) {
        lampz_free(z);
        return;
    }
    lamp_ui z_cap = __lampz_get_capacity(z);
    lamp_ui z_len = lammp::Arithmetic::get_sub_len(len_x, len_y);
    if (z_cap < z_len) {
        lampz_free(z);
        z = __lampz_talloc(z_len);
    }
    if (x_sign == y_sign) {
        lammp::Arithmetic::abs_sub_binary(x->begin, len_x, y->begin, len_y, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = x_sign ? z->len : -z->len;
    } else {
        lamp_si sign = lammp::Arithmetic::abs_difference_binary(x->begin, len_x, y->begin, len_y, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = sign > 0 ? z->len : -z->len;
    }
    return;
}

void lampz_add_x(lampz_t& z, const lampz_t x) {
    bool x_sign = x->len > 0;
    bool z_sign = z->len > 0;
    lamp_ui len_x = lampz_get_len(x);
    lamp_ui len_z = lampz_get_len(z);
    if (len_x == 0 || len_z == 0) {
        lampz_free(z);
        return;
    }
    lamp_ui z_len = lammp::Arithmetic::get_add_len(len_x, len_z);
    __lampz_calloc(z, z_len);
    if (x_sign == z_sign) {
        lammp::Arithmetic::abs_add_binary(x->begin, len_x, z->begin, len_z, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = x_sign ? z->len : -z->len;
    } else {
        lamp_si sign = lammp::Arithmetic::abs_difference_binary(x->begin, len_x, z->begin, len_z, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = sign > 0 ? z->len : -z->len;
    }
    return;
}

void lampz_sub_x(lampz_t& z, const lampz_t x) {
    bool x_sign = x->len > 0;
    bool z_sign = z->len > 0;
    lamp_ui len_x = lampz_get_len(x);
    lamp_ui len_z = lampz_get_len(z);
    if (len_x == 0 || len_z == 0) {
        lampz_free(z);
        return;
    }
    lamp_ui z_len = lammp::Arithmetic::get_sub_len(len_x, len_z);
    __lampz_calloc(z, z_len);
    if (x_sign == z_sign) {
        lammp::Arithmetic::abs_sub_binary(x->begin, len_x, z->begin, len_z, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = x_sign ? z->len : -z->len;
    } else {
        lamp_si sign = lammp::Arithmetic::abs_difference_binary(x->begin, len_x, z->begin, len_z, z->begin);
        z->len = lammp::Arithmetic::rlz(z->begin, z_len);
        z->len = sign > 0 ? z->len : -z->len;
    }
    return;
}