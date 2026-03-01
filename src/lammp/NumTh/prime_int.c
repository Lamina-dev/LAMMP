#include "../../../include/lammp/impl/prime_table.h"

void lmmp_prime_int_init_(pri_int* p, uint n) {
    lmmp_param_assert(n >= 2);
    lmmp_param_assert(p != NULL);
    p->N = n;
    size_t pri_est = lmmp_prime_size_(n);
    p->pri = ALLOC_TYPE(pri_est, uint);
    p->mmp_cal = (n + ULONG_BITS - 1) / ULONG_BITS + 1;
    p->mmp = ALLOC_TYPE(p->mmp_cal, ulong);

// 1=非质数，0=质数
#define is_not_prime(wori, i) (p->mmp[wori] & (1ULL << (i)))
#define set_not_prime(_i_) p->mmp[(_i_) / ULONG_BITS] |= (1ULL << ((_i_) % ULONG_BITS))

    // 0和1标记为非质数，其余初始为质数（标记0）
    lmmp_zero(p->mmp, p->mmp_cal);
    p->mmp[0] = PRI_MMP_ZERO;

    p->prin = 0;

    for (ulong num = 2; num <= p->N; ++num) {
        if (!is_not_prime(num / ULONG_BITS, num % ULONG_BITS)) {
            p->pri[p->prin++] = num;
            lmmp_debug_assert(p->prin <= pri_est);
        }

        for (size_t j = 0; j < p->prin; ++j) {
            ulong pro = num * p->pri[j];
            if (pro > p->N)
                break;
            set_not_prime(pro);
            if (num % p->pri[j] == 0)
                break;
        }
    }

#undef is_not_prime
#undef set_not_prime
}

void lmmp_prime_int_free_(pri_int* p) {
    lmmp_param_assert(p != NULL);
    lmmp_free(p->pri);
    lmmp_free(p->mmp);
    p->mmp_cal = 0;
    p->prin = 0;
    p->N = 0;
}