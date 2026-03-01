#include "../../include/lammp/lmmpn.h"
#include <math.h>

// This is a simplified version of mul.c
/*
        Areas where the different toom algorithms can be called
0         1/5 1/4  1/3 2/5  1/2 5/9 3/5 2/3 3/4 4/5  9/10   1  nb/na
                                5/12            11/16
                             |-------------------|xxxxxxxxxx+  toom22
                    |------------|xxxxxxx+xxxxxxx|----------|  toom32
               |----|xxxxxxxx+xxx|-------|                     toom42
                                         |-------|xxxxxxxxxx+  toom33
***********************    NOT IMPLEMENTED    ***********************
                                             |--------|xxxxx+  toom44
                                     |-------|xxx+xxxx|-----|  toom54
                             |------------|xx+xxx|----------|  toom43
                        |--------|xxx+xxxx|--|                 toom53
                    |----|xxx+xxx|---|                         toom63
           |--------|xxx+|---|                                 toom52
                                5/12            11/16
0         1/5 1/4  1/3 2/5  1/2 5/9 3/5 2/3 3/4 4/5  9/10   1  nb/na
*/

void lmmp_mul_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb) {
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
    } else if (((na + nb) >> 1) < MUL_FFT_THRESHOLD || 2 * nb < MUL_FFT_THRESHOLD) {
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
        } else {
            TEMP_DECL;
            mp_limb_t* ws = SALLOC_TYPE(nb, mp_limb_t);
            lmmp_mul_toom42_history_(dst, numa, 2 * nb, numb, nb);
            dst += 2 * nb;
            numa += 2 * nb;
            na -= 2 * nb;
            lmmp_copy(ws, dst, nb);
            while (2 * na >= 5 * nb) {
                lmmp_mul_toom42_history_(dst, numa, 2 * nb, numb, nb);
                if (lmmp_add_n_(dst, dst, ws, nb))
                    lmmp_inc(dst + nb);
                dst += 2 * nb;
                numa += 2 * nb;
                na -= 2 * nb;
                lmmp_copy(ws, dst, nb);
            }
            lmmp_mul_toom42_history_free_();
            // 0.5 nb <= na < 2.5 nb
            if (na >= nb)
                lmmp_mul_(dst, numa, na, numb, nb);
            else
                lmmp_mul_(dst, numb, nb, numa, na);
            if (lmmp_add_n_(dst, dst, ws, nb))
                lmmp_inc(dst + nb);
            TEMP_FREE;
        }
    } else {
        if (na < 8 * nb)
            lmmp_mul_fft_(dst, numa, na, numb, nb);
        else {
            mp_ptr ws = ALLOC_TYPE(nb, mp_limb_t);
            mp_size_t sna = 3 * nb;
            mp_size_t hn = lmmp_fft_next_size_((sna + nb + 1) >> 1);
            sna = (hn << 1) - 1 - nb;
        
            lmmp_mul_fft_history_(dst, hn, numa, sna, numb, nb);
            dst += sna;
            numa += sna;
            na -= sna;
            lmmp_copy(ws, dst, nb);
            while (na >= sna) {
                lmmp_mul_fft_history_(dst, hn, numa, sna, numb, nb);
                if (lmmp_add_n_(dst, dst, ws, nb))
                    lmmp_inc(dst + nb);
                dst += sna;
                numa += sna;
                na -= sna;
                lmmp_copy(ws, dst, nb);
            }
            lmmp_mul_fft_history_free_();
            // remaining na < sna
            if (na >= nb)
                lmmp_mul_(dst, numa, na, numb, nb);
            else if (na > 0)
                lmmp_mul_(dst, numb, nb, numa, na);
            else // na == 0 
                lmmp_zero(dst, nb);
            if (lmmp_add_n_(dst, dst, ws, nb))
                lmmp_inc(dst + nb);
            lmmp_free(ws);
        }
    }
}
