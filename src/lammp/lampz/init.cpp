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