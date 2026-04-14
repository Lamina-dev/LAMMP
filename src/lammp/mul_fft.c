/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include "../../include/lammp/impl/mparam.h"
#include "../../include/lammp/impl/tmp_alloc.h"
#include "../../include/lammp/lmmpn.h"

// ((mp_size_t)3 << (2 * (n) - 5)) + 1 是预计算的阈值，n是对应的k值
#define _FFT_TABLE_ENTRY(n) {((mp_size_t)3 << (2 * (n) - 5)) + 1, (n)}
#define _FFT_TABLE_ENTRY4(n) \
    _FFT_TABLE_ENTRY(n), _FFT_TABLE_ENTRY((n) + 1), _FFT_TABLE_ENTRY((n) + 2), _FFT_TABLE_ENTRY((n) + 3)

// best_k_(next_size_(n)) = best_k_(n)
// table[i+1][0]-1 必须是 2^(table[i][1]-LOG2_LIMB_BITS) 的整数倍
// LOG2_LIMB_BITS：每个 limb 的比特数的2对数，为 log2(64) = 6
static const mp_size_t lmmp_fft_table_[][2] = {
    {0, 6},
    {1597, 7},
    {1655, 6},
    {1917, 7},
    {3447, 8},
    {3565, 7},
    {3831, 8},
    {7661, 9},
    {8145, 8},
    {8685, 9},
    {14289, 10},
    {16289, 9},
    {20433, 10},
    {24481, 9},
    {26577, 10},
    {28593, 11},
    {32545, 10},
    {57249, 11},
    {65313, 10},
    {73633, 11},
    {98081, 12},
    {130625, 11},
    {196385, 12},
    {261697, 11},
    {294689, 12},
    {392769, 13},
    {523265, 12},
    {654913, 11},
    {917281, 13},
    {1047553, 11},
    {1600001, 12},
    {1834561, 14},
    {2095105, 12},
    _FFT_TABLE_ENTRY4(13),
    _FFT_TABLE_ENTRY4(17),
    _FFT_TABLE_ENTRY4(21),
    _FFT_TABLE_ENTRY4(25),
    {(mp_size_t)-1, 127}};

typedef struct {
    mp_ptr temp_coef;       // 用于数据交换的临时系数数组
    mp_size_t lenw;         // 系数的机器字（limb）长度
    mp_ssize_t maxdepth;    // 内存栈的最大深度（已分配的层数）
    mp_ssize_t tempdepth;   // 内存栈的当前深度（正在使用的层数）
    void* mem[16];          // 存储16层内存块的指针
    mp_size_t memsize[16];  // 存储每层内存块的大小（以字节为单位）
} fft_memstack;

/**
 * @brief 查找对于 m>=n 的模 B^m+1 FFT运算的最优k值
 * @param n - 输入的机器字长度
 * @return 最优的k值
 */
static inline mp_size_t lmmp_fft_best_k_(mp_size_t n) {
    mp_size_t k = 0;
    while (n >= lmmp_fft_table_[k + 1][0]) ++k;
    return lmmp_fft_table_[k][1];
}

/**
 * @brief 计算FFT运算所需的最小规整化长度（向上取整到2^k的倍数）
 * @param n - 原始长度
 * @return 规整后的长度（为2^k的倍数）
 */
mp_size_t lmmp_fft_next_size_(mp_size_t n) {
    mp_size_t k = lmmp_fft_best_k_(n);
    lmmp_debug_assert(k >= LOG2_LIMB_BITS);
    k -= LOG2_LIMB_BITS;
    n = (((n - 1) >> k) + 1) << k;
    return n;
}

/**
 * @brief FFT内存栈的分配/释放接口
 * @param ms - 内存栈结构体栈帧
 * @param size - 分配大小（字节），size=0表示释放当前层内存
 * @return 分配成功：返回mp_ptr*；释放：返回0
 */
static void* lmmp_fft_memstack_(fft_memstack* ms, mp_size_t size) {
    if (size) {
        if (++ms->tempdepth > ms->maxdepth) {
            ms->mem[++ms->maxdepth] = lmmp_alloc(size);
            ms->memsize[ms->maxdepth] = size;
        }
        lmmp_debug_assert(ms->memsize[ms->tempdepth] == size);
        return ms->mem[ms->tempdepth];
    } else {
        if (--ms->tempdepth < 0) {
            for (mp_size_t i = 0; i <= (mp_size_t)(ms->maxdepth); ++i) lmmp_free(ms->mem[i]);
            ms->maxdepth = -1;
        }
        return 0;
    }
}

/**
 * @brief [dst,lenw+1] = [(bit*)numa+bitoffset, bits]
 * @param dst - 输出系数数组（长度lenw+1）
 * @param numa - 输入大数指针
 * @param bitoffset - 起始比特偏移量（>=0）
 * @param bits - 提取的比特数（0 < bits <= LIMB_BITS*lenw）
 * @param lenw - 输出系数的机器字长度
 * @warning bitoffset>=0, 0<bits<=LIMB_BITS*lenw, sep(dst,numa)
 */
static void lmmp_fft_extract_coef_(mp_ptr dst, mp_srcptr numa, mp_size_t bitoffset, mp_size_t bits, mp_size_t lenw) {
    // shr = 机器字内的比特偏移（0~LIMB_BITS-1）
    // offset = 起始机器字的索引
    mp_size_t shr = bitoffset & (LIMB_BITS - 1), offset = bitoffset / LIMB_BITS;

    mp_size_t lena = (bitoffset + bits - 1) / LIMB_BITS - offset + 1, endp = (bits - 1) / LIMB_BITS;

    if (shr)
        lmmp_shr_(dst, numa + offset, lena, shr);
    else
        lmmp_copy(dst, numa + offset, lena);

    dst[endp] &= LIMB_MAX >> (-bits & (LIMB_BITS - 1));

    lmmp_zero(dst + endp + 1, lenw - endp);
}

/**
 * @brief 对模 2^n+1 的系数执行左移操作
 * @param ms - 内存栈结构体指针
 * @param coef - 输入输出系数数组指针（指针的指针，用于交换内存）
 * @param shl - 左移的比特数（0<shl<2*n）
 * @warning n = ms->lenw * LIMB_BITS
 *         *coef 已伪归一化（mod 2^n+1）
 *         ms->temp_coef 至少有 lenw+1 个机器字
 */
static void lmmp_fft_shl_coef_(fft_memstack* ms, mp_ptr* coef, mp_size_t shl) {
    mp_size_t l = ms->lenw;         // 系数的机器字长度
    mp_size_t w = shl / LIMB_BITS;  // 左移的机器字数量
    shl &= LIMB_BITS - 1;           // 剩余的比特偏移（0~LIMB_BITS-1）
    mp_ptr src = *coef;             // 源系数数组
    mp_ptr dst = ms->temp_coef;     // 目标临时数组
    mp_limb_t cc, rd;               // 进位变量（cc=carry, rd=read）

    if (w >= l) {
        w -= l;
        if (shl) {
            lmmp_shl_(dst, src + l - w, w + 1, shl);
            rd = dst[w];
            cc = lmmp_shlnot_(dst + w, src, l - w, shl);
        } else {
            if (w)
                lmmp_copy(dst, src + l - w, w);
            rd = src[l];
            lmmp_not_(dst + w, src, l - w);
            cc = 0;
        }
        dst[l] = 0;
        ++cc;
        lmmp_inc_1(dst, cc);

        if (++rd == 0)
            lmmp_inc(dst + w + 1);
        else
            lmmp_inc_1(dst + w, rd);
    } else {
        if (shl) {
            lmmp_shlnot_(dst, src + l - w, w + 1, shl);
            rd = ~dst[w];
            cc = lmmp_shl_(dst + w, src, l - w, shl);
        } else {
            if (w)
                lmmp_not_(dst, src + l - w, w);
            rd = src[l];

            lmmp_copy(dst + w, src, l - w);
            cc = 0;
        }
        dst[l] = 2;
        lmmp_inc_1(dst, 3);
        lmmp_dec_1(dst, cc);

        if (++rd == 0)
            lmmp_dec(dst + w + 1);
        else
            lmmp_dec_1(dst + w, rd);

        cc = dst[l];
        dst[l] = dst[0] < cc;
        lmmp_dec_1(dst, cc - dst[l]);
    }

    ms->temp_coef = src;
    *coef = dst;
}

/**
 * @brief 对模 2^n+1 的系数执行右移操作
 * 右移shr位 = 左移(2n - shr)位（mod 2^n+1的循环特性）
 * @param ms - 内存栈结构体指针
 * @param coef - 输入输出系数数组指针
 * @param shr - 右移的比特数（0 < shr < 2*n）
 */
static void lmmp_fft_shr_coef_(fft_memstack* ms, mp_ptr* coef, mp_size_t shr) {
    lmmp_fft_shl_coef_(ms, coef, 2 * ms->lenw * LIMB_BITS - shr);
}

/**
 * @brief FFT蝶形运算（Butterfly Operation）
 * (a,b) = (a + b, (a-b) << w ) mod 2^n+1
 * a=[coef[0],ms->lenw+1], b=[coef[wing],ms->lenw+1], n=ms->lenw * LIMB_BITS
 * @param ms - 内存栈结构体指针
 * @param coef - 系数数组指针数组（coef[0]=a, coef[wing]=b）
 * @param wing - b的索引
 * @param w - 左移的比特数（0<=w<n）
 * @warning n = ms->lenw * LIMB_BITS
 *          a,b 均已伪归一化（mod 2^n+1）
 *          ms->temp_coef 有至少 lenw + 1 个字长
 */
static void lmmp_fft_bfy_(fft_memstack* ms, mp_ptr* coef, mp_size_t wing, mp_size_t w) {
    mp_ptr numa = coef[0];                // 系数a
    mp_ptr numb = coef[wing];             // 系数b
    mp_ptr numc = ms->temp_coef;          // 临时数组（存储a-b<<w）
    mp_size_t shl = w & (LIMB_BITS - 1);  // 比特级左移量
    w /= LIMB_BITS;                       // 机器字级左移量
    mp_size_t l = ms->lenw;               // 系数长度（机器字）

    mp_slimb_t acyo = 0, scyo = 0, ch;
    mp_limb_t shlcyo = 0, chp = 0, chn = 0;

    for (mp_size_t off = 0; off < l - w; off += PART_SIZE) {
        mp_size_t cursize = LMMP_MIN(l - w - off, PART_SIZE);
        scyo = lmmp_sub_nc_(numc + w + off, numa + off, numb + off, cursize, scyo);
        acyo = lmmp_add_nc_(numa + off, numa + off, numb + off, cursize, acyo);
        if (shl)
            shlcyo = lmmp_shl_c_(numc + w + off, numc + w + off, cursize, shl, shlcyo);
    }

    ch = shlcyo + (-scyo << shl);
    if (ch > 0)
        chp = ch;
    else
        chn = -ch;

    scyo = 0;
    shlcyo = 0;

    for (mp_size_t off = l - w; off < l; off += PART_SIZE) {
        mp_size_t cursize = LMMP_MIN(l - off, PART_SIZE);
        scyo = lmmp_sub_nc_(numc + off - (l - w), numb + off, numa + off, cursize, scyo);
        acyo = lmmp_add_nc_(numa + off, numa + off, numb + off, cursize, acyo);
        if (shl)
            shlcyo = lmmp_shl_c_(numc + off - (l - w), numc + off - (l - w), cursize, shl, shlcyo);
    }

    numc[w] += shlcyo;                 // 左移进位加到numc[w]
    scyo = -scyo + numb[l] - numa[l];  // 调整借位（包含最高位）
    acyo += numa[l] + numb[l];         // 调整进位（包含最高位）

    numa[l] = numa[0] < (mp_limb_t)(acyo);
    lmmp_dec_1(numa, acyo - numa[l]);

    numc[l] = 1;
    ++chn;
    if (scyo > 0)
        lmmp_inc_1(numc + w, scyo << shl);
    else if (scyo < 0) {
        if (scyo == -2 && shl == LIMB_BITS - 1)
            lmmp_dec(numc + w + 1);
        else
            lmmp_dec_1(numc + w, -scyo << shl);
    }
    chp += numc[l];

    if (chn >= chp) {
        numc[l] = 0;
        lmmp_inc_1(numc, chn - chp);
    } else {
        chp -= chn;
        numc[l] = numc[0] < chp;
        lmmp_dec_1(numc, chp - numc[l]);
    }

    coef[wing] = numc;
    ms->temp_coef = numb;
}

/**
 * @brief FFT蝶形运算（Butterfly Operation）
 * (a,b) = (a+(b>>w), a-(b>>w)) mod 2^n+1
 * a=[coef[0],ms->lenw+1], b=[coef[wing],ms->lenw+1], n=ms->lenw * LIMB_BITS
 * @param ms - 内存栈结构体指针
 * @param coef - 系数数组指针数组（coef[0]=a, coef[wing]=b）
 * @param wing - b的索引
 * @param w - 左移的比特数（0<=w<n）
 * @warning n = ms->lenw * LIMB_BITS
 *          a,b 均已伪归一化（mod 2^n+1）
 *          ms->temp_coef 有至少 lenw + 1 个字长
 */
static void lmmp_ifft_bfy_(fft_memstack* ms, mp_ptr* coef, mp_size_t wing, mp_size_t w) {
    mp_ptr numa = coef[0];                // 系数a
    mp_ptr numb = coef[wing];             // 系数b
    mp_ptr numc = ms->temp_coef;          // 临时数组（存储a-(b>>w)）
    mp_size_t shr = w & (LIMB_BITS - 1);  // 比特级右移量
    w /= LIMB_BITS;                       // 机器字级右移量
    mp_size_t l = ms->lenw;               // 系数长度

    mp_slimb_t bcyo = 0, acyo = 0, ah;
    mp_limb_t shrcyo = shr ? numb[0] << (LIMB_BITS - shr) : 0;

    for (mp_size_t off = l - w; off < l; off += PART_SIZE) {
        mp_size_t cursize = LMMP_MIN(l - off, PART_SIZE);
        if (shr)
            lmmp_shr_c_(numb + off - (l - w), numb + off - (l - w), cursize, shr,
                        numb[off - (l - w) + cursize] << (LIMB_BITS - shr));
        bcyo = lmmp_add_nc_(numc + off, numa + off, numb + off - (l - w), cursize, bcyo);
        acyo = lmmp_sub_nc_(numa + off, numa + off, numb + off - (l - w), cursize, acyo);
    }

    for (mp_size_t off = 0; off < l - w; off += PART_SIZE) {
        mp_size_t cursize = LMMP_MIN(l - w - off, PART_SIZE);
        if (shr)
            lmmp_shr_c_(numb + w + off, numb + w + off, cursize, shr, numb[off + w + cursize] << (LIMB_BITS - shr));
        bcyo = lmmp_sub_nc_(numc + off, numa + off, numb + w + off, cursize, bcyo);
        acyo = lmmp_add_nc_(numa + off, numa + off, numb + w + off, cursize, acyo);
    }

    acyo += numb[l] >> shr;
    bcyo = -bcyo - (numb[l] >> shr);

    acyo -= numa[l - w - 1] < shrcyo;
    numa[l - w - 1] -= shrcyo;
    numc[l - w - 1] += shrcyo;
    bcyo += numc[l - w - 1] < shrcyo;

    ah = numa[l];

    numa[l] += 1;
    if (w == 0)
        numa[l] += acyo;
    else {
        if (acyo < 0)
            lmmp_dec(numa + l - w);
        else
            lmmp_inc_1(numa + l - w, acyo);
    }
    acyo = numa[l] - 1;
    if (acyo < 0) {
        numa[l] = 0;
        lmmp_inc(numa);
    } else {
        numa[l] = numa[0] < (mp_limb_t)acyo;
        lmmp_dec_1(numa, acyo - numa[l]);
    }

    numc[l] = ah + 2;
    if (w == 0)
        numc[l] += bcyo;
    else {
        if (bcyo > 0)
            lmmp_inc(numc + l - w);
        else
            lmmp_dec_1(numc + l - w, -bcyo);
    }
    bcyo = numc[l] - 2;
    if (bcyo <= 0) {
        numc[l] = 0;
        lmmp_inc_1(numc, -bcyo);
    } else {
        numc[l] = numc[0] < (mp_limb_t)bcyo;
        lmmp_dec_1(numc, bcyo - numc[l]);
    }

    coef[wing] = numc;
    ms->temp_coef = numb;
}

/**
 * @brief FFT递归函数
 * @param ms - 内存栈结构体指针
 * @param coef - 系数数组指针数组
 * @param dis - 索引步长
 * @param k - FFT层数（递归深度）
 * @param w - 每次蝶形运算的移位基数
 * @param w0 - 初始移位偏移
 */
static void lmmp_fft_b1_(fft_memstack* ms, mp_ptr* coef, mp_size_t dis, mp_size_t k, mp_size_t w, mp_size_t w0) {
    if (k == 1)
        lmmp_fft_bfy_(ms, coef, dis, w0);
    else {
        k -= 2;
        mp_size_t Kq = dis << k;
        for (mp_size_t i = 0; i < Kq; i += dis) {
            lmmp_fft_bfy_(ms, coef + i, 2 * Kq, i * w + w0);
            lmmp_fft_bfy_(ms, coef + i + Kq, 2 * Kq, (i + Kq) * w + w0);
            lmmp_fft_bfy_(ms, coef + i, Kq, 2 * (i * w + w0));
            lmmp_fft_bfy_(ms, coef + i + Kq * 2, Kq, 2 * (i * w + w0));
        }
        if (k > 0) {
            lmmp_fft_b1_(ms, coef, dis, k, 4 * w, 4 * w0);
            lmmp_fft_b1_(ms, coef + Kq, dis, k, 4 * w, 4 * w0);
            lmmp_fft_b1_(ms, coef + Kq * 2, dis, k, 4 * w, 4 * w0);
            lmmp_fft_b1_(ms, coef + Kq * 3, dis, k, 4 * w, 4 * w0);
        }
    }
}

static void lmmp_fft_4_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
    if (k == 1)
        lmmp_fft_bfy_(ms, coef, 1, 0);
    else {
        k -= 2;
        mp_size_t Kq = ((mp_size_t)1) << k;
        for (mp_size_t i = 0; i < Kq; ++i) {
            lmmp_fft_bfy_(ms, coef + i, Kq * 2, i * w);
            lmmp_fft_bfy_(ms, coef + i + Kq, Kq * 2, (i + Kq) * w);
            lmmp_fft_bfy_(ms, coef + i, Kq, 2 * i * w);
            lmmp_fft_bfy_(ms, coef + i + 2 * Kq, Kq, 2 * i * w);
        }
        if (k > 0) {
            lmmp_fft_4_(ms, coef, k, w * 4);
            lmmp_fft_4_(ms, coef + Kq, k, w * 4);
            lmmp_fft_4_(ms, coef + 2 * Kq, k, w * 4);
            lmmp_fft_4_(ms, coef + 3 * Kq, k, w * 4);
        }
    }
}

static void lmmp_fft_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
    mp_size_t k1 = k >> 1;                // k1 = k/2（右移1位等价于除以2）
    k -= k1;                              // k = k - k1（剩余层数）
    mp_size_t Kp = ((mp_size_t)1) << k;   // Kp = 2^k
    mp_size_t Kq = ((mp_size_t)1) << k1;  // Kq = 2^k1

    for (mp_size_t i = 0; i < Kp; ++i) lmmp_fft_b1_(ms, coef + i, Kp, k1, w, i * w);

    for (mp_size_t i = 0; i < Kq; ++i) lmmp_fft_4_(ms, coef + Kp * i, k, w * Kq);
}

static void lmmp_ifft_b1_(fft_memstack* ms, mp_ptr* coef, mp_size_t dis, mp_size_t k, mp_size_t w, mp_size_t w0) {
    if (k == 1)
        lmmp_ifft_bfy_(ms, coef, dis, w0);
    else {
        k -= 2;
        mp_size_t Kq = dis << k;
        if (k > 0) {
            lmmp_ifft_b1_(ms, coef, dis, k, 4 * w, 4 * w0);
            lmmp_ifft_b1_(ms, coef + Kq, dis, k, 4 * w, 4 * w0);
            lmmp_ifft_b1_(ms, coef + Kq * 2, dis, k, 4 * w, 4 * w0);
            lmmp_ifft_b1_(ms, coef + Kq * 3, dis, k, 4 * w, 4 * w0);
        }
        for (mp_size_t i = 0; i < Kq; i += dis) {
            lmmp_ifft_bfy_(ms, coef + i, Kq, 2 * (i * w + w0));
            lmmp_ifft_bfy_(ms, coef + i + Kq * 2, Kq, 2 * (i * w + w0));
            lmmp_ifft_bfy_(ms, coef + i, 2 * Kq, i * w + w0);
            lmmp_ifft_bfy_(ms, coef + i + Kq, 2 * Kq, (i + Kq) * w + w0);
        }
    }
}

static void lmmp_ifft_4_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
    if (k == 1)
        lmmp_ifft_bfy_(ms, coef, 1, 0);
    else {
        k -= 2;
        mp_size_t Kq = ((mp_size_t)1) << k;
        if (k > 0) {
            lmmp_ifft_4_(ms, coef, k, w * 4);
            lmmp_ifft_4_(ms, coef + Kq, k, w * 4);
            lmmp_ifft_4_(ms, coef + 2 * Kq, k, w * 4);
            lmmp_ifft_4_(ms, coef + 3 * Kq, k, w * 4);
        }
        for (mp_size_t i = 0; i < Kq; ++i) {
            lmmp_ifft_bfy_(ms, coef + i, Kq, 2 * i * w);
            lmmp_ifft_bfy_(ms, coef + i + 2 * Kq, Kq, 2 * i * w);
            lmmp_ifft_bfy_(ms, coef + i, Kq * 2, i * w);
            lmmp_ifft_bfy_(ms, coef + i + Kq, Kq * 2, (i + Kq) * w);
        }
    }
}

static void lmmp_ifft_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
    mp_size_t k1 = k >> 1;                // k1 = k/2
    k -= k1;                              // k = k - k1
    mp_size_t Kp = ((mp_size_t)1) << k;   // Kp = 2^k
    mp_size_t Kq = ((mp_size_t)1) << k1;  // Kq = 2^k1

    for (mp_size_t i = 0; i < Kq; ++i) lmmp_ifft_4_(ms, coef + Kp * i, k, w * Kq);

    for (mp_size_t i = 0; i < Kp; ++i) lmmp_ifft_b1_(ms, coef + i, Kp, k1, w, i * w);
}

/**
 * @brief 费马变换 模 B^n+1 乘法的结果合并
 * @param ms - 内存栈结构体指针
 * @param dst - 输出结果数组
 * @param pfca - FFT系数数组指针数组
 * @param K - FFT块数（2^k）
 * @param k - FFT层数
 * @param n - 系数总比特数
 * @param M - 每个块的比特数
 * @param rn - 结果长度（机器字）
 */
static void lmmp_mul_fermat_recombine_(
    fft_memstack* ms,
    mp_ptr       dst,
    mp_ptr*     pfca,
    mp_size_t      K,
    mp_size_t      k,
    mp_size_t      n,
    mp_size_t      M,
    mp_size_t     rn
) {
    mp_size_t rhead = 0, nlen = ms->lenw + 1;
    mp_slimb_t borrow = 0, maxc = 0;

    for (mp_size_t i = 0; i < K; ++i) {
        lmmp_fft_shr_coef_(ms, pfca + i, (i * n >> k) + k);
        mp_ptr nums = pfca[i];

        if (nums[nlen - 1]) {
            lmmp_dec(nums);
            --nums[nlen - 1];
        }
        if (nums[nlen - 1] == 0 && nums[nlen - 2] >> (LIMB_BITS - 1)) {
            lmmp_dec(nums);
            --nums[nlen - 1];
        }

        if (borrow) {
            mp_size_t brshift = borrow - 1 + n - M;
            mp_size_t bshl = brshift & (LIMB_BITS - 1);
            brshift /= LIMB_BITS;
            --nums[nlen - 1];
            lmmp_dec_1(nums + brshift, (mp_limb_t)1 << bshl);
            ++nums[nlen - 1];
        }
        borrow = -nums[nlen - 1];
        nums[nlen - 1] = 0;

        mp_size_t roffset = i * M;
        mp_size_t shl = roffset & (LIMB_BITS - 1);
        roffset /= LIMB_BITS;

        if (shl)
            lmmp_shl_(nums, nums, nlen, shl);

        if (i == 0) {
            lmmp_copy(dst, nums, nlen);
            rhead = nlen;
        } else if (roffset + nlen <= rn) {
            lmmp_add_(dst + roffset, nums, nlen, dst + roffset, rhead - roffset);
            rhead = roffset + nlen;
        } else {
            maxc += lmmp_add_(dst + roffset, nums, rn - roffset, dst + roffset, rhead - roffset);
            maxc -= lmmp_sub_(dst, dst, rn, nums + rn - roffset, nlen + roffset - rn);
            rhead = rn;
        }
    }

    if (borrow) {
        mp_size_t cyshift = borrow - 1 + n - M;
        mp_size_t cshl = cyshift & (LIMB_BITS - 1);
        cyshift /= LIMB_BITS;
        maxc += lmmp_add_1_(dst + cyshift, dst + cyshift, rn - cyshift, (mp_limb_t)1 << cshl);
    }

    if (maxc > 0) {
        dst[rn] = dst[0] < (mp_limb_t)maxc;
        lmmp_dec_1(dst, maxc - dst[rn]);
    } else {
        dst[rn] = 0;
        lmmp_inc_1(dst, -maxc);
    }
}

/**
 * @brief 费马变换乘法递归函数（核心乘法逻辑）
 * @param ms - 内存栈结构体指针
 * @param pc1 - 第一个数的FFT系数数组指针数组
 * @param pc2 - 第二个数的FFT系数数组指针数组
 * @param K0 - FFT块数
 * @warning K0>0
 *          所有系数均已伪归一化（mod B^lenw+1）
 *          nsqr=1表示乘法，nsqr=0表示平方
 */
static void lmmp_mul_fermat_recurse_(fft_memstack* ms, mp_ptr* pc1, mp_ptr* pc2, mp_size_t K0) {
    int nsqr = pc1 != pc2;  // 判断是否为平方运算
    mp_ptr push_temp_coef = ms->temp_coef;
    mp_size_t rn = ms->lenw;  // 当前系数长度

    // 小于阈值则不使用FFT
    if (rn < MUL_FFT_MODF_THRESHOLD) {
        mp_ptr temp_mul = (mp_ptr)lmmp_fft_memstack_(ms, (rn + 1) * 2 * LIMB_BYTES);
        for (mp_size_t i = 0; i < K0; ++i) {
            if (nsqr)
                lmmp_mul_n_(temp_mul, pc1[i], pc2[i], rn + 1);
            else
                lmmp_sqr_(temp_mul, pc1[i], rn + 1);

            // 模 B^rn+1 归一化：temp_mul - temp_mul[rn ...]
            mp_limb_t maxc = lmmp_sub_n_(pc1[i], temp_mul, temp_mul + rn, rn) + temp_mul[rn * 2];
            pc1[i][rn] = 0;
            lmmp_inc_1(pc1[i], maxc);
        }
        lmmp_fft_memstack_(ms, 0);
    } else {
        mp_size_t N = rn * LIMB_BITS;        // 总比特数
        mp_size_t k = lmmp_fft_best_k_(rn);  // 最优FFT层数
        mp_size_t K = ((mp_size_t)1) << k;   // FFT块数（2^k）
        lmmp_debug_assert(!(N & (K - 1)));
        mp_size_t M = N >> k;         // 每个块的比特数（N/K）
        mp_size_t n = 2 * M + k + 2;  // 扩展系数长度（保证归一化）

        // 规整n：必须是LIMB_BITS和K的整数倍
        n = (n + LIMB_BITS - 1) & (-LIMB_BITS);  // 向上取整到LIMB_BITS的倍数
        n = (((n - 1) >> k) + 1) << k;           // 向上取整到K的倍数

        ms->lenw = n / LIMB_BITS;
        mp_size_t nlen = ms->lenw + 1;

        ms->temp_coef = (mp_ptr)lmmp_fft_memstack_(ms, (((nlen + 1) << (k + nsqr)) + nlen) * LIMB_BYTES);
        mp_ptr *pfca = (mp_ptr*)(ms->temp_coef + nlen), *pfcb = pfca;
        for (mp_size_t i = 0; i < K; ++i) pfca[i] = (mp_ptr)(pfca + K) + i * nlen;
        if (nsqr) {
            pfcb += (nlen + 1) << k;
            for (mp_size_t i = 0; i < K; ++i) pfcb[i] = (mp_ptr)(pfcb + K) + i * nlen;
        }

        for (mp_size_t j = 0; j < K0; ++j) {
            mp_ptr numa = pc1[j];
            mp_ptr numb = pc2[j];

            for (mp_size_t i = 0; i < K; ++i) {
                lmmp_fft_extract_coef_(pfca[i], numa, M * i, M + (i == K - 1), ms->lenw);
                if (i > 0)
                    lmmp_fft_shl_coef_(ms, pfca + i, i * n >> k);
            }
            lmmp_fft_(ms, pfca, k, n >> (k - 1));

            if (nsqr) {
                for (mp_size_t i = 0; i < K; ++i) {
                    lmmp_fft_extract_coef_(pfcb[i], numb, M * i, M + (i == K - 1), ms->lenw);
                    if (i > 0)
                        lmmp_fft_shl_coef_(ms, pfcb + i, i * n >> k);
                }
                lmmp_fft_(ms, pfcb, k, n >> (k - 1));
            }

            // dot product
            lmmp_mul_fermat_recurse_(ms, pfca, pfcb, K);

            lmmp_ifft_(ms, pfca, k, n >> (k - 1));

            lmmp_mul_fermat_recombine_(ms, numa, pfca, K, k, n, M, rn);
        }
        lmmp_fft_memstack_(ms, 0);
    }

    ms->temp_coef = push_temp_coef;
    ms->lenw = rn;
}

void lmmp_mul_fermat_(mp_ptr dst, mp_size_t rn, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb) {
    int nsqr = numa != numb || na != nb;  // 判断是否为平方运算
    mp_size_t N = rn * LIMB_BITS;         // 结果总比特数
    mp_size_t k = lmmp_fft_best_k_(rn);   // 最优FFT层数
    mp_size_t K = ((mp_size_t)1) << k;    // FFT块数（2^k）
    lmmp_debug_assert(!(N & (K - 1)));
    mp_size_t M = N >> k;         // 每个块的比特数
    mp_size_t n = 2 * M + k + 2;  // 扩展系数长度

    n = (n + LIMB_BITS - 1) & (-LIMB_BITS);
    n = (((n - 1) >> k) + 1) << k;

    // 初始化内存栈
    fft_memstack msr;
    msr.maxdepth = -1;
    msr.tempdepth = -1;
    msr.lenw = n / LIMB_BITS;       // 系数长度（机器字）
    mp_size_t nlen = msr.lenw + 1;  // 系数总长度

    msr.temp_coef = (mp_ptr)lmmp_fft_memstack_(&msr, (((nlen + 1) << (k + nsqr)) + nlen) * LIMB_BYTES);

    mp_ptr *pfca = (mp_ptr*)(msr.temp_coef + nlen), *pfcb = pfca;
    mp_size_t narest = na * LIMB_BITS, nbrest = nb * LIMB_BITS;

    for (mp_size_t i = 0; i < K; ++i) {
        mp_size_t coeflen;
        pfca[i] = (mp_ptr)(pfca + K) + i * nlen;
        if (narest > 0) {
            coeflen = M + (i == K - 1);
            coeflen = LMMP_MIN(narest, coeflen);
            narest -= coeflen;
            lmmp_fft_extract_coef_(pfca[i], numa, M * i, coeflen, msr.lenw);
            // 非第一个块：左移补偿
            if (i > 0)
                lmmp_fft_shl_coef_(&msr, pfca + i, i * n >> k);
        } else {
            lmmp_zero(pfca[i], nlen);
        }
    }
    lmmp_fft_(&msr, pfca, k, n >> (k - 1));

    if (nsqr) {
        pfcb += (nlen + 1) << k;
        for (mp_size_t i = 0; i < K; ++i) {
            mp_size_t coeflen;
            pfcb[i] = (mp_ptr)(pfcb + K) + i * nlen;
            if (nbrest > 0) {
                coeflen = M + (i == K - 1);
                coeflen = LMMP_MIN(nbrest, coeflen);
                nbrest -= coeflen;
                lmmp_fft_extract_coef_(pfcb[i], numb, M * i, coeflen, msr.lenw);
                if (i > 0)
                    lmmp_fft_shl_coef_(&msr, pfcb + i, i * n >> k);
            } else {
                lmmp_zero(pfcb[i], nlen);
            }
        }
        lmmp_fft_(&msr, pfcb, k, n >> (k - 1));
    }

    lmmp_mul_fermat_recurse_(&msr, pfca, pfcb, K);

    lmmp_ifft_(&msr, pfca, k, n >> (k - 1));

    lmmp_mul_fermat_recombine_(&msr, dst, pfca, K, k, n, M, rn);

    // 处理模 B^rn+1 的溢出
    if (dst[rn] && !lmmp_zero_q_(dst, rn)) {
        dst[rn] = 0;
        lmmp_dec(dst);
    }

    lmmp_fft_memstack_(&msr, 0);
}

void lmmp_mul_mersenne_(mp_ptr dst, mp_size_t rn, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb) {
    int nsqr = numa != numb || na != nb;  // 判断是否为平方运算
    mp_size_t N = rn * LIMB_BITS;         // 结果总比特数
    mp_size_t k = lmmp_fft_best_k_(rn);   // 最优FFT层数
    mp_size_t K = ((mp_size_t)1) << k;    // FFT块数（2^k）
    // 断言：N必须是K的整数倍
    lmmp_debug_assert(!(N & (K - 1)));
    mp_size_t M = N >> k;     // 每个块的比特数
    mp_size_t n = 2 * M + k;  // 扩展系数长度（梅森数比费马数少2）

    // 规整n：必须是LIMB_BITS和K/2的整数倍
    n = (n + LIMB_BITS - 1) & (-LIMB_BITS);
    n = (((n - 1) >> (k - 1)) + 1) << (k - 1);

    // 初始化内存栈
    fft_memstack msr;
    msr.maxdepth = -1;
    msr.tempdepth = -1;
    msr.lenw = n / LIMB_BITS;       // 系数长度（机器字）
    mp_size_t nlen = msr.lenw + 1;  // 系数总长度

    msr.temp_coef = (mp_ptr)lmmp_fft_memstack_(&msr, (((nlen + 1) << (k + nsqr)) + nlen) * LIMB_BYTES);

    mp_ptr *pfca = (mp_ptr*)(msr.temp_coef + nlen), *pfcb = pfca;
    mp_size_t narest = na * LIMB_BITS, nbrest = nb * LIMB_BITS;

    for (mp_size_t i = 0; i < K; ++i) {
        mp_size_t coeflen;
        pfca[i] = (mp_ptr)(pfca + K) + i * nlen;
        if (narest > 0) {
            coeflen = LMMP_MIN(narest, M);
            narest -= coeflen;
            lmmp_fft_extract_coef_(pfca[i], numa, M * i, coeflen, msr.lenw);
        } else {
            lmmp_zero(pfca[i], nlen);
        }
    }
    lmmp_fft_(&msr, pfca, k, n >> (k - 1));

    if (nsqr) {
        pfcb += (nlen + 1) << k;
        for (mp_size_t i = 0; i < K; ++i) {
            mp_size_t coeflen;
            pfcb[i] = (mp_ptr)(pfcb + K) + i * nlen;
            if (nbrest > 0) {
                coeflen = LMMP_MIN(nbrest, M);
                nbrest -= coeflen;
                lmmp_fft_extract_coef_(pfcb[i], numb, M * i, coeflen, msr.lenw);
            } else {
                lmmp_zero(pfcb[i], nlen);
            }
        }
        lmmp_fft_(&msr, pfcb, k, n >> (k - 1));
    }

    lmmp_mul_fermat_recurse_(&msr, pfca, pfcb, K);

    lmmp_ifft_(&msr, pfca, k, n >> (k - 1));

    mp_size_t rhead = 0, maxc = 0;
    for (mp_size_t i = 0; i < K; ++i) {
        lmmp_fft_shr_coef_(&msr, pfca + i, k);
        mp_ptr nums = pfca[i];

        if (nums[nlen - 1]) {
            lmmp_dec(nums);
            lmmp_debug_assert(nums[nlen - 1] == 1);
            nums[nlen - 1] = 0;
        }

        mp_size_t roffset = i * M;
        mp_size_t shl = roffset & (LIMB_BITS - 1);
        roffset /= LIMB_BITS;

        if (shl)
            lmmp_shl_(nums, nums, nlen, shl);

        if (i == 0) {
            lmmp_copy(dst, nums, nlen);
            rhead = nlen;
        } else if (roffset + nlen <= rn) {
            lmmp_add_(dst + roffset, nums, nlen, dst + roffset, rhead - roffset);
            rhead = roffset + nlen;
        } else {
            maxc += lmmp_add_(dst + roffset, nums, rn - roffset, dst + roffset, rhead - roffset);
            maxc += lmmp_add_(dst, dst, rn, nums + rn - roffset, nlen + roffset - rn);
            rhead = rn;
        }
    }

    if (!lmmp_add_1_(dst, dst, rn, 1 + maxc))
        lmmp_dec(dst);

    lmmp_fft_memstack_(&msr, 0);
}

typedef struct {
    fft_memstack msr_fermat;
    fft_memstack msr_mersenne;
    mp_ptr temp_coef_fermat;
    mp_ptr temp_coef_mersenne;
    int fermat_flag;    // 是否分配了费马内存
    int mersenne_flag;  // 是否分配了梅森内存
} fft_cache;

static inline void lmmp_mul_fft_cache_free_(fft_cache* GH) {
    if (GH->fermat_flag)
        lmmp_fft_memstack_(&GH->msr_fermat, 0);
    if (GH->mersenne_flag)
        lmmp_fft_memstack_(&GH->msr_mersenne, 0);
}

static void lmmp_mul_fermat_single_(
    mp_ptr     dst,
    mp_size_t   rn,
    mp_srcptr numa,
    mp_size_t   na,
    mp_srcptr numb,
    mp_size_t   nb,
    fft_cache*  GH
) {
    int nsqr = numa != numb || na != nb;  // 1为非平方，0为平方
    lmmp_assert(nsqr);
    mp_size_t N = rn * LIMB_BITS;        // 结果总比特数
    mp_size_t k = lmmp_fft_best_k_(rn);  // 最优FFT层数
    mp_size_t K = ((mp_size_t)1) << k;   // FFT块数（2^k）
    lmmp_debug_assert(!(N & (K - 1)));
    mp_size_t M = N >> k;         // 每个块的比特数
    mp_size_t n = 2 * M + k + 2;  // 扩展系数长度

    n = (n + LIMB_BITS - 1) & (-LIMB_BITS);
    n = (((n - 1) >> k) + 1) << k;

    fft_memstack* bmsr = NULL;
    fft_memstack amsr;
    amsr.maxdepth = -1;
    amsr.tempdepth = -1;
    amsr.lenw = n / LIMB_BITS;       // 系数长度（机器字）
    mp_size_t nlen = amsr.lenw + 1;  // 系数总长度
    mp_size_t a_size = (((nlen + 1) << (k)) + nlen) * LIMB_BYTES;
    mp_size_t b_size = (((nlen + 1) << (k)) + nlen) * LIMB_BYTES;
    amsr.temp_coef = (mp_ptr)lmmp_fft_memstack_(&amsr, a_size);

    mp_ptr* pfca = (mp_ptr*)(amsr.temp_coef + nlen);
    mp_ptr* pfcb = NULL;

    if (GH->fermat_flag) {
        bmsr = &GH->msr_fermat;
        bmsr->lenw = n / LIMB_BITS;
        pfcb = (mp_ptr*)(GH->temp_coef_fermat + nlen);
    } else {
        bmsr = &GH->msr_fermat;
        bmsr->maxdepth = -1;
        bmsr->tempdepth = -1;
        bmsr->lenw = n / LIMB_BITS;
        bmsr->temp_coef = (mp_ptr)lmmp_fft_memstack_(bmsr, b_size);
        GH->temp_coef_fermat = bmsr->temp_coef;
        pfcb = (mp_ptr*)(bmsr->temp_coef + nlen);
    }

    mp_size_t narest = na * LIMB_BITS, nbrest = nb * LIMB_BITS;
    for (mp_size_t i = 0; i < K; ++i) {
        mp_size_t coeflen;
        pfca[i] = (mp_ptr)(pfca + K) + i * nlen;
        if (narest > 0) {
            coeflen = M + (i == K - 1);
            coeflen = LMMP_MIN(narest, coeflen);
            narest -= coeflen;
            lmmp_fft_extract_coef_(pfca[i], numa, M * i, coeflen, amsr.lenw);
            if (i > 0)
                lmmp_fft_shl_coef_(&amsr, pfca + i, i * n >> k);
        } else {
            lmmp_zero(pfca[i], nlen);
        }
    }
    lmmp_fft_(&amsr, pfca, k, n >> (k - 1));

    if (!GH->fermat_flag) {
        GH->fermat_flag = 1;
        for (mp_size_t i = 0; i < K; ++i) {
            mp_size_t coeflen;
            pfcb[i] = (mp_ptr)(pfcb + K) + i * nlen;
            if (nbrest > 0) {
                coeflen = M + (i == K - 1);
                coeflen = LMMP_MIN(nbrest, coeflen);
                nbrest -= coeflen;
                lmmp_fft_extract_coef_(pfcb[i], numb, M * i, coeflen, bmsr->lenw);
                if (i > 0)
                    lmmp_fft_shl_coef_(bmsr, pfcb + i, i * n >> k);
            } else {
                lmmp_zero(pfcb[i], nlen);
            }
        }
        lmmp_fft_(bmsr, pfcb, k, n >> (k - 1));
    }

    lmmp_mul_fermat_recurse_(&amsr, pfca, pfcb, K);

    lmmp_ifft_(&amsr, pfca, k, n >> (k - 1));

    lmmp_mul_fermat_recombine_(&amsr, dst, pfca, K, k, n, M, rn);

    if (dst[rn] && !lmmp_zero_q_(dst, rn)) {
        dst[rn] = 0;
        lmmp_dec(dst);
    }

    lmmp_fft_memstack_(&amsr, 0);
}

static void lmmp_mul_mersenne_single_(
    mp_ptr     dst,
    mp_size_t   rn,
    mp_srcptr numa,
    mp_size_t   na,
    mp_srcptr numb,
    mp_size_t   nb,
    fft_cache*  GH
) {
    int nsqr = numa != numb || na != nb;  // 1为非平方，0为平方
    lmmp_assert(nsqr);
    mp_size_t N = rn * LIMB_BITS;        // 结果总比特数
    mp_size_t k = lmmp_fft_best_k_(rn);  // 最优FFT层数
    mp_size_t K = ((mp_size_t)1) << k;   // FFT块数（2^k）
    // 断言：N必须是K的整数倍
    lmmp_debug_assert(!(N & (K - 1)));
    mp_size_t M = N >> k;     // 每个块的比特数
    mp_size_t n = 2 * M + k;  // 扩展系数长度（梅森数比费马数少2）

    // 规整n：必须是LIMB_BITS和K/2的整数倍
    n = (n + LIMB_BITS - 1) & (-LIMB_BITS);
    n = (((n - 1) >> (k - 1)) + 1) << (k - 1);

    // 初始化内存栈
    fft_memstack* bmsr = NULL;
    fft_memstack amsr;
    amsr.maxdepth = -1;
    amsr.tempdepth = -1;
    amsr.lenw = n / LIMB_BITS;       // 系数长度（机器字）
    mp_size_t nlen = amsr.lenw + 1;  // 系数总长度
    mp_size_t a_size = (((nlen + 1) << (k)) + nlen) * LIMB_BYTES;
    mp_size_t b_size = (((nlen + 1) << (k)) + nlen) * LIMB_BYTES;
    amsr.temp_coef = (mp_ptr)lmmp_fft_memstack_(&amsr, a_size);

    mp_ptr* pfca = (mp_ptr*)(amsr.temp_coef + nlen);
    mp_ptr* pfcb = NULL;

    if (GH->mersenne_flag) {
        bmsr = &GH->msr_mersenne;
        bmsr->lenw = n / LIMB_BITS;
        pfcb = (mp_ptr*)(GH->temp_coef_mersenne + nlen);
    } else {
        bmsr = &GH->msr_mersenne;
        bmsr->maxdepth = -1;
        bmsr->tempdepth = -1;
        bmsr->lenw = n / LIMB_BITS;
        bmsr->temp_coef = (mp_ptr)lmmp_fft_memstack_(bmsr, b_size);
        GH->temp_coef_mersenne = bmsr->temp_coef;
        pfcb = (mp_ptr*)(bmsr->temp_coef + nlen);
    }

    mp_size_t narest = na * LIMB_BITS, nbrest = nb * LIMB_BITS;

    for (mp_size_t i = 0; i < K; ++i) {
        mp_size_t coeflen;
        pfca[i] = (mp_ptr)(pfca + K) + i * nlen;
        if (narest > 0) {
            coeflen = LMMP_MIN(narest, M);
            narest -= coeflen;
            lmmp_fft_extract_coef_(pfca[i], numa, M * i, coeflen, amsr.lenw);
        } else {
            lmmp_zero(pfca[i], nlen);
        }
    }
    lmmp_fft_(&amsr, pfca, k, n >> (k - 1));

    if (!GH->mersenne_flag) {
        GH->mersenne_flag = 1;
        for (mp_size_t i = 0; i < K; ++i) {
            mp_size_t coeflen;
            pfcb[i] = (mp_ptr)(pfcb + K) + i * nlen;
            if (nbrest > 0) {
                coeflen = LMMP_MIN(nbrest, M);
                nbrest -= coeflen;
                lmmp_fft_extract_coef_(pfcb[i], numb, M * i, coeflen, bmsr->lenw);
            } else {
                lmmp_zero(pfcb[i], nlen);
            }
        }
        lmmp_fft_(bmsr, pfcb, k, n >> (k - 1));
    }

    lmmp_mul_fermat_recurse_(&amsr, pfca, pfcb, K);

    lmmp_ifft_(&amsr, pfca, k, n >> (k - 1));

    mp_size_t rhead = 0, maxc = 0;
    for (mp_size_t i = 0; i < K; ++i) {
        lmmp_fft_shr_coef_(&amsr, pfca + i, k);
        mp_ptr nums = pfca[i];

        if (nums[nlen - 1]) {
            lmmp_dec(nums);
            lmmp_debug_assert(nums[nlen - 1] == 1);
            nums[nlen - 1] = 0;
        }

        mp_size_t roffset = i * M;
        mp_size_t shl = roffset & (LIMB_BITS - 1);
        roffset /= LIMB_BITS;

        if (shl)
            lmmp_shl_(nums, nums, nlen, shl);

        if (i == 0) {
            lmmp_copy(dst, nums, nlen);
            rhead = nlen;
        } else if (roffset + nlen <= rn) {
            lmmp_add_(dst + roffset, nums, nlen, dst + roffset, rhead - roffset);
            rhead = roffset + nlen;
        } else {
            maxc += lmmp_add_(dst + roffset, nums, rn - roffset, dst + roffset, rhead - roffset);
            maxc += lmmp_add_(dst, dst, rn, nums + rn - roffset, nlen + roffset - rn);
            rhead = rn;
        }
    }

    if (!lmmp_add_1_(dst, dst, rn, 1 + maxc))
        lmmp_dec(dst);

    lmmp_fft_memstack_(&amsr, 0);
}

void lmmp_mul_fft_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb) {
    lmmp_param_assert(na > 0 && nb > 0);
    lmmp_param_assert(na >= nb);
    mp_size_t hn = lmmp_fft_next_size_((na + nb + 1) >> 1);
    lmmp_assert(na + nb > hn);
    mp_ptr tp = ALLOC_TYPE(hn + 1, mp_limb_t);

    mp_srcptr amodm = numa;
    mp_size_t nam = na;
    if (na > hn) {
        /*
          Z = B^hb - 1
          amodm = a mod Z
         */
        if (lmmp_add_(dst, numa, hn, numa + hn, na - hn))
            lmmp_inc(dst);
        amodm = dst;
        nam = hn;
    }
    lmmp_mul_mersenne_(dst, hn, amodm, nam, numb, nb);

    mp_srcptr amodp = numa;
    mp_size_t nap = na;
    if (na > hn) {
        /*
          Z = B^hp - 1
          amodp = a mod Z
         */
        tp[hn] = 0;
        if (lmmp_sub_(tp, numa, hn, numa + hn, na - hn))
            lmmp_inc(tp);
        amodp = tp;
        nap = hn + 1;
    }
    lmmp_mul_fermat_(tp, hn, amodp, nap, numb, nb);

    mp_limb_t cy = lmmp_shr1add_nc_(dst, dst, tp, hn, tp[hn]);
    cy <<= LIMB_BITS - 1;
    dst[hn - 1] += cy;
    if (dst[hn - 1] < cy)
        lmmp_inc(dst);

    if (na + nb == 2 * hn) {
        cy = tp[hn] + lmmp_sub_n_(dst + hn, dst, tp, hn);
        // cy==1 means [tp,hn+1]!=0, then [dst,hn]!=0
        // cy==2 is impossible since [tp,hn+1] is normalized.
        // so the following dec won't overflow.
        lmmp_dec_1(dst, cy);
    } else {
        cy = lmmp_sub_n_(dst + hn, dst, tp, na + nb - hn);
        cy = tp[hn] + lmmp_sub_nc_(tp + na + nb - hn, dst + na + nb - hn, tp + na + nb - hn, 2 * hn - (na + nb), cy);
        cy = lmmp_sub_1_(dst, dst, na + nb, cy);
    }
    lmmp_free(tp);
}

static void lmmp_mul_fft_cache_(
    mp_ptr     dst,
    mp_size_t   hn,
    mp_srcptr numa,
    mp_size_t   na,
    mp_srcptr numb,
    mp_size_t   nb,
    fft_cache*  GH
) {
    lmmp_param_assert(na > 0 && nb > 0);
    lmmp_param_assert(na >= nb);
    lmmp_assert(na + nb > hn);
    mp_ptr tp = ALLOC_TYPE(hn + 1, mp_limb_t);

    mp_srcptr amodm = numa;
    mp_size_t nam = na;
    if (na > hn) {
        /*
          Z = B^hb - 1
          amodm = a mod Z
         */
        if (lmmp_add_(dst, numa, hn, numa + hn, na - hn))
            lmmp_inc(dst);
        amodm = dst;
        nam = hn;
    }
    lmmp_mul_mersenne_single_(dst, hn, amodm, nam, numb, nb, GH);

    mp_srcptr amodp = numa;
    mp_size_t nap = na;
    if (na > hn) {
        /*
          Z = B^hp - 1
          amodp = a mod Z
         */
        tp[hn] = 0;
        if (lmmp_sub_(tp, numa, hn, numa + hn, na - hn))
            lmmp_inc(tp);
        amodp = tp;
        nap = hn + 1;
    }
    lmmp_mul_fermat_single_(tp, hn, amodp, nap, numb, nb, GH);

    mp_limb_t cy = lmmp_shr1add_nc_(dst, dst, tp, hn, tp[hn]);
    cy <<= LIMB_BITS - 1;
    dst[hn - 1] += cy;
    if (dst[hn - 1] < cy)
        lmmp_inc(dst);

    if (na + nb == 2 * hn) {
        cy = tp[hn] + lmmp_sub_n_(dst + hn, dst, tp, hn);
        // cy==1 means [tp,hn+1]!=0, then [dst,hn]!=0
        // cy==2 is impossible since [tp,hn+1] is normalized.
        // so the following dec won't overflow.
        lmmp_dec_1(dst, cy);
    } else {
        cy = lmmp_sub_n_(dst + hn, dst, tp, na + nb - hn);
        cy = tp[hn] + lmmp_sub_nc_(tp + na + nb - hn, dst + na + nb - hn, tp + na + nb - hn, 2 * hn - (na + nb), cy);
        cy = lmmp_sub_1_(dst, dst, na + nb, cy);
    }
    lmmp_free(tp);
}

void lmmp_mul_fft_unbalance_(
    mp_ptr    restrict  dst,
    mp_srcptr restrict numa,
    mp_size_t            na,
    mp_srcptr restrict numb,
    mp_size_t            nb
) {
    lmmp_param_assert(na >= 3 * nb);
    mp_ptr restrict ws = ALLOC_TYPE(nb, mp_limb_t);
    mp_size_t sna = 3 * nb;
    mp_size_t hn = lmmp_fft_next_size_((sna + nb + 1) >> 1);
    sna = (hn << 1) - 1 - nb;
    fft_cache GH = {.mersenne_flag = 0, .fermat_flag = 0};
    lmmp_mul_fft_cache_(dst, hn, numa, sna, numb, nb, &GH);
    dst += sna;
    numa += sna;
    na -= sna;
    lmmp_copy(ws, dst, nb);
    while (na >= sna) {
        lmmp_mul_fft_cache_(dst, hn, numa, sna, numb, nb, &GH);
        if (lmmp_add_n_(dst, dst, ws, nb))
            lmmp_inc(dst + nb);
        dst += sna;
        numa += sna;
        na -= sna;
        lmmp_copy(ws, dst, nb);
    }
    lmmp_mul_fft_cache_free_(&GH);
    // remaining na < sna
    if (na >= nb)
        lmmp_mul_(dst, numa, na, numb, nb);
    else if (na > 0)
        lmmp_mul_(dst, numb, nb, numa, na);
    else  // na == 0
        lmmp_zero(dst, nb);
    if (lmmp_add_n_(dst, dst, ws, nb))
        lmmp_inc(dst + nb);
    lmmp_free(ws);
}
