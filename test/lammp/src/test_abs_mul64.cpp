#include <vector>

#include "../include/test_short.hpp"


namespace test_short {

void test_abs_mul64_base() {
    using namespace lammp::Arithmetic;
    std::vector<uint64_t> vec1 = {1000000000000000000, 1000000000000000000, 1000000000000000000, 1000000000000000000,
                                  1000000000000000000};
    std::vector<uint64_t> vec2 = {1000000000000000000, 1000000000000000000, 1000000000000000000, 1000000000000000000,
                                  1000000000000000000};
    size_t res_len = vec1.size() + vec2.size();
    std::vector<uint64_t> result1(res_len, 0);
    std::vector<uint64_t> result2(res_len, 0);

    size_t base_num = 10000000000000000000ull;
    abs_mul64_ntt_base(vec1.data(), vec1.size(), vec2.data(), vec2.size(), result1.data(), base_num);

    for (size_t i = 0; i < res_len; i++) {
        std::cout << result1[i] << " ";
    }
    std::cout << std::endl;

}

};  // namespace test_short
