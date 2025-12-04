/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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


