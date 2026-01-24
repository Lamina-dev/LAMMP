/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include "../../../include/lammp/lammp.hpp"
#include "../../../include/lammp/lampz.h"
#include "../../../include/lammp/inter_buffer.hpp"

using namespace lammp::Arithmetic;

static void __lampz_div_common(lampz_t q, lampz_t r, const lampz_t x, const lampz_t y) {
    if (lampz_is_nan(x) || lampz_is_nan(y)) {
        if (q) lampz_free(q);
        if (r) lampz_free(r);
        return;
    }
    if (lampz_is_zero(y)) {
        // Division by zero
        // Do nothing or abort?
        return;
    }
    if (lampz_is_zero(x)) {
        if (q) {
             lampz_new(q, 0); 
             // len is 0 already from lampz_new logic if bit=0? 
             // lampz_new with bit=0 sets len=0 but allocates min len.
             q->len = 0;
        }
        if (r) {
             lampz_new(r, 0);
             r->len = 0;
        }
        return;
    }

    lamp_sz x_len = lampz_get_len(x);
    lamp_sz y_len = lampz_get_len(y);
    lamp_ptr x_ptr = x->begin;
    lamp_ptr y_ptr = y->begin;

    // Use internal buffers to avoid aliasing issues and manage memory automatically
    lammp::_internal_buffer<0> q_buf(x_len + 2, 0); // Allocate enough
    lammp::_internal_buffer<0> r_buf(y_len + 1, 0);
    
    bool q_write = false;
    bool r_write = false;

    if (x_len < y_len) {
        if (q) {
             // q = 0
             q_write = true;
             // q_buf is already 0
        }
        if (r) {
             std::copy(x_ptr, x_ptr + x_len, r_buf.data());
             r_write = true;
        }
    } else {
        // Normalize
        const int shift = lammp::lammp_clz(y_ptr[y_len - 1]);
        lammp::_internal_buffer<0> x_shifted(x_len + 2, 0);
        lammp::_internal_buffer<0> y_shifted(y_len, 0);

        lshift_in_word(x_ptr, x_len, x_shifted.data(), shift);
        lshift_in_word(y_ptr, y_len, y_shifted.data(), shift);

        lamp_sz x_shifted_len = rlz(x_shifted.data(), x_len + 2);
        
        // Output buffers for knuth
        lammp::_internal_buffer<0> knuth_rem(y_len, 0);

        abs_div_knuth(x_shifted.data(), x_shifted_len, y_shifted.data(), y_len, q_buf.data(), knuth_rem.data());
        
        if (q) q_write = true;
        if (r) {
            rshift_in_word(knuth_rem.data(), y_len, r_buf.data(), shift);
            r_write = true;
        }
    }

    if (q && q_write) {
        lamp_sz q_len_real = rlz(q_buf.data(), x_len + 2); // Calculate real len
        if (lampz_new(q, q_len_real * 64)) {
             std::copy(q_buf.data(), q_buf.data() + q_len_real, q->begin);
             q->len = q_len_real;
             if (q->len != 0) {
                 q->len = (lampz_get_sign(x) == lampz_get_sign(y)) ? q->len : -q->len;
             }
        }
    }
    
    if (r && r_write) {
        lamp_sz r_len_real = rlz(r_buf.data(), y_len + 1);
        if (lampz_new(r, r_len_real * 64)) {
            std::copy(r_buf.data(), r_buf.data() + r_len_real, r->begin);
            r->len = r_len_real;
            if (r->len != 0) {
                r->len = (lampz_get_sign(x) == 1) ? r->len : -r->len;
            }
        }
    }
}

void lampz_div_mod_xy(lampz_t q, lampz_t r, const lampz_t x, const lampz_t y) {
    __lampz_div_common(q, r, x, y);
}

void lampz_div_xy(lampz_t z, const lampz_t x, const lampz_t y) {
    __lampz_div_common(z, nullptr, x, y);
}

void lampz_mod_xy(lampz_t z, const lampz_t x, const lampz_t y) {
    __lampz_div_common(nullptr, z, x, y);
}

void lampz_div_x(lampz_t z, const lampz_t x) {
    __lampz_div_common(z, nullptr, z, x);
}

void lampz_mod_x(lampz_t z, const lampz_t x) {
    __lampz_div_common(nullptr, z, z, x);
}
