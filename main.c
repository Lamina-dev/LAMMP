#include "include/lammp/version.h"
#include <stdio.h>

int main(void) {
    printf("Hello, LAMMP! \n");
    printf("LAMMP version: %s\n", LAMMP_VERSION);
    printf("LAMMP alpha year: %s\n", LAMMP_ALPHA_YEAR);
    printf("LAMMP compiler: %s\n", LAMMP_COMPILER);
    return 0;
}
