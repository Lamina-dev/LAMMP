#include "../../include/lammp/lmmpn.h"

// ((mp_size_t)3 << (2 * (n) - 5)) + 1 是预计算的阈值，n是对应的k值
#define _FFT_TABLE_ENTRY(n) {((mp_size_t)3 << (2 * (n) - 5)) + 1, (n)}
#define _FFT_TABLE_ENTRY4(n) \
    _FFT_TABLE_ENTRY(n), _FFT_TABLE_ENTRY((n) + 1), _FFT_TABLE_ENTRY((n) + 2), _FFT_TABLE_ENTRY((n) + 3)

// best_k_(next_size_(n)) = best_k_(n)
// table[i+1][0]-1 必须是 2^(table[i][1]-LOG2_LIMB_BITS) 的整数倍
// LOG2_LIMB_BITS：每个 limb 的比特数的2对数
static const mp_size_t lmmp_fft_table_[][2] = {
    {0, 6},                 // 阈值0，对应k=6
    {1597, 7},              // 阈值1597，对应k=7
    {1655, 6},              // 阈值1655，对应k=6
    {1917, 7},              // 阈值1917，对应k=7
    {3447, 8},              // 阈值3447，对应k=8
    {3565, 7},              // 阈值3565，对应k=7
    {3831, 8},              // 阈值3831，对应k=8
    {7661, 9},              // 阈值7661，对应k=9
    {8145, 8},              // 阈值8145，对应k=8
    {8685, 9},              // 阈值8685，对应k=9
    {14289, 10},            // 阈值14289，对应k=10
    {16289, 9},             // 阈值16289，对应k=9
    {20433, 10},            // 阈值20433，对应k=10
    {24481, 9},             // 阈值24481，对应k=9
    {26577, 10},            // 阈值26577，对应k=10
    {28593, 11},            // 阈值28593，对应k=11
    {32545, 10},            // 阈值32545，对应k=10
    {57249, 11},            // 阈值57249，对应k=11
    {65313, 10},            // 阈值65313，对应k=10
    {73633, 11},            // 阈值73633，对应k=11
    {98081, 12},            // 阈值98081，对应k=12
    {130625, 11},           // 阈值130625，对应k=11
    {196385, 12},           // 阈值196385，对应k=12
    {261697, 11},           // 阈值261697，对应k=11
    {294689, 12},           // 阈值294689，对应k=12
    {392769, 13},           // 阈值392769，对应k=13
    {523265, 12},           // 阈值523265，对应k=12
    {654913, 11},           // 阈值654913，对应k=11
    {917281, 13},           // 阈值917281，对应k=13
    {1047553, 11},          // 阈值1047553，对应k=11
    {1600001, 12},          // 阈值1600001，对应k=12
    {1834561, 14},          // 阈值1834561，对应k=14
    {2095105, 12},          // 阈值2095105，对应k=12
    _FFT_TABLE_ENTRY4(13),  
    _FFT_TABLE_ENTRY4(17), 
    _FFT_TABLE_ENTRY4(21),  
    _FFT_TABLE_ENTRY4(25),  
    {(mp_size_t)-1, 127}    
};

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
mp_size_t lmmp_fft_best_k_(mp_size_t n) {
    mp_size_t k = 0;
    while (n >= lmmp_fft_table_[k + 1][0]) ++k;
    return lmmp_fft_table_[k][1];
}

/**
 * @brief 计算FFT运算所需的最小规整化长度（向上取整到2^k的倍数）
 * @param n - 原始长度
 * @return 规整后的长度（满足2^k的倍数，保证FFT并行性）
 */
mp_size_t lmmp_fft_next_size_(mp_size_t n) {
    mp_size_t k = lmmp_fft_best_k_(n);
    // 保证每个块至少包含1个 limb
    lmmp_debug_assert(k >= LOG2_LIMB_BITS);
    k -= LOG2_LIMB_BITS;
    n = (((n - 1) >> k) + 1) << k;
    return n;
}

/**
 * @brief FFT内存栈的分配/释放接口
 * @param ms - 内存栈结构体栈帧
 * @param size - 分配大小（字节），size=0表示释放当前层内存
 * @return 分配成功：返回内存地址；释放：返回0
 */
void* lmmp_fft_memstack_(fft_memstack* ms, mp_size_t size) {
    if (size) {  // size>0：分配内存
        // 临时深度+1，若超过最大深度则分配新内存块
        if (++ms->tempdepth > ms->maxdepth) {
            ms->mem[++ms->maxdepth] = lmmp_alloc(size); 
            ms->memsize[ms->maxdepth] = size;           
        }
        lmmp_debug_assert(ms->memsize[ms->tempdepth] == size);
        return ms->mem[ms->tempdepth];  
    } else {    // size=0：释放内存
        if (--ms->tempdepth < 0) {
            for (mp_size_t i = 0; i <= (mp_size_t)(ms->maxdepth); ++i) lmmp_free(ms->mem[i]);
            ms->maxdepth = -1;  
        }
        return 0; 
    }
}

/**
 * @brief dst,lenw+1] = [(bit*)numa+bitoffset, bits]
 * @param dst - 输出系数数组（长度lenw+1）
 * @param numa - 输入大数指针
 * @param bitoffset - 起始比特偏移量（>=0）
 * @param bits - 提取的比特数（0 < bits <= LIMB_BITS*lenw）
 * @param lenw - 输出系数的机器字长度
 * @warning dst与numa的内存地址必须分离（sep(dst,numa)）
 */
void lmmp_fft_extract_coef_(mp_ptr dst, mp_srcptr numa, mp_size_t bitoffset, mp_size_t bits, mp_size_t lenw) {
    // shr = 机器字内的比特偏移（0~LIMB_BITS-1）
    // offset = 起始机器字的索引
    mp_size_t shr = bitoffset & (LIMB_BITS - 1), offset = bitoffset / LIMB_BITS;

    // lena = 从offset开始需要读取的机器字数
    // endp = 最后一个有效机器字的索引（bits-1)/LIMB_BITS
    mp_size_t lena = (bitoffset + bits - 1) / LIMB_BITS - offset + 1, endp = (bits - 1) / LIMB_BITS;

    if (shr)
        lmmp_shr_(dst, numa + offset, lena, shr);  
    else
        lmmp_copy(dst, numa + offset, lena);  

    // 清除最后一个机器字中超出bits范围的高位
    // -bits & (LIMB_BITS-1) = LIMB_BITS - (bits % LIMB_BITS)（若bits%LIMB_BITS≠0）
    dst[endp] &= LIMB_MAX >> (-bits & (LIMB_BITS - 1));

    // 将剩余未使用的机器字置零
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
void lmmp_fft_shl_coef_(fft_memstack* ms, mp_ptr* coef, mp_size_t shl) {
    mp_size_t l = ms->lenw;         // 系数的机器字长度
    mp_size_t w = shl / LIMB_BITS;  // 左移的机器字数量
    shl &= LIMB_BITS - 1;           // 剩余的比特偏移（0~LIMB_BITS-1）
    mp_ptr src = *coef;             // 源系数数组
    mp_ptr dst = ms->temp_coef;     // 目标临时数组
    mp_limb_t cc, rd;               // 进位变量（cc=carry, rd=read）

    // 左移的机器字数 >= 系数长度（w >= l）
    if (w >= l) {
        w -= l;   
        if (shl) {  // 有比特偏移
            // 1. 处理超出部分：src[l-w ... l] 左移shl位到dst[0 ... w]
            lmmp_shl_(dst, src + l - w, w + 1, shl);
            rd = dst[w];  // 保存dst[w]的值
            // 2. 处理循环部分：src[0 ... l-w-1] 左移shl位并取反（mod 2^n+1特性）
            cc = lmmp_shlnot_(dst + w, src, l - w, shl);
        } else {    // 无比特偏移
            if (w) 
                lmmp_copy(dst, src + l - w, w);
            rd = src[l]; 
            // 取反循环部分（mod 2^n+1）
            lmmp_not_(dst + w, src, l - w);
            cc = 0;  
        }
        dst[l] = 0;           // 清空最高位
        ++cc;                 // 进位+1（mod 2^n+1补偿）
        lmmp_inc_1(dst, cc);  // 对dst加cc（处理进位）

        // 处理rd的进位
        if (++rd == 0)
            lmmp_inc(dst + w + 1);  // rd溢出，dst[w+1 ...] +1
        else
            lmmp_inc_1(dst + w, rd);  // 非溢出，dst[w] + rd
    }
    // 左移的机器字数 < 系数长度（w < l）
    else {
        if (shl) {  // 有比特偏移
            // 1. 处理循环部分：src[l-w ... l] 左移shl位并取反
            lmmp_shlnot_(dst, src + l - w, w + 1, shl);
            rd = ~dst[w];  // 取反dst[w]
            // 2. 处理非循环部分：src[0 ... l-w-1] 左移shl位
            cc = lmmp_shl_(dst + w, src, l - w, shl);
        } else {    // 无比特偏移
            if (w)  // 取反循环部分
                lmmp_not_(dst, src + l - w, w);
            rd = src[l];  // 保存src[l]
            // 拷贝非循环部分
            lmmp_copy(dst + w, src, l - w);
            cc = 0;  // 无进位
        }
        dst[l] = 2;           // 初始化最高位
        lmmp_inc_1(dst, 3);   // dst +3（mod 2^n+1补偿）
        lmmp_dec_1(dst, cc);  // 减去进位cc

        // 处理rd的进位
        if (++rd == 0)
            lmmp_dec(dst + w + 1);  // 溢出
        else
            lmmp_dec_1(dst + w, rd);

        // 处理最高位进位
        cc = dst[l];
        dst[l] = dst[0] < cc;          // 若dst[0] < cc则为1，否则为0
        lmmp_dec_1(dst, cc - dst[l]);  // 调整dst
    }

    // 交换临时数组和源数组（节省内存分配）
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
void lmmp_fft_shr_coef_(fft_memstack* ms, mp_ptr* coef, mp_size_t shr) {
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
void lmmp_fft_bfy_(fft_memstack* ms, mp_ptr* coef, mp_size_t wing, mp_size_t w) {
    mp_ptr numa = coef[0];                // 系数a
    mp_ptr numb = coef[wing];             // 系数b
    mp_ptr numc = ms->temp_coef;          // 临时数组（存储a-b<<w）
    mp_size_t shl = w & (LIMB_BITS - 1);  // 比特级左移量
    w /= LIMB_BITS;                       // 机器字级左移量
    mp_size_t l = ms->lenw;               // 系数长度（机器字）

    // 进位/借位变量：
    // acyo = a+b的进位, scyo = a-b的借位
    // ch = 总进位, chp=正进位, chn=负进位
    mp_slimb_t acyo = 0, scyo = 0, ch;
    mp_limb_t shlcyo = 0, chp = 0, chn = 0;

    // 处理 0 <= off < l - w 的区域
    for (mp_size_t off = 0; off < l - w; off += PART_SIZE) {
        // 每次处理PART_SIZE个机器字
        mp_size_t cursize = LMMP_MIN(l - w - off, PART_SIZE);
        // numc[w+off ...] = numa[off ...] - numb[off ...]（带借位）
        scyo = lmmp_sub_nc_(numc + w + off, numa + off, numb + off, cursize, scyo);
        // numa[off ...] = numa[off ...] + numb[off ...]（带进位）
        acyo = lmmp_add_nc_(numa + off, numa + off, numb + off, cursize, acyo);
        // 若有比特左移，对numc执行左移（带进位）
        if (shl)
            shlcyo = lmmp_shl_c_(numc + w + off, numc + w + off, cursize, shl, shlcyo);
    }

    // 计算总进位：shlcyo（左移进位） + (-scyo << shl)（借位补偿）
    // 范围：-2^63 < ch < 2^63-1（mp_slimb_t为有符号64位）
    ch = shlcyo + (-scyo << shl);
    // 分离正负进位
    if (ch > 0)
        chp = ch;
    else
        chn = -ch;

    scyo = 0;
    shlcyo = 0;

    // 处理 l - w <= off < l 的区域（循环部分）
    for (mp_size_t off = l - w; off < l; off += PART_SIZE) {
        mp_size_t cursize = LMMP_MIN(l - off, PART_SIZE);
        // numc[off-(l-w) ...] = numb[off ...] - numa[off ...]（带借位）
        scyo = lmmp_sub_nc_(numc + off - (l - w), numb + off, numa + off, cursize, scyo);
        // numa[off ...] = numa[off ...] + numb[off ...]（带进位）
        acyo = lmmp_add_nc_(numa + off, numa + off, numb + off, cursize, acyo);
        // 若有比特左移，对numc执行左移（带进位）
        if (shl)
            shlcyo = lmmp_shl_c_(numc + off - (l - w), numc + off - (l - w), cursize, shl, shlcyo);
    }

    // 处理最高位的进位/借位
    numc[w] += shlcyo;                 // 左移进位加到numc[w]
    scyo = -scyo + numb[l] - numa[l];  // 调整借位（包含最高位）
    acyo += numa[l] + numb[l];         // 调整进位（包含最高位）

    // 归一化a（numa）：
    numa[l] = numa[0] < (mp_limb_t)(acyo);  // 若numa[0] < acyo则为1
    lmmp_dec_1(numa, acyo - numa[l]);       // 调整numa

    // 归一化b（numc）：
    numc[l] = 1;  // 初始化最高位
    ++chn;        // 负进位+1
    // 处理scyo的补偿
    if (scyo > 0)
        lmmp_inc_1(numc + w, scyo << shl);  // 正scyo：numc[w] + scyo<<shl
    else if (scyo < 0) {
        // 特殊情况：scyo=-2且shl=LIMB_BITS-1（溢出处理）
        if (scyo == -2 && shl == LIMB_BITS - 1)
            lmmp_dec(numc + w + 1);
        else
            lmmp_dec_1(numc + w, -scyo << shl);  // 负scyo：numc[w] - |scyo|<<shl
    }
    chp += numc[l];  // 正进位 += numc最高位

    // 最终归一化numc：
    if (chn >= chp) {
        numc[l] = 0;
        lmmp_inc_1(numc, chn - chp); 
    } else {
        chp -= chn;
        numc[l] = numc[0] < chp;          // 若numc[0] < chp则为1
        lmmp_dec_1(numc, chp - numc[l]);  // 调整numc
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
void lmmp_ifft_bfy_(fft_memstack* ms, mp_ptr* coef, mp_size_t wing, mp_size_t w) {
    mp_ptr numa = coef[0];                // 系数a
    mp_ptr numb = coef[wing];             // 系数b
    mp_ptr numc = ms->temp_coef;          // 临时数组（存储a-(b>>w)）
    mp_size_t shr = w & (LIMB_BITS - 1);  // 比特级右移量
    w /= LIMB_BITS;                       // 机器字级右移量
    mp_size_t l = ms->lenw;               // 系数长度

    // 进位/借位变量：
    // bcyo = numc的进位, acyo = numa的进位
    // shrcyo = 右移进位（最高位）
    mp_slimb_t bcyo = 0, acyo = 0, ah;
    mp_limb_t shrcyo = shr ? numb[0] << (LIMB_BITS - shr) : 0;

    // 处理 l - w <= off < l 的区域（循环部分）
    for (mp_size_t off = l - w; off < l; off += PART_SIZE) {
        mp_size_t cursize = LMMP_MIN(l - off, PART_SIZE);
        // 对numb执行右移（带进位）
        if (shr)
            lmmp_shr_c_(numb + off - (l - w), numb + off - (l - w), cursize, shr,
                        numb[off - (l - w) + cursize] << (LIMB_BITS - shr));
        // 计算 numc[off ...] = numa[off ...] + (b>>w)（带进位）
        bcyo = lmmp_add_nc_(numc + off, numa + off, numb + off - (l - w), cursize, bcyo);
        // 计算 numa[off ...] = numa[off ...] - (b>>w)（带借位）
        acyo = lmmp_sub_nc_(numa + off, numa + off, numb + off - (l - w), cursize, acyo);
    }

    // 处理 0 <= off < l - w 的区域（非循环部分）
    for (mp_size_t off = 0; off < l - w; off += PART_SIZE) {
        mp_size_t cursize = LMMP_MIN(l - w - off, PART_SIZE);
        // 对numb执行右移（带进位）
        if (shr)
            lmmp_shr_c_(numb + w + off, numb + w + off, cursize, shr, numb[off + w + cursize] << (LIMB_BITS - shr));
        // 计算 numc[off ...] = numa[off ...] - (b>>w)（带借位）
        bcyo = lmmp_sub_nc_(numc + off, numa + off, numb + w + off, cursize, bcyo);
        // 计算 numa[off ...] = numa[off ...] + (b>>w)（带进位）
        acyo = lmmp_add_nc_(numa + off, numa + off, numb + w + off, cursize, acyo);
    }

    // 处理最高位的进位/借位
    acyo += numb[l] >> shr;           // 右移后的最高位进位
    bcyo = -bcyo - (numb[l] >> shr);  // 调整bcyo

    // 处理右移的边界进位（shrcyo）
    acyo -= numa[l - w - 1] < shrcyo;  // 若numa[l-w-1] < shrcyo则acyo-1
    numa[l - w - 1] -= shrcyo;         // 减去shrcyo
    numc[l - w - 1] += shrcyo;         // 加上shrcyo
    bcyo += numc[l - w - 1] < shrcyo;  // 若numc溢出则bcyo+1

    ah = numa[l];  // 保存numa的最高位

    // 归一化a（numa）：
    numa[l] += 1;  // 初始化+1
    if (w == 0)
        numa[l] += acyo;  // w=0直接加进位
    else {
        if (acyo < 0)
            lmmp_dec(numa + l - w);  // 负进位：numa[l-w ...] -1
        else
            lmmp_inc_1(numa + l - w, acyo);  // 正进位：numa[l-w] + acyo
    }
    acyo = numa[l] - 1;  // 还原进位
    if (acyo < 0) {
        numa[l] = 0;
        lmmp_inc(numa);  // 溢出：整体+1
    } else {
        numa[l] = numa[0] < (mp_limb_t)acyo;  // 归一化标志
        lmmp_dec_1(numa, acyo - numa[l]);     
    }

    // 归一化b（numc）：
    numc[l] = ah + 2;  
    if (w == 0)
        numc[l] += bcyo;  // w=0直接加进位
    else {
        if (bcyo > 0)
            lmmp_inc(numc + l - w);  // 正进位：numc[l-w ...] +1
        else
            lmmp_dec_1(numc + l - w, -bcyo);  // 负进位：numc[l-w] - |bcyo|
    }
    bcyo = numc[l] - 2;  // 还原进位
    if (bcyo <= 0) {
        numc[l] = 0;
        lmmp_inc_1(numc, -bcyo);  // 负进位：整体 + |bcyo|
    } else {
        numc[l] = numc[0] < (mp_limb_t)bcyo;  // 归一化标志
        lmmp_dec_1(numc, bcyo - numc[l]);     
    }

    // 交换内存：将numc设为新的b，原b设为临时数组
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
void lmmp_fft_b1_(fft_memstack* ms, mp_ptr* coef, mp_size_t dis, mp_size_t k, mp_size_t w, mp_size_t w0) {
    if (k == 1)
        lmmp_fft_bfy_(ms, coef, dis, w0);
    else {
        k -= 2;                   // 递归深度减2（4路FFT）
        mp_size_t Kq = dis << k;  
        for (mp_size_t i = 0; i < Kq; i += dis) {
            lmmp_fft_bfy_(ms, coef + i, 2 * Kq, i * w + w0);             
            lmmp_fft_bfy_(ms, coef + i + Kq, 2 * Kq, (i + Kq) * w + w0);  
            lmmp_fft_bfy_(ms, coef + i, Kq, 2 * (i * w + w0));           
            lmmp_fft_bfy_(ms, coef + i + Kq * 2, Kq, 2 * (i * w + w0));  
        }
        // 递归处理4个子块
        if (k > 0) {
            lmmp_fft_b1_(ms, coef, dis, k, 4 * w, 4 * w0);          
            lmmp_fft_b1_(ms, coef + Kq, dis, k, 4 * w, 4 * w0);    
            lmmp_fft_b1_(ms, coef + Kq * 2, dis, k, 4 * w, 4 * w0);  
            lmmp_fft_b1_(ms, coef + Kq * 3, dis, k, 4 * w, 4 * w0);  
        }
    }
}

void lmmp_fft_4_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
    if (k == 1)
        // 单次蝶形运算
        lmmp_fft_bfy_(ms, coef, 1, 0);
    else {
        k -= 2;                              // 递归深度减2
        mp_size_t Kq = ((mp_size_t)1) << k;  // 子块大小（2^k）
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

void lmmp_fft_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
    mp_size_t k1 = k >> 1;                // k1 = k/2（右移1位等价于除以2）
    k -= k1;                              // k = k - k1（剩余层数）
    mp_size_t Kp = ((mp_size_t)1) << k;   // Kp = 2^k
    mp_size_t Kq = ((mp_size_t)1) << k1;  // Kq = 2^k1

    for (mp_size_t i = 0; i < Kp; ++i) lmmp_fft_b1_(ms, coef + i, Kp, k1, w, i * w);

    for (mp_size_t i = 0; i < Kq; ++i) lmmp_fft_4_(ms, coef + Kp * i, k, w * Kq);
}

void lmmp_ifft_b1_(fft_memstack* ms, mp_ptr* coef, mp_size_t dis, mp_size_t k, mp_size_t w, mp_size_t w0) {
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

void lmmp_ifft_4_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
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

void lmmp_ifft_(fft_memstack* ms, mp_ptr* coef, mp_size_t k, mp_size_t w) {
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
void lmmp_mul_fermat_recombine_(fft_memstack* ms,
                                mp_ptr dst,
                                mp_ptr* pfca,
                                mp_size_t K,
                                mp_size_t k,
                                mp_size_t n,
                                mp_size_t M,
                                mp_size_t rn) {
    mp_size_t rhead = 0, nlen = ms->lenw + 1;  // rhead=结果当前长度，nlen=系数长度
    mp_slimb_t borrow = 0, maxc = 0;           // borrow=借位，maxc=总进位

    // 遍历所有FFT块，合并结果
    for (mp_size_t i = 0; i < K; ++i) {
        // 1. 对第i块执行右移：(i*n >>k) +k 比特（补偿FFT移位）
        lmmp_fft_shr_coef_(ms, pfca + i, (i * n >> k) + k);
        mp_ptr nums = pfca[i];  // 当前块的系数

        // 2. 归一化处理：去除伪归一化标志
        if (nums[nlen - 1]) {
            lmmp_dec(nums);    // 整体减1
            --nums[nlen - 1];  // 清除最高位标志
        }
        // 3. 二次归一化：处理边界情况
        if (nums[nlen - 1] == 0 && nums[nlen - 2] >> (LIMB_BITS - 1)) {
            lmmp_dec(nums);    // 整体减1
            --nums[nlen - 1];  // 清除最高位标志
        }

        // 4. 处理借位补偿
        if (borrow) {
            // 计算借位的移位量
            mp_size_t brshift = borrow - 1 + n - M;
            mp_size_t bshl = brshift & (LIMB_BITS - 1);
            brshift /= LIMB_BITS;
            // 调整系数：nums[brshift] -= 1<<bshl
            --nums[nlen - 1];
            lmmp_dec_1(nums + brshift, (mp_limb_t)1 << bshl);
            ++nums[nlen - 1];
        }
        borrow = -nums[nlen - 1];
        nums[nlen - 1] = 0;       

        // 5. 计算结果偏移：i*M比特（块偏移）
        mp_size_t roffset = i * M;
        mp_size_t shl = roffset & (LIMB_BITS - 1); 
        roffset /= LIMB_BITS;                      

        // 6. 左移对齐（将当前块移到正确位置）
        if (shl)
            lmmp_shl_(nums, nums, nlen, shl);

        // 7. 合并到结果数组
        if (i == 0) {
            // 第一个块：直接拷贝
            lmmp_copy(dst, nums, nlen);
            rhead = nlen;
        } else if (roffset + nlen <= rn) {
            // 无溢出：直接相加
            lmmp_add_(dst + roffset, nums, nlen, dst + roffset, rhead - roffset);
            rhead = roffset + nlen;
        } else {
            // 有溢出：分两部分处理（模 B^rn 特性）
            // 第一部分：不溢出区域相加
            maxc += lmmp_add_(dst + roffset, nums, rn - roffset, dst + roffset, rhead - roffset);
            // 第二部分：溢出区域相减（模 B^rn+1）
            maxc -= lmmp_sub_(dst, dst, rn, nums + rn - roffset, nlen + roffset - rn);
            rhead = rn;
        }
    }

    // 8. 处理剩余借位
    if (borrow) {
        mp_size_t cyshift = borrow - 1 + n - M;
        mp_size_t cshl = cyshift & (LIMB_BITS - 1);
        cyshift /= LIMB_BITS;
        // 借位补偿：dst[cyshift ...] + 1<<cshl
        maxc += lmmp_add_1_(dst + cyshift, dst + cyshift, rn - cyshift, (mp_limb_t)1 << cshl);
    }

    // 9. 最终归一化结果
    if (maxc > 0) {
        dst[rn] = dst[0] < (mp_limb_t)maxc;  // 归一化标志
        lmmp_dec_1(dst, maxc - dst[rn]);     // 调整结果
    } else {
        dst[rn] = 0;
        lmmp_inc_1(dst, -maxc);  // 负进位补偿
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
void lmmp_mul_fermat_recurse_(fft_memstack* ms, mp_ptr* pc1, mp_ptr* pc2, mp_size_t K0) {
    int nsqr = pc1 != pc2;  // 判断是否为平方运算
    mp_ptr push_temp_coef = ms->temp_coef;
    mp_size_t rn = ms->lenw;  // 当前系数长度

    // 小于阈值则不使用FFT
    if (rn < MUL_FFT_MODF_THRESHOLD) {
        // 分配临时乘法缓冲区（2*(rn+1)个机器字）
        mp_ptr temp_mul = (mp_ptr)lmmp_fft_memstack_(ms, (rn + 1) * 2 * LIMB_BYTES);
        // 遍历所有块执行乘法/平方
        for (mp_size_t i = 0; i < K0; ++i) {
            if (nsqr)
                lmmp_mul_n_(temp_mul, pc1[i], pc2[i], rn + 1);  // 乘法：temp_mul = pc1[i] * pc2[i]
            else
                lmmp_sqr_(temp_mul, pc1[i], rn + 1);  // 平方：temp_mul = pc1[i]^2

            // 模 B^rn+1 归一化：temp_mul - temp_mul[rn ...]
            mp_limb_t maxc = lmmp_sub_n_(pc1[i], temp_mul, temp_mul + rn, rn) + temp_mul[rn * 2];
            pc1[i][rn] = 0;            // 清空最高位
            lmmp_inc_1(pc1[i], maxc);  // 调整结果
        }
        lmmp_fft_memstack_(ms, 0);  // 释放临时缓冲区
    }
    else {
        mp_size_t N = rn * LIMB_BITS;        // 总比特数
        mp_size_t k = lmmp_fft_best_k_(rn);  // 最优FFT层数
        mp_size_t K = ((mp_size_t)1) << k;   // FFT块数（2^k）
        lmmp_debug_assert(!(N & (K - 1)));
        mp_size_t M = N >> k;         // 每个块的比特数（N/K）
        mp_size_t n = 2 * M + k + 2;  // 扩展系数长度（保证归一化）

        // 规整n：必须是LIMB_BITS和K的整数倍
        n = (n + LIMB_BITS - 1) & (-LIMB_BITS);  // 向上取整到LIMB_BITS的倍数
        n = (((n - 1) >> k) + 1) << k;           // 向上取整到K的倍数

        ms->lenw = n / LIMB_BITS;       // 更新系数长度（机器字）
        mp_size_t nlen = ms->lenw + 1;  // 系数总长度（+1用于进位）

        // 计算需要分配的内存大小：
        // - (nlen << (k + nsqr))：系数数组（2^(k+nsqr) * nlen）
        // - (1 << (k + nsqr))：指针数组（2^(k+nsqr) 个指针）
        // - nlen：临时系数
        ms->temp_coef = (mp_ptr)lmmp_fft_memstack_(ms, (((nlen + 1) << (k + nsqr)) + nlen) * LIMB_BYTES);
        mp_ptr *pfca = (mp_ptr*)(ms->temp_coef + nlen), *pfcb = pfca;
        // 为每个FFT块分配系数数组
        for (mp_size_t i = 0; i < K; ++i) pfca[i] = (mp_ptr)(pfca + K) + i * nlen;
        if (nsqr) {
            pfcb += (nlen + 1) << k;  
            for (mp_size_t i = 0; i < K; ++i) pfcb[i] = (mp_ptr)(pfcb + K) + i * nlen;
        }

        for (mp_size_t j = 0; j < K0; ++j) {
            mp_ptr numa = pc1[j]; 
            mp_ptr numb = pc2[j];  

            // 提取第一个数的FFT系数
            for (mp_size_t i = 0; i < K; ++i) {
                // 提取第i块的系数：从M*i比特开始，长度M+(i==K-1)
                lmmp_fft_extract_coef_(pfca[i], numa, M * i, M + (i == K - 1), ms->lenw);
                // 非第一个块：左移补偿
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

    // 规整n：必须是LIMB_BITS和K的整数倍
    n = (n + LIMB_BITS - 1) & (-LIMB_BITS);
    n = (((n - 1) >> k) + 1) << k;

    // 初始化内存栈
    fft_memstack msr;
    msr.maxdepth = -1;
    msr.tempdepth = -1;
    msr.lenw = n / LIMB_BITS;       // 系数长度（机器字）
    mp_size_t nlen = msr.lenw + 1;  // 系数总长度

    // 分配内存：系数数组 + 指针数组 + 临时系数
    msr.temp_coef = (mp_ptr)lmmp_fft_memstack_(&msr, (((nlen + 1) << (k + nsqr)) + nlen) * LIMB_BYTES);

    // 初始化指针数组
    mp_ptr *pfca = (mp_ptr*)(msr.temp_coef + nlen), *pfcb = pfca;
    mp_size_t narest = na * LIMB_BITS, nbrest = nb * LIMB_BITS;  // 剩余未处理的比特数

    for (mp_size_t i = 0; i < K; ++i) {
        mp_size_t coeflen;
        pfca[i] = (mp_ptr)(pfca + K) + i * nlen;  // 分配第i块的系数数组
        if (narest > 0) {
            // 计算当前块的系数长度
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
    // 执行FFT
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

    // 分配内存：系数数组 + 指针数组 + 临时系数
    msr.temp_coef = (mp_ptr)lmmp_fft_memstack_(&msr, (((nlen + 1) << (k + nsqr)) + nlen) * LIMB_BYTES);

    // 初始化指针数组
    mp_ptr *pfca = (mp_ptr*)(msr.temp_coef + nlen), *pfcb = pfca;
    mp_size_t narest = na * LIMB_BITS, nbrest = nb * LIMB_BITS;  // 剩余未处理的比特数

    for (mp_size_t i = 0; i < K; ++i) {
        mp_size_t coeflen;
        pfca[i] = (mp_ptr)(pfca + K) + i * nlen;
        if (narest > 0) {
            coeflen = LMMP_MIN(narest, M);  // 梅森数固定M长度
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

    // 梅森数结果重组
    mp_size_t rhead = 0, maxc = 0;
    for (mp_size_t i = 0; i < K; ++i) {
        // 右移k比特补偿
        lmmp_fft_shr_coef_(&msr, pfca + i, k);
        mp_ptr nums = pfca[i];

        // 归一化处理
        if (nums[nlen - 1]) {
            lmmp_dec(nums);
            lmmp_debug_assert(nums[nlen - 1] == 1);  // 梅森数归一化标志为1
            nums[nlen - 1] = 0;
        }

        // 计算结果偏移
        mp_size_t roffset = i * M;
        mp_size_t shl = roffset & (LIMB_BITS - 1);
        roffset /= LIMB_BITS;

        if (shl)
            lmmp_shl_(nums, nums, nlen, shl);

        // 梅森数只需要加法即可
        if (i == 0) {
            lmmp_copy(dst, nums, nlen);
            rhead = nlen;
        } else if (roffset + nlen <= rn) {
            lmmp_add_(dst + roffset, nums, nlen, dst + roffset, rhead - roffset);
            rhead = roffset + nlen;
        } else {
            // 梅森数溢出区域也是加法（模 B^rn-1）
            maxc += lmmp_add_(dst + roffset, nums, rn - roffset, dst + roffset, rhead - roffset);
            maxc += lmmp_add_(dst, dst, rn, nums + rn - roffset, nlen + roffset - rn);
            rhead = rn;
        }
    }

    // 最终归一化（梅森数加1补偿）
    if (!lmmp_add_1_(dst, dst, rn, 1 + maxc))
        lmmp_dec(dst);

    lmmp_fft_memstack_(&msr, 0);
}

void lmmp_mul_fft_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb) {
    lmmp_debug_assert(na > 0 && nb > 0);
    lmmp_debug_assert(na >= nb);
    mp_size_t hn = lmmp_fft_next_size_((na + nb + 1) >> 1);
    lmmp_debug_assert(na + nb > hn);
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
