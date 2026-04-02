/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#ifndef __LAMMP_TEST_SHORT_H__
#define __LAMMP_TEST_SHORT_H__
#include <iostream>
#include "../../../include/lammp/impl/_umul64to128_.h"
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/mprand.h"
#include "../../../include/lammp/matrix.h"


void test_add_nc();
void test_umul64to128();
void test_sqrt();
int test_pow();
void test_powmod();
void test_isprime_ulong();
void test_factorial();
void test_multinomial();
void test_perm();
void test_bino();
void test_gcd();
void test_gcd2();
void test_rand();
void test_hash();
void test_matrix();
void test_vec_ele_mul();
void test_mod();
void test_mullo();
void test_arith_seqprod();
void test_bninv();

#endif // __LAMMP_TEST_SHORT_H__