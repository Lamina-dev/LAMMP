#include "../../include/lammp/lmmp.h"
#include "../../include/lammp/lmmpn.h"

mp_size_t lmmp_extract_bits_(mp_srcptr num, mp_size_t n, mp_limb_t* ext, int bits) {
    lmmp_debug_assert(bits <= LIMB_BITS && bits > 0);
    lmmp_debug_assert(n > 0);
    if (n == 1) {
        int lb = lmmp_limb_bits_(num[0]);
        if (lb <= bits) {
            *ext = num[0];
            return 0;
        }
        else {
            *ext = num[0] >> (lb - bits);
            return lb - bits;
        }
    } else {
        int lb = lmmp_limb_bits_(num[n - 1]);
        if (lb <= bits) {
            *ext = num[n - 1] << (bits - lb);
            *ext |= num[n - 2] >> (LIMB_BITS - bits + lb);
            return LIMB_BITS * (n - 1) - (bits - lb);
        } else {
            *ext = num[n - 1] >> (lb - bits);
            return LIMB_BITS * (n - 1) + (lb - bits);
        }
    }
}