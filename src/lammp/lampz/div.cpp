/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include "../../../include/lammp/lammp.hpp"
#include "../../../include/lammp/lampz.h"

void lampz_div_x(lampz_t z, const lampz_t x) {
    if (lampz_is_nan(x) || lampz_is_nan(z) || lampz_is_zero(x)) {
        lampz_free(z);
        return;
    }
    lamp_sz x_len = lampz_get_len(x);
    lamp_sz z_len = lampz_get_len(z);
    lamp_si x_sign = lampz_get_sign(x);
    lamp_si z_sign = lampz_get_sign(z);
    if (z_len < x_len) {
        memset(z->begin, 0, z_len * sizeof(lamp_ui));
        z->len = 1;
        return;
    }
    lammp::Arithmetic::abs_div64(z->begin, z_len, x->begin, x_len, z->begin);
    z_len = lammp::Arithmetic::rlz(z->begin, lammp::Arithmetic::get_div_len(z_len, x_len));
    if (z_sign == x_sign) {
        z->len = z_len;
    } else {
        z->len = -z_len;
    }
}