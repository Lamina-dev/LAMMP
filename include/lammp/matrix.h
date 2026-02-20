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

#ifndef LAMMP_MATRIX_H
#define LAMMP_MATRIX_H

/*
 一些约定：
        mat表示矩阵，为matrix简称。umat表示unsigned matrix，u表示unsigned
        smat表示signed matrix，s表示signed。由于我们只对有符号的进行计算，大部分场合
        都使用signed matrix，所以我们用mat表示signed mat。数字字面量或者m,n表示行列数，
        单数字字面量或者n表示为方阵的阶数。
        
        vec表示向量，vector简称。uvec表示unsigned vector，u表示unsigned。
        svec表示signed vector，s表示signed。由于我们只对有符号的进行计算，大部分场合
        都使用signed vector，所以我们用vec表示signed vec。数字字面量或者n表示向量长度。
        向量参与运算时默认为列向量，等价于矩阵的列。

        我们使用有符号整数类型表示limb长度，符号表明这个数的正负性，指针存储的是绝对值。
        同时需要注意的是，在此处，mp_ptr若为空和limb长度为0，这两个情况称之为语义0，
        计算行为等价为0。同时，需要注意的是，在无明确要求的情况下，我们不希望指针非空的
        同时limb长度为0，因为这可能导致未定义行为。

        矩阵或向量的各个元素的地址我们并不要求完全分离或相同，当sep或eqsep用于矩阵或向量时，
        语义和意义与sep或eqsep的一般用法相同。sep表示mat和vec指针分离，eqsep表示mat和vec
        指针分离或相同，elemsep表示mat或vec各元素指针分离，eqelemsep表示mat或vec各元素指针
        分离或相同。

        nonull()表示矩阵或向量中的各元素全部非空

*/

#include "lmmp.h"
#include <stdio.h>
// 元素连乘朴素连乘空间长度
#define LIMB_ELEMMUL_MP_THRESHOLD 20

// 向量连乘朴素连乘空间长度
#define VEC_ELEMMUL_MP_THRESHOLD 40

// 2x2矩阵乘法选择STRASSEN算法的阈值
#define MAT22_MUL_STRASSEN_THRESHOLD 60

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t mat_size_t;

typedef struct {
    mp_ptr a00;
    mp_ptr a01;
    mp_ptr a10;
    mp_ptr a11;
    mp_ssize_t n00;
    mp_ssize_t n01;
    mp_ssize_t n10;
    mp_ssize_t n11;
} lmmp_mat22_t;
typedef lmmp_mat22_t lmmp_smat22_t;

typedef struct {
    mp_ptr a0;
    mp_ptr a1;
    mp_ssize_t n0;
    mp_ssize_t n1;
} lmmp_mat21_t;
typedef lmmp_mat21_t lmmp_vec2_t;
typedef lmmp_mat21_t lmmp_smat21_t;
typedef lmmp_mat21_t lmmp_svec2_t;

typedef struct {
    mp_ptr* num;
    mp_ssize_t* len;
    mat_size_t n;
} lmmp_svecn_t;
typedef lmmp_svecn_t lmmp_matn1_t;
typedef lmmp_svecn_t lmmp_vecn_t;

/**
 * @brief 计算向量的累乘
 * @param dst 结果向量，将会被覆盖为累乘结果指针，将会自动分配内存
 * @param vec 被累乘向量
 * @warning dst!=NULL, vec!=NULL, vec->num!=NULL, vec->len!=NULL, vec->n>0
 * @note 当vec存在语义0时，*dst将会被置为NULL，并返回0。其余情况，*dst会被置为结果指针，并返回实际长度。
 * @return 结果dst的实际长度（为负数表示此数为负数，绝对值表示实际长度）
 */
mp_ssize_t lmmp_vec_elem_mul_(mp_ptr* dst, lmmp_vecn_t* vec);

/**
 * @brief 计算limb向量的累乘
 * @param dst 结果向量，将会被覆盖为累乘结果指针，将会自动分配内存
 * @param vec 被累乘向量
 * @warning dst!=NULL, vec!=NULL
 * @note 当limb数组存在0值时，*dst将会被置为NULL，并返回0。其余情况，*dst会被置为结果指针，并返回实际长度。
 * @return 结果dst的实际长度
 */
mp_size_t lmmp_limb_elem_mul_(mp_ptr* dst, mp_limb_t* limb, mp_size_t n);

/**
 * @brief 计算slimb向量的累乘
 * @param dst 结果向量，将会被覆盖为累乘结果指针，将会自动分配内存
 * @param vec 被累乘向量
 * @warning dst!=NULL, vec!=NULL
 * @note 当limb数组存在0值时，*dst将会被置为NULL，并返回0。其余情况，*dst会被置为结果指针，并返回实际长度。
 * @return 结果dst的实际长度（为负数表示此数为负数，绝对值表示实际长度）
 */
mp_ssize_t lmmp_slimb_elem_mul_(mp_ptr* dst, mp_slimb_t* slimb, mp_size_t n);

/**
 * @brief 计算2x2矩阵和向量的乘积
 * @param dst 结果向量，dst内的内存请调用者自行分配，自行保证拥有足够的空间
 * @param mat 2x2矩阵
 * @param vec 2x1向量
 * @return 无，结果保存在dst中
 * @warning dst!=NULL, mat!=NULL, vec!=NULL, nonull(mat), nonull(vec), nonull(dst), eqsep(dst, vec)
 */
void lmmp_mat22_mul_vec2_(lmmp_vec2_t* dst, const lmmp_mat22_t* mat, const lmmp_vec2_t* vec);

int lmmp_mat22_mul_size_(lmmp_mat22_t* dst, const lmmp_mat22_t* matA, const lmmp_mat22_t* matB, mp_size_t* tn, mp_size_t* maxa);

void lmmp_mat22_mul_basecase_(lmmp_mat22_t* dst, const lmmp_mat22_t* matA, const lmmp_mat22_t* matB, mp_ptr tp,
                              mp_size_t tn);

void lmmp_mat22_mul_strassen_(lmmp_mat22_t* dst, const lmmp_mat22_t* matA, const lmmp_mat22_t* matB, mp_size_t tn,
                              mp_size_t maxa);

void lmmp_mat22_mul_(lmmp_mat22_t* dst, const lmmp_mat22_t* matA, const lmmp_mat22_t* matB, int choose, mp_size_t tn);

void lmmp_mat22_sqr_(lmmp_mat22_t* dst, const lmmp_mat22_t* mat);


#ifdef __cplusplus
}
#endif

#endif // LAMMP_MATRIX_H