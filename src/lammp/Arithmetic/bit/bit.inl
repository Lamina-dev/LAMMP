
#ifndef __LAMMP_BIT_BIT_INL__
#define __LAMMP_BIT_BIT_INL__
namespace lammp::Arithmetic {
inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui bit_pos, bool value) {
    assert(bit_pos < len * 64);
    const lamp_ui word_pos = bit_pos / 64;
    const lamp_ui bit_in_word_pos = bit_pos % 64;
    in_out[word_pos] |= (value ? 1ull : 0ull) << bit_in_word_pos;
}

inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui word_pos, lamp_ui bit_pos, bool value) {
    assert(word_pos * 64 + bit_pos < len * 64);
    in_out[word_pos] |= (value ? 1ull : 0ull) << bit_pos;
}

inline bool get_bit(const lamp_ptr in, lamp_ui len, lamp_ui bit_pos) {
    assert(bit_pos < len * 64);
    const lamp_ui word_pos = bit_pos / 64;
    const lamp_ui bit_in_word_pos = bit_pos % 64;
    return (in[word_pos] >> bit_in_word_pos) & 1;
}

inline lamp_ui bit_length(lamp_ptr in, lamp_ui len) {
    constexpr lamp_ui WORD_BITS = sizeof(lamp_ui) * CHAR_BIT;
    assert(in != nullptr);
    len = rlz(in, len);
    return len * WORD_BITS - lammp_clz(in[len - 1]);
}
};  // namespace lammp::Arithmetic

#endif  // __LAMMP_BIT_BIT_INL__