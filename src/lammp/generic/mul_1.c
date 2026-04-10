/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../../include/lammp/impl/_umul64to128_.h"
#include "../../../include/lammp/lmmpn.h"

mp_limb_t lmmp_mul_1_(mp_ptr restrict dst, mp_srcptr restrict numa, mp_size_t na, mp_limb_t x) {
    mp_limb_t cl = 0;
    mp_size_t i = 0;

    mp_limb_t ul0, ul1, ul2, ul3;
    mp_limb_t lpl0, hpl0, lpl1, hpl1;
    mp_limb_t lpl2, hpl2, lpl3, hpl3;

    if (dst == numa) {
        for (; i + 4 <= na; i += 4) {
            ul0 = dst[i + 0];
            ul1 = dst[i + 1];
            ul2 = dst[i + 2];
            ul3 = dst[i + 3];

            _umul64to128_(ul0, x, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;

            _umul64to128_(ul1, x, &lpl1, &hpl1);
            lpl1 += cl;
            cl = (lpl1 < cl) + hpl1;

            _umul64to128_(ul2, x, &lpl2, &hpl2);
            lpl2 += cl;
            cl = (lpl2 < cl) + hpl2;

            _umul64to128_(ul3, x, &lpl3, &hpl3);
            lpl3 += cl;
            cl = (lpl3 < cl) + hpl3;

            dst[i + 0] = lpl0;
            dst[i + 1] = lpl1;
            dst[i + 2] = lpl2;
            dst[i + 3] = lpl3;
        }

        for (; i < na; i++) {
            ul0 = dst[i];
            _umul64to128_(ul0, x, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            dst[i] = lpl0;
        }
    }

    else {
        for (; i + 4 <= na; i += 4) {
            ul0 = numa[i + 0];
            ul1 = numa[i + 1];
            ul2 = numa[i + 2];
            ul3 = numa[i + 3];

            _umul64to128_(ul0, x, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;

            _umul64to128_(ul1, x, &lpl1, &hpl1);
            lpl1 += cl;
            cl = (lpl1 < cl) + hpl1;

            _umul64to128_(ul2, x, &lpl2, &hpl2);
            lpl2 += cl;
            cl = (lpl2 < cl) + hpl2;

            _umul64to128_(ul3, x, &lpl3, &hpl3);
            lpl3 += cl;
            cl = (lpl3 < cl) + hpl3;

            dst[i + 0] = lpl0;
            dst[i + 1] = lpl1;
            dst[i + 2] = lpl2;
            dst[i + 3] = lpl3;
        }

        for (; i < na; i++) {
            ul0 = numa[i];
            _umul64to128_(ul0, x, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            dst[i] = lpl0;
        }
    }

    return cl;
}

mp_limb_t lmmp_addmul_1_(mp_ptr restrict numa, mp_srcptr restrict numb, mp_size_t n, mp_limb_t b) {
    mp_limb_t cl = 0;
    mp_size_t i = 0;

    mp_limb_t ul0, ul1, ul2, ul3;
    mp_limb_t lpl0, hpl0, rl0;
    mp_limb_t lpl1, hpl1, rl1;
    mp_limb_t lpl2, hpl2, rl2;
    mp_limb_t lpl3, hpl3, rl3;

    if (numa == numb) {
        for (; i + 4 <= n; i += 4) {
            ul0 = numa[i + 0];
            ul1 = numa[i + 1];
            ul2 = numa[i + 2];
            ul3 = numa[i + 3];

            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = ul0 + lpl0;
            cl += (lpl0 < ul0);

            _umul64to128_(ul1, b, &lpl1, &hpl1);
            lpl1 += cl;
            cl = (lpl1 < cl) + hpl1;
            lpl1 = ul1 + lpl1;
            cl += (lpl1 < ul1);

            _umul64to128_(ul2, b, &lpl2, &hpl2);
            lpl2 += cl;
            cl = (lpl2 < cl) + hpl2;
            lpl2 = ul2 + lpl2;
            cl += (lpl2 < ul2);

            _umul64to128_(ul3, b, &lpl3, &hpl3);
            lpl3 += cl;
            cl = (lpl3 < cl) + hpl3;
            lpl3 = ul3 + lpl3;
            cl += (lpl3 < ul3);

            numa[i + 0] = lpl0;
            numa[i + 1] = lpl1;
            numa[i + 2] = lpl2;
            numa[i + 3] = lpl3;
        }
        for (; i < n; i++) {
            ul0 = numa[i];
            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = ul0 + lpl0;
            cl += (lpl0 < ul0);
            numa[i] = lpl0;
        }
    } else {
        for (; i + 4 <= n; i += 4) {
            ul0 = numb[i + 0];
            ul1 = numb[i + 1];
            ul2 = numb[i + 2];
            ul3 = numb[i + 3];
            rl0 = numa[i + 0];
            rl1 = numa[i + 1];
            rl2 = numa[i + 2];
            rl3 = numa[i + 3];

            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = rl0 + lpl0;
            cl += (lpl0 < rl0);

            _umul64to128_(ul1, b, &lpl1, &hpl1);
            lpl1 += cl;
            cl = (lpl1 < cl) + hpl1;
            lpl1 = rl1 + lpl1;
            cl += (lpl1 < rl1);

            _umul64to128_(ul2, b, &lpl2, &hpl2);
            lpl2 += cl;
            cl = (lpl2 < cl) + hpl2;
            lpl2 = rl2 + lpl2;
            cl += (lpl2 < rl2);

            _umul64to128_(ul3, b, &lpl3, &hpl3);
            lpl3 += cl;
            cl = (lpl3 < cl) + hpl3;
            lpl3 = rl3 + lpl3;
            cl += (lpl3 < rl3);

            numa[i + 0] = lpl0;
            numa[i + 1] = lpl1;
            numa[i + 2] = lpl2;
            numa[i + 3] = lpl3;
        }
        for (; i < n; i++) {
            ul0 = numb[i];
            rl0 = numa[i];
            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = rl0 + lpl0;
            cl += (lpl0 < rl0);
            numa[i] = lpl0;
        }
    }
    return cl;
}

mp_limb_t lmmp_submul_1_(mp_ptr restrict numa, mp_srcptr restrict numb, mp_size_t n, mp_limb_t b) {
    mp_limb_t cl = 0;
    mp_size_t i = 0;

    mp_limb_t ul0, ul1, ul2, ul3;
    mp_limb_t lpl0, hpl0, rl0;
    mp_limb_t lpl1, hpl1, rl1;
    mp_limb_t lpl2, hpl2, rl2;
    mp_limb_t lpl3, hpl3, rl3;

    if (numa == numb) {
        for (; i + 4 <= n; i += 4) {
            ul0 = numa[i + 0];
            ul1 = numa[i + 1];
            ul2 = numa[i + 2];
            ul3 = numa[i + 3];

            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = ul0 - lpl0;
            cl += (lpl0 > ul0);

            _umul64to128_(ul1, b, &lpl1, &hpl1);
            lpl1 += cl;
            cl = (lpl1 < cl) + hpl1;
            lpl1 = ul1 - lpl1;
            cl += (lpl1 > ul1);

            _umul64to128_(ul2, b, &lpl2, &hpl2);
            lpl2 += cl;
            cl = (lpl2 < cl) + hpl2;
            lpl2 = ul2 - lpl2;
            cl += (lpl2 > ul2);

            _umul64to128_(ul3, b, &lpl3, &hpl3);
            lpl3 += cl;
            cl = (lpl3 < cl) + hpl3;
            lpl3 = ul3 - lpl3;
            cl += (lpl3 > ul3);

            numa[i + 0] = lpl0;
            numa[i + 1] = lpl1;
            numa[i + 2] = lpl2;
            numa[i + 3] = lpl3;
        }
        for (; i < n; i++) {
            ul0 = numa[i];
            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = ul0 - lpl0;
            cl += (lpl0 > ul0);
            numa[i] = lpl0;
        }
    } else {
        for (; i + 4 <= n; i += 4) {
            ul0 = numb[i + 0];
            ul1 = numb[i + 1];
            ul2 = numb[i + 2];
            ul3 = numb[i + 3];
            rl0 = numa[i + 0];
            rl1 = numa[i + 1];
            rl2 = numa[i + 2];
            rl3 = numa[i + 3];

            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = rl0 - lpl0;
            cl += (lpl0 > rl0);

            _umul64to128_(ul1, b, &lpl1, &hpl1);
            lpl1 += cl;
            cl = (lpl1 < cl) + hpl1;
            lpl1 = rl1 - lpl1;
            cl += (lpl1 > rl1);

            _umul64to128_(ul2, b, &lpl2, &hpl2);
            lpl2 += cl;
            cl = (lpl2 < cl) + hpl2;
            lpl2 = rl2 - lpl2;
            cl += (lpl2 > rl2);

            _umul64to128_(ul3, b, &lpl3, &hpl3);
            lpl3 += cl;
            cl = (lpl3 < cl) + hpl3;
            lpl3 = rl3 - lpl3;
            cl += (lpl3 > rl3);

            numa[i + 0] = lpl0;
            numa[i + 1] = lpl1;
            numa[i + 2] = lpl2;
            numa[i + 3] = lpl3;
        }
        for (; i < n; i++) {
            ul0 = numb[i];
            rl0 = numa[i];
            _umul64to128_(ul0, b, &lpl0, &hpl0);
            lpl0 += cl;
            cl = (lpl0 < cl) + hpl0;
            lpl0 = rl0 - lpl0;
            cl += (lpl0 > rl0);
            numa[i] = lpl0;
        }
    }
    return cl;
}