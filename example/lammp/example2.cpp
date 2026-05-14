/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for full license details.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../include/lammp/numth.h"

void print_help(void) {
    printf("=============================================\n");
    printf("           LAMMP Factorial Calculator\n");
    printf("=============================================\n");
    printf("Usage:\n");
    printf("  Direct mode:  ./fact 10000\n");
    printf("  Manual mode:  ./fact (then input number)\n");
    printf("\nPlease input a non-negative integer.\n");
}

int is_number(const char* s) {
    if (!s || *s == '\0')
        return 0;
    while (*s) {
        if (!isdigit((unsigned char)*s))
            return 0;
        s++;
    }
    return 1;
}

void calculate_factorial(uint n) {
    mp_bitcnt_t bits;
    clock_t start = clock();

    printf("\nCalculating %d! ...\n", n);

    mp_size_t len = lmmp_factorial_size_(n, &bits);
    mp_ptr dst = (mp_ptr)lmmp_alloc(len * sizeof(mp_limb_t));
    len = lmmp_factorial_(dst, bits, len, n);

    clock_t end = clock();
    double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
    double decimal_digits = LIMB_BITS * len * 0.30102999566;  // log10(2)

    printf("Completed!\n");
    printf("Bits length   : %llu bits\n", (unsigned long long)LIMB_BITS * len);
    printf("Decimal digits: %.0f digits\n", decimal_digits);
    printf("Time used     : %.2f ms\n", ms);
    printf("Result range  : %llx ... %llx\n", (unsigned long long)dst[len - 1], (unsigned long long)dst[0]);

    lmmp_free(dst);
}

// 交互式输入模式处理函数
void interactive_mode(void) {
    print_help();
    printf("\nPlease enter number: ");

    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin)) {
        printf("Input error.\n");
        return;
    }

    // 去除换行符
    buf[strcspn(buf, "\n\r")] = 0;

    if (!is_number(buf)) {
        printf("Error: Only non-negative integers are allowed!\n");
        return;
    }

    uint n = (uint)atol(buf);
    calculate_factorial(n);
}

// 命令行参数模式处理函数
void cli_mode(const char* arg) {
    if (!is_number(arg)) {
        printf("Error: Invalid input!\n");
        print_help();
        return;
    }

    uint n = (uint)atol(arg);
    calculate_factorial(n);
}

int main(int argc, char* argv[]) {
    // 线程或进程资源初始化
    lmmp_global_init();

    // 分支判断：命令行参数模式 / 交互式模式
    if (argc >= 2) {
        cli_mode(argv[1]);
    } else {
        interactive_mode();
    }

    // 线程或进程资源释放
    lmmp_global_deinit();
    return 0;
}