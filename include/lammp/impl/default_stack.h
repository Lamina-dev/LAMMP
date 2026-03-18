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
#ifndef __LAMMP_DEFAULT_STACK_H__
#define __LAMMP_DEFAULT_STACK_H__

/**
 * @brief 默认栈获取栈顶
 */
static void* lmmp_default_stack_get_top(void);

/**
 * @brief 默认栈设置栈顶
 */
static void lmmp_default_stack_set_top(void* top);

#endif // __LAMMP_DEFAULT_STACK_H__