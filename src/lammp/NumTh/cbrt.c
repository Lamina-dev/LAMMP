/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/inlines.h"
#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/lmmpn.h"


/**
 * @brief 计算 [numa, na] 的立方
 * @param dst 目标数组（3*na个limb）
 * @param numa 源数组
 * @param na 源数组的长度
 * @param tp 临时数组（2*na个limb）
 * @return 返回结果的数组长度
 */
static inline mp_size_t lmmp_cube_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_ptr restrict tp) {
    lmmp_sqr_(tp, numa, na);
    lmmp_mul_(dst, tp, 2 * na, numa, na);
    na *= 3;
    while (na > 1 && dst[na - 1] == 0) --na;
    return na;
}