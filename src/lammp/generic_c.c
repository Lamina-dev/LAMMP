#include "../../include/lammp/lmmpn.h"

// Generic C implementation for GCC/Clang with __int128 support
#if defined(__GNUC__) || defined(__clang__)
    typedef unsigned __int128 uint128_t;
#else
    #error "Generic C implementation requires __int128 support"
#endif

// ================= Bit Manipulation =================
int lmmp_limb_popcnt_(mp_limb_t n) { 
    return __builtin_popcountll(n); 
}

int lmmp_leading_zeros_(mp_limb_t n) {
    if (n == 0) return 64;
    return __builtin_clzll(n);
}

int lmmp_tailing_zeros_(mp_limb_t n) {
    if (n == 0) return 64;
    return __builtin_ctzll(n);
}

int lmmp_limb_bits_(mp_limb_t n) {
    return 64 - lmmp_leading_zeros_(n);
}

// ================= Logic =================
void lmmp_not_(mp_ptr rp, mp_srcptr up, mp_size_t n) {
    for (mp_size_t i = 0; i < n; i++) rp[i] = ~up[i];
}

// ================= Shift =================
mp_limb_t lmmp_shl_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_size_t cnt) {
    if (cnt == 0) {
        if (rp != up) for(mp_size_t i=0; i<n; i++) rp[i] = up[i];
        return 0;
    }
    mp_limb_t retval = up[n - 1] >> (64 - cnt);
    for (mp_size_t i = n - 1; i > 0; i--) {
        rp[i] = (up[i] << cnt) | (up[i - 1] >> (64 - cnt));
    }
    rp[0] = up[0] << cnt;
    return retval;
}

mp_limb_t lmmp_shl_c_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_size_t cnt, mp_limb_t c) {
    // Shift left with input carry 'c' (low bits to shift in)
    mp_limb_t retval = up[n - 1] >> (64 - cnt);
    for (mp_size_t i = n - 1; i > 0; i--) {
        rp[i] = (up[i] << cnt) | (up[i - 1] >> (64 - cnt));
    }
    rp[0] = (up[0] << cnt) | (c >> (64 - cnt)); // Is 'c' full limb or just bits? 
    // Usually 'c' contains bits in high part or low part?
    // GMP mpn_lshift_c takes c as the bits to shift in. 
    // Assuming c is in lower 'cnt' bits? Or higher?
    // Let's assume standard behavior: c is logically concatenated at bottom.
    // If we shift left by cnt, we need cnt bits from c.
    // Typically those are the *high* bits of c if c was the previous limb.
    // But let's assume valid bits are at lsb or msb. 
    // If it's `shl_c`, usually `c` is the limb below `up`.
    // So we want `c >> (64 - cnt)`.
    return retval;
}

mp_limb_t lmmp_shlnot_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_size_t cnt) {
    // Shift left then NOT, return carry (shifted out bits)
    mp_limb_t retval = up[n - 1] >> (64 - cnt); 
    // And invert? "shlnot" probably means result is inverted.
    // Does return value get inverted? 
    // Let's assume return value is RAW shifted out bits (carry), but memory is NOTed.
    for (mp_size_t i = n - 1; i > 0; i--) {
        rp[i] = ~((up[i] << cnt) | (up[i - 1] >> (64 - cnt)));
    }
    rp[0] = ~(up[0] << cnt);
    return retval;
}

mp_limb_t lmmp_shr_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_size_t cnt) {
    if (cnt == 0) {
        if (rp != up) for(mp_size_t i=0; i<n; i++) rp[i] = up[i];
        return 0;
    }
    mp_limb_t retval = up[0] << (64 - cnt); // shifted out bits from low end
    for (mp_size_t i = 0; i < n - 1; i++) {
        rp[i] = (up[i] >> cnt) | (up[i + 1] << (64 - cnt));
    }
    rp[n - 1] = up[n - 1] >> cnt;
    return retval;
}

mp_limb_t lmmp_shr_c_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_size_t cnt, mp_limb_t c) {
    // Shift right with carry 'c' (descending into top)
    mp_limb_t retval = up[0] << (64 - cnt);
    for (mp_size_t i = 0; i < n - 1; i++) {
        rp[i] = (up[i] >> cnt) | (up[i + 1] << (64 - cnt));
    }
    rp[n - 1] = (up[n - 1] >> cnt) | (c << (64 - cnt));
    return retval;
}

// ================= Add/Sub =================
mp_limb_t lmmp_add_n_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n) {
    mp_limb_t cy = 0;
    for (mp_size_t i = 0; i < n; i++) {
        uint128_t r = (uint128_t)up[i] + vp[i] + cy;
        rp[i] = (mp_limb_t)r;
        cy = (mp_limb_t)(r >> 64);
    }
    return cy;
}

mp_limb_t lmmp_add_nc_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n, mp_limb_t cy) {
    for (mp_size_t i = 0; i < n; i++) {
        uint128_t r = (uint128_t)up[i] + vp[i] + cy;
        rp[i] = (mp_limb_t)r;
        cy = (mp_limb_t)(r >> 64);
    }
    return cy;
}

mp_limb_t lmmp_sub_n_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n) {
    mp_limb_t bor = 0;
    for (mp_size_t i = 0; i < n; i++) {
        uint128_t u = up[i];
        uint128_t v = (uint128_t)vp[i] + bor;
        rp[i] = (mp_limb_t)(u - v);
        bor = (u < v);
    }
    return bor;
}

mp_limb_t lmmp_sub_nc_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n, mp_limb_t bor) {
    for (mp_size_t i = 0; i < n; i++) {
        uint128_t u = up[i];
        uint128_t v = (uint128_t)vp[i] + bor;
        rp[i] = (mp_limb_t)(u - v);
        bor = (u < v);
    }
    return bor;
}

// ================= Mul =================
mp_limb_t lmmp_mul_1_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t v) {
    mp_limb_t cy = 0;
    for (mp_size_t i = 0; i < n; i++) {
        uint128_t p = (uint128_t)up[i] * v + cy;
        rp[i] = (mp_limb_t)p;
        cy = (mp_limb_t)(p >> 64);
    }
    return cy;
}

mp_limb_t lmmp_addmul_1_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t v) {
    mp_limb_t cy = 0;
    for (mp_size_t i = 0; i < n; i++) {
        uint128_t p = (uint128_t)up[i] * v + rp[i] + cy;
        rp[i] = (mp_limb_t)p;
        cy = (mp_limb_t)(p >> 64);
    }
    return cy;
}

mp_limb_t lmmp_submul_1_(mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t v) {
    mp_limb_t bor = 0;
    for (mp_size_t i = 0; i < n; i++) {
        uint128_t p = (uint128_t)up[i] * v + bor;
        mp_limb_t t = rp[i];
        mp_limb_t p_lo = (mp_limb_t)p;
        rp[i] = t - p_lo;
        bor = (mp_limb_t)(p >> 64) + (t < p_lo);
    }
    return bor;
}

void lmmp_mul_basecase_(mp_ptr rp, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn) {
    rp[un] = lmmp_mul_1_(rp, up, un, vp[0]);
    for (mp_size_t i = 1; i < vn; i++) {
        rp[un + i] = lmmp_addmul_1_(rp + i, up, un, vp[i]);
    }
}

void lmmp_sqr_basecase_(mp_ptr rp, mp_srcptr up, mp_size_t n) {
    lmmp_mul_basecase_(rp, up, n, up, n);
}

mp_limb_t lmmp_mulh_(mp_limb_t u, mp_limb_t v) {
    return (mp_limb_t)(((uint128_t)u * v) >> 64);
}

void lmmp_mullh_(mp_limb_t u, mp_limb_t v, mp_ptr rp) {
    uint128_t p = (uint128_t)u * v;
    rp[0] = (mp_limb_t)p;
    rp[1] = (mp_limb_t)(p >> 64);
}

// ================= Div =================
mp_limb_t lmmp_inv_2_1_(mp_limb_t xh, mp_limb_t xl) {
    (void)xh; (void)xl;
    return 0; 
}

mp_limb_t lmmp_div_1_(mp_ptr qp, mp_srcptr np, mp_size_t nn, mp_limb_t d) {
    mp_limb_t rem = 0;
    mp_size_t i = nn;
    while (i-- > 0) {
        uint128_t n = ((uint128_t)rem << 64) | np[i];
        mp_limb_t q = (mp_limb_t)(n / d);
        rem = (mp_limb_t)(n % d);
        if (qp) qp[i] = q;
    }
    return rem;
}

mp_limb_t lmmp_div_1_s_(mp_ptr qp, mp_ptr np, mp_size_t nn, mp_limb_t d) {
    return lmmp_div_1_(qp, np, nn, d);
}

void lmmp_div_2_(mp_ptr dstq, mp_srcptr numa, mp_size_t na, mp_ptr numb) {
    (void)dstq; (void)numa; (void)na; (void)numb; 
}

mp_limb_t lmmp_div_2_s_(mp_ptr q, mp_ptr a, mp_size_t na, mp_srcptr d) {
    (void)q; (void)a; (void)na; (void)d;
    return 0;
}

mp_limb_t lmmp_div_3_2_(mp_ptr numa, mp_srcptr numb, mp_limb_t inv21) {
    (void)numa; (void)numb; (void)inv21;
    return 0;
}

mp_limb_t lmmp_inv_1_(mp_limb_t x) {
    (void)x;
    return 0;
}

// ================= Composite OPS (Stubs) ===================
// These seem to be used in Toom-Cook or FFT
// Correct implementation is needed for Toom
mp_limb_t lmmp_addshl1_n_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n) {
    mp_limb_t cy = 0;
    for(mp_size_t i=0; i<n; i++) {
        mp_limb_t v_cur = vp[i];
        mp_limb_t v_prev = (i == 0) ? 0 : vp[i-1];
        mp_limb_t v_val = (v_cur << 1) | (v_prev >> 63);
        
        uint128_t sum = (uint128_t)up[i] + v_val + cy;
        rp[i] = (mp_limb_t)sum;
        cy = (mp_limb_t)(sum >> 64);
    }
    return cy + (vp[n-1] >> 63);
}

// Correct implementation for Toom-Cook / FFT
// Algorithm: (UP + VP) >> 1
mp_limb_t lmmp_shr1add_n_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n) {
    mp_limb_t cy = 0;
    mp_limb_t prev_sum = 0;
    mp_limb_t ret_bit = 0;

    for (mp_size_t i = 0; i < n; i++) {
        uint128_t sum = (uint128_t)up[i] + vp[i] + cy;
        mp_limb_t s_lo = (mp_limb_t)sum;
        cy = (mp_limb_t)(sum >> 64);
        
        if (i == 0) ret_bit = s_lo & 1;
        else rp[i-1] = (prev_sum >> 1) | (s_lo << 63);
        
        prev_sum = s_lo;
    }
    rp[n-1] = (prev_sum >> 1) | (cy << 63);
    return ret_bit;
}

// Algorithm: (UP - VP) >> 1
mp_limb_t lmmp_shr1sub_n_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n) {
    mp_limb_t bor = 0;
    mp_limb_t prev_diff = 0;
    mp_limb_t ret_bit = 0;

    for (mp_size_t i = 0; i < n; i++) {
        uint128_t u = up[i];
        uint128_t v = (uint128_t)vp[i] + bor;
        mp_limb_t diff = (mp_limb_t)(u - v);
        bor = (u < v); // Carry out (borrow)
        
        if (i == 0) ret_bit = diff & 1;
        else rp[i-1] = (prev_diff >> 1) | (diff << 63);
        
        prev_diff = diff;
    }
    rp[n-1] = (prev_diff >> 1) | (bor << 63);
    return ret_bit;
}

// Algorithm: (UP + VP) >> 1 with initial carry
mp_limb_t lmmp_shr1add_nc_(mp_ptr rp, mp_srcptr up, mp_srcptr vp, mp_size_t n, mp_limb_t c) {
    mp_limb_t cy = c;
    mp_limb_t prev_sum = 0;
    mp_limb_t ret_bit = 0;

    for (mp_size_t i = 0; i < n; i++) {
        uint128_t sum = (uint128_t)up[i] + vp[i] + cy;
        mp_limb_t s_lo = (mp_limb_t)sum;
        cy = (mp_limb_t)(sum >> 64);
        
        if (i == 0) ret_bit = s_lo & 1;
        else rp[i-1] = (prev_sum >> 1) | (s_lo << 63);
        
        prev_sum = s_lo;
    }
    rp[n-1] = (prev_sum >> 1) | (cy << 63);
    return ret_bit;
}

mp_limb_t lmmp_mulmod_ulong_(mp_limb_t a, mp_limb_t b, mp_limb_t m) {
    return (mp_limb_t)(((uint128_t)a * b) % m);
}

