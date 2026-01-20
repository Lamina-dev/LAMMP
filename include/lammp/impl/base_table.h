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

#ifndef __LAMMP_BASE_TABLE_H__
#define __LAMMP_BASE_TABLE_H__
#include "../lmmpn.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INLINE_
#define INLINE_ static inline
#endif

extern const mp_base_t lmmp_bases_[257];

/**
 * @brief 计算大数的基数位数（用于字符串转换）
 * @param numa 输入大数，长度为na
 * @param na 大数的单精度数(limb)长度
 * @param base 目标基数（2~256）
 * @return 大数在指定基数下的位数
 * @note 先找到最高非零位，再结合基数参数计算位数
 */
INLINE_ mp_size_t lmmp_digits_(mp_srcptr numa, mp_size_t na, int base) {
    int mslbits = 0;
    if (numa) {
        do {
            if (na == 0)
                return 1;
        } while (numa[--na] == 0);
        mslbits = lmmp_limb_bits_(numa[na]);
    }
    return lmmp_mulh_(na * 64 + mslbits, lmmp_bases_[base].inv_lg_base) + 1;
}

/**
 * @brief 计算字符串转大数所需的limb数
 * @param src 输入字符串指针
 * @param len 字符串长度
 * @param base 字符串的基数（2~256）
 * @return 存储该字符串数值所需的最小limb数
 * @note 先找到最后一个非零字符，再结合基数参数计算limb数
 */
INLINE_ mp_size_t lmmp_limbs_(const mp_byte_t* src, mp_size_t len, int base) {
    if (src) {
        do {
            if (len == 0)
                return 1;
        } while (src[--len] == 0);
        ++len;
    }
    return lmmp_mulh_(len, lmmp_bases_[base].lg_base) + 1;
}

#ifdef INLINE_
#undef INLINE_
#endif

#ifdef __cplusplus
}
#endif

#endif // __LAMMP_BASE_TABLE_H__