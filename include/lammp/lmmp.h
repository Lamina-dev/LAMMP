#ifndef LAMMP_LMMP_H
#define LAMMP_LMMP_H

/*************************************
        符号说明:

        B           基数，固定为 2^64 

        [p,n,b]     表示指针p指向的、以b为基数的n位数
                    p[i-1] 代表其第i位最低有效位 (0<i<=n)
                    如果省略b，则默认基数为B

        sep         指针指向的内存区域不能重叠

        eqsep       重叠或分离，指针可以指向同一内存区域（原地操作）或不同区域

        [x|y]       x或y，用于表示参数或返回值的取值范围
**************************************/

#include <stddef.h> 
#include <stdint.h>

struct ilmp_compile_time_error_ {
    char error_sizeof_pointer_should_be_8[sizeof(void*) == 8 ? 1 : -1];
};

typedef uint8_t mp_byte_t;           // 字节类型 (8位无符号整数)
typedef uint64_t mp_limb_t;          // 基本运算单元(limb)类型 (64位无符号整数)
typedef uint64_t mp_size_t;          // 表示limb数量的无符号整数类型
typedef int64_t mp_slimb_t;          // 有符号limb类型 (64位有符号整数)
typedef int64_t mp_ssize_t;          // 表示limb数量的有符号整数类型
typedef mp_limb_t* mp_ptr;           // 指向limb类型的指针
typedef const mp_limb_t* mp_srcptr;  // 指向const limb类型的指针（源操作数指针）

#ifdef __cplusplus
extern "C" {
#endif

// ===================== lmmp 内存管理安全函数 =====================
/**
 * 内存分配函数
 * @param size 要分配的内存字节数
 * @return 成功返回指向分配内存的指针，失败返回NULL
 * @note 是标准malloc的安全封装版本
 */
void* lmmp_alloc(size_t);

/**
 * 内存重分配函数
 * @param ptr 已分配的内存指针
 * @param size 新的内存大小（字节）
 * @return 成功返回指向新内存区域的指针，失败返回NULL
 * @note 是标准realloc的安全封装版本
 */
void* lmmp_realloc(void*, size_t);

/**
 * 内存释放函数
 * @param ptr 要释放的内存指针
 * @note 是标准free的安全封装版本，确保空指针释放安全
 */
void lmmp_free(void*);

// ===================== lmmp_ 底层不安全运算函数 =====================
/**
 * 带进位的n位加法
 * 功能: [dst,n] = [numa,n] + [numb,n] + c
 * 要求: c只能是0或1, n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n 操作数的位数（limb数量）
 * @param c 初始进位值 (0或1)
 * @return 运算后的最终进位值 (0或1)
 */
mp_limb_t lmmp_add_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * 无进位的n位加法
 * 功能: [dst,n] = [numa,n] + [numb,n]
 * 要求: n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n 操作数的位数（limb数量）
 * @return 运算后的最终进位值 (0或1)
 */
mp_limb_t lmmp_add_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 带借位的n位减法
 * 功能: [dst,n] = [numa,n] - [numb,n] - c
 * 要求: c只能是0或1, n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n 操作数的位数（limb数量）
 * @param c 初始借位值 (0或1)
 * @return 运算后的最终借位值 (0或1)
 */
mp_limb_t lmmp_sub_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * 无借位的n位减法
 * 功能: [dst,n] = [numa,n] - [numb,n]
 * 要求: n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n 操作数的位数（limb数量）
 * @return 运算后的最终借位值 (0或1)
 */
mp_limb_t lmmp_sub_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 同时执行n位加法和减法
 * 功能: ([dsta,n],[dstb,n]) = ([numa,n]+[numb,n],[numa,n]-[numb,n])
 * 要求: n>0, dsta与numa/numb内存可重叠, dstb与numa/numb内存可重叠
 * @param dsta 加法结果输出指针
 * @param dstb 减法结果输出指针
 * @param numa 第一个操作数指针（被加数/被减数）
 * @param numb 第二个操作数指针（加数/减数）
 * @param n 操作数的位数（limb数量）
 * @return 组合返回值 cb = 2*c + b (c为加法进位, b为减法借位)
 *         返回值范围: 0(无进位无借位),1(无进位有借位),2(有进位无借位),3(有进位有借位)
 */
mp_limb_t lmmp_add_n_sub_n_(mp_ptr dsta, mp_ptr dstb, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 加法后右移1位
 * 功能: [dst,n] = ([numa,n] + [numb,n]) >> 1
 * 要求: n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n 操作数的位数（limb数量）
 * @return 右移操作产生的进位值 (0或1)
 */
mp_limb_t lmmp_shr1add_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 带进位加法后右移1位
 * 功能: [dst,n] = ([numa,n] + [numb,n] + c) >> 1
 * 要求: n>0, c只能是0或1, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 第一个加数指针
 * @param numb 第二个加数指针
 * @param n 操作数的位数（limb数量）
 * @param c 初始进位值 (0或1)
 * @return 右移操作产生的进位值 (0或1)
 */
mp_limb_t lmmp_shr1add_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * 减法后右移1位
 * 功能: [dst,n] = ([numa,n] - [numb,n]) >> 1
 * 要求: n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n 操作数的位数（limb数量）
 * @return 右移操作产生的进位值 (0或1)
 */
mp_limb_t lmmp_shr1sub_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 带借位减法后右移1位
 * 功能: [dst,n] = ([numa,n] - [numb,n] - c) >> 1
 * 要求: n>0, c只能是0或1, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针
 * @param n 操作数的位数（limb数量）
 * @param c 初始借位值 (0或1)
 * @return 右移操作产生的进位值 (0或1)
 */
mp_limb_t lmmp_shr1sub_nc_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t c);

/**
 * 大数右移操作
 * 功能: [dst,na] = [numa,na] >> shr (整体右移shr位)
 *       dst的高shr位填充0
 * 要求: na>0, 0<=shr<64, dst与numa的内存区域可以重叠(原地操作)
 *       支持dst < numa的内存布局（原地长右移）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 * @param shr 右移的位数 (0~63)
 * @return 移出的高shr位值（返回值的高shr位为numa的低shr位，其余位为0）
 */
mp_limb_t lmmp_shr_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shr);

/**
 * 带进位的大数右移操作
 * 功能: [dst,na] = [numa,na] >> shr (整体右移shr位)
 *       dst的高shr位填充c的高shr位
 * 要求: na>0, 0<=shr<64, dst与numa的内存区域可以重叠(原地操作)
 *       c的低(64-shr)位必须为0
 *       支持dst < numa的内存布局（原地长右移）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 * @param shr 右移的位数 (0~63)
 * @param c 进位值（其低(64-shr)位必须为0）
 * @return 移出的高shr位值（返回值的高shr位为numa的低shr位，其余位为0）
 */
mp_limb_t lmmp_shr_c_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shr, mp_limb_t c);

/**
 * 大数左移操作
 * 功能: [dst,na] = [numa,na] << shl (整体左移shl位)
 *       dst的低shl位填充0
 * 要求: na>0, 0<=shl<64, dst与numa的内存区域可以重叠(原地操作)
 *       支持dst > numa的内存布局（原地长左移）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 * @param shl 左移的位数 (0~63)
 * @return 移出的低shl位值（返回值的低shl位为numa的高shl位，其余位为0）
 */
mp_limb_t lmmp_shl_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shl);

/**
 * 带进位的大数左移操作
 * 功能: [dst,na] = [numa,na] << shl (整体左移shl位)
 *       dst的低shl位填充c的低shl位
 * 要求: na>0, 0<=shl<64, dst与numa的内存区域可以重叠(原地操作)
 *       c的高(64-shl)位必须为0
 *       支持dst > numa的内存布局（原地长左移）
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 * @param shl 左移的位数 (0~63)
 * @param c 进位值（其高(64-shl)位必须为0）
 * @return 移出的低shl位值（返回值的低shl位为numa的高shl位，其余位为0）
 */
mp_limb_t lmmp_shl_c_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shl, mp_limb_t c);

/**
 * 大数按位取反操作
 * 功能: [dst,na] = ~[numa,na] (对每个limb执行按位非操作)
 * 要求: na>0, dst与numa的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 */
void lmmp_not_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * 左移后按位取反操作
 * 功能: [dst,na] = ~([numa,na] << shl) (先左移再按位取反)
 *       dst的低shl位填充1
 * 要求: na>0, 0<=shl<64, dst与numa的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 * @param shl 左移的位数 (0~63)
 * @return 移出的低shl位值（返回值的低shl位为numa的高shl位，其余位为0）
 */
mp_limb_t lmmp_shlnot_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t shl);

/**
 * 加法结合左移1位操作
 * 功能: [dst,n] = [numa,n] + ([numb,n] << 1)
 * 要求: n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被加数指针
 * @param numb 加数指针（先左移1位）
 * @param n 操作数的位数（limb数量）
 * @return 运算后的进位值 (0,1或2)
 */
mp_limb_t lmmp_addshl1_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 减法结合左移1位操作
 * 功能: [dst,n] = [numa,n] - ([numb,n] << 1)
 * 要求: n>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param numb 减数指针（先左移1位）
 * @param n 操作数的位数（limb数量）
 * @return 运算后的借位值 (0,1或2)
 */
mp_limb_t lmmp_subshl1_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 大数乘以单limb并累加操作
 * 功能: [numa,n] += [numb,n] * B (B为内部基数2^64)
 *       等价于将numb左移一个limb位置后加到numa上
 * 要求: n>0, numa与numb的内存区域不能重叠
 * @param numa 被加数指针（结果也存储在此）
 * @param numb 乘数指针
 * @param n 操作数的位数（limb数量）
 * @param B 内部基数（固定为2^64）
 * @return 运算后的进位limb值
 */
mp_limb_t lmmp_addmul_1_(mp_ptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t B);

/**
 * 大数乘以单limb并累减操作
 * 功能: [numa,n] -= [numb,n] * B (B为内部基数2^64)
 *       等价于将numb左移一个limb位置后从numa中减去
 * 要求: n>0, numa与numb的内存区域不能重叠
 * @param numa 被减数指针（结果也存储在此）
 * @param numb 乘数指针
 * @param n 操作数的位数（limb数量）
 * @param B 内部基数（固定为2^64）
 * @return 运算后的借位limb值
 */
mp_limb_t lmmp_submul_1_(mp_ptr numa, mp_srcptr numb, mp_size_t n, mp_limb_t B);

/**
 * 大数平方操作
 * 功能: [dst,2*na] = [numa,na]^2 (计算numa的平方)
 * 要求: na>0, dst与numa的内存区域必须分离（不能重叠）
 * @param dst 平方结果输出指针（需要2*na个limb空间）
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 */
void lmmp_sqr_(mp_ptr dst, mp_srcptr numa, mp_size_t na);

/**
 * 大数乘以单limb操作
 * 功能: [dst,na] = [numa,na] * x (x为单个limb值)
 * 要求: na>0, dst与numa的内存区域可以重叠(原地操作)
 *       支持dst <= numa+1的内存布局
 * @param dst 结果输出指针
 * @param numa 被乘数指针
 * @param na 操作数的位数（limb数量）
 * @param x 单个limb乘数
 * @return 运算后的进位limb值
 */
mp_limb_t lmmp_mul_1_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_limb_t x);

/**
 * 等长大数乘法操作
 * 功能: [dst,2*n] = [numa,n] * [numb,n] (两个n位大数相乘)
 * 要求: n>0, dst与numa/numb的内存区域必须分离（不能重叠）
 *       特殊情况: n==1时dst<=numa+1是允许的
 *                 n==2时dst<=numa是允许的
 * @param dst 乘积结果输出指针（需要2*n个limb空间）
 * @param numa 第一个乘数指针
 * @param numb 第二个乘数指针
 * @param n 操作数的位数（limb数量）
 */
void lmmp_mul_n_(mp_ptr dst, mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 不等长大数乘法操作
 * 功能: [dst,na+nb] = [numa,na] * [numb,nb] (两个不同长度大数相乘)
 * 要求: 0<nb<=na, dst与numa/numb的内存区域必须分离（不能重叠）
 *      特殊情况:  nb==1时dst<=numa+1是允许的
 *                nb==2时dst<=numa是允许的
 * @param dst 乘积结果输出指针（需要na+nb个limb空间）
 * @param numa 第一个乘数指针（较长的操作数）
 * @param na 第一个操作数的位数（limb数量）
 * @param numb 第二个乘数指针（较短的操作数）
 * @param nb 第二个操作数的位数（limb数量）
 */
void lmmp_mul_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * 大数除法和取模操作
 * @note 如果dstq不为NULL: [dstq,na-nb+1] = [numa,na] / [numb,nb] (商)
 *       如果dstr不为NULL: [dstr,nb] = [numa,na] mod [numb,nb] (余数)
 * @warning 0<nb<=na, numb的最高位不能为0,
 *          dstq与numa/numb的内存区域必须分离,
 *          dstr与numa/numb的内存区域可以重叠(原地操作)
 *          特殊情况: nb==1时dstq>=numa-1是允许的
 *                   nb==2时dstq>=numa是允许的
 * @param dstq 商结果输出指针（NULL表示不计算商）
 * @param dstr 余数结果输出指针（NULL表示不计算余数）
 * @param numa 被除数指针
 * @param na 被除数的位数（limb数量）
 * @param numb 除数指针
 * @param nb 除数的位数（limb数量）
 */
void lmmp_div_(mp_ptr dstq, mp_ptr dstr, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * 大数平方根和取余操作
 * 功能: 如果dstr不为NULL: [dsts,nf+na/2+1],[dstr,nf+na/2+1] = sqrtrem([numa,na]*B^(2*nf))
 *       如果dstr为NULL: [dsts,nf+na/2+1] = 向下取整/四舍五入(sqrt([numa,na]*B^(2*nf)))
 * 要求: na>0, numa的最高位不能为0,
 *       dsts与numa的内存区域可以重叠(原地操作),
 *       dstr与numa的内存区域可以重叠(原地操作)
 * @param dsts 平方根结果输出指针
 * @param dstr 余数结果输出指针（NULL表示不计算余数）
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 * @param nf 精度因子（用于扩展计算精度）
 */
void lmmp_sqrt_(mp_ptr dsts, mp_ptr dstr, mp_srcptr numa, mp_size_t na, mp_size_t nf);

/**
 * 大数求逆操作
 * 功能: [dst,na+nf+1] = (B^(2*(na+nf)) - 1) / ([numa,na]*B^nf) + [0|-1]
 *       计算大数的近似逆元
 * 要求: na>0, numa的最高位不能为0,
 *       dst与numa的内存区域可以重叠(原地操作)
 * @param dst 逆元结果输出指针
 * @param numa 源操作数指针
 * @param na 操作数的位数（limb数量）
 * @param nf 精度因子（用于扩展计算精度）
 */
void lmmp_inv_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_size_t nf);

/**
 * 字符串转大数操作
 * 功能: 将指定基数的字符串转换为lmmp内部大数格式
 * 要求: len>=0, 2<=base<=256 (支持2到256进制)
 * @param dst 大数结果输出指针
 * @param src 字符串源指针
 * @param len 字符串长度
 * @param base 字符串的进制基数
 * @return 转换后的大数limb数量
 */
mp_size_t lmmp_from_str_(mp_ptr dst, const mp_byte_t* src, mp_size_t len, int base);

/**
 * 大数转字符串操作
 * 功能: 将lmmp内部大数格式转换为指定基数的字符串
 * 要求: na>=0, 2<=base<=256 (支持2到256进制)
 * @param dst 字符串结果输出指针
 * @param numa 大数源指针
 * @param na 大数的limb数量
 * @param base 目标字符串的进制基数
 * @return 转换后的字符串长度
 */
mp_size_t lmmp_to_str_(mp_byte_t* dst, mp_srcptr numa, mp_size_t na, int base);

/**
 * 大数比较操作
 * 功能: 比较两个n位大数的大小
 * 要求: n>0
 * @param numa 第一个比较数指针
 * @param numb 第二个比较数指针
 * @param n 操作数的位数（limb数量）
 * @return 比较结果: 1(numa>numb), 0(numa==numb), -1(numa<numb)
 */
//int lmmp_cmp_(mp_srcptr numa, mp_srcptr numb, mp_size_t n);

/**
 * 大数判零操作
 * 功能: 判断指定大数是否为零
 * 要求: n>0
 * @param p 大数指针
 * @param n 大数的limb数量
 * @return 判零结果: 1(全零), 0(非零)
 */
//int lmmp_zero_q_(mp_srcptr p, mp_size_t n);

/**
 * 不等长大数加法
 * 功能: [dst,na] = [numa,na] + [numb,nb]
 * 要求: na>=nb>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 第一个加数指针（较长的操作数）
 * @param na 第一个操作数的位数（limb数量）
 * @param numb 第二个加数指针（较短的操作数）
 * @param nb 第二个操作数的位数（limb数量）
 * @return 运算后的最终进位值 (0或1)
 */
//mp_limb_t lmmp_add_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * 不等长大数减法
 * 功能: [dst,na] = [numa,na] - [numb,nb]
 * 要求: na>=nb>0, dst与numa/numb的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被减数指针（较长的操作数）
 * @param na 被减数的位数（limb数量）
 * @param numb 减数指针（较短的操作数）
 * @param nb 减数的位数（limb数量）
 * @return 运算后的最终借位值 (0或1)
 */
//mp_limb_t lmmp_sub_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_srcptr numb, mp_size_t nb);

/**
 * 大数加单limb操作
 * 功能: [dst,na] = [numa,na] + x (x为单个limb值)
 * 要求: na>0, dst与numa的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被加数指针
 * @param na 操作数的位数（limb数量）
 * @param x 单个limb加数
 * @return 运算后的最终进位值 (0或1)
 */
//mp_limb_t lmmp_add_1_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_limb_t x);

/**
 * 大数减单limb操作
 * 功能: [dst,na] = [numa,na] - x (x为单个limb值)
 * 要求: na>0, dst与numa的内存区域可以重叠(原地操作)
 * @param dst 结果输出指针
 * @param numa 被减数指针
 * @param na 操作数的位数（limb数量）
 * @param x 单个limb减数
 * @return 运算后的最终借位值 (0或1)
 */
//mp_limb_t lmmp_sub_1_(mp_ptr dst, mp_srcptr numa, mp_size_t na, mp_limb_t x);

/**
 * 计算字符串转大数所需的limb数量
 * 功能: 如果src不为NULL: 返回[src,len,base]转换为大数所需的limb数量（可能比精确值大1）
 *       如果src为NULL: 返回len位base进制数的最大可能limb数量
 * 要求: len>=0, 2<=base<=256
 * @param src 字符串源指针（NULL表示计算最大值）
 * @param len 字符串长度
 * @param base 字符串的进制基数
 * @return 所需的limb数量
 */
//mp_size_t lmmp_limbs_(const mp_byte_t* src, mp_size_t len, int base);

/**
 * 计算大数转字符串所需的字符数量
 * 功能: 如果numa不为NULL: 返回[numa,na]转换为字符串所需的字符数量（可能比精确值大1）
 *       如果numa为NULL: 返回na个limb的大数的最大可能字符数量
 * 要求: na>=0, 2<=base<=256
 * @param numa 大数源指针（NULL表示计算最大值）
 * @param na 大数的limb数量
 * @param base 目标字符串的进制基数
 * @return 所需的字符数量
 */
//mp_size_t lmmp_digits_(mp_srcptr numa, mp_size_t na, int base);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // LAMMP_LMMP_H