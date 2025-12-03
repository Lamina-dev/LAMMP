
#include "../../../../include/lammp/lammp.hpp"

namespace lammp::Arithmetic::Numeral {

namespace BaseTable {
struct BaseInfo {
    lamp_ui base_num;
    lamp_ui base_len;
    double base_d;
    BaseInfo(lamp_ui base) {
        assert(base != 0 && base != 1);
        if (base > 1 && base <= 36) {
            base_num = table1[base - 2][0];
            base_len = table1[base - 2][1];
            base_d = table2[base - 2];
            return;
        } else {
            lamp_ui t = 0xFFFFFFFFFFFFFFFFull;
            base_len = 0;
            while (t >= base) {  // 使用>=确保最后一次除法有效
                t /= base;
                base_len++;
            }
            /*
            warning：使用pow函数计算base_num的幂，可能会导致溢出，因此使用乘法计算
            */
            base_num = 1;
            for (lamp_ui i = 0; i < base_len; i++) base_num *= base;
            base_d = double(64.0) / (base_len * std::log2(double(base)));
            return;
        }
    }
    void base_d_inv() { base_d = 1.0 / base_d; }
};

template <lamp_ui Base>
struct ShortBaseInfo {
    static_assert(Base >= 2 && Base <= 36, "Base must be in [2, 36]");
    static constexpr lamp_ui index = Base - 2;
    static constexpr lamp_ui base_num = table1[index][0];
    static constexpr lamp_ui base_len = table1[index][1];
    static constexpr double base_d = table2[index];
    static constexpr double base_d_inv() { return 1.0 / base_d; }
};
};

// 返回逆序的字符串，低位数字在前
// 对于10进制以上的进制，使用大写字母A-Z，A表示10，B表示11，以此类推
std::string to_string_base(lamp_ui num, const lamp_ui base, const lamp_ui base_len) {
    std::string res(base_len, '0');
    for (lamp_ui i = 0; i < base_len; ++i) {
        num %= base;
        res[i] = (num < 10) ? ('0' + num) : ('A' + num - 10);
        num /= base;
    }
    return res;
}

// 将in数组表示的数从2^64进制转换为base_num进制，存储在res数组中，返回值为res的长度
// in数组会被修改
// res数组需要足够大，至少为 get_buffer_size(len, base_d)
lamp_ui num2base_classic(lamp_ptr in, lamp_ui len, const lamp_ui base_num, lamp_ptr res) {
    lamp_ui res_i = 0;
    while (len != 0 && in[len - 1] != 0) {
        res[res_i++] = abs_div_rem_num64(in, len, in, base_num);
        len = rlz(in, len);
    }
    return res_i;
}

// 将in数组表示的数从base_num进制转换为2^64进制，存储在res数组中，返回值为res的长度
// in数组会被修改
// res数组需要足够大，至少为 get_buffer_size(len, base_d)
// 一个值得注意的特性是：
//      虽然应当保证 in 数组中的每个元素都小于 base_num，
//      但是即使 in 数组中的某些元素大于 base_num，函数依然可以正确工作，并可以按照正常的 base_num 进制进行转换。
//      超过 base_num 的元素能够正确的进位。
lamp_ui base2num_classic(lamp_ptr in, lamp_ui len, const lamp_ui base_num, lamp_ptr res) {
    lamp_ui in_len = len, res_len = 0;
    while (in_len != 0) {
        lamp_ui temp = 0, product_lo = 0, product_hi = 0;
        for (lamp_ui i = in_len; i-- != 0;) {
            // 此assert可以保证输入的数符合进制要求
            // 由于函数本身可以处理不符合进制要求的数，因此可以选择忽略该assert
            assert(in[i] < base_num);
            mul64x64to128(temp, base_num, product_lo, product_hi);
            product_lo += in[i];
            product_hi += (product_lo < in[i]) ? 1 : 0;
            in[i] = product_hi;
            temp = product_lo;
        }
        res[res_len++] = temp;
        in_len = rlz(in, in_len);
    }
    return res_len;
}

// res为(2^64)^(index)在base_num进制下的表示，返回值为res的长度
lamp_ui _2_64power_index_classic(const lamp_ui base_num, const lamp_ui index, lamp_ptr res) {
    lamp_ui len = index + 1;
    _internal_buffer<0> _2pow64_index(len, 0);
    _2pow64_index.set(index, 1);
    // size_t res_i = 0;
    // while (len != 0 && _2pow64_index[len - 1] != 0)
    // {
    //     res[res_i++] = abs_div_rem_num64(_2pow64_index.data(), len, _2pow64_index.data(), base_num);
    //     len = remove_leading_zeros(_2pow64_index.data(), len);
    // }
    // return res_i;
    return num2base_classic(_2pow64_index.data(), len, base_num, res);
}

// res为(base_num)^(index)在 2^64 进制下的表示，返回值为res的长度
lamp_ui base_power_index_classic(const lamp_ui base_num, const lamp_ui index, lamp_ptr res) {
    lamp_ui len = index + 1;
    _internal_buffer<0> base_pow_index(len, 0);
    base_pow_index.set(index, 1);
    return base2num_classic(base_pow_index.data(), len, base_num, res);
}

// 进制转换后的长度，额外加一防止溢出
lamp_ui get_buffer_size(lamp_ui len, double base_d) { return static_cast<lamp_ui>(std::ceil(base_d * len)) + 1; }

// 计算 in * 2^64，并存储在 in 数组中，in 数组会被修改
// in 数组为 base_num 进制下的数
void abs_mul2pow64_base(lamp_ptr in, lamp_ui len, const lamp_ui base_num) {
    lamp_ui temp = 0;
    for (lamp_ui i = 0; i < len; i++) {
        // 计算in[i] * 2^64
        // 由于in[i]小于2^64，乘以2^64相当于直接移至高位
        lamp_ui product_lo = temp;
        temp = div128by64to64(in[i], product_lo, base_num);
        in[i] = product_lo;
        // 需要说明的是，in数组为base_num进制下的数(小于 B - 1)，因此在计算in*2^64时，
        // 保留的进位最多为 (B - 1)*2^64 / B, 不会超过2^64 - 1，因此不需要考虑temp的高位问题。
    }
    while (temp != 0) {
        in[len++] = temp % base_num;
        temp /= base_num;
    }
}

// 计算 in * base，并存储在 in 数组中，in 数组会被修改
// in 数组为 2^64 进制下的数
void abs_mul_base(lamp_ptr in, lamp_ui len, const lamp_ui base_num) {
    lamp_ui temp = 0;
    for (lamp_ui i = 0; i < len; i++) {
        // 计算in[i] * base_num
        // 在2^64进制下
        lamp_ui product_lo = 0, product_hi = 0;
        mul64x64to128(in[i], base_num, product_lo, product_hi);
        product_lo += temp;
        product_hi += (product_lo < temp) ? 1 : 0;
        temp = product_hi;
        in[i] = product_lo;
    }
    in[len] = temp;
}

// 计算进制数为 base_num 的(2^64)^index 值，并存储在 res 数组中
lamp_ui _2_64_power_index(const lamp_ui base_num, const lamp_ui index, lamp_ptr res, lamp_ui res_len) {
    assert(index > 0);

    if (index <= 32) {
        return _2_64power_index_classic(base_num, index, res);
    }
    if (index % 2 == 0) {
        lamp_ui temp_len = res_len / 2 + 1;
        _internal_buffer<0> temp(temp_len, 0);
        temp_len = _2_64_power_index(base_num, index / 2, temp.data(), temp_len);
        abs_sqr64_ntt_base(temp.data(), temp_len, res, base_num);
        return rlz(res, 2 * temp_len);
    } else {
        res_len = _2_64_power_index(base_num, index - 1, res, res_len);
        abs_mul2pow64_base(res, res_len, base_num);
        res_len = rlz(res, res_len + 2);
        return rlz(res, res_len);
    }
}

// 计算进制数为 2^64 的(base_num)^index 值，并存储在 res 数组中
lamp_ui base_power_index(const lamp_ui base_num, const lamp_ui index, lamp_ptr res, lamp_ui res_len) {
    assert(index > 0);

    if (index <= 32) {
        return base_power_index_classic(base_num, index, res);
    }
    if (index % 2 == 0) {
        lamp_ui temp_len = res_len / 2 + 1;
        _internal_buffer<0> temp(temp_len, 0);
        temp_len = base_power_index(base_num, index / 2, temp.data(), temp_len);
        abs_mul64(temp.data(), temp_len, temp.data(), temp_len, res);
        return rlz(res, 2 * temp_len);
    } else {
        res_len = base_power_index(base_num, index - 1, res, res_len);
        abs_mul_base(res, res_len, base_num);
        res_len = rlz(res, res_len + 2);
        return rlz(res, res_len);
    }
}

// 只能处理 len 为二的次幂的情况，in 会被修改
// 将2^64进制转换为base_num进制，并存储在out数组中
lamp_ui num_base_recursive_core(lamp_ptr in,
                                       lamp_ui len,
                                       const lamp_ui base_num,
                                       const double base_d,
                                       lamp_ptr out,
                                       const _2pow64_index_list list) {
    // assert len != 0 && len为2的幂
    assert(len != 0 && (len & (len - 1)) == 0);

    if (len <= MIN_LEN) {
        return num2base_classic(in, len, base_num, out);
    }

    assert(list != nullptr);
    assert(list->index == len / 2);

    lamp_ui half_len = len / 2, pow_len = list->length, buffer_len = get_buffer_size(half_len, base_d);
    lamp_ptr base_pow = list->base_index.data();
    _internal_buffer<0> buffer(buffer_len, 0);
    // low
    buffer_len = num_base_recursive_core(in, half_len, base_num, base_d, buffer.data(), list->front);
    // high
    lamp_ui out_len = num_base_recursive_core(in + half_len, half_len, base_num, base_d, out, list->front);
    // high * base_pow
    abs_mul64_ntt_base(out, out_len, base_pow, pow_len, out, base_num);
    out_len = rlz(out, out_len + pow_len);
    // out <= high * base_pow + low
    abs_add_base(buffer.data(), buffer_len, out, out_len, out, base_num);
    return rlz(out, get_add_len(out_len, buffer_len));
}

/// 创建2^64的指数列表，下面用 M 表示 2^(64 * MIN_LEN)
/// index 为 M 的指数，length 为 M^index 在 base_num 进制下的长度，
/// _base_index 为 M^index 的值
_2pow64_index_list create_2pow64_index_list(lamp_ui max_index, const lamp_ui base_num, const double base_d) {
    ///   head
    ///     |
    ///     ↓
    ///     +--------+ <--front---+--------+ <--front---+--------+ <--...
    ///     |  M^1   |            |  M^2   |            |  M^4   |
    ///     +--------+ ---back--->+--------+ ---back--->+--------+ ----...
    /// 2^64的指数列表， M 表示 2^(64 * MIN_LEN)
    assert(max_index > MIN_LEN);
    assert((max_index & (max_index - 1)) == 0);

    _2pow64_index_list head = new base_index_node(MIN_LEN, base_d);
    head->length = _2_64_power_index(base_num, MIN_LEN, head->base_index.data(), head->length);

    _2pow64_index_list current = head;

    for (lamp_ui i = MIN_LEN << 1; i <= max_index; i <<= 1) {
        current->back = new base_index_node(i, base_d);
        // 无任意基数乘法的权宜之计，直接用 NTT 乘法计算
        abs_sqr64_ntt_base(current->base_index.data(), current->length, current->back->base_index.data(), base_num);
        current->back->length = rlz(current->back->base_index.data(), current->back->length);
        current->back->front = current;
        current = current->back;
    }
    return head;
}

// 只能处理 len 为二的次幂的情况，in 会被修改
// 将base_num进制转换为2^64进制，并存储在out数组中
lamp_ui base_num_recursive_core(lamp_ptr in,
                                       lamp_ui len,
                                       const lamp_ui base_num,
                                       const double base_d,
                                       lamp_ptr out,
                                       const _base_index_list list) {
    // assert len != 0 && len为2的幂
    assert(len != 0 && (len & (len - 1)) == 0);

    if (len <= MIN_LEN) {
        return base2num_classic(in, len, base_num, out);
    }

    assert(list != nullptr);
    assert(list->index == len / 2);

    lamp_ui half_len = len / 2, pow_len = list->length, buffer_len = get_buffer_size(half_len, base_d);
    lamp_ptr base_pow = list->base_index.data();
    _internal_buffer<0> buffer(buffer_len, 0);
    // low
    buffer_len = base_num_recursive_core(in, half_len, base_num, base_d, buffer.data(), list->front);
    // high
    lamp_ui out_len = base_num_recursive_core(in + half_len, half_len, base_num, base_d, out, list->front);
    // high * base_pow
    abs_mul64(out, out_len, base_pow, pow_len, out);
    out_len = rlz(out, out_len + pow_len);
    // out <= high * base_pow + low
    abs_add_binary(buffer.data(), buffer_len, out, out_len, out);
    return rlz(out, get_add_len(out_len, buffer_len));
}

/// 创建base_num的指数列表，下面用 M 表示 base_num^(MIN_LEN)
/// index 为 M 的指数，length 为 M^index 在 base_num 进制下的长度，
/// _base_index 为 M^index 的值
_base_index_list create_base_index_list(lamp_ui max_index, const double base_d, const lamp_ui base_num) {
    ///   head
    ///     |
    ///     ↓
    ///     +--------+ <--front---+--------+ <--front---+--------+ <--...
    ///     |  M^1   |            |  M^2   |            |  M^4   |
    ///     +--------+ ---back--->+--------+ ---back--->+--------+ ----...
    /// base_num的指数列表， M 表示 base_num^(MIN_LEN)
    assert(max_index > MIN_LEN);
    assert((max_index & (max_index - 1)) == 0);

    _2pow64_index_list head = new base_index_node(MIN_LEN, base_d);
    head->length = base_power_index(base_num, MIN_LEN, head->base_index.data(), head->length);

    _2pow64_index_list current = head;
    for (lamp_ui i = MIN_LEN << 1; i <= max_index; i <<= 1) {
        current->back = new base_index_node(i, base_d);
        abs_mul64(current->base_index.data(), current->length, current->base_index.data(), current->length,
                  current->back->base_index.data());
        current->back->length = rlz(current->back->base_index.data(), current->back->length);
        current->back->front = current;
        current = current->back;
    }
    return head;
}

// 在链表中查找 index 对应的节点
_2pow64_index_list find_head(_2pow64_index_list head, lamp_ui index) {
    while (head->back != nullptr) {
        if (head->index == index)
            return head;
        head = head->back;
    }
    if (head->index == index)
        return head;
    return nullptr;
}

/// @brief 将一个表示为64位块数组的大整数从二进制（基数2^64）转换为指定的较小基数
/// @param in 表示要转换的大整数的输入数组。数组的每个元素都是该整数的一个64位块
/// @param len 输入数组中64位块的数量
/// @param base 要转换到的基数
/// @param res 转换后的结果数组，数组的每个元素都是该整数在目标基数下的一个64位块
/// @return 转换后的结果数组的长度
/// @note
///  1. 该函数不会对 res 进行边界检查，调用者需要确保res有足够的空间来存储转换后的结果
///  2. 该函数会修改输入数组 in 的内容（正确计算后，应为全零），因此如果需要保留原始数据，调用者应在调用前进行备份
lamp_ui num2base(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res) {
    // 1. 分割策略：按 2 的幂次方长度分割
    //         每次处理的子部分长度都是 2^k（num_base_recursive_core会递归处理这部分），
    //         通过`63ull - hint_clz(current_len)` 计算当前最大可能的 2 的幂指数
    //             例如，若当前剩余长度为 10，则最大的 2 的幂是 8（2 ^ 3），先处理前 8 个元素，剩余 2 个元素下次处理
    //         代码中 `pri_len = 1ull << current_index` 就是计算当前要处理的子部分长度
    //
    // 2. 预计算缓存：避免重复计算
    //         通过create_2pow64_index_list创建预计算链表，存储不同长度（2 的幂）的子部分在目标进制下的表示
    //         链表的结构如下：
    ///             head
    ///              |
    ///              ↓
    ///              +--------+ <--front--- +--------+ <--front--- +--------+ <--...
    ///              |  M^1   |             |  M^2   |             |  M^4   |
    ///              +--------+ ---back---> +--------+ ---back---> +--------+ ----...
    ///         其中 M 表示 2^(64 * MIN_LEN)，`M^i` 表示 2^(64 * MIN_LEN * i)
    //         代码中通过`find_head(head, pri_len)` 用于查找当前子部分长度对应的预计算数据。
    //
    // 3. 子问题处理：
    //         对每个子部分（长度`pri_len`）调用 `num_base_recursive_core` 进行进制转换，
    //         这个递归函数会继续将子部分分割为更小的部分，直到达到 MIN_LEN 阈值后使用经典算法
    //
    // 4. 合并策略：
    //         基数幂乘法 + 加法 由于大整数的各子部分在原始表示中是 "高位在前" 的（类似a * B ^ n + b，其中 B
    //         是原始基数）， 合并时需要： 将剩余子部分的转换结果乘以 base ^ pow（pow是之前处理的总长度）
    //         与已累计的结果相加，代码中通过
    //         `abs_mul64_ntt_base`实现乘法，`abs_add_base`实现加法，`base_pow`数组动态维护当前需要的基数幂值，随处理过程更新
    //         需要注意的是：随着递归的进行，剩余子部分的转换结果乘以基数幂，这通常是一个不平衡乘法，
    //         使用NTT-crt可能并不能达到最佳性能，使用 Karatsuba 等方法可能会更合适
    //
    // 5. 终止条件：小规模使用经典算法 当剩余长度 <= MIN_LEN时，不再分割，直接调用 num2base_classic 处理。

    BaseTable::BaseInfo info(base);
    const lamp_ui max_len_2pow_index = 63ull - lammp_clz(len);

    if (len <= 2 * MIN_LEN) {
        return num2base_classic(in, len, info.base_num, res);
    }
    _2pow64_index_list head = create_2pow64_index_list(1ull << max_len_2pow_index, info.base_num, info.base_d);
    lamp_ptr current_in = in;

    lamp_ui pow_len = 0, res_len = 0;
    _internal_buffer<0> base_pow(get_buffer_size(len, info.base_d), 0);

    for (lamp_ui current_len = len; current_len > MIN_LEN;) {
        lamp_ui current_index = 63ull - lammp_clz(current_len);
        lamp_ui pri_len = 1ull << current_index;
        lamp_ui buffer_len = get_buffer_size(pri_len, info.base_d) + pow_len;

        _2pow64_index_list current_list = find_head(head, pri_len);
        assert(current_list != nullptr);
        assert(current_list->index == pri_len);

        // 计算 base_pow 并计算 buffer * base_pow => res
        if (pow_len == 0) {
            res_len =
                num_base_recursive_core(current_in, pri_len, info.base_num, info.base_d, res, current_list->front);
            std::copy(current_list->base_index.begin(), current_list->base_index.end(), base_pow.begin());
            pow_len = current_list->length;
        } else {
            _internal_buffer<0> buffer(buffer_len, 0);
            buffer_len = num_base_recursive_core(current_in, pri_len, info.base_num, info.base_d, buffer.data(),
                                                 current_list->front);
            // ntt 只是权宜之计，目前没有适配的任意基数卡拉楚巴乘法
            abs_mul64_ntt_base(buffer.data(), buffer_len, base_pow.data(), pow_len, buffer.data(), info.base_num);
            buffer_len = rlz(buffer.data(), get_mul_len(buffer_len, pow_len));
            abs_add_base(buffer.data(), buffer_len, res, res_len, res, info.base_num);
            res_len = rlz(res, get_add_len(res_len, buffer_len));

            // 更新 base_pow
            // 同上上述，这里的 ntt 也只是权宜之计，没有适配的任意基数卡拉楚巴乘法
            abs_mul64_ntt_base(base_pow.data(), pow_len, current_list->base_index.data(), current_list->length,
                               base_pow.data(), info.base_num);
            pow_len = rlz(base_pow.data(), get_mul_len(pow_len, current_list->length));
        }

        current_len -= pri_len;
        current_in += pri_len;

        if (current_len == 0) {
            return res_len;
        }

        if (current_len <= MIN_LEN) {
            buffer_len = get_buffer_size(pri_len, info.base_d) + pow_len;
            _internal_buffer<0> buffer(buffer_len, 0);
            buffer_len = num2base_classic(current_in, current_len, info.base_num, buffer.data());

            // ntt 只是权宜之计，目前没有适配的任意基数卡拉楚巴乘法
            abs_mul64_ntt_base(buffer.data(), buffer_len, base_pow.data(), pow_len, buffer.data(), info.base_num);
            buffer_len = rlz(buffer.data(), get_mul_len(buffer_len, pow_len));
            abs_add_base(buffer.data(), buffer_len, res, res_len, res, info.base_num);
            res_len = rlz(res, get_add_len(res_len, buffer_len));
            return res_len;
        }
    }
    assert(false);
    return 0;
}

/// @brief 将一个表示为64位块数组的大整数从base进制（基数base_num）转换为指定的二进制（基数2^64）
/// @param in 表示要转换的大整数的输入数组。数组的每个元素都是该整数的一个64位块
/// @param len 输入数组中64位块的数量
/// @param base 要转换到的基数
/// @param res 转换后的结果数组，数组的每个元素都是该整数在目标基数下的一个64位块
/// @return 转换后的结果数组的长度
/// @note
///  1. 该函数不会对 res 进行边界检查，调用者需要确保res有足够的空间来存储转换后的结果
///  2. 该函数会修改输入数组 in 的内容（正确计算后，应为全零），因此如果需要保留原始数据，调用者应在调用前进行备份
lamp_ui base2num(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res) {
    BaseTable::BaseInfo info(base);
    info.base_d_inv();
    const lamp_ui max_len_base_index = 63ull - lammp_clz(len);

    if (len <= 2 * MIN_LEN) {
        return base2num_classic(in, len, info.base_num, res);
    }
    _base_index_list head = create_base_index_list(1ull << max_len_base_index, info.base_d, info.base_num);
    lamp_ptr current_in = in;

    lamp_ui pow_len = 0, res_len = 0;
    _internal_buffer<0> _2_64_pow(get_buffer_size(len, info.base_d), 0);

    for (lamp_ui current_len = len; current_len > MIN_LEN;) {
        lamp_ui current_index = 63ull - lammp_clz(current_len);
        lamp_ui pri_len = 1ull << current_index;
        lamp_ui buffer_len = get_buffer_size(pri_len, info.base_d) + pow_len;

        _base_index_list current_list = find_head(head, pri_len);
        assert(current_list != nullptr);
        assert(current_list->index == pri_len);

        // 计算 base_pow 并计算 buffer * base_pow => res
        if (pow_len == 0) {
            res_len =
                base_num_recursive_core(current_in, pri_len, info.base_num, info.base_d, res, current_list->front);
            std::copy(current_list->base_index.begin(), current_list->base_index.end(), _2_64_pow.begin());
            pow_len = current_list->length;
        } else {
            _internal_buffer<0> buffer(buffer_len, 0);
            buffer_len = base_num_recursive_core(current_in, pri_len, info.base_num, info.base_d, buffer.data(),
                                                 current_list->front);
            abs_mul64(buffer.data(), buffer_len, _2_64_pow.data(), pow_len, buffer.data());
            buffer_len = rlz(buffer.data(), get_mul_len(buffer_len, pow_len));
            abs_add_binary(buffer.data(), buffer_len, res, res_len, res);
            res_len = rlz(res, get_add_len(res_len, buffer_len));

            // 更新 base_pow
            abs_mul64(_2_64_pow.data(), pow_len, current_list->base_index.data(), current_list->length,
                      _2_64_pow.data());
            pow_len = rlz(_2_64_pow.data(), get_mul_len(pow_len, current_list->length));
        }

        current_len -= pri_len;
        current_in += pri_len;

        if (current_len == 0) {
            return res_len;
        }

        if (current_len <= MIN_LEN) {
            buffer_len = get_buffer_size(pri_len, info.base_d) + pow_len;
            _internal_buffer<0> buffer(buffer_len, 0);
            buffer_len = base2num_classic(current_in, current_len, info.base_num, buffer.data());

            abs_mul64(buffer.data(), buffer_len, _2_64_pow.data(), pow_len, buffer.data());
            buffer_len = rlz(buffer.data(), get_mul_len(buffer_len, pow_len));
            abs_add_binary(buffer.data(), buffer_len, res, res_len, res);
            res_len = rlz(res, get_add_len(res_len, buffer_len));
            return res_len;
        }
    }
    assert(false);
    return 0;
}
};  // namespace Numeral