#include "../../include/lammp/lmmpn.h"

mp_limb_t lmmp_add_n_sub_n_(mp_ptr dsta, mp_ptr dstb, mp_srcptr numa, mp_srcptr numb, mp_size_t n) {
    mp_limb_t acyo = 0, scyo = 0;
    mp_size_t off, this_n;

    if (dsta != numa && dsta != numb) {
        for (off = 0; off < n; off += PART_SIZE) {
            this_n = LMMP_MIN(n - off, PART_SIZE);
            acyo = lmmp_add_nc_(dsta + off, numa + off, numb + off, this_n, acyo);
            scyo = lmmp_sub_nc_(dstb + off, numa + off, numb + off, this_n, scyo);
        }
    } else if (dstb != numa && dstb != numb) {
        for (off = 0; off < n; off += PART_SIZE) {
            this_n = LMMP_MIN(n - off, PART_SIZE);
            scyo = lmmp_sub_nc_(dstb + off, numa + off, numb + off, this_n, scyo);
            acyo = lmmp_add_nc_(dsta + off, numa + off, numb + off, this_n, acyo);
        }
    } else {
        mp_limb_t tp[PART_SIZE];
        for (off = 0; off < n; off += PART_SIZE) {
            this_n = LMMP_MIN(n - off, PART_SIZE);
            acyo = lmmp_add_nc_(tp, numa + off, numb + off, this_n, acyo);
            scyo = lmmp_sub_nc_(dstb + off, numa + off, numb + off, this_n, scyo);
            lmmp_copy(dsta + off, tp, this_n);
        }
    }
    return 2 * acyo + scyo;
}
