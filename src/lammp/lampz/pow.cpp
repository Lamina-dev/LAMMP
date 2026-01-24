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

void lampz_sqr_x(lampz_t z, const lampz_t x) {
    if (lampz_is_nan(x) || lampz_is_nan(z)) {
        if (z) lampz_free(z);
        return;
    }
    // TODO: implement optimized squaring
    // Current fallback to multiplication
    lampz_mul_xy(z, x, x);
}

void lampz_pow(lampz_t result, const lampz_t base, const lampz_t exponent) {
    if (lampz_is_nan(result) || lampz_is_nan(base) || lampz_is_nan(exponent)) {
        return;
    }

    if (lampz_is_zero(exponent)) {
        // x^0 = 1
        lampz_set_ui(result, 1);
        return;
    }
    
    if (lampz_is_zero(base)) {
        // 0^x = 0 (x > 0)
        lampz_set_ui(result, 0);
        return;
    }

    // Binary Exponentiation (Left-to-Right)
    // We scan bits from MSB to LSB.
    // However, getting MSB requires scanning words.
    
    // Copy base to temp because result might alias base
    lampz_t b;
    __lampz_init(b);
    lampz_copy(b, base);
    
    lampz_set_ui(result, 1);
    
    lamp_sz exp_len = lampz_get_len(exponent);
    // Iterate from most significant word
    bool started = false;
    
    for (lamp_si i = exp_len - 1; i >= 0; --i) {
        lamp_ui word = exponent->begin[i];
        // Iterate bits in word (63 down to 0)
        for (int j = 63; j >= 0; --j) {
            bool bit = (word >> j) & 1;
            if (!started) {
                if (bit) {
                    started = true;
                    // First 1 bit: result = base
                    lampz_copy(result, b);
                }
            } else {
                // Square
                lampz_sqr_x(result, result); // result = result * result
                if (bit) {
                    // Multiply
                    lampz_mul_x(result, b); // result = result * base
                }
            }
        }
    }
    
    lampz_free(b);
}
