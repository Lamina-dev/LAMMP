#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <string>

#include "../../include/lammp/lampz.h"

int main() {
    lampz_t z;
    __lampz_init(z);
    
    // set a value
    lampz_set_ui(z, 1234567890);
    printf("z = %lld\n", lampz_to_si(z)); // output: z = 1234567890

    lampz_set_sign(z, -1);
    printf("z = %lld\n", lampz_to_si(z)); // output: z = -1234567890


    /*  
        请注意，如果 z 的实际值超出了lamp_si或者lamp_ui的范围 
        将输出对应方向的最大值或最小值，而不是报错。    
    */
    lampz_set_ui(z, LAMP_UI_MAX);
    // 此时z的值为 0xffffffffffffffff，超出lamp_si范围，输出lamp_si最大值
    printf("z = %lld\n", lampz_to_si(z)); // output: z = 9223372036854775807

    lampz_set_sign(z, -1);
    // 此时z的值为 （负）0xffffffffffffffff，超出lamp_si范围，输出lamp_si最小值
    printf("z = %lld\n", lampz_to_si(z)); // output: z = -9223372036854775808

    std::string str = "123456789012321323126879617438";
    std::reverse(str.begin(), str.end()); // 字符串反转，以满足小端序
    lampz_set_str(z, str.c_str(), str.size(), 10);
    
    // 此时z的值为 123456789012321323126879617438 大于 lamp_ui 范围，输出 lamp_ui 最大值
    printf("z = %llu\n", lampz_to_ui(z)); // output: z = 18446744073709551615
    
    // 此时z的值为 -123456789012321323126879617438，超出lamp_ui范围，输出lamp_ui最小值
    lampz_set_sign(z, -1);
    printf("z = %llu\n", lampz_to_ui(z)); // output: z = 0


    // 请务必记得释放内存
    lampz_free(z);
    return 0;
}
