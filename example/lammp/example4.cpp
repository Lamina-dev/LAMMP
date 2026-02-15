#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "../../include/lammp/lmmpn.h"
#include "../../include/lammp/mprand.h"
int main() {
    TEMP_DECL;
    mp_size_t nb = 10680;
    mp_size_t na = 1324320;
    mp_ptr numa = BALLOC_TYPE(na, mp_limb_t);
    mp_ptr numb = BALLOC_TYPE(nb, mp_limb_t);
    na = lmmp_random_(numa, na);
    nb = lmmp_random_(numb, nb);
    mp_ptr dst = BALLOC_TYPE(na + nb, mp_limb_t);
    mp_ptr tmp = BALLOC_TYPE(na + nb, mp_limb_t);
    auto start = std::chrono::high_resolution_clock::now();
    lmmp_mul_(dst, numa, na, numb, nb);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time: " << duration.count() << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    lmmp_mul_ntt_unbal_(tmp, numa, na, numb, nb, 0);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time: " << duration.count() << " microseconds" << std::endl;

    for (mp_size_t i = 0; i < na + nb; i++) {
        if (dst[i] != tmp[i]) {
            std::cout << "Error: " << i << " " << dst[i] << " " << tmp[i] << std::endl;
            return 1;
        }
    }

    na = lmmp_random_(numa, na);
    nb = lmmp_random_(numb, nb);

    start = std::chrono::high_resolution_clock::now();
    lmmp_mul_(dst, numa, na, numb, nb);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time: " << duration.count() << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    lmmp_mul_ntt_unbal_(tmp, numa, na, numb, nb, 0);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Time: " << duration.count() << " microseconds" << std::endl;

    for (mp_size_t i = 0; i < na + nb; i++) {
        if (dst[i] != tmp[i]) {
            std::cout << "Error: " << i << " " << dst[i] << " " << tmp[i] << std::endl;
            return 1;
        }
    }

    TEMP_FREE;
    return 0;
}