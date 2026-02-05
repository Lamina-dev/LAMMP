#include "../include/benchmark.hpp"
#include <vector>

#include <immintrin.h>

#ifdef _MSC_VER
static inline uint64_t div128by64to64(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor) {
    uint64_t remainder;
    uint64_t quotient = _udiv128(dividend_hi64, dividend_lo64, divisor, &remainder);
    dividend_lo64 = remainder;
    return quotient;
}
#else
static inline uint64_t div128by64to64(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor) {
    uint64_t quotient_low, remainder;
    __asm__("div %[divisor]"                                                  // RDX:RAX ÷ 除数 → RAX=商, RDX=余数
            : "=a"(quotient_low), "=d"(remainder)                             // 输出：商=RAX，余数=RDX
            : "a"(dividend_lo64), "d"(dividend_hi64), [divisor] "r"(divisor)  // 输入：RAX=低64位,
                                                                              // RDX=高64位
            :                                                                 // 无寄存器污染
    );
    dividend_lo64 = remainder;  // 更新余数到被除数低位引用
    return quotient_low;
}
#endif

void bench_div_128() {
    uint64_t mod = 123291;
    int n = 1000000;
    mp_ptr a = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    mp_ptr b = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    mp_ptr c = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    mp_ptr d = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    mp_ptr e = (mp_ptr)lmmp_alloc(n * sizeof(mp_limb_t));
    ger_random_numbers(n, a, 1, mod-1);
    ger_random_numbers(n, b, 1, mod-1);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        mp_limb_t pro[2];
        lmmp_mullh_(a[i], b[i], pro);
        mp_size_t l = pro[1] == 0 ? 1 : 2;
        c[i] = lmmp_div_1_(NULL, pro, l, mod);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "bench_div_128: " << duration << " microseconds" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        mp_limb_t q = 0;
        d[i] = lmmp_mulmod_ulong_(a[i], b[i], mod, &q);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "bench_mod_128: " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        mp_limb_t pro[2];
        lmmp_mullh_(a[i], b[i], pro);
        e[i] = div128by64to64(pro[1], pro[0], mod);
        std::swap(pro[0], e[i]);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "bench_mod_128: " << duration << " microseconds" << std::endl;

    for (int i = 0; i < n; i++) {
        if (c[i] != d[i]) {
            std::cout << "i = " << i << std::endl;
            std::cout << "error: " << c[i] << " != " << d[i] << std::endl;
            break;
        }
        if (e[i] != d[i]) {
            std::cout << "i = " << i << std::endl;
            std::cout << "error: " << e[i] << " != " << d[i] << std::endl;
            break;
        }
    }

    lmmp_free(a);
    lmmp_free(b);
    lmmp_free(c);
    lmmp_free(d);
    lmmp_free(e);
}