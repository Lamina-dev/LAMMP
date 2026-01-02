#include "../include/test_short.hpp"
#include <vector>

namespace test_short {

void test_shift_in_word() {
    using namespace lammp;
    using namespace lammp::Arithmetic;

    size_t x_len = 4;
    std::vector<lamp_ui> x = {0xaaaaaaaaaaaaaaaa, 0x1111111111111111, 0xffffffffffffffff, 0xffffffffffffffff};
    size_t lshi = 16;
    std::vector<lamp_ui> x_lshi_inword_res0 = {0xaaaaaaaaaaaa0000, 0x111111111111aaaa, 0xffffffffffff1111,
                                               0xffffffffffffffff, 0xffff};
    size_t rshi = 8;
    std::vector<lamp_ui> x_rshi_inword_res0 = {0x11aaaaaaaaaaaaaa, 0xff11111111111111, 0xffffffffffffffff,
                                               0x00ffffffffffffff};

    std::cout << "test lshift of x by " << lshi << std::endl;

    std::vector<lamp_ui> x_lshi_inword_res(x_len + 1);
    lshift_in_word(x.data(), x_len, x_lshi_inword_res.data(), lshi);
    for (size_t i = 0; i < x_lshi_inword_res0.size(); i++) {
        if (x_lshi_inword_res[i] != x_lshi_inword_res0[i]) {
            std::cout << "error in lshift_inword" << std::endl;
            return;
        }
    }
    std::cout << "test lshift_inword passed" << std::endl;

    std::cout << "test rshift of x by " << rshi << std::endl;

    std::vector<lamp_ui> x_rshi_inword_res(x_len);
    rshift_in_word(x.data(), x_len, x_rshi_inword_res.data(), rshi);
    for (size_t i = 0; i < x_rshi_inword_res0.size(); i++) {
        if (x_rshi_inword_res[i] != x_rshi_inword_res0[i]) {
            std::cout << "error in rshift_inword" << std::endl;
            return;
        }
    }
    std::cout << "test rshift_inword passed" << std::endl;

}

void test_shift_bits() {
    using namespace lammp;
    using namespace lammp::Arithmetic;
    size_t x_len = 4;
    std::vector<lamp_ui> x = {0xaaaaaaaaaaaaaaaa, 0x1111111111111111, 0xffffffffffffffff, 0xffffffffffffffff};
    size_t lshi = 72;
    std::vector<lamp_ui> x_lshi_inword_res0 = {0x0000000000000000, 0xaaaaaaaaaaaaaa00, 0x11111111111111aa,
                                               0xffffffffffffff11, 0xffffffffffffffff, 0xff};
    size_t rshi = 120;
    std::vector<lamp_ui> x_rshi_inword_res0 = {0xffffffffffffff11, 0xffffffffffffffff, 0xff};

    std::cout << "test lshift of x by " << lshi << std::endl;

    std::vector<lamp_ui> x_lshi_inword_res(x_len + 2);
    lshift_bits(x.data(), x_len, x_lshi_inword_res.data(), lshi);
    for (size_t i = 0; i < x_lshi_inword_res0.size(); i++) {
        if (x_lshi_inword_res[i] != x_lshi_inword_res0[i]) {
            std::cout << "error in lshift_inword" << std::endl;
            return;
        }
    }
    std::cout << "test lshift_inword passed" << std::endl;

    std::cout << "test rshift of x by " << rshi << std::endl;

    std::vector<lamp_ui> x_rshi_inword_res(x_len);
    rshift_bits(x.data(), x_len, x_rshi_inword_res.data(), rshi);
    for (size_t i = 0; i < x_rshi_inword_res0.size(); i++) {
        if (x_rshi_inword_res[i] != x_rshi_inword_res0[i]) {
            std::cout << "error in rshift_inword" << std::endl;
            return;
        }
    }
    std::cout << "test rshift_inword passed" << std::endl;
}

};  // namespace test_short
