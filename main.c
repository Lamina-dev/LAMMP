#include "include/lammp/version.h"
#include <stdio.h>

int main(void) {
    printf("Hello, LAMMP! \n");
    printf("LAMMP version: %s\n", lmmp_get_version());
    printf("LAMMP build type: %s\n", lmmp_get_build_type());
    return 0;
}
