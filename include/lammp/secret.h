/*
 * ==============================================================================
 *                              免责声明 (DISCLAIMER)
 * ==============================================================================
 *
 * 【声明 - Statement】
 * 本密码学库（以下简称"本库"）仅为密码学理论学习、算法原理研究的教学用途而开发，
 * 开发者已尽合理努力保证代码的可读性和学习价值，但未对代码的安全性、正确性、完整性做任何承诺。
 * This cryptography library (hereinafter referred to as "the Library") is developed
 * solely for educational purposes of cryptography theory learning and algorithm principle research.
 * The developer has made reasonable efforts to ensure the readability and learning value of the code,
 * but makes no promises regarding the security, correctness, or completeness of the code.
 *
 * 【使用限制 - Usage Restrictions】
 * 1. 严禁将本库用于生产环境、商业场景、数据加密/解密、身份认证等任何需要保障数据安全的场景；
 *    It is strictly prohibited to use the Library in production environments, commercial scenarios,
 *    data encryption/decryption, identity authentication, or any scenarios requiring data security guarantees.
 *
 * 2. 本库可能存在未发现的安全漏洞、算法实现缺陷或逻辑错误，使用本库导致的任何直接/间接损失，
 *    包括但不限于数据泄露、财产损失、业务中断等，开发者不承担任何民事、行政或刑事责任；
 *    The Library may contain undiscovered security vulnerabilities, algorithm implementation flaws,
 *    or logical errors. The developer shall not be liable for any direct/indirect losses caused by 
 *    the use of the Library, including but not limited to data leakage, property loss, business interruption, etc.
 *
 * 3. 使用者在使用本库前，应充分知晓其学习属性和安全风险，所有使用行为均由使用者自行承担责任；
 *    Before using the Library, users shall fully understand its educational nature and security risks,
 *    and all usage behaviors shall be at the user's own risk.
 *
 * 【商用密码特别声明 - Special Statement on Commercial Cryptography】
 * 本库未经过国家密码管理局的商用密码检测认证，不包含合规的商用密码算法实现，
 * 严禁用于涉及商用密码应用的场景（如金融、政务、关键信息基础设施等）；
 * The Library has not passed the commercial cryptography testing and certification by the State Cryptography
 * Administration, and does not contain compliant commercial cryptography algorithm implementations. It is strictly
 * prohibited to use the Library in scenarios involving commercial cryptography applications (such as finance,
 * government affairs, critical information infrastructure, etc.).
 *
 * 【免责范围 - Scope of Disclaimer】
 * 开发者对本库的所有使用场景、衍生修改版本、二次分发行为均不承担任何责任；
 * 即使开发者被告知可能发生此类损失，也不承担任何赔偿责任。
 * The developer shall not be liable for any usage scenarios of the Library, modified derivative versions,
 * or secondary distribution behaviors.
 * Even if the developer is notified of the possibility of such losses, no liability for compensation shall be assumed.
 *
 * 【版本与更新 - Version and Update】
 * 本库可能随时更新或终止维护，开发者无义务通知使用者，也无义务修复已知/未知漏洞。
 * The Library may be updated or discontinued at any time, and the developer has no obligation to notify users
 * or fix known/unknown vulnerabilities.
 *
 * 使用者一旦下载、编译、运行或修改本库代码，即表示已阅读并同意本免责声明的全部条款。
 * By downloading, compiling, running, or modifying the Library's code, the user indicates that they have read
 * and agreed to all terms of this disclaimer.
 *
 * 起草者信息 (Developer Info)：HJimmyK (https://github.com/HJimmyK)
 * 起草日期 (Development Date)：[2026-01-23]
 * 有效期 (Validity)：永久有效
 * ==============================================================================
 */

#ifndef LAMMP_SECRET_H
#define LAMMP_SECRET_H

/*
  LAMMP中实现了两种hash函数：SipHash-2-4 和 xxhash。其中前者通常被认为安全性更好，而后者提供更快的速度，测量发现：xxhash
  生成的hash值统计性能微优，同时生成速度比 SipHash-2-4 快四倍左右。两者生成的 hash 值均具有非常良好的统计性能。

  需要注意的是，两种hash函数都不是标准处理任意字节流的 hash 函数，因此在 LAMMP 中，它们仅用于对整数数据进行 hash 计算，
  尽管这可能带来未知的安全风险，但如果仅作为hash表的键值，则影响不大。但对于字节流数据，应使用标准的 hash 函数，我们建议
  使用其他更强的加密算法或协议来处理。
 */

#include "lmmpn.h"

#ifndef INLINE_
#define INLINE_ static inline
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef const uint64_t srckey64_t;
typedef const uint64_t srckey128_t[2];
typedef const uint64_t srckey256_t[4];

typedef uint64_t key64_t;
typedef uint64_t key128_t[2];
typedef uint64_t key256_t[4];


INLINE_ uint64_t rotl64(uint64_t x, int b) { 
    b &= 63; 
    return (x << b) | (x >> (64 - b)); 
}

/**
 * @brief SipHash-2-4 函数（并非标准处理任意字节流的 SipHash-2-4）
 * @param in 输入数据，可以为 NULL
 * @param inlen 输入数据长度
 * @param key 128-bit 秘钥，可以为 NULL
 * @warning 若 key 为 NULL，则使用全零秘钥
 * @return 64-bit hash 值
 */
uint64_t lmmp_siphash24_(mp_srcptr in, mp_size_t inlen, srckey128_t key);

/**
 * @brief xxhash 函数（标准处理任意字节流的 xxhash）
 * @param in 输入数据，可以为 NULL
 * @param inlen 输入数据长度
 * @param seed 种子，可以为 NULL
 * @warning 若 seed 为 NULL，则使用全零种子
 */
uint64_t lmmp_xxhash_(mp_srcptr in, mp_size_t inlen, srckey64_t key);

#ifdef INLINE_
#undef INLINE_  
#endif

#ifdef __cplusplus
}
#endif

#endif // LAMMP_SECRET_H

