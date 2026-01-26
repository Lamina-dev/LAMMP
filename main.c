#include "include/lammp/version.h"
#include <stdio.h>

int main() {
    printf("Hello, LAMMP! \n");
    printf("LAMMP version: %s\n", LAMMP_VERSION);
    printf("LAMMP alpha year: %s\n", LAMMP_ALPHA_YEAR);
    printf("LAMMP date: %s\n", LAMMP_DATE);
    printf("LAMMP compiler: %s\n", LAMMP_COMPILER);
    printf("======================================================\n");
    printf("LAMMP CopyRight: %s\n", LAMMP_COPYRIGHT);
    printf("======================================================\n");
    return 0;
}
