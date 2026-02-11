/*
 * ==============================================================================
 *                       SECURITY & LIABILITY DISCLAIMER
 *                           安全与责任免责声明
 * ==============================================================================
 *
 * This library provides implementations of cryptographic and hash functions
 * as the underlying computation library for an open-source programming language.
 * 本库实现加密算法与哈希函数，作为一门开源编程语言的底层计算库。
 *
 * 1. NO SECURITY WARRANTY 无安全担保
 *    While the developer strives to implement algorithms correctly,
 *    this library is provided "AS IS" without any explicit or implied
 *    warranty of correctness, security, robustness, or fitness for any purpose.
 *    Undiscovered vulnerabilities, implementation flaws, or side-channel risks may exist.
 *
 *    开发者力求算法实现正确，但不对代码的绝对安全性、正确性、完整性
 *    或适用于特定用途作出任何明示或暗示保证。
 *    本库可能存在未发现的安全漏洞、实现缺陷或侧信道风险。
 *
 * 2. COMMITMENT AGAINST MALICIOUS VULNERABILITIES
 *    不主动植入恶意漏洞承诺
 *    The developer solemnly undertakes that the Library will NOT
 *    intentionally contain, reserve, or add any known or foreseeable
 *    security vulnerabilities, backdoors, or flaws that could be maliciously exploited.
 *
 *    开发者郑重承诺：不会在本库中故意保留、隐藏或添加
 *    任何明确存在、可预见、或可被恶意利用的安全漏洞、后门或缺陷。
 *
 * 3. PRODUCTION USE AT YOUR OWN RISK
 *    生产环境使用，风险自担
 *    Users may integrate this library into production environments at their own risk.
 *    It is the user's responsibility to conduct security audits, testing, validation,
 *    and comply with applicable laws and regulations before deployment.
 *
 *    使用者可将本库用于生产环境，但一切风险自行承担。
 *    使用者有义务自行完成安全审计、充分测试与合规验证，并承担相关责任。
 *
 * 4. NO LIABILITY FOR SECURITY ISSUES
 *    安全问题责任豁免
 *    The developer shall NOT be liable for any damages, losses, or incidents
 *    arising from security vulnerabilities, including but not limited to
 *    data breaches, system compromise, financial loss, or business interruption.
 *
 *    因本库安全问题所引发的一切损失、数据泄露、系统入侵、财产损害或业务中断，
 *    开发者均不承担任何法律责任。
 *
 * 5. WELCOME SECURITY DISCLOSURES
 *    欢迎安全漏洞反馈
 *    The developer welcomes responsible security disclosures, code reviews,
 *    and reports of potential vulnerabilities.
 *    Feedback helps improve the library, but does not impose any obligation
 *    of immediate fix or guaranteed maintenance.
 *
 *    开发者欢迎负责任的安全披露、代码审查与漏洞报告。
 *    社区反馈将用于改进本库，但开发者不承诺立即修复或持续维护。
 *
 * 6. GENERAL DISCLAIMER 通用免责
 *    The developer is not liable for any direct, indirect, incidental,
 *    or consequential damages resulting from the use of this library,
 *    including modified versions and redistributions.
 *
 *    开发者不对使用本库（含修改版本、二次分发）所产生的任何直接、间接损失承担责任。
 *
 * By using, compiling, modifying, or redistributing this library,
 * you acknowledge that you have read and agreed to this disclaimer.
 *
 * 使用、编译、修改或分发本库即表示你已阅读并同意本声明。
 *
 * Developer: HJimmyK (https://github.com/HJimmyK)
 * Date: 2026-02-11
 * This disclaimer applies permanently to this and future versions.
 * 本免责声明永久适用于当前版本及未来版本。
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

#ifdef __cplusplus
extern "C" {
#endif

typedef const uint64_t srckey64_t[1];
typedef const uint64_t srckey128_t[2];
typedef const uint64_t srckey256_t[4];

typedef uint64_t key64_t[1];
typedef uint64_t key128_t[2];
typedef uint64_t key256_t[4];


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

#ifdef __cplusplus
}
#endif

#endif // LAMMP_SECRET_H

