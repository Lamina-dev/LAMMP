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
    if (z == nullptr) {
        return 0;
    } else if (abs(z->len) == 0){
        return 0;
    } else if (abs(z->len) == 1 && z->begin[0] == 0) {
        return 1;
    } else {
        return -1;
    }
}

void lampz_set_ui(lampz_t& z, lamp_ui value) {
    if (z == nullptr) {
        z = __lampz_malloc(1);
    }
    z->len = 1;
    z->begin[0] = value;
    return;
}

void lampz_set_si(lampz_t& z, lamp_si value) {
    if (z == nullptr) {
        z = __lampz_malloc(1);
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

void lampz_copy(lampz_t &z1, const lampz_t z2) {
    if (z2 == nullptr) {
        z1 = nullptr;
        return;
    }
    if (z1 == nullptr) {
        z1 = __lampz_malloc(z2->len);
    }
    z1->len = z2->len;
    std::copy(z2->begin, z2->end, z1->begin);
    return;
}

void lampz_move(lampz_t &z1, lampz_t &z2) {
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
    }
    z1 = z2;
    z2 = nullptr;
    return;
}

void lampz_swap(lampz_t &z1, lampz_t &z2) {
    std::swap(z1, z2);
}