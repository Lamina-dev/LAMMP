#include "include/lammp/ccstdlib.h"
#include "include/lammp/version.h"

int main() {
    if (LAMMP_COMPILER_VERSION == 0) {
        printf("Unknown compiler\n");
    } else {
        printf("Compiler: %s\n", LAMMP_COMPILER);
        printf("Compiler version: %d\n", LAMMP_COMPILER_VERSION);
    }
    printf("LAMMP version: %s\n", LAMMP_VERSION);
    printf("LAMMP alpha year: %s\n", LAMMP_ALPHA_YEAR);
    printf("LAMMP date: %s\n", LAMMP_DATE);
    printf("LAMMP copyright: %s\n", LAMMP_COPYRIGHT);
    printf("===================================================\n");
    printf("Hello, lammp!\n");
    return 0;
}
