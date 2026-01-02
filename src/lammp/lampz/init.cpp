/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include <algorithm>
#include "../../../include/lammp/lampz.h"

lamp_si lampz_is_zero(const lampz_t z) {
    if (lampz_is_nan(z)) {
        return 0;
    } else if (abs(z->len) == 1 && z->begin[0] == 0) {
        return 1;
    } else {
        return 0;
    }
}

void lampz_set_ui(lampz_t z, lamp_ui value) {
    if (z->begin == nullptr) {
        __lampz_malloc(z, 1);
    }
    z->len = 1;
    z->begin[0] = value;
    return;
}

void lampz_set_si(lampz_t z, lamp_si value) {
    if (z->begin == nullptr) {
        __lampz_malloc(z, 1);
    }
    if (value < 0) {
        z->len = -1;
        z->begin[0] = (lamp_ui)(-value);
    } else {
        z->len = 1;
        z->begin[0] = (lamp_ui)value;
    }
    return;
}

lamp_si lampz_to_si(const lampz_t z) {
    if (lampz_is_nan(z)) {
        return LAMP_SI_MAX;
    }
    const int arr_len = abs(z->len);         
    const int sign = (z->len > 0) ? 1 : -1; 
    if (arr_len != 1) {
        if (z->len == 0) {
            return LAMP_SI_MAX;
        }
        return (sign > 0) ? LAMP_SI_MAX : LAMP_SI_MIN;
    }
    const uint64_t abs_val = z->begin[0];
    if (sign > 0) { 
        if (abs_val > (uint64_t)LAMP_SI_MAX) {
            return LAMP_SI_MAX;
        }
        return (lamp_si)abs_val;
    } else { 
        if (abs_val == 0) {
            return 0;
        }
        if (abs_val == (uint64_t)LAMP_SI_MIN) {
            return LAMP_SI_MIN;
        }
        if (abs_val > (uint64_t)LAMP_SI_MAX) {
            return LAMP_SI_MIN;
        }
        return -(lamp_si)abs_val;
    }
    return LAMP_SI_MAX;
}

lamp_ui lampz_to_ui(const lampz_t z) {
    if (lampz_is_nan(z) || z->len > 1) {
        return LAMP_UI_MAX;
    }
    if (z->len < 0) {
        return 0;
    }
    return z->begin[0];
}

void lampz_copy(lampz_t z1, const lampz_t z2) {
    lamp_sz z2_len = lampz_get_len(z2);
    if (__lampz_get_capacity(z1) < z2_len) {
        __lampz_malloc(z1, z2_len);
    }
    z1->len = z2->len;
    std::copy(z2->begin, z2->begin + z2_len, z1->begin);
    return;
}

void lampz_move(lampz_t z1, lampz_t z2) {
    if (z1 == z2) {
        return;
    }
    if (z1 != nullptr) {
        LAMMP_FREE(z1->begin);
        z1->begin = z2->begin;
        z1->end = z2->end;
        z1->len = z2->len;
        z2->begin = nullptr;
        z2->end = nullptr;
        z2->len = 0;
        return;
    }
    return;
}

void lampz_swap(lampz_t z1, lampz_t z2) {
    std::swap(z1->len, z2->len);
    std::swap(z1->begin, z2->begin);
    std::swap(z1->end, z2->end);
    return;
}