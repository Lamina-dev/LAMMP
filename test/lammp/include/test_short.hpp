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

#ifndef __LAMMP_TEST_SHORT_H__
#define __LAMMP_TEST_SHORT_H__
#include <iostream>
#include "../../../include/lammp/lmmpn.h"
#include "../../../include/lammp/numth.h"
#include "../../../include/lammp/mprand.h"


void test_add_nc();
void test_sqrt();
int test_pow();
void test_powmod();
void test_isprime_uint();
void test_isprime_ulong();
void test_next_prime();
void test_factorial();
void test_2factorial();
void test_hyperfac();
void test_superfac();
void test_primefac();
void test_multinomial();
void test_perm();
void test_bino();
void test_gcd();
void test_gcd2();
void test_rand();
void test_hash();
void test_mod();
void test_mullo();
void test_arith_seqprod();
void test_bninv();
void test_trialdiv();
void test_remove();
void test_binvert();
void test_binvert_unbalanced();
void test_divexact();
void test_divexact_basecase();
void test_divexact_unbalanced();

#endif // __LAMMP_TEST_SHORT_H__