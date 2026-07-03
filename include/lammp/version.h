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

#ifndef LAMMP_VERSION_H
#define LAMMP_VERSION_H

#include "lmmp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取 LAMMP 的版本字符串.
 */
LAMMP_API const char* lmmp_get_version(void);

/**
* @brief 获取 LAMMP 的构建类型字符串.
*/
LAMMP_API const char* lmmp_get_build_type(void);

#ifdef __cplusplus
}
#endif

#endif /* LAMMP_VERSION_H */
