#include "../../../include/lammp/numth.h"

ulong lmmp_powmod_ulong_(ulong base, ulong exp, ulong mod) {
    ulong dst = 1;
    ulong p = 0;
    while (1) {
        if (exp & 1)
            dst = lmmp_mulmod_ulong_(dst, base, mod, &p);
        exp >>= 1;
        if (exp == 0)
            break;
        base = lmmp_mulmod_ulong_(base, base, mod, &p);
    }
    return dst;
}

#define qpow(a, b, c) lmmp_powmod_ulong_(a, b, c)
#define qmul(a, b, c) lmmp_mulmod_ulong_(a, b, c, &q)

bool lmmp_is_prime_ulong_(ulong n) {
    /** 
     *  Miller-Rabin primality test
     *  in 2^64 range, using the 7th Miller-Rabin test, and is precise 
     */
    static const ulong a[7] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
    ulong q = 0;
    if (n < 3 || n % 2 == 0)
        return n == 2;  
    ulong u = n - 1, t = 0, v = 0;
    while (u % 2 == 0) u /= 2, ++t;
    for (uint i = 0; i < 7; ++i) {
        v = qpow(a[i], u, n);
        if (v == 1 || v == n - 1 || v == 0)
            continue;  
        for (ulong j = 1; j <= t; ++j) {
            v = qmul(v, v, n);
            if (v == n - 1 && j != t) {
                v = 1;
                break;
            }
            if (v == 1)
                return false;
        }
        if (v != 1)
            return false;
    }

    return true;
}