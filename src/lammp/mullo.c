#include "../../include/lammp/lmmpn.h"

void lmmp_mullo_fft_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n) {
    lmmp_param_assert(n > 0);
    mp_size_t hn = lmmp_fft_next_size_((n + n + 1) >> 1);
    lmmp_assert(n + n >= hn);
    mp_ptr tp = ALLOC_TYPE(hn + 1, mp_limb_t);
    mp_ptr mp = ALLOC_TYPE(hn, mp_limb_t);

    mp_srcptr amodm = numa;
    mp_size_t nam = n;
    if (n > hn) {
        /*
          Z = B^hb - 1
          amodm = a mod Z
         */
        if (lmmp_add_(mp, numa, hn, numa + hn, n - hn))
            lmmp_inc(mp);
        amodm = mp;
        nam = hn;
    }
    lmmp_mul_mersenne_(mp, hn, amodm, nam, numb, n);

    mp_srcptr amodp = numa;
    mp_size_t nap = n;
    if (n > hn) {
        /*
          Z = B^hp - 1
          amodp = a mod Z
         */
        tp[hn] = 0;
        if (lmmp_sub_(tp, numa, hn, numa + hn, n - hn))
            lmmp_inc(tp);
        amodp = tp;
        nap = hn + 1;
    }
    lmmp_mul_fermat_(tp, hn, amodp, nap, numb, n);

    mp_limb_t c = lmmp_shr1add_n_(mp, mp, tp, hn);
    lmmp_copy(dst, mp, n);
    if (c)
        dst[n - 1] |= (mp_limb_t)1 << (LIMB_BITS - 1);
    lmmp_free(tp);
    lmmp_free(mp);
}

/*
       <---t---><---m--->
       |--a1---|---a0---|
       |--b1---|---b0---|

  ,
  |\
  | \
  |  \
  +-----,
  |     |
  |     |\
  |     | \
  |     |  \
  +-----+---`
  ^  m  ^ t ^

 此算法是一种不平衡分块的算法，朴素的想法是计算平衡分块，计算一次完整的乘法，然后两次递归的调用此函数计算低位，
 事实上，我们也可以不平衡的分块，以减少递归深度，具体分析如下：
 取a和b的低位一定宽度为m，高位宽度为t，则有：
 计算一次完整的平衡乘法m，然后递归调用计算mullo，长度为t
 复杂度模型：
   ML(n) = 2*ML(an) + M((1-a)n) => k*M(n) = 2*k*M(n)*a^e + M(n)*(1-a)^e
 其中ML为mullo的复杂度，M为mul_n的复杂度，e为Toom-Cook系列的复杂度幂次。
 e=log(3)/log(2) [Toom-2] -> a ~= 1/2
 e=log(5)/log(3) [Toom-3] -> a ~= 9/40
 e=log(7)/log(4) [Toom-4] -> a ~= 7/39
 e=log(11)/log(6) [Toom-6] -> a ~= 1/8
 e=log(15)/log(8) [Toom-8] -> a ~= 1/10
*/

void lmmp_mullo_dc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_ptr tp, mp_size_t n) {
    if (n < MULLO_BASECASE_THRESHOLD) {
        lmmp_mul_1_(dst, numa, n, numb[0]);
        for (mp_size_t i = 1; i < n; ++i) {
            lmmp_mul_1_(tp, numa, n - i, numb[i]);
            lmmp_add_n_(dst + i, dst + i, tp, n - i);
        }
        return;
    } else {
        mp_size_t m, t;
        if (n < MUL_TOOM33_THRESHOLD) {
            m = 25 * n / 36;
        } else {
            m = 31 * n / 40;
        }
        t = n - m;

#define a0 (numa)
#define a1 (numa + m)
#define b0 (numb)
#define b1 (numb + m)
#define c0 (dst)
#define c1 (dst + m)
#define lo1 (tp)              // [tp,  2*t]
#define tp1 (tp + 2 * t)      // [tp+2*t, 2*t]
#define lo2 (tp + 2 * t)      // [tp+2*t, 2*t]
#define tp2 (tp + 4 * t)      // [tp+2*t, 2*t]
        lmmp_mul_n_(tp, a0, b0, m);
        lmmp_copy(c0, tp, n);
        lmmp_mullo_dc_(lo1, a1, b0, tp1, t);
        lmmp_mullo_dc_(lo2, a0, b1, tp2, t);
        lmmp_add_n_(c1, c1, lo1, t);
        lmmp_add_n_(c1, c1, lo2, t);
        return;
    }
}