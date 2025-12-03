/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __LAMMP_BITS_U128_INL__
#define __LAMMP_BITS_U128_INL__
#include <cstddef>

namespace lammp {
// Compute Integer multiplication, 64bit x 64bit to 128bit, basic algorithm
// first is low 64bit, second is high 64bit
constexpr void mul64x64to128_base(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
    /*
    //// 一种更易于读懂的实现，但由于其存在两个分支预测，其性能略低于现行算法。
        const uint64_t al = a >> 32,
                    ar = uint32_t(a),
                    bl = b >> 32,
                    br = uint32_t(b);
        low  = ar * br;
        high = al * bl;

        const uint64_t t1 = al * br,
                    t2 = ar * bl,
                    t1addt2 = t1 + t2;

        if (t1addt2 < t2)high += 1ULL << 32;//如果溢出，即进位那么加上一

        low += t1addt2 << 32;
        if (low < (t1addt2 << 32))high += 1;//作用同上

        high += t1addt2 >> 32;
    */
    uint64_t ah = a >> 32, bh = b >> 32;
    a = uint32_t(a), b = uint32_t(b);
    uint64_t r0 = a * b, r1 = a * bh, r2 = ah * b, r3 = ah * bh;
    r3 += (r1 >> 32) + (r2 >> 32);
    r1 = uint32_t(r1), r2 = uint32_t(r2);
    r1 += r2;
    r1 += (r0 >> 32);
    high = r3 + (r1 >> 32);
    low = (r1 << 32) | uint32_t(r0);
}

static void mul64x64to128(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
#if defined(__x86_64__) || defined(_M_X64)
// x86-64架构：优先内联汇编（GCC/Clang/MSVC均支持）
// mul指令：将RAX与操作数相乘，结果存于RDX:RAX（高64位:低64位）
#if defined(_LAMMP_MSVC)
    // MSVC的内联汇编语法（与GCC略有不同）
    __asm {
            mov rax, a;  // RAX = a
            mul qword ptr b;  // RDX:RAX = a * b
            mov low, rax;  // 低64位存入low
            mov high, rdx;  // 高64位存入high
    }
#elif defined(_LAMMP_GCC)
    // GCC/Clang的内联汇编语法
    __asm__("mul %[b]"  // 执行 RDX:RAX = RAX * b（a已在RAX中）
            : "=a"(low),
              "=d"(high)          // 输出：low = RAX（低64位），high = RDX（高64位）
            : "a"(a), [b] "r"(b)  // 输入：a存入RAX，b存入任意寄存器
            :                     // 无额外寄存器被修改
    );
#endif

#elif defined(__aarch64__)
    // ARM64架构：优先内联汇编（GCC/Clang支持）
    // umull指令：无符号64x64乘法，结果存于两个64位寄存器
    __asm__("umull %[low], %[high], %[a], %[b]"   // low:high = a * b
            : [low] "=r"(low), [high] "=r"(high)  // 输出：low和high
            : [a] "r"(a), [b] "r"(b)              // 输入：a和b（任意寄存器）
            :                                     // 无寄存器污染
    );

#else
// 其他架构：优先使用编译器原生128位支持，再fallback到基础实现
#if defined(UINT128T)
    __uint128_t res = static_cast<__uint128_t>(a) * b;
    low = static_cast<uint64_t>(res);
    high = static_cast<uint64_t>(res >> 64);
#elif defined(_LAMMP_MSVC)
    // MSVC在非x86_64架构（如ARM64）可使用_umul128
    unsigned long long hi;
    low = _umul128(a, b, &hi);
    high = hi;
#else
    mul64x64to128_base(a, b, low, high);
#endif
#endif
}

constexpr uint32_t div128by32_base(uint64_t& dividend_hi64, uint64_t& dividend_lo64, uint32_t divisor) {
    uint32_t quot_hi32 = 0, quot_lo32 = 0;
    uint64_t dividend = dividend_hi64 >> 32;
    quot_hi32 = dividend / divisor;
    dividend %= divisor;

    dividend = (dividend << 32) | uint32_t(dividend_hi64);
    quot_lo32 = dividend / divisor;
    dividend %= divisor;
    dividend_hi64 = (uint64_t(quot_hi32) << 32) | quot_lo32;

    dividend = (dividend << 32) | uint32_t(dividend_lo64 >> 32);
    quot_hi32 = dividend / divisor;
    dividend %= divisor;

    dividend = (dividend << 32) | uint32_t(dividend_lo64);
    quot_lo32 = dividend / divisor;
    dividend %= divisor;
    dividend_lo64 = (uint64_t(quot_hi32) << 32) | quot_lo32;
    return dividend;
}

// 96bit integer divided by 64bit integer, input make sure the quotient smaller
// than 2^32.
constexpr uint32_t div96by64to32_base(uint32_t dividend_hi32, uint64_t& dividend_lo64, uint64_t divisor) {
    if (0 == dividend_hi32) {
        uint32_t quotient = dividend_lo64 / divisor;
        dividend_lo64 %= divisor;
        return quotient;
    }
    uint64_t divid2 = (uint64_t(dividend_hi32) << 32) | (dividend_lo64 >> 32);
    uint64_t divis1 = divisor >> 32;
    divisor = uint32_t(divisor);
    uint64_t qhat = divid2 / divis1;
    divid2 %= divis1;
    divid2 = (divid2 << 32) | uint32_t(dividend_lo64);
    uint64_t product = qhat * divisor;
    divis1 <<= 32;
    if (product > divid2) {
        qhat--;
        product -= divisor;
        divid2 += divis1;
        // if divid2 <= divis1, the addtion of divid2 is overflow, so product
        // must not be larger than divid2.
        if ((divid2 > divis1) && (product > divid2)) {
            qhat--;
            product -= divisor;
            divid2 += divis1;
        }
    }
    divid2 -= product;
    dividend_lo64 = divid2;
    return uint32_t(qhat);
}

// 128bit integer divided by 64bit integer, input make sure the quotient smaller
// than 2^64.
constexpr uint64_t div128by64to64_base(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor) {
    int k = 0;
    if (divisor < (uint64_t(1) << 63)) {
        k = lammp_clz(divisor);
        divisor <<= k;  // Normalization.
        dividend_hi64 = (dividend_hi64 << k) | (dividend_lo64 >> (64 - k));
        dividend_lo64 <<= k;
    }
    uint32_t divid_hi32 = dividend_hi64 >> 32;
    uint64_t divid_lo64 = (dividend_hi64 << 32) | (dividend_lo64 >> 32);
    uint64_t quotient = div96by64to32_base(divid_hi32, divid_lo64, divisor);

    divid_hi32 = divid_lo64 >> 32;
    dividend_lo64 = uint32_t(dividend_lo64) | (divid_lo64 << 32);
    quotient = (quotient << 32) | div96by64to32_base(divid_hi32, dividend_lo64, divisor);
    dividend_lo64 >>= k;
    return quotient;
}

// 128位无符号数除以64位无符号数（调用方式与div128by64to64_base一致）
// 输入：dividend_hi64（被除数高64位）、dividend_lo64（被除数低64位，引用）、divisor（除数）
// 输出：返回64位商（要求商 < 2^64，由调用者保证），通过dividend_lo64更新为余数
static inline uint64_t div128by64to64(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor) {
    // assert(divisor != 0 && "Division by zero in div128by64to64");
#if defined(__x86_64__) && !defined(_LAMMP_MSVC)
    // x86-64架构（GCC/Clang）：内联汇编实现，编译期不支持汇编
    uint64_t quotient_low, remainder;
    __asm__("div %[divisor]"                                                  // RDX:RAX ÷ 除数 → RAX=商, RDX=余数
            : "=a"(quotient_low), "=d"(remainder)                             // 输出：商=RAX，余数=RDX
            : "a"(dividend_lo64), "d"(dividend_hi64), [divisor] "r"(divisor)  // 输入：RAX=低64位,
                                                                              // RDX=高64位
            :                                                                 // 无寄存器污染
    );
    dividend_lo64 = remainder;  // 更新余数到被除数低位引用
    return quotient_low;

#elif defined(_M_X64) && defined(_LAMMP_MSVC)
    // x86-64架构（MSVC）：内联汇编，编译期不支持
    uint64_t quotient_low, remainder;
    __asm {
            mov rax, dividend_lo64;  // RAX = 被除数低64位
            mov rdx, dividend_hi64;  // RDX = 被除数高64位 → RDX:RAX为128位被除数
            div qword ptr divisor;  // 除法：RAX=商，RDX=余数
            mov quotient_low, rax;  // 保存商
            mov remainder, rdx;  // 保存余数
    }
    dividend_lo64 = remainder;
    return quotient_low;
#else
// 非x86-64架构：优先编译器128位支持
#if defined(UMUL128)
    __uint128_t dividend = static_cast<__uint128_t>(dividend_hi64) << 64 | dividend_lo64;
    __uint128_t div = static_cast<__uint128_t>(divisor);
    uint64_t quotient = static_cast<uint64_t>(dividend / div);
    dividend_lo64 = static_cast<uint64_t>(dividend % div);  // 更新余数
    return quotient;

#elif defined(_LAMMP_GCC)
    // MSVC（非x86-64）：128位除法，编译期支持有限
    unsigned __int128 dividend = static_cast<unsigned __int128>(dividend_hi64) << 64 | dividend_lo64;
    unsigned __int128 quotient = dividend / divisor;
    dividend_lo64 = static_cast<uint64_t>(dividend % divisor);
    return static_cast<uint64_t>(quotient);

#else
    return div128by64to64to64_base(dividend_hi64, dividend_lo64, divisor);
#endif
#endif
}

// uint64_t to std::string
inline std::string ui64to_string_base10(uint64_t input, uint8_t digits) {
    std::string result(digits, '0');
    for (uint8_t i = 0; i < digits; i++) {
        result[digits - i - 1] = static_cast<char>(input % 10 + '0');
        input /= 10;
    }
    return result;
}

// 计算 128位 × 128位 → 256位结果
// a = (a_high << 64) | a_low
// b = (b_high << 64) | b_low
// 结果通过 res[0]（最低64位）到 res[3]（最高64位）输出
static inline void umul128_to_256(uint64_t a_high, uint64_t a_low, uint64_t b_high, uint64_t b_low, uint64_t res[4]) {
    // 中间变量：存储4个64×64乘法的结果（高64位+低64位）
    // uint64_t p0_low, p0_high;  // p0 = a_low × b_low
    uint64_t p1_low, p1_high;  // p1 = a_low × b_high
    uint64_t p2_low, p2_high;  // p2 = a_high × b_low
    // uint64_t p3_low, p3_high;  // p3 = a_high × b_high
    mul64x64to128(a_low, b_low, res[0], res[1]);
    mul64x64to128(a_low, b_high, p1_low, p1_high);
    mul64x64to128(a_high, b_low, p2_low, p2_high);
    mul64x64to128(a_high, b_high, res[2], res[3]);
    /*
        | res0 | res1 | res2 | res3 |
        |  p0l |  p0h |      |      |
               |  p1l |  p1h |      |
               |  p2l |  p2h |      |
               |      |  p3l |  p3h |
    */
    res[1] += p1_low;
    uint64_t carry = (res[1] < p1_low) ? 1 : 0;
    res[1] += p2_low;
    carry += (res[1] < p2_low) ? 1 : 0;

    res[2] += carry;
    carry = (res[2] < carry) ? 1 : 0;
    res[2] += p1_high;
    carry += (res[2] < p1_high) ? 1 : 0;
    res[2] += p2_high;
    carry += (res[2] < p2_high) ? 1 : 0;

    res[3] += carry;
}
};
#endif // __LAMMP_BITS_U128_INL__