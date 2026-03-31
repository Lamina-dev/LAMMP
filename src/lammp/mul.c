/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../include/lammp/lmmpn.h"

/*
        Areas where the different toom algorithms can be called
0    1/6  1/5 1/4  1/3 2/5  1/2 5/9 3/5 2/3 3/4 4/5  9/10   1  nb/na

                             (-------------------(xxxxxxxxxx|  toom22
                    (------------(xxxxxxxxxxxxxxx|----------)  toom32
               (----(xxxxxxxxxxxx|-------)                     toom42
                                         (-------(xxxxxxxxxx|  toom33
(xxxxxxxxxxxxxxxxxxx|                                          toom42-unblanced

                                             (---(xxxxxxxxxx|  toom44
                             (-------(xxxxxxxxxxx|----------)  toom43
                        (--(xxxxxxxxx|-------)                 toom53
           (--------(xxxxxx|-)                                 toom52
      (----(xxxxxxxx|---)  |                                   toom62
(xxxxxxxxxx|               |                                   toom62-unblanced
                          9/20
0    1/6  1/5 1/4  1/3 2/5  1/2 5/9 3/5 2/3 3/4 4/5  9/10   1  nb/na
*/

void lmmp_mul_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_srcptr restrict numb, mp_size_t nb) {
    lmmp_param_assert(na >= nb);
    lmmp_param_assert(nb > 0);
    if (na == nb) {
        if (numa == numb)
            lmmp_sqr_(dst, numa, na);
        else
            lmmp_mul_n_(dst, numa, numb, na);
    } else if ((nb < MUL_TOOM22_THRESHOLD || nb < MUL_TOOMX2_THRESHOLD) && !(4 * na < 5 * nb)) {
        if (na <= PART_SIZE || nb <= 2)
            lmmp_mul_basecase_(dst, numa, na, numb, nb);
        else {
            mp_limb_t tp[MUL_TOOMX2_THRESHOLD];
            lmmp_mul_basecase_(dst, numa, PART_SIZE, numb, nb);
            dst += PART_SIZE;
            numa += PART_SIZE;
            na -= PART_SIZE;
            lmmp_copy(tp, dst, nb);
            while (na > PART_SIZE) {
                lmmp_mul_basecase_(dst, numa, PART_SIZE, numb, nb);
                if (lmmp_add_n_(dst, dst, tp, nb))
                    lmmp_inc(dst + nb);
                dst += PART_SIZE;
                numa += PART_SIZE;
                na -= PART_SIZE;
                lmmp_copy(tp, dst, nb);
            }
            if (na >= nb)
                lmmp_mul_basecase_(dst, numa, na, numb, nb);
            else
                lmmp_mul_basecase_(dst, numb, nb, numa, na);
            if (lmmp_add_n_(dst, dst, tp, nb))
                lmmp_inc(dst + nb);
        }
    } else if (((na + nb) >> 1) < MUL_TOOM44_THRESHOLD || 2 * nb < MUL_TOOM44_THRESHOLD) {
        if (na < 3 * nb) {
            if (4 * na < 5 * nb) {
                if (nb < MUL_TOOM33_THRESHOLD)
                    lmmp_mul_toom22_(dst, numa, na, numb, nb);
                else
                    lmmp_mul_toom33_(dst, numa, na, numb, nb);
            } else if (5 * na < 9 * nb)
                lmmp_mul_toom32_(dst, numa, na, numb, nb);
            else
                lmmp_mul_toom42_(dst, numa, na, numb, nb);
        } else 
            lmmp_mul_toom42_unbalance_(dst, numa, na, numb, nb);
    } else if (((na + nb) >> 1) < MUL_FFT_THRESHOLD || 2 * nb < MUL_FFT_THRESHOLD) {
        if (na < 5 * nb) {
            if (4 * na < 5 * nb) 
                lmmp_mul_toom44_(dst, numa, na, numb, nb);
            else if (3 * na < 5 * nb)
                lmmp_mul_toom43_(dst, numa, na, numb, nb);
            else if (9 * na < 20 * nb)
                lmmp_mul_toom53_(dst, numa, na, numb, nb);
            else if (na < 3 * nb)
                lmmp_mul_toom52_(dst, numa, na, numb, nb);
            else 
                lmmp_mul_toom62_(dst, numa, na, numb, nb);
        } else 
            lmmp_mul_toom62_unbalance_(dst, numa, na, numb, nb);
    } else {
        if (na < 8 * nb)
            lmmp_mul_fft_(dst, numa, na, numb, nb);
        else 
            lmmp_mul_fft_unbalance_(dst, numa, na, numb, nb);
    }
}
