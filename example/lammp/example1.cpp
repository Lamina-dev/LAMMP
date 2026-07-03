/**
 *  Copyright (C) 2026 HJimmyK(Jericho Knox)
 *
 *  This file is part of LAMMP.
 *
 *  LAMMP is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser General Public License (LGPL) as published
 *   by the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed WITHOUT ANY WARRANTY.
 *
 *  See <https://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <chrono>
#include "../../include/lammp/lmmpn.h"
#include "../../include/lammp/mprand.h"

int main() {
    lmmp_global_init(); // 初始化LAMMP线程或进程环境

    mp_size_t len1 = 50000, len2 = len1 * 500;
    mp_size_t len = len1 + len2;

    // 分配空间，如果失败将会直接触发lmmp_abort()
    mp_ptr a = (mp_ptr)lmmp_alloc(len1 * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_alloc(len2 * sizeof(mp_limb_t));

    len1 = lmmp_random_(a, len1);
    len2 = lmmp_random_(b, len2);

    mp_ptr c = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    auto start = std::chrono::high_resolution_clock::now();
    // 调用不平衡乘法算子
    lmmp_mul_(c, b, len2, a, len1);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "multiplication time: " << duration.count() << " microseconds" << std::endl;
    // 释放空间
    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    lmmp_global_deinit(); // 销毁LAMMP线程或进程环境（可以调用lmmp_global_init()重建环境）
    return 0;
}
