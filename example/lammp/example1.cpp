#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <string>

#include "../../include/lammp/lampz.h"


int main() {
    lampz_t z;
    __lampz_init(z); /* 初始化为 NAN 也即空 */
    // 请注意，初始化为空参与计算，将导致计算结果也为空，这是明确行为，而不是未定义行为。
    // 因此，请务必在使用其计算前注意是否为空。
    // 例如，使用 lampz_add_x(z, x); 若 x 为空，则结果也为空，z 的内部内存将会被安全释放，即 z 变为 NAN 空。
    // lampz_new(z); /* 申请 0 比特的内存，z 的数值将会设置为零，将会至少分配一个字长的空间 */
    // lampz_new(z, 1024); /* 申请 1024 比特的内存，z 的数值将会设置为零，无论空间有多大 */

    /* 小端序的字符串，0 为最低有效位 1 为第二位，以此类推 */
    /* 输入的字符将不会做任何检查，非法输入将导致未定义行为 */
    /* 支持2-36进制，但不支持负数，即输入和输出的字符串均为绝对值，此为明确行为，而不是未定义行为 */
    /* 注意：字符串长度不一定等于比特长度，请注意区分 */
    /* 大小写均可，但输出的字符串将会全部使用小写字母 */
    /* 对于十进制以上进制，将使用 a - z 作为字符，依次表示 10 - 35 */
    std::string str = "01237817927876726574923467362786482368238090123768293486732874";
    std::string str_copy(str.size(), '\0');
    std::copy(str.begin(), str.end(), str_copy.begin());
    std::reverse(str_copy.begin(), str_copy.end());
    std::cout << "computer " << str_copy << " + " << str_copy << " = " << std::endl;

    /* 字符串转化为 lampz 类型，z 可以为空，自动分配内存 */
    lampz_set_str(z, str.data(), str.size(), 10);

    /* 计算 z *= z */
    lampz_mul_x(z, z);
    // lampz_mul_xy(z, z, z); // 等价

    /* 输出 z 的字符串表示 */
    /* 注意：字符串缓冲区长度由函数lampz_to_str_len()返回 */
    /* 注意：此值请与你实际输出的进制一致  ↓  不一致可能导致缓冲区不足，而溢出 */
    std::string out(lampz_to_str_len(z, 10), '\0');
    lamp_sz out_len = lampz_to_str(out.data(), out.capacity(), z, 10);
    /* 十进制输出---------------------------------------------------^ */

    /* 输出字符串依然为小端序，但输出的字符为小写字母 */
    /* 输出结果（字符串低位即数字低位），所以反向输出字符串 */
    for (size_t i = out_len; i-- != 0;) {
        std::cout << out[i];
    }
    std::cout << std::endl;

    /* 释放内存，z 本身为栈空间内存，无需释放，内部存在数组指针，调用此函数释放 */
    /* 调用此函数后，z 变为 NAN 空，效果等价于 __lampz_init(z); 此后也可以继续使用 */
    /* 当然如果当 z 离开作用域时，请务必调用此函数释放内存 */
    lampz_free(z);
    return 0;
}
