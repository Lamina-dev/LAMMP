// #include "../../../include/lammp/numth.h"

// typedef struct {
//     slong m11, m12;
//     slong m21, m22;
// } mp_gcd_lehmer_t;

// static mp_limb_t lmmp_gcd_lehmer_step_(mp_limb_t u, mp_limb_t v, mp_gcd_lehmer_t* gcd) {
// #define A (gcd->m11)
// #define B (gcd->m12)
// #define C (gcd->m21)
// #define D (gcd->m22)
//     A = 1; B = 0; C = 0; D = 1;


//     while (v != 0) {
//         if (u < v) 
//             LMMP_SWAP(u, v, mp_limb_t);
//         slong q = u / v;
//         slong t = u % v;
//         u = v;
//         v = t;
        
//         t = A - q * C;
//         A = C;
//         C = t;
//         t = B - q * D;
//         B = D;
//         D = t;

//     }
//     return 0;

// #undef A
// #undef B
// #undef C
// #undef D
// }


// mp_size_t lmmp_gcd_lehmer_(mp_ptr numa, mp_size_t na, mp_ptr numb, mp_size_t nb) {
//     if (na < nb || (na == nb && numa[na - 1] < numb[nb - 1])) {
//         LMMP_SWAP(numa, numb, mp_ptr);
//         LMMP_SWAP(na, nb, mp_size_t);
//     }

//     while (nb >= 2) {
//         mp_limb_t q = lmmp_gcd_lehmer_step_(numa + na - 2, numb + nb - 2);
//         lmmp_debug_assert(q != 0);
//         mp_limb_t c = lmmp_submul_1_(numa, )
        
//         dstq[--nq] = q;
//     }
//     return qh;
// }