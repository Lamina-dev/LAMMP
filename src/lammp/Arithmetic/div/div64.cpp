/*
 * Copyright (C) 2025 HJimmyK/LAMINA
 *
 * This file is part of LAMMP, which is licensed under the GNU LGPL v2.1.
 * See the LICENSE file in the project root for full license details, or visit:
 * <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>
 */

#include "../../../../include/lammp/inter_buffer.hpp"
#include "../../../../include/lammp/lammp.hpp"

namespace lammp::Arithmetic {

void abs_div64(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr qr) {
    assert(in1 != nullptr && in2 != nullptr && qr != nullptr);
    len1 = rlz(in1, len1);
    len2 = rlz(in2, len2);
    assert(len2 > 0);
    if (in1 == in2) {
        qr[0] = 1;
        return;
    }
    if (len1 < len2) {
        qr[0] = 0;
        return;
    }
    const int shift = lammp::lammp_clz(in2[len2 - 1]);
    _internal_buffer<0> _in1_shifted(len1 + 2);
    _in1_shifted.set(len1 + 1, 0);
    _in1_shifted.set(len1, 0);
    _internal_buffer<0> _in2_shifted(len2);
    lshift_in_word(in1, len1, _in1_shifted.data(), shift);
    lshift_in_word(in2, len2, _in2_shifted.data(), shift);
    lamp_ui len1_shifted = rlz(_in1_shifted.data(), len1 + 2);

    abs_div_knuth(_in1_shifted.data(), len1_shifted, _in2_shifted.data(), len2, qr, nullptr);
}

}; // namespace lammp::Arithmetic
