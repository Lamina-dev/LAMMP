
#ifndef __LAMMP_GETLEN_INL__
#define __LAMMP_GETLEN_INL__
namespace lammp::Arithmetic {
// remove leading zeros and return new length
constexpr lamp_ui rlz(const lamp_ptr array, lamp_ui length) {
    if (array == nullptr) {
        return 0;
    }
    while (length > 0 && array[length - 1] == 0) {
        length--;
    }
    return length;
}

inline lamp_ui get_add_len(lamp_ui l_len, lamp_ui r_len) { return std::max(l_len, r_len) + 1; }

inline lamp_ui get_sub_len(lamp_ui l_len, lamp_ui r_len) { return std::max(l_len, r_len); }

inline lamp_ui get_mul_len(lamp_ui l_len, lamp_ui r_len) {
    if (l_len == 0 || r_len == 0) {
        return 0;
    }
    return l_len + r_len;
}

inline lamp_ui get_div_len(lamp_ui l_len, lamp_ui r_len) { return l_len - r_len + 1; }
};  // namespace lammp::Arithmetic
#endif  // __LAMMP_GETLEN_INL__
