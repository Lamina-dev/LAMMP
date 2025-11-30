/*
 * [LAMMP]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky & HJimmyK

https://github.com/Twilight-Dream-Of-Magic/
https://github.com/With-Sky
https://github.com/HJimmyK

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __LAMMP_HPP__
#define __LAMMP_HPP__

#include <math.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


// Windows 64bit fast multiply macro.
#if defined(_WIN64)
#include <intrin.h>
#define UMUL128
#endif  //_WIN64

// GCC 64bit fast multiply macro.
#if defined(__SIZEOF_INT128__)
#define UINT128T
#endif  //__SIZEOF_INT128__

#if defined(_MSC_VER)
// MSVC：需 SSE4.2 支持，若目标平台不支持可考虑 fallback
#include <nmmintrin.h>
#define _LAMMP_MSVC
#elif defined(__GNUC__)
#define _LAMMP_GCC
#endif  //_MSC_VER

namespace lammp {
/*******************************   begin  *******************************/
//* ---------------------------   Allocate   --------------------------*//
/*******************************   begin  *******************************/

// 默认分配器
class _default__allocator {
   public:
    static void* allocate(size_t size, size_t alignment) {
#ifdef _ISOC11_SOURCE
        return aligned_alloc(alignment, size);
#else
#ifdef _WIN32
        return _aligned_malloc(size, alignment);
#else
        void* ptr = nullptr;
        if (posix_memalign(&ptr, alignment, size) == 0) {
            return ptr;
        }
        return nullptr;
#endif
#endif
    }

    static void deallocate(void* ptr) {
#ifdef _WIN32
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
};

// 主模板 - 适用于 StackCapacity > 0 的情况
template <size_t StackCapacity, size_t Alignment = 64, typename Allocator = _default__allocator>
class _internal_buffer {
   private:
    static_assert(StackCapacity > 0, "StackCapacity must be positive for primary template");
    static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");

    alignas(Alignment) uint64_t stack_buffer_[StackCapacity];
    uint64_t* data_;
    size_t capacity_;
    Allocator allocator_;

   public:
    explicit _internal_buffer(size_t size, uint64_t value) : allocator_() {
        if (size <= StackCapacity) {
            data_ = stack_buffer_;
            capacity_ = StackCapacity;
        } else {
            capacity_ = size;
            size_t bytes_needed = capacity_ * sizeof(uint64_t);
            data_ = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
            if (!data_)
                throw std::bad_alloc();
        }
        std::fill(data_, data_ + capacity_, value);
    }

    explicit _internal_buffer(size_t size) : allocator_() {
        if (size <= StackCapacity) {
            data_ = stack_buffer_;
            capacity_ = StackCapacity;
        } else {
            capacity_ = size;
            size_t bytes_needed = capacity_ * sizeof(uint64_t);
            data_ = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
            if (!data_)
                throw std::bad_alloc();
        }
    }
    explicit _internal_buffer() : allocator_() {
        data_ = stack_buffer_;
        capacity_ = StackCapacity;
    }

    template <typename A = Allocator>
    _internal_buffer(size_t size, uint64_t value, A&& alloc) : allocator_(std::forward<A>(alloc)) {
        if (size <= StackCapacity) {
            data_ = stack_buffer_;
            capacity_ = StackCapacity;
        } else {
            capacity_ = size;
            size_t bytes_needed = capacity_ * sizeof(uint64_t);
            data_ = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
            if (!data_)
                throw std::bad_alloc();
        }
        std::fill(data_, data_ + capacity_, value);
    }
    template <typename A = Allocator>
    _internal_buffer(size_t size, A&& alloc) : allocator_(std::forward<A>(alloc)) {
        if (size <= StackCapacity) {
            data_ = stack_buffer_;
            capacity_ = StackCapacity;
        } else {
            capacity_ = size;
            size_t bytes_needed = capacity_ * sizeof(uint64_t);
            data_ = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
            if (!data_)
                throw std::bad_alloc();
        }
    }

    ~_internal_buffer() {
        if (is_on_heap()) {
            allocator_.deallocate(data_);
        }
    }

    // 禁用拷贝
    _internal_buffer(const _internal_buffer&) = delete;
    _internal_buffer& operator=(const _internal_buffer&) = delete;

    // 移动构造
    _internal_buffer(_internal_buffer&& other) noexcept
        : data_(other.data_), capacity_(other.capacity_), allocator_(std::move(other.allocator_)) {
        if (!other.is_on_heap()) {
            data_ = stack_buffer_;
            std::copy(other.stack_buffer_, other.stack_buffer_ + StackCapacity, stack_buffer_);
        }

        other.data_ = nullptr;
        other.capacity_ = 0;
    }

    // 移动赋值
    _internal_buffer& operator=(_internal_buffer&& other) noexcept {
        if (this != &other) {
            if (is_on_heap()) {
                allocator_.deallocate(data_);
            }

            data_ = other.data_;
            capacity_ = other.capacity_;
            allocator_ = std::move(other.allocator_);

            if (!other.is_on_heap()) {
                data_ = stack_buffer_;
                std::copy(other.stack_buffer_, other.stack_buffer_ + StackCapacity, stack_buffer_);
            }

            other.data_ = nullptr;
            other.capacity_ = 0;
        }
        return *this;
    }

    uint64_t operator[](size_t i) const {
        assert(i < capacity_);
        return data_[i]; 
    }
    void set(size_t i, uint64_t value) {
        assert(i < capacity_);
        data_[i] = value; 
    }

    // 获取数据指针
    uint64_t* data() { return data_; }
    const uint64_t* data() const { return data_; }
    uint64_t* begin() { return data_; }
    uint64_t* end() { return data_ + capacity_; }

    // 工具函数
    size_t capacity() const { return capacity_; }
    bool is_on_heap() const { return data_ != stack_buffer_; }

    // 检查对齐
    bool is_properly_aligned() const { return (reinterpret_cast<uintptr_t>(data_) % Alignment) == 0; }

    // 获取分配器
    Allocator& get_allocator() { return allocator_; }
    const Allocator& get_allocator() const { return allocator_; }

    // 对象大小信息
    static constexpr size_t stack_capacity() { return StackCapacity; }
    static constexpr bool has_stack_buffer() { return true; }

};  // end define _internal_buffer

// 特化版本 - 适用于 StackCapacity = 0 的情况（完全堆分配）
template <size_t Alignment, typename Allocator>
class _internal_buffer<0, Alignment, Allocator> {
   private:
    static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be power of 2");

    uint64_t* data_;
    size_t capacity_;
    Allocator allocator_;

   public:
    explicit _internal_buffer(size_t size, uint64_t value) : allocator_() {
        capacity_ = size;
        size_t bytes_needed = capacity_ * sizeof(uint64_t);
        data_ = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
        if (!data_)
            throw std::bad_alloc();
        std::fill(data_, data_ + size, value);
    }
    explicit _internal_buffer(size_t size) : allocator_() {
        if (size == 0) {
            capacity_ = 0;
            data_ = nullptr;
            return;
        }
        capacity_ = size;
        size_t bytes_needed = capacity_ * sizeof(uint64_t);
        data_ = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
        if (!data_)
            throw std::bad_alloc();
    }
    explicit _internal_buffer() : allocator_() {
        capacity_ = 0;
        data_ = nullptr;
    }

    template <typename A = Allocator>
    _internal_buffer(size_t size, uint64_t value, A&& alloc) : allocator_(std::forward<A>(alloc)) {
        capacity_ = size;
        size_t bytes_needed = capacity_ * sizeof(uint64_t);
        data_ = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
        if (!data_)
            throw std::bad_alloc();
        std::fill(data_, data_ + size, value);
    }

    ~_internal_buffer() { allocator_.deallocate(data_); }

    // 禁用拷贝
    _internal_buffer(const _internal_buffer&) = delete;
    _internal_buffer& operator=(const _internal_buffer&) = delete;

    // 移动构造 - 更简单，因为没有栈缓冲区
    _internal_buffer(_internal_buffer&& other) noexcept
        : data_(other.data_), capacity_(other.capacity_), allocator_(std::move(other.allocator_)) {
        other.data_ = nullptr;
        other.capacity_ = 0;
    }

    // 移动赋值 - 更简单，因为没有栈缓冲区
    _internal_buffer& operator=(_internal_buffer&& other) noexcept {
        if (this != &other) {
            allocator_.deallocate(data_);

            data_ = other.data_;
            capacity_ = other.capacity_;
            allocator_ = std::move(other.allocator_);

            other.data_ = nullptr;
            other.capacity_ = 0;
        }
        return *this;
    }

    uint64_t operator[](size_t i) const {
        assert(i < capacity_);
        return data_[i];
    }
    void set(size_t i, uint64_t value) {
        assert(i < capacity_);
        data_[i] = value;
    }
    uint64_t* begin() { return data_; }
    uint64_t* end() { return data_ + capacity_; }

    // 获取数据指针
    uint64_t* data() { return data_; }
    const uint64_t* data() const { return data_; }

    // 工具函数
    size_t capacity() const { return capacity_; }
    bool is_on_heap() const { return true; }  // 始终在堆上

    // 检查对齐
    constexpr bool is_properly_aligned() const { return (reinterpret_cast<uintptr_t>(data_) % Alignment) == 0; }

    // 获取分配器
    Allocator& get_allocator() { return allocator_; }
    const Allocator& get_allocator() const { return allocator_; }

    // 对象大小信息
    static constexpr size_t stack_capacity() { return 0; }
    static constexpr bool has_stack_buffer() { return false; }

    // 重新分配方法 - 只在堆分配版本中提供
    void resize(size_t new_size) {
        if (new_size <= capacity_)
            return;

        size_t new_capacity = new_size;
        size_t bytes_needed = new_capacity * sizeof(uint64_t);
        uint64_t* new_data = static_cast<uint64_t*>(allocator_.allocate(bytes_needed, Alignment));
        if (!new_data)
            throw std::bad_alloc();

        // 复制旧数据
        std::copy(data_, data_ + capacity_, new_data);
        // 初始化新空间
        std::fill(new_data + capacity_, new_data + new_capacity, 0);

        // 释放旧数据并更新指针
        allocator_.deallocate(data_);
        data_ = new_data;
        capacity_ = new_capacity;
    }
};  // end define _internal_buffer<0, Alignment, Allocator>

/*******************************    end  *******************************/
//* ---------------------------   Allocate   --------------------------*//
/*******************************    end  *******************************/

// bits of 1, equals to 2^bits - 1
template <typename T>
constexpr T all_one(int bits) {
    T temp = T(1) << (bits - 1);
    return temp - 1 + temp;
}

// Leading zeros
template <typename IntTy>
constexpr int lammp_clz(IntTy x) {
    constexpr uint32_t MASK32 = uint32_t(0xFFFF) << 16;
    int res = sizeof(IntTy) * CHAR_BIT;
    if (x & MASK32) {
        res -= 16;
        x >>= 16;
    }
    if (x & (MASK32 >> 8)) {
        res -= 8;
        x >>= 8;
    }
    if (x & (MASK32 >> 12)) {
        res -= 4;
        x >>= 4;
    }
    if (x & (MASK32 >> 14)) {
        res -= 2;
        x >>= 2;
    }
    if (x & (MASK32 >> 15)) {
        res -= 1;
        x >>= 1;
    }
    return res - x;
}

// 32位无符号整数的前导零计数（x=0 时结果未定义）
static inline constexpr int lammp_clz(uint32_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    // _BitScanReverse 找到最高位1的位置，前导零 = 31 - 位置
    _BitScanReverse(&idx, x);
    return 31 - (int)idx;
#elif defined(_LAMMP_GCC)
    return __builtin_clz(x);
#endif
}

// 64位无符号整数的前导零计数（x=0 时结果未定义）
static inline constexpr int lammp_clz(uint64_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    _BitScanReverse64(&idx, x);  // 64位版本的位扫描
    return 63 - (int)idx;
#elif defined(_LAMMP_GCC)
    return __builtin_clzll(x);
#endif
}

// 32位无符号整数的尾随零计数（x=0 时结果未定义）
static inline constexpr int lammp_ctz(uint32_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    // _BitScanForward 找到最低位1的位置，即尾随零的数量
    _BitScanForward(&idx, x);
    return (int)idx;
#elif defined(_LAMMP_GCC)
    return __builtin_ctz(x);
#endif
}

// 64位无符号整数的尾随零计数（x=0 时结果未定义）
static inline constexpr int lammp_ctz(uint64_t x) {
#if defined(_LAMMP_MSVC)
    unsigned long idx;
    _BitScanForward64(&idx, x);  // 64位版本的位扫描
    return (int)idx;
#else
    return __builtin_ctzll(x);
#endif
}

// 32位无符号整数的位计数
static inline constexpr int lammp_cnt(uint32_t x) {
#if defined(_LAMMP_MSVC)
    return _mm_popcnt_u32(x);
#elif defined(_LAMMP_GCC)
    return __builtin_popcount(x);
#endif
}

// 64位无符号整数的位计数
static inline constexpr int lammp_cnt(uint64_t x) {
#if defined(_LAMMP_MSVC)
    return _mm_popcnt_u64(x);
#elif defined(_LAMMP_GCC)
    return __builtin_popcountll(x);
#endif
}

// 32位无符号整数的位长度（bit length）
static inline constexpr int lammp_bit_length(uint32_t x) { return (x == 0) ? 0 : (32 - lammp_clz(x)); }

// 64位无符号整数的位长度（bit length）
static inline constexpr int lammp_bit_length(uint64_t x) { return (x == 0) ? 0 : (64 - lammp_clz(x)); }

// 32位无符号整数的log2（返回最高位1的位置索引）
static inline constexpr int lammp_log2(uint32_t x) { return (x == 0) ? -1 : (31 - lammp_clz(x)); }

// 64位无符号整数的log2（返回最高位1的位置索引）
static inline constexpr int lammp_log2(uint64_t x) { return (x == 0) ? -1 : (63 - lammp_clz(x)); }

// Fast power
template <typename T, typename T1>
constexpr T qpow(T m, T1 n) {
    T result = 1;
    while (n > 0) {
        if ((n & 1) != 0) {
            result *= m;
        }
        m *= m;
        n >>= 1;
    }
    return result;
}

// Fast power with mod
template <typename T, typename T1>
constexpr T qpow(T m, T1 n, T mod) {
    T result = 1;
    while (n > 0) {
        if ((n & 1) != 0) {
            result *= m;
            result %= mod;
        }
        m *= m;
        m %= mod;
        n >>= 1;
    }
    return result;
}

// Get cloest power of 2 that not larger than n
template <typename T>
constexpr T int_floor2(T n) {
    constexpr int bits = sizeof(n) * CHAR_BIT;
    for (int i = 1; i < bits; i *= 2) {
        n |= (n >> i);
    }
    return (n >> 1) + 1;
}

// Get cloest power of 2 that not smaller than n
template <typename T>
constexpr T int_ceil2(T n) {
    constexpr int bits = sizeof(n) * CHAR_BIT;
    n--;
    for (int i = 1; i < bits; i *= 2) {
        n |= (n >> i);
    }
    return n + 1;
}

// x + y = sum with carry
template <typename UintTy>
constexpr UintTy add_half(UintTy x, UintTy y, bool& cf) {
    x = x + y;
    cf = (x < y);
    return x;
}

// x - y = diff with borrow
template <typename UintTy>
constexpr UintTy sub_half(UintTy x, UintTy y, bool& bf) {
    y = x - y;
    bf = (y > x);
    return y;
}

// x + y + cf = sum with carry
template <typename UintTy>
constexpr UintTy add_carry(UintTy x, UintTy y, bool& cf) {
    UintTy sum = x + cf;
    cf = (sum < x);
    sum += y;              // carry
    cf = cf || (sum < y);  // carry
    return sum;
}

// carry = (x + y) / base_num, return (x + y) mod base_num
template <typename UintTy>
constexpr UintTy add_half_base(UintTy x, UintTy y, bool& carry, const UintTy& base_num) {
    assert(x < base_num && y < base_num);
    // assert(base_num < (all_one<UintTy>(sizeof(UintTy) * CHAR_BIT - 1) + 1));
    UintTy sum = x + y;
    carry = (sum >= base_num) || (sum < x);
    return (carry) ? (sum - base_num) : (sum % base_num);
}

// carry = (x + y + carry) / base_num, return (x + y + carry) mod base_num
template <typename UintTy>
constexpr UintTy add_carry_base(UintTy x, UintTy y, bool& carry, const UintTy& base_num) {
    assert(x < base_num && y < base_num);
    // assert(base_num < (all_one<UintTy>(sizeof(UintTy) * CHAR_BIT - 1) + 1));
    UintTy sum = x + carry;
    carry = (sum < x);
    sum += y;                                         // carry
    carry = carry || (sum >= base_num) || (sum < y);  // carry
    return (carry) ? (sum - base_num) : (sum % base_num);
}

// x - y - bf = diff with borrow
template <typename UintTy>
constexpr UintTy sub_borrow(UintTy x, UintTy y, bool& bf) {
    UintTy diff = x - bf;
    bf = (diff > x);
    y = diff - y;           // borrow
    bf = bf || (y > diff);  // borrow
    return y;
}

// a * x + b * y = gcd(a,b)
template <typename IntTy>
constexpr IntTy exgcd(IntTy a, IntTy b, IntTy& x, IntTy& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    IntTy k = a / b;
    IntTy g = exgcd(b, a - k * b, y, x);
    y -= k * x;
    return g;
}

// return n^-1 mod mod
template <typename IntTy>
constexpr IntTy mod_inv(IntTy n, IntTy mod) {
    n %= mod;
    IntTy x = 0, y = 0;
    exgcd(n, mod, x, y);
    if (x < 0) {
        x += mod;
    } else if (x >= mod) {
        x -= mod;
    }
    return x;
}

// return n^-1 mod 2^pow, Newton iteration
constexpr uint64_t inv_mod2pow(uint64_t n, int pow) {
    const uint64_t mask = all_one<uint64_t>(pow);
    uint64_t xn = 1, t = n & mask;
    while (t != 1) {
        xn = (xn * (2 - t));
        t = (xn * n) & mask;
    }
    return xn & mask;
}

// Compute Integer multiplication, 64bit x 64bit to 128bit, basic algorithm
// first is low 64bit, second is high 64bit
constexpr void mul64x64to128_base(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
    /*
    //// 一种更易于读懂的实现，但由于其存在两个分支预测，其性能略低于现行算法。
        const uint64_t al = a >> 32,
                    ar = uint32_t(a),
                    bl = b >> 32,
                    br = uint32_t(b);
        low  = ar * br;
        high = al * bl;

        const uint64_t t1 = al * br,
                    t2 = ar * bl,
                    t1addt2 = t1 + t2;

        if (t1addt2 < t2)high += 1ULL << 32;//如果溢出，即进位那么加上一

        low += t1addt2 << 32;
        if (low < (t1addt2 << 32))high += 1;//作用同上

        high += t1addt2 >> 32;
    */
    uint64_t ah = a >> 32, bh = b >> 32;
    a = uint32_t(a), b = uint32_t(b);
    uint64_t r0 = a * b, r1 = a * bh, r2 = ah * b, r3 = ah * bh;
    r3 += (r1 >> 32) + (r2 >> 32);
    r1 = uint32_t(r1), r2 = uint32_t(r2);
    r1 += r2;
    r1 += (r0 >> 32);
    high = r3 + (r1 >> 32);
    low = (r1 << 32) | uint32_t(r0);
}

static void mul64x64to128(uint64_t a, uint64_t b, uint64_t& low, uint64_t& high) {
#if defined(__x86_64__) || defined(_M_X64)
// x86-64架构：优先内联汇编（GCC/Clang/MSVC均支持）
// mul指令：将RAX与操作数相乘，结果存于RDX:RAX（高64位:低64位）
#if defined(_LAMMP_MSVC)
    // MSVC的内联汇编语法（与GCC略有不同）
    __asm {
            mov rax, a;  // RAX = a
            mul qword ptr b;  // RDX:RAX = a * b
            mov low, rax;  // 低64位存入low
            mov high, rdx;  // 高64位存入high
    }
#elif defined(_LAMMP_GCC)
    // GCC/Clang的内联汇编语法
    __asm__("mul %[b]"  // 执行 RDX:RAX = RAX * b（a已在RAX中）
            : "=a"(low),
              "=d"(high)          // 输出：low = RAX（低64位），high = RDX（高64位）
            : "a"(a), [b] "r"(b)  // 输入：a存入RAX，b存入任意寄存器
            :                     // 无额外寄存器被修改
    );
#endif

#elif defined(__aarch64__)
    // ARM64架构：优先内联汇编（GCC/Clang支持）
    // umull指令：无符号64x64乘法，结果存于两个64位寄存器
    __asm__("umull %[low], %[high], %[a], %[b]"   // low:high = a * b
            : [low] "=r"(low), [high] "=r"(high)  // 输出：low和high
            : [a] "r"(a), [b] "r"(b)              // 输入：a和b（任意寄存器）
            :                                     // 无寄存器污染
    );

#else
// 其他架构：优先使用编译器原生128位支持，再fallback到基础实现
#if defined(UINT128T)
    __uint128_t res = static_cast<__uint128_t>(a) * b;
    low = static_cast<uint64_t>(res);
    high = static_cast<uint64_t>(res >> 64);
#elif defined(_LAMMP_MSVC)
    // MSVC在非x86_64架构（如ARM64）可使用_umul128
    unsigned long long hi;
    low = _umul128(a, b, &hi);
    high = hi;
#else
    mul64x64to128_base(a, b, low, high);
#endif
#endif
}

constexpr uint32_t div128by32_base(uint64_t& dividend_hi64, uint64_t& dividend_lo64, uint32_t divisor) {
    uint32_t quot_hi32 = 0, quot_lo32 = 0;
    uint64_t dividend = dividend_hi64 >> 32;
    quot_hi32 = dividend / divisor;
    dividend %= divisor;

    dividend = (dividend << 32) | uint32_t(dividend_hi64);
    quot_lo32 = dividend / divisor;
    dividend %= divisor;
    dividend_hi64 = (uint64_t(quot_hi32) << 32) | quot_lo32;

    dividend = (dividend << 32) | uint32_t(dividend_lo64 >> 32);
    quot_hi32 = dividend / divisor;
    dividend %= divisor;

    dividend = (dividend << 32) | uint32_t(dividend_lo64);
    quot_lo32 = dividend / divisor;
    dividend %= divisor;
    dividend_lo64 = (uint64_t(quot_hi32) << 32) | quot_lo32;
    return dividend;
}

// 96bit integer divided by 64bit integer, input make sure the quotient smaller
// than 2^32.
constexpr uint32_t div96by64to32_base(uint32_t dividend_hi32, uint64_t& dividend_lo64, uint64_t divisor) {
    if (0 == dividend_hi32) {
        uint32_t quotient = dividend_lo64 / divisor;
        dividend_lo64 %= divisor;
        return quotient;
    }
    uint64_t divid2 = (uint64_t(dividend_hi32) << 32) | (dividend_lo64 >> 32);
    uint64_t divis1 = divisor >> 32;
    divisor = uint32_t(divisor);
    uint64_t qhat = divid2 / divis1;
    divid2 %= divis1;
    divid2 = (divid2 << 32) | uint32_t(dividend_lo64);
    uint64_t product = qhat * divisor;
    divis1 <<= 32;
    if (product > divid2) {
        qhat--;
        product -= divisor;
        divid2 += divis1;
        // if divid2 <= divis1, the addtion of divid2 is overflow, so product
        // must not be larger than divid2.
        if ((divid2 > divis1) && (product > divid2)) {
            qhat--;
            product -= divisor;
            divid2 += divis1;
        }
    }
    divid2 -= product;
    dividend_lo64 = divid2;
    return uint32_t(qhat);
}

// 128bit integer divided by 64bit integer, input make sure the quotient smaller
// than 2^64.
constexpr uint64_t div128by64to64_base(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor) {
    int k = 0;
    if (divisor < (uint64_t(1) << 63)) {
        k = lammp_clz(divisor);
        divisor <<= k;  // Normalization.
        dividend_hi64 = (dividend_hi64 << k) | (dividend_lo64 >> (64 - k));
        dividend_lo64 <<= k;
    }
    uint32_t divid_hi32 = dividend_hi64 >> 32;
    uint64_t divid_lo64 = (dividend_hi64 << 32) | (dividend_lo64 >> 32);
    uint64_t quotient = div96by64to32_base(divid_hi32, divid_lo64, divisor);

    divid_hi32 = divid_lo64 >> 32;
    dividend_lo64 = uint32_t(dividend_lo64) | (divid_lo64 << 32);
    quotient = (quotient << 32) | div96by64to32_base(divid_hi32, dividend_lo64, divisor);
    dividend_lo64 >>= k;
    return quotient;
}

// 128位无符号数除以64位无符号数（调用方式与div128by64to64_base一致）
// 输入：dividend_hi64（被除数高64位）、dividend_lo64（被除数低64位，引用）、divisor（除数）
// 输出：返回64位商（要求商 < 2^64，由调用者保证），通过dividend_lo64更新为余数
static inline uint64_t div128by64to64(uint64_t dividend_hi64, uint64_t& dividend_lo64, uint64_t divisor) {
    // assert(divisor != 0 && "Division by zero in div128by64to64");
#if defined(__x86_64__) && !defined(_LAMMP_MSVC)
    // x86-64架构（GCC/Clang）：内联汇编实现，编译期不支持汇编
    uint64_t quotient_low, remainder;
    __asm__("div %[divisor]"                                                  // RDX:RAX ÷ 除数 → RAX=商, RDX=余数
            : "=a"(quotient_low), "=d"(remainder)                             // 输出：商=RAX，余数=RDX
            : "a"(dividend_lo64), "d"(dividend_hi64), [divisor] "r"(divisor)  // 输入：RAX=低64位,
                                                                              // RDX=高64位
            :                                                                 // 无寄存器污染
    );
    dividend_lo64 = remainder;  // 更新余数到被除数低位引用
    return quotient_low;

#elif defined(_M_X64) && defined(_LAMMP_MSVC)
    // x86-64架构（MSVC）：内联汇编，编译期不支持
    uint64_t quotient_low, remainder;
    __asm {
            mov rax, dividend_lo64;  // RAX = 被除数低64位
            mov rdx, dividend_hi64;  // RDX = 被除数高64位 → RDX:RAX为128位被除数
            div qword ptr divisor;  // 除法：RAX=商，RDX=余数
            mov quotient_low, rax;  // 保存商
            mov remainder, rdx;  // 保存余数
    }
    dividend_lo64 = remainder;
    return quotient_low;
#else
// 非x86-64架构：优先编译器128位支持
#if defined(UMUL128)
    __uint128_t dividend = static_cast<__uint128_t>(dividend_hi64) << 64 | dividend_lo64;
    __uint128_t div = static_cast<__uint128_t>(divisor);
    uint64_t quotient = static_cast<uint64_t>(dividend / div);
    dividend_lo64 = static_cast<uint64_t>(dividend % div);  // 更新余数
    return quotient;

#elif defined(_LAMMP_GCC)
    // MSVC（非x86-64）：128位除法，编译期支持有限
    unsigned __int128 dividend = static_cast<unsigned __int128>(dividend_hi64) << 64 | dividend_lo64;
    unsigned __int128 quotient = dividend / divisor;
    dividend_lo64 = static_cast<uint64_t>(dividend % divisor);
    return static_cast<uint64_t>(quotient);

#else
    return div128by64to64to64_base(dividend_hi64, dividend_lo64, divisor);
#endif
#endif
}

// uint64_t to std::string
inline std::string ui64to_string_base10(uint64_t input, uint8_t digits) {
    std::string result(digits, '0');
    for (uint8_t i = 0; i < digits; i++) {
        result[digits - i - 1] = static_cast<char>(input % 10 + '0');
        input /= 10;
    }
    return result;
}

// 计算 128位 × 128位 → 256位结果
// a = (a_high << 64) | a_low
// b = (b_high << 64) | b_low
// 结果通过 res[0]（最低64位）到 res[3]（最高64位）输出
static inline void umul128_to_256(uint64_t a_high, uint64_t a_low, uint64_t b_high, uint64_t b_low, uint64_t res[4]) {
    // 中间变量：存储4个64×64乘法的结果（高64位+低64位）
    // uint64_t p0_low, p0_high;  // p0 = a_low × b_low
    uint64_t p1_low, p1_high;  // p1 = a_low × b_high
    uint64_t p2_low, p2_high;  // p2 = a_high × b_low
    // uint64_t p3_low, p3_high;  // p3 = a_high × b_high
    mul64x64to128(a_low, b_low, res[0], res[1]);
    mul64x64to128(a_low, b_high, p1_low, p1_high);
    mul64x64to128(a_high, b_low, p2_low, p2_high);
    mul64x64to128(a_high, b_high, res[2], res[3]);
    /*
        | res0 | res1 | res2 | res3 |
        |  p0l |  p0h |      |      |
               |  p1l |  p1h |      |
               |  p2l |  p2h |      |
               |      |  p3l |  p3h |
    */
    res[1] += p1_low;
    uint64_t carry = (res[1] < p1_low) ? 1 : 0;
    res[1] += p2_low;
    carry += (res[1] < p2_low) ? 1 : 0;

    res[2] += carry;
    carry = (res[2] < carry) ? 1 : 0;
    res[2] += p1_high;
    carry += (res[2] < p1_high) ? 1 : 0;
    res[2] += p2_high;
    carry += (res[2] < p2_high) ? 1 : 0;

    res[3] += carry;
}

/*
 *=======================================================================
 * 快速数论变换
 *=======================================================================
 */
namespace Transform {
namespace number_theory {

constexpr uint64_t MOD0 = 2485986994308513793ull, ROOT0 = 5ull;
constexpr uint64_t MOD1 = 1945555039024054273ull, ROOT1 = 5ull;
constexpr uint64_t MOD2 = 4179340454199820289ull, ROOT2 = 3ull;
constexpr uint64_t MOD3 = 754974721ull, ROOT3 = 11ull;
constexpr uint64_t MOD4 = 469762049ull, ROOT4 = 3ull;
constexpr uint64_t MOD5 = 3489660929ull, ROOT5 = 3ull;
constexpr uint64_t MOD6 = 3221225473ull, ROOT6 = 5ull;

/*
 *=======================================================================
 * 128位无符号整数类（并非完整功能，请勿轻易代替__uint128_t）
 *=======================================================================
 */
class _uint128 {
   private:
    uint64_t low, high;

   public:
    constexpr _uint128(uint64_t l = 0, uint64_t h = 0) : low(l), high(h) {}
    constexpr _uint128(std::pair<uint64_t, uint64_t> p) : low(p.first), high(p.second) {}

    constexpr _uint128 operator+(_uint128 rhs) const {
        rhs.low += low;
        rhs.high += high + (rhs.low < low);
        return rhs;
    }
    constexpr _uint128 operator-(_uint128 rhs) const {
        rhs.low = low - rhs.low;
        rhs.high = high - rhs.high - (rhs.low > low);
        return rhs;
    }
    constexpr _uint128 operator+(uint64_t rhs) const {
        rhs = low + rhs;
        return _uint128(rhs, high + (rhs < low));
    }
    constexpr _uint128 operator-(uint64_t rhs) const {
        rhs = low - rhs;
        return _uint128(rhs, high - (rhs > low));
    }
    // Only compute the low * rhs.low
    _uint128 operator*(_uint128 rhs) const {
        _uint128 res;
        mul64x64to128(low, rhs.low, res.low, res.high);
        return res;
    }
    // Only compute the low * rhs
    _uint128 operator*(uint64_t rhs) const {
        _uint128 res;
        mul64x64to128(low, rhs, res.low, res.high);
        return res;
    }
    // Only compute the 128bit / 64 bit
    constexpr _uint128 operator/(_uint128 rhs) const { return *this / rhs.low; }
    // Only compute the 128bit % 64 bit
    constexpr _uint128 operator%(const _uint128& rhs) const { return *this % rhs.low; }
    // Only compute the 128bit / 64 bit
    constexpr _uint128 operator/(uint64_t rhs) const {
        _uint128 quot = *this;
        quot.selfDivRem(rhs);
        return quot;
    }
    // Only compute the 128bit % 64 bit
    constexpr _uint128 operator%(uint64_t rhs) const {
        _uint128 quot = *this;
        uint64_t rem = quot.selfDivRem(rhs);
        return _uint128(rem);
    }
    constexpr _uint128& operator+=(const _uint128& rhs) { return *this = *this + rhs; }
    constexpr _uint128& operator-=(const _uint128& rhs) { return *this = *this - rhs; }
    constexpr _uint128& operator+=(uint64_t rhs) { return *this = *this + rhs; }
    constexpr _uint128& operator-=(uint64_t rhs) { return *this = *this - rhs; }
    // Only compute the low * rhs.low
    constexpr _uint128& operator*=(const _uint128& rhs) {
        mul64x64to128_base(low, rhs.low, low, high);
        return *this;
    }
    constexpr _uint128& operator/=(const _uint128& rhs) { return *this = *this / rhs; }
    constexpr _uint128& operator%=(const _uint128& rhs) { return *this = *this % rhs; }
    // Return *this % divisor, *this /= divisor
    constexpr uint64_t selfDivRem(uint64_t divisor) {
        if ((divisor >> 32) == 0) {
            return div128by32_base(high, low, uint32_t(divisor));
        }
        uint64_t divid1 = high % divisor, divid0 = low;
        high /= divisor;
        low = div128by64to64_base(divid1, divid0, divisor);
        return divid0;
    }
    uint64_t self_div_rem(uint64_t divisor) {
        assert((divisor >> 32) > 0);
        uint64_t divid1 = high % divisor, divid0 = low;
        high /= divisor;
        low = div128by64to64(divid1, divid0, divisor);
        return divid0;
    }
    uint32_t self_div_rem(uint32_t divisor) { return div128by32_base(high, low, uint32_t(divisor)); }
    static constexpr _uint128 mul64x64(uint64_t a, uint64_t b) {
        _uint128 res;
        mul64x64to128_base(a, b, res.low, res.high);
        return res;
    }
    static _uint128 mul64x64_fast(uint64_t a, uint64_t b) {
        _uint128 res;
        mul64x64to128(a, b, res.low, res.high);
        return res;
    }
    constexpr bool operator<(const _uint128& rhs) const {
        if (high != rhs.high) {
            return high < rhs.high;
        }
        return low < rhs.low;
    }
    constexpr bool operator==(const _uint128& rhs) const { return high == rhs.high && low == rhs.low; }
    constexpr _uint128 operator<<(int shift) const {
        if (shift == 0) {
            return *this;
        }
        shift %= 128;
        shift = shift < 0 ? shift + 128 : shift;
        if (shift < 64) {
            return _uint128(low << shift, (high << shift) | (low >> (64 - shift)));
        }
        return _uint128(0, low << (shift - 64));
    }
    constexpr _uint128 operator>>(int shift) const {
        if (shift == 0) {
            return *this;
        }
        shift %= 128;
        shift = shift < 0 ? shift + 128 : shift;
        if (shift < 64) {
            return _uint128((low >> shift) | (high << (64 - shift)), high >> shift);
        }
        return _uint128(high >> (shift - 64), 0);
    }
    constexpr _uint128& operator<<=(int shift) { return *this = *this << shift; }
    constexpr _uint128& operator>>=(int shift) { return *this = *this >> shift; }
    constexpr uint64_t high64() const { return high; }
    constexpr uint64_t low64() const { return low; }
    constexpr operator uint64_t() const { return low64(); }
    std::string toStringBase10() const {
        if (high == 0) {
            return std::to_string(low);
        }
        constexpr uint64_t BASE(10000'0000'0000'0000);
        _uint128 copy(*this);
        std::string s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        return std::to_string(uint64_t(copy.selfDivRem(BASE))) + s;
    }
    void printDec() const { std::cout << std::dec << toStringBase10() << '\n'; }
    void printHex() const { std::cout << std::hex << "0x" << high << ' ' << low << std::dec << '\n'; }
};  // _uint128

/*
 *=======================================================================
 * 192位无符号整数类
 *=======================================================================
 */
class _uint192 {
    friend _uint128;

   private:
    uint64_t low, mid, high;

   public:
    constexpr _uint192() : low(0), mid(0), high(0) {}
    constexpr _uint192(uint64_t low, uint64_t mi = 0, uint64_t high = 0) : low(low), mid(mi), high(high) {}
    constexpr _uint192(_uint128 n) : low(n.low64()), mid(n.high64()), high(0) {}
    constexpr _uint192 operator+(_uint192 rhs) const {
        bool cf = false;
        rhs.low = add_half(low, rhs.low, cf);
        rhs.mid = add_carry(mid, rhs.mid, cf);
        rhs.high = high + rhs.high + cf;
        return rhs;
    }
    constexpr _uint192 operator-(_uint192 rhs) const {
        bool bf = false;
        rhs.low = sub_half(low, rhs.low, bf);
        rhs.mid = sub_borrow(mid, rhs.mid, bf);
        rhs.high = high - rhs.high - bf;
        return rhs;
    }
    constexpr _uint192 operator/(uint64_t rhs) const {
        _uint192 result(*this);
        result.selfDivRem(rhs);
        return result;
    }
    constexpr _uint192 operator%(uint64_t rhs) const {
        _uint192 result(*this);
        return result.selfDivRem(rhs);
    }
    constexpr _uint192& operator+=(const _uint192& rhs) { return *this = *this + rhs; }
    constexpr _uint192& operator-=(const _uint192& rhs) { return *this = *this - rhs; }
    constexpr _uint192& operator/=(const _uint192& rhs) { return *this = *this / rhs; }
    constexpr _uint192& operator%=(const _uint192& rhs) { return *this = *this % rhs; }
    constexpr _uint192 operator<<(int shift) const {
        if (shift == 0) {
            return *this;
        }
        shift %= 192;
        shift = shift < 0 ? shift + 192 : shift;
        if (shift < 64) {
            return _uint192(low << shift, (mid << shift) | (low >> (64 - shift)),
                            (high << shift) | (mid >> (64 - shift)));
        } else if (shift < 128) {
            shift -= 64;
            return _uint192(0, low << shift, (mid << shift) | (low >> (64 - shift)));
        }
        return _uint192(0, 0, low << (shift - 128));
    }
    friend constexpr bool operator<(const _uint192& lhs, const _uint192& rhs) {
        if (lhs.high != rhs.high) {
            return lhs.high < rhs.high;
        }
        if (lhs.mid != rhs.mid) {
            return lhs.mid < rhs.mid;
        }
        return lhs.low < rhs.low;
    }
    friend constexpr bool operator<=(const _uint192& lhs, const _uint192& rhs) { return !(rhs > lhs); }
    friend constexpr bool operator>(const _uint192& lhs, const _uint128& rhs) { return rhs < lhs; }
    friend constexpr bool operator>=(const _uint192& lhs, const _uint192& rhs) { return !(lhs < rhs); }
    friend constexpr bool operator==(const _uint192& lhs, const _uint192& rhs) {
        return lhs.low == rhs.low && lhs.mid == rhs.mid && lhs.high == rhs.high;
    }
    friend constexpr bool operator!=(const _uint192& lhs, const _uint192& rhs) { return !(lhs == rhs); }

    static constexpr _uint192 mul128x64(_uint128 a, uint64_t b) {
        auto prod1 = _uint128::mul64x64(b, a.low64());
        auto prod2 = _uint128::mul64x64(b, a.high64());
        _uint192 result;
        result.low = prod1.low64();
        result.mid = prod1.high64() + prod2.low64();
        result.high = prod2.high64() + (result.mid < prod1.high64());
        return result;
    }
    static constexpr _uint192 mul64x64x64(uint64_t a, uint64_t b, uint64_t c) {
        return mul128x64(_uint128::mul64x64(a, b), c);
    }

    static _uint192 mul128x64_fast(_uint128 a, uint64_t b) {
        _uint192 res;
        mul64x64to128(a.low64(), b, res.low, res.mid);
        uint64_t tmp;
        mul64x64to128(a.high64(), b, tmp, res.high);
        res.mid += tmp;
        res.high += (res.mid < tmp) ? 1 : 0;
        return res;
    }
    static _uint192 mul64x64x64_fast(uint64_t a, uint64_t b, uint64_t c) {
        // 默认实现（保持原逻辑）
        uint64_t p1, p2, p3;
        _uint192 res;
        mul64x64to128(a, b, p1, p2);
        mul64x64to128(p1, c, res.low, res.mid);
        mul64x64to128(p2, c, p1, p3);
        res.mid += p1;
        res.high = p3 + (res.mid < p1);
        return res;
    }
    // 编译期计算，返回余数
    constexpr uint64_t selfDivRem(uint64_t divisor) {
        uint64_t divid1 = high % divisor, divid0 = mid;
        high /= divisor;
        mid = div128by64to64_base(divid1, divid0, divisor);
        divid1 = divid0, divid0 = low;
        low = div128by64to64_base(divid1, divid0, divisor);
        return divid0;
    }
    // 运行期计算（同时更快，返回余数）
    uint64_t self_div_rem(uint64_t divisor) {
        uint64_t divid1 = high % divisor, divid0 = mid;
        high /= divisor;
        mid = div128by64to64(divid1, divid0, divisor);
        divid1 = divid0, divid0 = low;
        low = div128by64to64(divid1, divid0, divisor);
        return divid0;
    }
    constexpr _uint192 rShift64() const { return _uint192(mid, high, 0); }
    constexpr _uint192 lShift64() const { return _uint192(0, low, high); }
    constexpr void set_low(uint64_t low) { this->low = low; }
    constexpr void set_mid(uint64_t mid) { this->mid = mid; }
    constexpr void set_high(uint64_t high) { this->high = high; }

    constexpr operator uint64_t() const { return low; }

    std::string toStringBase10() const {
        if (high == 0) {
            return _uint128(mid, low).toStringBase10();
        }
        constexpr uint64_t BASE(10000'0000'0000'0000);
        _uint192 copy(*this);
        std::string s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        s = ui64to_string_base10(uint64_t(copy.selfDivRem(BASE)), 16) + s;
        return std::to_string(uint64_t(copy.selfDivRem(BASE))) + s;
    }
    constexpr uint64_t low64() const { return low; }
    constexpr uint64_t mid64() const { return mid; }
    constexpr uint64_t high64() const { return high; }

    void printDec() const { std::cout << std::dec << toStringBase10() << '\n'; }
    void printHex() const { std::cout << std::hex << "0x" << high << ' ' << mid << ' ' << low << std::dec << '\n'; }
};

template <typename Int128Type>
constexpr uint64_t high64(const Int128Type& n) {
    return n >> 64;
}
constexpr uint64_t high64(const _uint128& n) { return n.high64(); }

#ifdef UINT128T
using uint128_default = __uint128_t;
#else
using uint128_default = _uint128;
#endif  // UINT128T

/*
 * ============================================================
 * 使用蒙哥马利域计算，消除模乘
 * 计算的编译期常量模数不可以超过 2^62，同时必须大于 2^32
 * ============================================================
 */
template <uint64_t MOD, typename Int128Type = _uint128>
class MontInt64Lazy {
   private:
    static_assert(MOD > UINT32_MAX, "Montgomery64 modulus must be greater than 2^32");
    static_assert(lammp_log2(MOD) < 62, "MOD can't be larger than 62 bits");
    uint64_t data;

   public:
    using IntType = uint64_t;

    constexpr MontInt64Lazy() : data(0) {}
    constexpr MontInt64Lazy(uint64_t n) : data(mulMontCompileTime(n, rSquare())) {}

    constexpr MontInt64Lazy operator+(MontInt64Lazy rhs) const {
        rhs.data = data + rhs.data;
        rhs.data = rhs.data < mod2() ? rhs.data : rhs.data - mod2();
        return rhs;
    }
    constexpr MontInt64Lazy operator-(MontInt64Lazy rhs) const {
        rhs.data = data - rhs.data;
        rhs.data = rhs.data > data ? rhs.data + mod2() : rhs.data;
        return rhs;
    }
    MontInt64Lazy operator*(MontInt64Lazy rhs) const {
        rhs.data = mulMontRunTimeLazy(data, rhs.data);
        return rhs;
    }
    constexpr MontInt64Lazy& operator+=(const MontInt64Lazy& rhs) { return *this = *this + rhs; }
    constexpr MontInt64Lazy& operator-=(const MontInt64Lazy& rhs) { return *this = *this - rhs; }
    constexpr MontInt64Lazy& operator*=(const MontInt64Lazy& rhs) {
        data = mulMontCompileTime(data, rhs.data);
        return *this;
    }
    constexpr MontInt64Lazy largeNorm2() const {
        MontInt64Lazy res;
        res.data = data >= mod2() ? data - mod2() : data;
        return res;
    }
    constexpr MontInt64Lazy rawAdd(MontInt64Lazy rhs) const {
        rhs.data = data + rhs.data;
        return rhs;
    }
    constexpr MontInt64Lazy rawSub(MontInt64Lazy rhs) const {
        rhs.data = data - rhs.data + mod2();
        return rhs;
    }
    constexpr operator uint64_t() const { return toInt(data); }

    static constexpr uint64_t mod() { return MOD; }
    static constexpr uint64_t mod2() { return MOD * 2; }
    static constexpr uint64_t modInv() {
        constexpr uint64_t mod_inv = inv_mod2pow(mod(), 64);  //(mod_inv * mod)%(2^64) = 1
        return mod_inv;
    }
    static constexpr uint64_t modInvNeg() {
        constexpr uint64_t mod_inv_neg = uint64_t(0 - modInv());  //(mod_inv_neg + mod_inv)%(2^64) = 0
        return mod_inv_neg;
    }
    static constexpr uint64_t rSquare() {
        constexpr Int128Type r = (Int128Type(1) << 64) % Int128Type(mod());  // R % mod
        constexpr uint64_t r2 = uint64_t(qpow(r, 2, Int128Type(mod())));     // R^2 % mod
        return r2;
    }
    static_assert((mod() * modInv()) == 1, "mod_inv not correct");

    static constexpr uint64_t toMont(uint64_t n) { return mulMontCompileTime(n, rSquare()); }
    static constexpr uint64_t toInt(uint64_t n) { return redc(Int128Type(n)); }

    static uint64_t redcFastLazy(const Int128Type& input) {
        Int128Type n = uint64_t(input) * modInvNeg();
        n = n * mod();
        n += input;
        return high64(n);
    }
    static uint64_t redcFast(const Int128Type& input) {
        uint64_t n = redcFastLazy(input);
        return n < mod() ? n : n - mod();
    }
    static constexpr uint64_t redc(const Int128Type& input) {
        Int128Type n = uint64_t(input) * modInvNeg();
        n *= Int128Type(mod());
        n += input;
        uint64_t m = high64(n);
        return m < mod() ? m : m - mod();
    }
    static uint64_t mulMontRunTime(uint64_t a, uint64_t b) { return redcFast(Int128Type(a) * b); }
    static uint64_t mulMontRunTimeLazy(uint64_t a, uint64_t b) { return redcFastLazy(Int128Type(a) * b); }
    static constexpr uint64_t mulMontCompileTime(uint64_t a, uint64_t b) {
        Int128Type prod(a);
        prod *= Int128Type(b);
        return redc(prod);
    }
};  // class MontInt64Lazy

template <typename IntType>
constexpr bool check_inv(uint64_t n, uint64_t n_inv, uint64_t mod) {
    n %= mod;
    n_inv %= mod;
    IntType m(n);
    m *= IntType(n_inv);
    m %= IntType(mod);
    return m == IntType(1);
}

// 3 modulars Chinese Remainder Theorem (CRT) with 192 bit result.
template <typename ModInt1, typename ModInt2, typename ModInt3>
inline _uint192 crt3(ModInt1 n1, ModInt2 n2, ModInt3 n3) {
    constexpr uint64_t MOD1 = ModInt1::mod(), MOD2 = ModInt2::mod(), MOD3 = ModInt3::mod();
    constexpr _uint192 MOD123 = _uint192::mul64x64x64(MOD1, MOD2, MOD3);  // MOD1*MOD2*MOD3
    constexpr _uint128 MOD12 = _uint128::mul64x64(MOD1, MOD2);            // MOD1*MOD2
    constexpr _uint128 MOD23 = _uint128::mul64x64(MOD2, MOD3);            // MOD2*MOD3
    constexpr _uint128 MOD13 = _uint128::mul64x64(MOD1, MOD3);            // MOD1*MOD3
    constexpr uint64_t MOD23_M1 =
        _uint128::mul64x64(MOD2 % MOD1, MOD3 % MOD1) % _uint128(MOD1);  // (MOD2*MOD3)  mod MOD1
    constexpr uint64_t MOD13_M2 =
        _uint128::mul64x64(MOD1 % MOD2, MOD3 % MOD2) % _uint128(MOD2);  // (MOD1*MOD3)  mod MOD2
    constexpr uint64_t MOD12_M3 =
        _uint128::mul64x64(MOD1 % MOD3, MOD2 % MOD3) % _uint128(MOD3);  // (MOD1*MOD2)  mod MOD3
    constexpr ModInt1 MOD23_INV1 = mod_inv<int64_t>(MOD23_M1, MOD1);    // (MOD2*MOD3)^-1 mod MOD1
    constexpr ModInt2 MOD13_INV2 = mod_inv<int64_t>(MOD13_M2, MOD2);    // (MOD1*MOD3)^-1 mod MOD2
    constexpr ModInt3 MOD12_INV3 = mod_inv<int64_t>(MOD12_M3, MOD3);    // (MOD1*MOD2)^-1 mod MOD3
    static_assert(check_inv<_uint128>(MOD23_INV1, MOD23_M1, MOD1), "INV1 error");
    static_assert(check_inv<_uint128>(MOD13_INV2, MOD13_M2, MOD2), "INV2 error");
    static_assert(check_inv<_uint128>(MOD12_INV3, MOD12_M3, MOD3), "INV3 error");
    n1 = n1 * MOD23_INV1;
    n2 = n2 * MOD13_INV2;
    n3 = n3 * MOD12_INV3;
    _uint192 result = _uint192::mul128x64_fast(MOD23, uint64_t(n1));
    result += _uint192::mul128x64_fast(MOD13, uint64_t(n2));
    result += _uint192::mul128x64_fast(MOD12, uint64_t(n3));
    result = result < MOD123 ? result : result - MOD123;
    return result < MOD123 ? result : result - MOD123;
}

/*
 * =====================================================================
 *  Split-Radix NTT  模板
 * =====================================================================
 */
namespace SplitRadix {

template <typename T>
inline void transform2(T& sum, T& diff) {
    T temp0 = sum, temp1 = diff;
    sum = temp0 + temp1;
    diff = temp0 - temp1;
}

template <uint64_t ROOT, typename ModIntType>
inline ModIntType mul_w41(ModIntType n) {
    constexpr ModIntType W_4_1 = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / 4);
    return n * W_4_1;
}

// in: in_out0<4p, in_ou1<4p; in_out2<2p, in_ou3<2p
// out: in_out0<4p, in_ou1<4p; in_out2<4p, in_ou3<4p
template <uint64_t ROOT, typename ModIntType>
inline void dit_butterfly244(ModIntType& in_out0, ModIntType& in_out1, ModIntType& in_out2, ModIntType& in_out3) {
    ModIntType temp0, temp1, temp2, temp3;
    temp0 = in_out0.largeNorm2();
    temp1 = in_out1.largeNorm2();
    temp2 = in_out2 + in_out3;
    temp3 = in_out2.rawSub(in_out3);
    temp3 = mul_w41<ROOT>(temp3);
    in_out0 = temp0.rawAdd(temp2);
    in_out2 = temp0.rawSub(temp2);
    in_out1 = temp1.rawAdd(temp3);
    in_out3 = temp1.rawSub(temp3);
}

// in: in_out0<2p, in_ou1<2p; in_out2<2p, in_ou3<2p
// out: in_out0<2p, in_ou1<2p; in_out2<4p, in_ou3<4p
template <uint64_t ROOT, typename ModIntType>
inline void dif_butterfly244(ModIntType& in_out0, ModIntType& in_out1, ModIntType& in_out2, ModIntType& in_out3) {
    ModIntType temp0, temp1, temp2, temp3;
    temp0 = in_out0.rawAdd(in_out2);
    temp2 = in_out0 - in_out2;
    temp1 = in_out1.rawAdd(in_out3);
    temp3 = in_out1.rawSub(in_out3);
    temp3 = mul_w41<ROOT>(temp3);
    in_out0 = temp0.largeNorm2();
    in_out1 = temp1.largeNorm2();
    in_out2 = temp2.rawAdd(temp3);
    in_out3 = temp2.rawSub(temp3);
}

// in: in_out0<4p, in_ou1<4p
// out: in_out0<4p, in_ou1<4p
template <typename ModIntType>
inline void dit_butterfly2(ModIntType& in_out0, ModIntType& in_out1, const ModIntType& omega) {
    auto x = in_out0.largeNorm2();
    auto y = in_out1 * omega;
    in_out0 = x.rawAdd(y);
    in_out1 = x.rawSub(y);
}

// in: in_out0<2p, in_ou1<2p
// out: in_out0<2p, in_ou1<2p
template <typename ModIntType>
inline void dif_butterfly2(ModIntType& in_out0, ModIntType& in_out1, const ModIntType& omega) {
    auto x = in_out0 + in_out1;
    auto y = in_out0.rawSub(in_out1);
    in_out0 = x;
    in_out1 = y * omega;
}

template <size_t MAX_LEN, uint64_t ROOT, typename ModIntType>
struct NTTShort {
    static constexpr size_t NTT_LEN = MAX_LEN;
    static constexpr int LOG_LEN = lammp_log2(NTT_LEN);
    struct TableType {
        std::array<ModIntType, NTT_LEN> omega_table;
        // Compute in compile time if need.
        /*constexpr*/ TableType() {
            for (int omega_log_len = 0; omega_log_len <= LOG_LEN; omega_log_len++) {
                size_t omega_len = size_t(1) << omega_log_len, omega_count = omega_len / 2;
                auto it = &omega_table[omega_len / 2];
                ModIntType root = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / omega_len);
                ModIntType omega(1);
                for (size_t i = 0; i < omega_count; i++) {
                    it[i] = omega;
                    omega *= root;
                }
            }
        }
        constexpr ModIntType& operator[](size_t i) { return omega_table[i]; }
        constexpr const ModIntType& operator[](size_t i) const { return omega_table[i]; }
        constexpr const ModIntType* getOmegaIt(size_t len) const { return &omega_table[len / 2]; }
    };

    static TableType table;

    static void dit(ModIntType in_out[], size_t len) {
        len = std::min(NTT_LEN, len);
        size_t rank = len;
        if (lammp_log2(len) % 2 == 0) {
            NTTShort<4, ROOT, ModIntType>::dit(in_out, len);
            for (size_t i = 4; i < len; i += 4) {
                NTTShort<4, ROOT, ModIntType>::dit(in_out + i);
            }
            rank = 16;
        } else {
            NTTShort<8, ROOT, ModIntType>::dit(in_out, len);
            for (size_t i = 8; i < len; i += 8) {
                NTTShort<8, ROOT, ModIntType>::dit(in_out + i);
            }
            rank = 32;
        }
        for (; rank <= len; rank *= 4) {
            size_t gap = rank / 4;
            auto omega_it = table.getOmegaIt(rank), last_omega_it = table.getOmegaIt(rank / 2);
            auto it0 = in_out, it1 = in_out + gap, it2 = in_out + gap * 2, it3 = in_out + gap * 3;
            for (size_t j = 0; j < len; j += rank) {
                for (size_t i = 0; i < gap; i++) {
                    auto temp0 = it0[j + i], temp1 = it1[j + i], temp2 = it2[j + i], temp3 = it3[j + i],
                         omega = last_omega_it[i];
                    dit_butterfly2(temp0, temp1, omega);
                    dit_butterfly2(temp2, temp3, omega);
                    dit_butterfly2(temp0, temp2, omega_it[i]);
                    dit_butterfly2(temp1, temp3, omega_it[gap + i]);
                    it0[j + i] = temp0, it1[j + i] = temp1, it2[j + i] = temp2, it3[j + i] = temp3;
                }
            }
        }
    }
    static void dif(ModIntType in_out[], size_t len) {
        len = std::min(NTT_LEN, len);
        size_t rank = len;
        for (; rank >= 16; rank /= 4) {
            size_t gap = rank / 4;
            auto omega_it = table.getOmegaIt(rank), last_omega_it = table.getOmegaIt(rank / 2);
            auto it0 = in_out, it1 = in_out + gap, it2 = in_out + gap * 2, it3 = in_out + gap * 3;
            for (size_t j = 0; j < len; j += rank) {
                for (size_t i = 0; i < gap; i++) {
                    auto temp0 = it0[j + i], temp1 = it1[j + i], temp2 = it2[j + i], temp3 = it3[j + i],
                         omega = last_omega_it[i];
                    dif_butterfly2(temp0, temp2, omega_it[i]);
                    dif_butterfly2(temp1, temp3, omega_it[gap + i]);
                    dif_butterfly2(temp0, temp1, omega);
                    dif_butterfly2(temp2, temp3, omega);
                    it0[j + i] = temp0, it1[j + i] = temp1, it2[j + i] = temp2, it3[j + i] = temp3;
                }
            }
        }
        if (lammp_log2(rank) % 2 == 0) {
            NTTShort<4, ROOT, ModIntType>::dif(in_out, len);
            for (size_t i = 4; i < len; i += 4) {
                NTTShort<4, ROOT, ModIntType>::dif(in_out + i);
            }
        } else {
            NTTShort<8, ROOT, ModIntType>::dif(in_out, len);
            for (size_t i = 8; i < len; i += 8) {
                NTTShort<8, ROOT, ModIntType>::dif(in_out + i);
            }
        }
    }
};
template <size_t LEN, uint64_t ROOT, typename ModIntType>
typename NTTShort<LEN, ROOT, ModIntType>::TableType NTTShort<LEN, ROOT, ModIntType>::table;
template <size_t LEN, uint64_t ROOT, typename ModIntType>
constexpr size_t NTTShort<LEN, ROOT, ModIntType>::NTT_LEN;
template <size_t LEN, uint64_t ROOT, typename ModIntType>
constexpr int NTTShort<LEN, ROOT, ModIntType>::LOG_LEN;

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<0, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {}
    static void dif(ModIntType in_out[]) {}
    static void dit(ModIntType in_out[], size_t len) {}
    static void dif(ModIntType in_out[], size_t len) {}
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<1, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {}
    static void dif(ModIntType in_out[]) {}
    static void dit(ModIntType in_out[], size_t len) {}
    static void dif(ModIntType in_out[], size_t len) {}
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<2, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) { transform2(in_out[0], in_out[1]); }
    static void dif(ModIntType in_out[]) { transform2(in_out[0], in_out[1]); }
    static void dit(ModIntType in_out[], size_t len) {
        if (len < 2) {
            return;
        }
        dit(in_out);
    }
    static void dif(ModIntType in_out[], size_t len) {
        if (len < 2) {
            return;
        }
        dif(in_out);
    }
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<4, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];

        transform2(temp0, temp1);
        transform2(temp2, temp3);
        temp3 = mul_w41<ROOT>(temp3);

        in_out[0] = temp0 + temp2;
        in_out[1] = temp1 + temp3;
        in_out[2] = temp0 - temp2;
        in_out[3] = temp1 - temp3;
    }
    static void dif(ModIntType in_out[]) {
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];

        transform2(temp0, temp2);
        transform2(temp1, temp3);
        temp3 = mul_w41<ROOT>(temp3);

        in_out[0] = temp0 + temp1;
        in_out[1] = temp0 - temp1;
        in_out[2] = temp2 + temp3;
        in_out[3] = temp2 - temp3;
    }
    static void dit(ModIntType in_out[], size_t len) {
        if (len < 4) {
            NTTShort<2, ROOT, ModIntType>::dit(in_out, len);
            return;
        }
        dit(in_out);
    }
    static void dif(ModIntType in_out[], size_t len) {
        if (len < 4) {
            NTTShort<2, ROOT, ModIntType>::dif(in_out, len);
            return;
        }
        dif(in_out);
    }
};

template <uint64_t ROOT, typename ModIntType>
struct NTTShort<8, ROOT, ModIntType> {
    static void dit(ModIntType in_out[]) {
        static constexpr ModIntType w1 = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / 8);
        static constexpr ModIntType w2 = qpow(w1, 2);
        static constexpr ModIntType w3 = qpow(w1, 3);
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];
        auto temp4 = in_out[4];
        auto temp5 = in_out[5];
        auto temp6 = in_out[6];
        auto temp7 = in_out[7];

        transform2(temp0, temp1);
        transform2(temp2, temp3);
        transform2(temp4, temp5);
        transform2(temp6, temp7);
        temp3 = mul_w41<ROOT>(temp3);
        temp7 = mul_w41<ROOT>(temp7);

        transform2(temp0, temp2);
        transform2(temp1, temp3);
        transform2(temp4, temp6);
        transform2(temp5, temp7);
        temp5 = temp5 * w1;
        temp6 = temp6 * w2;
        temp7 = temp7 * w3;

        in_out[0] = temp0 + temp4;
        in_out[1] = temp1 + temp5;
        in_out[2] = temp2 + temp6;
        in_out[3] = temp3 + temp7;
        in_out[4] = temp0 - temp4;
        in_out[5] = temp1 - temp5;
        in_out[6] = temp2 - temp6;
        in_out[7] = temp3 - temp7;
    }
    static void dif(ModIntType in_out[]) {
        static constexpr ModIntType w1 = qpow(ModIntType(ROOT), (ModIntType::mod() - 1) / 8);
        static constexpr ModIntType w2 = qpow(w1, 2);
        static constexpr ModIntType w3 = qpow(w1, 3);
        auto temp0 = in_out[0];
        auto temp1 = in_out[1];
        auto temp2 = in_out[2];
        auto temp3 = in_out[3];
        auto temp4 = in_out[4];
        auto temp5 = in_out[5];
        auto temp6 = in_out[6];
        auto temp7 = in_out[7];

        transform2(temp0, temp4);
        transform2(temp1, temp5);
        transform2(temp2, temp6);
        transform2(temp3, temp7);
        temp5 = temp5 * w1;
        temp6 = temp6 * w2;
        temp7 = temp7 * w3;

        transform2(temp0, temp2);
        transform2(temp1, temp3);
        transform2(temp4, temp6);
        transform2(temp5, temp7);
        temp3 = mul_w41<ROOT>(temp3);
        temp7 = mul_w41<ROOT>(temp7);

        in_out[0] = temp0 + temp1;
        in_out[1] = temp0 - temp1;
        in_out[2] = temp2 + temp3;
        in_out[3] = temp2 - temp3;
        in_out[4] = temp4 + temp5;
        in_out[5] = temp4 - temp5;
        in_out[6] = temp6 + temp7;
        in_out[7] = temp6 - temp7;
    }
    static void dit(ModIntType in_out[], size_t len) {
        if (len < 8) {
            NTTShort<4, ROOT, ModIntType>::dit(in_out, len);
            return;
        }
        dit(in_out);
    }
    static void dif(ModIntType in_out[], size_t len) {
        if (len < 8) {
            NTTShort<4, ROOT, ModIntType>::dif(in_out, len);
            return;
        }
        dif(in_out);
    }
};

template <uint64_t MOD, uint64_t ROOT, typename Int128Type = uint128_default>
struct NTT {
    static constexpr uint64_t mod() { return MOD; }
    static constexpr uint64_t root() { return ROOT; }
    static constexpr uint64_t rootInv() {
        constexpr uint64_t IROOT = mod_inv<int64_t>(ROOT, MOD);
        return IROOT;
    }

    static_assert(root() < mod(), "ROOT must be smaller than MOD");
    static_assert(check_inv<Int128Type>(root(), rootInv(), mod()), "IROOT * ROOT % MOD must be 1");
    static constexpr int MOD_BITS = lammp_log2(mod()) + 1;
    static constexpr int MAX_LOG_LEN = lammp_ctz(mod() - 1);

    static constexpr size_t getMaxLen() {
        if constexpr (MAX_LOG_LEN < sizeof(size_t) * CHAR_BIT) {
            return size_t(1) << MAX_LOG_LEN;
        }
        return size_t(1) << (sizeof(size_t) * CHAR_BIT - 1);
    }
    static constexpr size_t NTT_MAX_LEN = getMaxLen();

    using INTT = NTT<mod(), rootInv(), Int128Type>;
    using ModInt64Type = MontInt64Lazy<MOD, Int128Type>;
    using ModIntType = ModInt64Type;
    using IntType = typename ModIntType::IntType;

    static constexpr size_t L2_BYTE = size_t(1) << 20;  // 1MB L2 cache size, change this
                                                        // if you know your cache size.
    static constexpr size_t LONG_THRESHOLD = std::min(L2_BYTE / sizeof(ModIntType), NTT_MAX_LEN);
    using NTTTemplate = NTTShort<LONG_THRESHOLD, root(), ModIntType>;

    static void dit244(ModIntType in_out[], size_t ntt_len) {
        ntt_len = std::min(int_floor2(ntt_len), NTT_MAX_LEN);
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dit(in_out, ntt_len);
            return;
        }
        size_t quarter_len = ntt_len / 4;
        dit244(in_out + quarter_len * 3, ntt_len / 4);
        dit244(in_out + quarter_len * 2, ntt_len / 4);
        dit244(in_out, ntt_len / 2);
        const ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        const ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        auto it0 = in_out, it1 = in_out + quarter_len, it2 = in_out + quarter_len * 2, it3 = in_out + quarter_len * 3;
        for (size_t i = 0; i < quarter_len; i++) {
            ModIntType temp0 = it0[i], temp1 = it1[i], temp2 = it2[i] * omega1, temp3 = it3[i] * omega3;
            dit_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
            it0[i] = temp0, it1[i] = temp1, it2[i] = temp2, it3[i] = temp3;
            omega1 = omega1 * unit_omega1;
            omega3 = omega3 * unit_omega3;
        }
    }
    static void dif244(ModIntType in_out[], size_t ntt_len) {
        ntt_len = std::min(int_floor2(ntt_len), NTT_MAX_LEN);
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dif(in_out, ntt_len);
            return;
        }
        size_t quarter_len = ntt_len / 4;
        const ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        const ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        auto it0 = in_out, it1 = in_out + quarter_len, it2 = in_out + quarter_len * 2, it3 = in_out + quarter_len * 3;
        for (size_t i = 0; i < quarter_len; i++) {
            ModIntType temp0 = it0[i], temp1 = it1[i], temp2 = it2[i], temp3 = it3[i];
            dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
            it0[i] = temp0, it1[i] = temp1, it2[i] = temp2 * omega1, it3[i] = temp3 * omega3;
            omega1 = omega1 * unit_omega1;
            omega3 = omega3 * unit_omega3;
        }
        dif244(in_out, ntt_len / 2);
        dif244(in_out + quarter_len * 3, ntt_len / 4);
        dif244(in_out + quarter_len * 2, ntt_len / 4);
    }
    static void convolution(ModIntType in1[],
                            ModIntType in2[],
                            ModIntType out[],
                            size_t ntt_len,
                            bool normlize = true) {
        dif244(in1, ntt_len);
        dif244(in2, ntt_len);
        if (normlize) {
            const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
            for (size_t i = 0; i < ntt_len; i++) {
                out[i] = in1[i] * in2[i] * inv_len;
            }
        } else {
            for (size_t i = 0; i < ntt_len; i++) {
                out[i] = in1[i] * in2[i];
            }
        }
        INTT::dit244(out, ntt_len);
    }
    static void convolutionRecursion(ModIntType in1[],
                                     ModIntType in2[],
                                     ModIntType out[],
                                     size_t ntt_len,
                                     bool normlize = true) {
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dif(in1, ntt_len);
            if (in1 != in2) {
                NTTTemplate::dif(in2, ntt_len);
            }
            if (normlize) {
                const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i] * inv_len;
                }
            } else {
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i];
                }
            }
            INTT::NTTTemplate::dit(out, ntt_len);
            return;
        }
        const size_t quarter_len = ntt_len / 4;
        ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        if (in1 != in2) {
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = in1[i], temp1 = in1[quarter_len + i], temp2 = in1[quarter_len * 2 + i],
                           temp3 = in1[quarter_len * 3 + i];
                dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
                in1[i] = temp0, in1[quarter_len + i] = temp1, in1[quarter_len * 2 + i] = temp2 * omega1,
                in1[quarter_len * 3 + i] = temp3 * omega3;

                temp0 = in2[i], temp1 = in2[quarter_len + i], temp2 = in2[quarter_len * 2 + i],
                temp3 = in2[quarter_len * 3 + i];
                dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
                in2[i] = temp0, in2[quarter_len + i] = temp1, in2[quarter_len * 2 + i] = temp2 * omega1,
                in2[quarter_len * 3 + i] = temp3 * omega3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        } else {
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = in1[i], temp1 = in1[quarter_len + i], temp2 = in1[quarter_len * 2 + i],
                           temp3 = in1[quarter_len * 3 + i];
                dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
                in1[i] = temp0, in1[quarter_len + i] = temp1, in1[quarter_len * 2 + i] = temp2 * omega1,
                in1[quarter_len * 3 + i] = temp3 * omega3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        }

        convolutionRecursion(in1, in2, out, ntt_len / 2, false);
        convolutionRecursion(in1 + quarter_len * 2, in2 + quarter_len * 2, out + quarter_len * 2, ntt_len / 4, false);
        convolutionRecursion(in1 + quarter_len * 3, in2 + quarter_len * 3, out + quarter_len * 3, ntt_len / 4, false);

        unit_omega1 = qpow(ModIntType(rootInv()), (mod() - 1) / ntt_len);
        unit_omega3 = qpow(unit_omega1, 3);
        if (normlize) {
            const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
            omega1 = inv_len, omega3 = inv_len;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i] * inv_len, temp1 = out[quarter_len + i] * inv_len,
                           temp2 = out[quarter_len * 2 + i] * omega1, temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        } else {
            omega1 = 1, omega3 = 1;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i], temp1 = out[quarter_len + i], temp2 = out[quarter_len * 2 + i] * omega1,
                           temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        }
    }
    // in1 has been transformed
    static void convolutionRecursion_rep(const ModIntType in1[],
                                         ModIntType in2[],
                                         ModIntType out[],
                                         size_t ntt_len,
                                         bool normlize = true) {
        assert(in1 != in2);
        if (ntt_len <= LONG_THRESHOLD) {
            NTTTemplate::dif(in2, ntt_len);
            if (normlize) {
                const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i] * inv_len;
                }
            } else {
                for (size_t i = 0; i < ntt_len; i++) {
                    out[i] = in1[i] * in2[i];
                }
            }
            INTT::NTTTemplate::dit(out, ntt_len);
            return;
        }
        const size_t quarter_len = ntt_len / 4;
        ModIntType unit_omega1 = qpow(ModIntType(root()), (mod() - 1) / ntt_len);
        ModIntType unit_omega3 = qpow(unit_omega1, 3);
        ModIntType omega1(1), omega3(1);
        for (size_t i = 0; i < quarter_len; i++) {
            ModIntType temp0 = in2[i], temp1 = in2[quarter_len + i], temp2 = in2[quarter_len * 2 + i],
                       temp3 = in2[quarter_len * 3 + i];
            dif_butterfly244<ROOT>(temp0, temp1, temp2, temp3);
            in2[i] = temp0, in2[quarter_len + i] = temp1, in2[quarter_len * 2 + i] = temp2 * omega1,
            in2[quarter_len * 3 + i] = temp3 * omega3;

            omega1 = omega1 * unit_omega1;
            omega3 = omega3 * unit_omega3;
        }

        convolutionRecursion_rep(in1, in2, out, ntt_len / 2, false);
        convolutionRecursion_rep(in1 + quarter_len * 2, in2 + quarter_len * 2, out + quarter_len * 2, ntt_len / 4,
                                 false);
        convolutionRecursion_rep(in1 + quarter_len * 3, in2 + quarter_len * 3, out + quarter_len * 3, ntt_len / 4,
                                 false);

        unit_omega1 = qpow(ModIntType(rootInv()), (mod() - 1) / ntt_len);
        unit_omega3 = qpow(unit_omega1, 3);
        if (normlize) {
            const ModIntType inv_len(qpow(ModIntType(ntt_len), mod() - 2));
            omega1 = inv_len, omega3 = inv_len;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i] * inv_len, temp1 = out[quarter_len + i] * inv_len,
                           temp2 = out[quarter_len * 2 + i] * omega1, temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        } else {
            omega1 = 1, omega3 = 1;
            for (size_t i = 0; i < quarter_len; i++) {
                ModIntType temp0 = out[i], temp1 = out[quarter_len + i], temp2 = out[quarter_len * 2 + i] * omega1,
                           temp3 = out[quarter_len * 3 + i] * omega3;
                dit_butterfly244<rootInv()>(temp0, temp1, temp2, temp3);
                out[i] = temp0, out[quarter_len + i] = temp1, out[quarter_len * 2 + i] = temp2,
                out[quarter_len * 3 + i] = temp3;

                omega1 = omega1 * unit_omega1;
                omega3 = omega3 * unit_omega3;
            }
        }
    }
};
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr int NTT<MOD, ROOT, Int128Type>::MOD_BITS;
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr int NTT<MOD, ROOT, Int128Type>::MAX_LOG_LEN;
template <uint64_t MOD, uint64_t ROOT, typename Int128Type>
constexpr size_t NTT<MOD, ROOT, Int128Type>::NTT_MAX_LEN;
}  // namespace SplitRadix

using NTT0 = SplitRadix::NTT<MOD0, ROOT0>;  // using 64bit integer, Montgomery speed up
using NTT1 = SplitRadix::NTT<MOD1, ROOT1>;  // using 64bit integer, Montgomery speed up
using NTT2 = SplitRadix::NTT<MOD2, ROOT2>;  // using 64bit integer, Montgomery speed up

};  // namespace number_theory

};  // namespace Transform

namespace Arithmetic {

typedef uint64_t lamp_ui;
typedef uint64_t* lamp_ptr;
typedef int64_t lamp_si;

using Transform::number_theory::_uint128;

class lampz {
   protected:
    lamp_si _len;                   /* 绝对值表示大整数的非前导零长度，负值即代表为负数 */
    _internal_buffer<0> _lamp_data; /*数组缓冲区*/
   public:
    lamp_ptr get_ptr() { return _lamp_data.data(); }
    lamp_ui get_len() const { return std::abs(_len); }
    lamp_si get_sign() const { return _len < 0 ? -1 : 1; }
};  // class lampz

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

inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui bit_pos, bool value = true) {
    assert(bit_pos < len * 64);
    const lamp_ui word_pos = bit_pos / 64;
    const lamp_ui bit_in_word_pos = bit_pos % 64;
    in_out[word_pos] |= (value ? 1ull : 0ull) << bit_in_word_pos;
}

inline void set_bit(lamp_ptr in_out, lamp_ui len, lamp_ui word_pos, lamp_ui bit_pos, bool value = true) {
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
    assert(in!= nullptr);
    len = rlz(in, len);
    return len * WORD_BITS - lammp_clz(in[len - 1]);
}

/*
 * @brief 左移大整数的每个部分（半移位器）
 *
 * 该函数将一个以64位无符号整数数组表示的大整数左移指定的位数，并将结果存储到另一个数组中。
 * 如果左移操作导致高位溢出，溢出的部分会被返回。
 *
 * @param in 指向输入数组的指针，存储需要左移的整数。
 * @param len 输入数组的长度，即需要左移的64位无符号整数个数。
 * @param out 指向输出数组的指针，存储左移后的整数。
 * @param shift 左移的位数。
 * @return lamp_ui 左移操作后被丢弃的高位部分。
 * @warning shift 必须在0到63（对于64位整数）之间。
 */
constexpr lamp_ui lshift_in_word_half(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift) {
    constexpr int WORD_BITS = sizeof(lamp_ui) * CHAR_BIT;
    assert(shift >= 0 && shift < WORD_BITS);
    if (0 == len) {
        return 0;
    }
    if (0 == shift) {
        std::copy(in, in + len, out);
        return 0;
    }
    // [n,last] -> [?,n >> shift_rem | last << shift]
    lamp_ui last = in[len - 1], ret = last;
    const int shift_rem = WORD_BITS - shift;
    lamp_ui i = len - 1;
    while (i > 0) {
        i--;
        lamp_ui n = in[i];
        out[i + 1] = (last << shift) | (n >> shift_rem);
        last = n;
    }
    out[0] = last << shift;
    return ret >> shift_rem;
}
/*
 * @brief 左移大整数的每个部分（全移位器）
 *
 * 该函数将一个以64位无符号整数数组表示的大整数左移指定的位数，并将结果存储到另一个数组中。
 * 如果左移操作导致高位溢出，溢出的部分会被赋值给最后一个输出元素。
 * 
 * @param in 指向输入数组的指针，存储需要左移的整数。
 * @param len 输入数组的长度，即需要左移的64位无符号整数个数。
 * @param out 指向输出数组的指针，存储左移后的整数。
 * @param shift 左移的位数。
 * @warning shift 必须在0到63（对于64位整数）之间。且输出数组必须至少有 len+1 个元素。
 */
constexpr void lshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift) {
    if (0 == len) {
        return;
    }
    assert(shift >= 0 && lamp_ui(shift) < sizeof(lamp_ui) * CHAR_BIT);
    lamp_ui last = lshift_in_word_half(in, len, out, shift);
    out[len] = last;
}

/*
 * @brief 右移大整数的每个部分（全移位器）
 * 
 * 该函数将一个以64位无符号整数数组表示的大整数右移指定的位数，并将结果存储到另一个数组中。
 * 如果右移操作导致低位溢出，溢出的部分会被舍弃。
 * 
 * @param in 指向输入数组的指针，存储需要右移的整数。
 * @param len 输入数组的长度，即需要右移的64位无符号整数个数。
 * @param out 指向输出数组的指针，存储右移后的整数。
 * @param shift 右移的位数。
 * @warning shift 必须在0到63（对于64位整数）之间。且输出数组必须至少有 len+1 个元素。
 */
constexpr void rshift_in_word(lamp_ptr in, lamp_ui len, lamp_ptr out, int shift) {
    constexpr int WORD_BITS = sizeof(lamp_ui) * CHAR_BIT;
    if (0 == len) {
        return;
    }
    if (0 == shift) {
        std::copy(in, in + len, out);
        return;
    }
    assert(shift >= 0 && lamp_ui(shift) < sizeof(lamp_ui) * CHAR_BIT);
    lamp_ui last = in[0];
    const int shift_rem = WORD_BITS - shift;
    for (lamp_ui i = 1; i < len; i++) {
        lamp_ui n = in[i];
        out[i - 1] = (last >> shift) | (n << shift_rem);
        last = n;
    }
    out[len - 1] = last >> shift;
}

constexpr void rshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift) {
    lamp_ui shr_word = shift / 64;
    lamp_ui shr_bits = shift % 64;
    assert(shr_word < len);
    if (shr_bits == 0) {
        std::copy(in + shr_word, in + len, out);
    } else {
        rshift_in_word(in + shr_word, len - shr_word, out, shr_bits);
    }
}

constexpr void lshr_bits(lamp_ptr in, lamp_ui len, lamp_ptr out, lamp_ui shift) {
    lamp_ui shr_word = shift / 64;
    lamp_ui shr_bits = shift % 64;
    assert(shr_word < len);
    if (shr_bits == 0) {
        std::copy(in + shr_word, in + len, out);
    } else {
        rshift_in_word(in + shr_word, len - shr_word, out, shr_bits);
    }
}

// Binary absolute addtion a+b=sum, return the carry
constexpr bool abs_add_binary_half(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum) {
    bool carry = false;
    lamp_ui i = 0, min_len = std::min(len_a, len_b);
    for (; i < min_len; i++) {
        sum[i] = add_carry(a[i], b[i], carry);
    }
    for (; i < len_a; i++) {
        sum[i] = add_half(a[i], lamp_ui(carry), carry);
    }   
    for (; i < len_b; i++) {
        sum[i] = add_half(b[i], lamp_ui(carry), carry);
    }
    return carry;
}

constexpr bool abs_add_half_base(
          lamp_ptr a, lamp_ui len_a,
          lamp_ptr b, lamp_ui len_b,
          lamp_ptr sum,
    const lamp_ui base_num
) {
    bool carry = false;
    lamp_ui i = 0, min_len = std::min(len_a, len_b);
    for (; i < min_len; i++) {
        sum[i] = add_carry_base(a[i], b[i], carry, base_num);
    }
    for (; i < len_a; i++) {
        sum[i] = add_half_base(a[i], lamp_ui(carry), carry, base_num);
    }
    for (; i < len_b; i++) {
        sum[i] = add_half_base(b[i], lamp_ui(carry), carry, base_num);
    }
    return carry;
}
// Binary absolute addtion a+b=sum, return the carry
constexpr void abs_add_binary(lamp_ptr a, lamp_ui len_a, lamp_ptr b, lamp_ui len_b, lamp_ptr sum) {
    bool carry = abs_add_binary_half(a, len_a, b, len_b, sum);
    if (carry) {
        sum[std::max(len_a, len_b)] = carry;
    }
}

constexpr void abs_add_base(
    lamp_ptr a, lamp_ui len_a,
    lamp_ptr b, lamp_ui len_b,
    lamp_ptr sum,
    lamp_ui base_num
) {
    lamp_ui carry = abs_add_half_base(a, len_a, b, len_b, sum, base_num);
    sum[std::max(len_a, len_b)] = carry;
}

// Binary absolute subtraction a-b=diff, return the borrow
constexpr bool abs_sub_binary(
    lamp_ptr a, lamp_ui len_a,
    lamp_ptr b, lamp_ui len_b,
    lamp_ptr diff, 
    bool assign_borow = false
) {
    bool borrow = false;
    lamp_ui i = 0, min_len = std::min(len_a, len_b);
    for (; i < min_len; i++) {
        diff[i] = sub_borrow(a[i], b[i], borrow);
    }
    for (; i < len_a; i++) {
        diff[i] = sub_half(a[i], lamp_ui(borrow), borrow);
    }
    for (; i < len_b; i++) {
        diff[i] = sub_half(lamp_ui(0) - lamp_ui(borrow), b[i], borrow);
    }
    if (assign_borow) {
        diff[i] = lamp_ui(borrow);  // 借位
    }
    return borrow;
}

// a - num
constexpr bool abs_sub_binary_num(lamp_ptr a, lamp_ui len_a, lamp_ui num, lamp_ptr diff) {
    assert(len_a > 0);
    bool borrow = false;
    lamp_ui i = 1;
    diff[0] = sub_half(a[0], num, borrow);
    for (; i < len_a; i++) {
        diff[i] = sub_half(a[i], lamp_ui(borrow), borrow);
    }
    return borrow;
}

// Absolute compare, return 1 if a > b, -1 if a < b, 0 if a == b
// Return the diffence length if a != b
[[nodiscard]] constexpr auto abs_compare(const lamp_ptr in1, const lamp_ptr in2, lamp_ui len) {
    struct CompareResult {
        lamp_ui diff_len;
        int cmp = 0;
    };
    while (len > 0) {
        len--;
        if (in1[len] != in2[len]) {
            CompareResult result{len + 1, 0};
            result.cmp = in1[len] > in2[len] ? 1 : -1;
            return result;
        }
    }
    return CompareResult{0, 0};
}

// Absolute compare, return 1 if a > b, -1 if a < b, 0 if a == b
[[nodiscard]] constexpr int abs_compare(const lamp_ptr in1, lamp_ui len1, const lamp_ptr in2, lamp_ui len2) {
    if (len1 != len2) {
        return len1 > len2 ? 1 : -1;
    }
    return abs_compare(in1, in2, len1).cmp;
}

/**
 * @brief 计算两个二进制表示的大整数的绝对值差，并返回符号
 *
 * 该函数用于处理以数组形式存储的大整数（每个元素为 lamp_ui 类型的数字片段），
 * 计算两者的绝对值差并存储到输出数组中，同时通过返回值指示原始两个数的大小关系。
 * 内部确保用大数减去小数，避免负数结果，简化差值计算逻辑。
 *
 * @tparam lamp_ui 数组元素类型，必须为无符号整数类型
 * @param[in] a 第一个大整数的二进制表示数组
 * @param[in] len1 a 数组的长度（元素个数）
 * @param[in] b 第二个大整数的二进制表示数组
 * @param[in] len2 b 数组的长度（元素个数）
 * @param[out] diff 输出参数，用于存储 a 和 b 的绝对值差结果（需提前分配足够空间）
 * @return int 符号：
 *          1 表示 a > b（diff 存储 a - b）
 *          -1 表示 a < b（diff 存储 b - a）
 *          0 表示 a == b（diff 存储全 0）
 * @warning diff 数组的长度需至少为两个输入数组中的最大长度，将不会进行越界检查。
 */
[[nodiscard]] constexpr lamp_si abs_difference_binary(
    lamp_ptr a, lamp_ui len1,
    lamp_ptr b, lamp_ui len2,
    lamp_ptr diff
) {
    int sign = 1;
    if (len1 == len2) {
        auto cmp = abs_compare(a, b, len1);
        sign = cmp.cmp;
        std::fill(diff + cmp.diff_len, diff + len1, lamp_ui(0));
        len1 = len2 = cmp.diff_len;
        if (sign < 0) {
            std::swap(a, b);
        }
    } else if (len1 < len2) {
        std::swap(a, b);
        std::swap(len1, len2);
        sign = -1;
    }
    abs_sub_binary(a, len1, b, len2, diff);
    return sign;
}

inline lamp_ui abs_mul_add_num64_half(
    const lamp_ptr in,  lamp_ui len,
          lamp_ptr out, lamp_ui num_add, lamp_ui num_mul
) {
    lamp_ui i = 0;
    lamp_ui prod_lo, prod_hi;
    for (const lamp_ui rem_len = len - len % 4; i < rem_len; i += 4) {
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);

        mul64x64to128(in[i + 1], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i + 1] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);

        mul64x64to128(in[i + 2], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i + 2] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);

        mul64x64to128(in[i + 3], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i + 3] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);
    }
    for (; i < len; i++) {
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo += num_add;
        out[i] = prod_lo;
        num_add = prod_hi + (prod_lo < num_add);
    }
    return num_add;
}

/// @brief 2^64 base long integer multiply 64bit number, add another 64bit number to product.
/// @param in Input long integer.
/// @param length Number of 64-bit blocks in the input array.
/// @param out Output long integer, equals to input * num_mul + num_add
/// @param num_add The 64 bit number to add.
/// @param num_mul The 64 bit number to multiply.
/// @details
/// The function performs multiplication and addition on a large integer represented by multiple 64-bit blocks:
/// 1. For each block of the large integer from index 0 to `length-1`:
///    a. Multiply the current block `in[i]` by `num_mul`.
///    b. Add the current value of `num_add` to the product.
///    c. Store the lower 64 bits of the result in `out[i]`.
///    d. Update `num_add` with the higher 64 bits of the product (carry-over to the next block).
/// 2. After processing all blocks, store the final value of `num_add` (the carry-over) in `out[length]`.
inline void abs_mul_add_num64(const lamp_ptr in, lamp_ui length, lamp_ptr out, lamp_ui num_add, lamp_ui num_mul) {
    for (lamp_ui i = 0; i < length; i++) {
        _uint128 product = _uint128(in[i]) * num_mul + num_add;
        out[i] = lamp_ui(product);
        num_add = product.high64();
    }
    out[length] = num_add;
}

// in * num_mul + in_out -> in_out
inline void mul64_sub_proc(const lamp_ptr in, lamp_ui len, lamp_ptr in_out, lamp_ui num_mul) {
    lamp_ui carry = 0;
    lamp_ui i = 0;
    for (const lamp_ui rem_len = len - len % 4; i < rem_len; i += 4) {
        bool cf;
        lamp_ui prod_lo, prod_hi;
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i], cf);
        prod_hi += cf;
        in_out[i] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;

        mul64x64to128(in[i + 1], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i + 1], cf);
        prod_hi += cf;
        in_out[i + 1] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;

        mul64x64to128(in[i + 2], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i + 2], cf);
        prod_hi += cf;
        in_out[i + 2] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;

        mul64x64to128(in[i + 3], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i + 3], cf);
        prod_hi += cf;
        in_out[i + 3] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;
    }
    for (; i < len; i++) {
        bool cf;
        uint64_t prod_lo, prod_hi;
        mul64x64to128(in[i], num_mul, prod_lo, prod_hi);
        prod_lo = add_half(prod_lo, in_out[i], cf);
        prod_hi += cf;
        in_out[i] = add_half(prod_lo, carry, cf);
        carry = prod_hi + cf;
    }
    in_out[len] = carry;
}

constexpr size_t KARATSUBA_MIN_THRESHOLD = 24;
constexpr size_t KARATSUBA_MAX_THRESHOLD = 1536;

// 朴素乘法
inline void abs_mul64_classic(
    lamp_ptr in1, lamp_ui len1, 
    lamp_ptr in2, lamp_ui len2, 
    lamp_ptr out, 
    lamp_ptr work_begin, lamp_ptr work_end
) {
    const lamp_ui out_len = get_mul_len(len1, len2);
    len1 = rlz(in1, len1);
    len2 = rlz(in2, len2);
    if (len1 < len2) {
        std::swap(in1, in2);
        std::swap(len1, len2);  // Let in1 be the loonger one
    }
    if (0 == len2 || nullptr == in1 || nullptr == in2) {
        std::fill_n(out, out_len, lamp_ui(0));
        return;
    }
    if (1 == len2) {
        abs_mul_add_num64(in1, len1, out, 0, in2[0]);
        return;
    }
    // Get enough work memory
    _internal_buffer<0> work_mem(0);
    const lamp_ui work_size = get_mul_len(len1, len2);
    if (work_begin + work_size > work_end) {
        work_mem.resize(work_size);
        work_begin = work_mem.data();
        work_end = work_begin + work_mem.capacity();
    } else {
        // Clear work_mem that may used
        std::fill_n(work_begin, work_size, lamp_ui(0));
    }
    auto out_temp = work_begin;
    for (lamp_ui i = 0; i < len1; i++) {
        mul64_sub_proc(in2, len2, out_temp + i, in1[i]);
    }
    std::copy(out_temp, out_temp + work_size, out);
    std::fill(out + work_size, out + out_len, lamp_ui(0));
}

// Karatsuba 乘法
inline void abs_mul64_karatsuba_buffered(
    lamp_ptr in1, lamp_ui len1, 
    lamp_ptr in2, lamp_ui len2, 
    lamp_ptr out, 
    lamp_ptr buffer_begin, lamp_ptr buffer_end
) {

    const lamp_ui out_len = get_mul_len(len1, len2);
    len1 = rlz(in1, len1);
    len2 = rlz(in2, len2);
    if (len1 < len2) {
        std::swap(in1, in2);
        std::swap(len1, len2);  // Let in1 be the loonger one
    }
    if (0 == len2 || nullptr == in1 || nullptr == in2) {
        std::fill_n(out, out_len, lamp_ui(0));
        return;
    }
    if (len2 < KARATSUBA_MIN_THRESHOLD) {
        abs_mul64_classic(in1, len1, in2, len2, out, buffer_begin, buffer_end);
        std::fill(out + len1 + len2, out + out_len, lamp_ui(0));
        return;
    }
    // Split A * B -> (AH * BASE + AL) * (BH * BASE + BL)
    // (AH * BASE + AL) * (BH * BASE + BL) = AH * BH * BASE^2 + (AH * BL + AL * BH) * BASE + AL * BL
    // Let M  = AL * BL,
    //     N  = AH * BH,
    //     K1 = (AH - AL),
    //     K2 = (BH - BL),
    //     K  = K1 * K2
    //        = AH * BH - (AH * BL + AL * BH) + AL * BL
    //
    // A * B = N * BASE^2 + (M + N - K) * BASE + M
    const lamp_ui base_len = (len1 + 1) / 2;
    lamp_ui len1_low = base_len, len1_high = len1 - base_len;
    lamp_ui len2_low = base_len, len2_high = len2 - base_len;
    if (len2 <= base_len) {
        len2_low = len2;
        len2_high = 0;
    }
    // Get length of every part
    lamp_ui m_len = get_mul_len(len1_low, len2_low);
    lamp_ui n_len = get_mul_len(len1_high, len2_high);

    // Get enough buffer
    _internal_buffer<0> buffer(0);
    const lamp_ui buffer_size = m_len + n_len + get_mul_len(len1_low, len2_low);
    if (buffer_begin + buffer_size > buffer_end) {
        buffer.resize(buffer_size * 2 + 1);
        buffer_begin = buffer.data();
        buffer_end = buffer_begin + buffer.capacity();
    }
    // Set pointer of every part
    auto m = buffer_begin, n = m + m_len, k1 = n + n_len, k2 = k1 + len1_low, k = k1;

    // Compute M,N
    abs_mul64_karatsuba_buffered(in1, len1_low, in2, len2_low, m, buffer_begin + buffer_size, buffer_end);
    abs_mul64_karatsuba_buffered(in1 + base_len, len1_high, in2 + base_len, len2_high, n, buffer_begin + buffer_size,
                                 buffer_end);

    // Compute K1,K2
    len1_low = rlz(in1, len1_low);
    len2_low = rlz(in2, len2_low);
    int cmp1 = abs_difference_binary(in1, len1_low, in1 + base_len, len1_high, k1);
    int cmp2 = abs_difference_binary(in2, len2_low, in2 + base_len, len2_high, k2);
    lamp_ui k1_len = rlz(k1, get_sub_len(len1_low, len1_high));
    lamp_ui k2_len = rlz(k2, get_sub_len(len2_low, len2_high));

    // Compute K1*K2 = K
    abs_mul64_karatsuba_buffered(k1, k1_len, k2, k2_len, k, buffer_begin + buffer_size, buffer_end);
    lamp_ui k_len = rlz(k, get_mul_len(k1_len, k2_len));

    // Combine the result
    {
        // out = M + N * BASE ^ 2 + (M + N) ^ BASE
        std::fill(out + m_len, out + base_len * 2, lamp_ui(0));
        std::fill(out + base_len * 2 + n_len, out + out_len, lamp_ui(0));
        std::copy(m, m + m_len, out);
        std::copy(n, n + n_len, out + base_len * 2);
        m_len = std::min(m_len, out_len - base_len);
        n_len = std::min(n_len, out_len - base_len);
        {
            if (m_len < n_len) {
                std::swap(m_len, n_len);
                std::swap(m, n);
            }
            uint8_t carry = 0;
            lamp_ui i = 0;
            auto out_p = out + base_len;
            for (; i < n_len; i++) {
                bool cf;
                lamp_ui sum = add_half(m[i], lamp_ui(carry), cf);
                carry = cf;
                sum = add_half(n[i], sum, cf);
                carry += cf;
                out_p[i] = add_half(out_p[i], sum, cf);
                carry += cf;
            }
            for (; i < m_len; i++) {
                bool cf;
                lamp_ui sum = add_half(m[i], lamp_ui(carry), cf);
                carry = cf;
                out_p[i] = add_half(out_p[i], sum, cf);
                carry += cf;
            }
            for (; i < out_len - base_len; i++) {
                bool cf;
                out_p[i] = add_half(out_p[i], lamp_ui(carry), cf);
                carry = cf;
            }
        }

        // out = M + N * BASE ^ 2 + (M + N - K) ^ BASE
        k_len = std::min(k_len, out_len - base_len);
        if (cmp1 * cmp2 > 0) {
            abs_sub_binary(out + base_len, out_len - base_len, k, k_len, out + base_len);
        } else {
            abs_add_binary_half(out + base_len, out_len - base_len, k, k_len, out + base_len);
        }
    }
}

inline void abs_mul64_karatsuba(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out) {
    abs_mul64_karatsuba_buffered(in1, len1, in2, len2, out, nullptr, nullptr);
}

// NTT square
inline void abs_sqr64_ntt(lamp_ptr in, lamp_ui len, lamp_ptr out) {
    using namespace lammp::Transform::number_theory;
    if (0 == len || in == nullptr) {
        return;
    }
    lamp_ui out_len = len * 2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::copy(in, in + len, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer1.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer2(ntt_len);
    {
        std::copy(in, in + len, buffer2.begin());
        NTT1::convolutionRecursion(buffer2.data(), buffer2.data(), buffer2.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer3(ntt_len);
    {
        std::copy(in, in + len, buffer3.begin());
        NTT2::convolutionRecursion(buffer3.data(), buffer3.data(), buffer3.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer2[i], buffer3[i]);
        out[i] = lamp_ui(carry);
        carry = carry.rShift64();
    }
    out[conv_len] = lamp_ui(carry);
}

// NTT multiplication
inline void abs_mul64_ntt(lamp_ptr in1, lamp_ui len1, lamp_ptr in2, lamp_ui len2, lamp_ptr out) {
    if (0 == len1 || 0 == len2 || in1 == nullptr || in2 == nullptr) {
        return;
    }
    if (in1 == in2) {
        abs_sqr64_ntt(in1, len1, out);  // Square
        return;
    }
    using namespace lammp::Transform::number_theory;
    lamp_ui out_len = len1 + len2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::vector<NTT0::ModIntType> buffer2(ntt_len);
        std::copy(in2, in2 + len2, buffer2.begin());
        std::copy(in1, in1 + len1, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer2.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer3(ntt_len);
    {
        std::vector<NTT1::ModIntType> buffer4(ntt_len);
        std::copy(in2, in2 + len2, buffer4.begin());
        std::copy(in1, in1 + len1, buffer3.begin());
        NTT1::convolutionRecursion(buffer3.data(), buffer4.data(), buffer3.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer5(ntt_len);
    {
        std::vector<NTT2::ModIntType> buffer6(ntt_len);
        std::copy(in2, in2 + len2, buffer6.begin());
        std::copy(in1, in1 + len1, buffer5.begin());
        NTT2::convolutionRecursion(buffer5.data(), buffer6.data(), buffer5.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
        out[i] = lamp_ui(carry);
        carry = carry.rShift64();
    }
    out[conv_len] = lamp_ui(carry);
}

inline void abs_mul64_ntt_unbalanced(lamp_ptr in1,
                                     lamp_ui len1,
                                     lamp_ptr in2,
                                     lamp_ui len2,
                                     lamp_ui M,
                                     lamp_ptr out) {
    assert(in1 != in2 && len1 > len2);
    using namespace lammp::Transform::number_theory;
    lamp_ui min_sum = len2 + std::max(len2, M);

    min_sum -= ((min_sum & (min_sum - 1)) == 0) ? len1 : 0;

    int highest_bit = 63 - lammp_clz(min_sum);
    uint64_t next_power = 1ULL << (highest_bit + 1);

    lamp_ui balance_len = next_power, conv_len = balance_len - 1, single_len = balance_len - len2;
    assert(single_len <= len1 && "please use balanced version");

    lamp_ui ntt_len = int_ceil2(conv_len), rem = len1 % single_len;

    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    std::vector<NTT0::ModIntType> buffer2(ntt_len);
    std::copy(in2, in2 + len2, buffer2.begin());
    std::copy(in1, in1 + single_len, buffer1.begin());
    NTT0::convolutionRecursion(buffer1.data(), buffer2.data(), buffer1.data(), ntt_len);

    std::vector<NTT1::ModIntType> buffer3(ntt_len);
    std::vector<NTT1::ModIntType> buffer4(ntt_len);
    std::copy(in2, in2 + len2, buffer4.begin());
    std::copy(in1, in1 + single_len, buffer3.begin());
    NTT1::convolutionRecursion(buffer3.data(), buffer4.data(), buffer3.data(), ntt_len);

    std::vector<NTT2::ModIntType> buffer5(ntt_len);
    std::vector<NTT2::ModIntType> buffer6(ntt_len);
    std::copy(in2, in2 + len2, buffer6.begin());
    std::copy(in1, in1 + single_len, buffer5.begin());
    NTT2::convolutionRecursion(buffer5.data(), buffer6.data(), buffer5.data(), ntt_len);

    _internal_buffer<0> balance_prod(balance_len);

    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
        out[i] = lamp_ui(carry);
        carry = carry.rShift64();
    }
    out[conv_len] = lamp_ui(carry);
    //
    //             len2 = 2
    // balance_prod_len = 4
    // +---+---+---+---+
    // | 1 | 2 | 3 | 4 |
    // +---+---+---+---+
    //         |              prod
    //         +---+---+---+---+
    //         | 2 | 3 | 4 | 5 |
    //         +---+---+---+---+
    //                 |   out+len2    out
    //                 +---+---+---+---+
    //                 | 3 | 4 | 5 | 6 |
    //                 +---+---+---+---+
    lamp_ui len = single_len;
    auto in1_p = in1;
    for (; len < len1 - rem; len += single_len) {
        in1_p += single_len;
        std::fill(buffer1.begin() + single_len, buffer1.begin() + ntt_len, NTT0::ModIntType(0));
        std::fill(buffer3.begin() + single_len, buffer3.begin() + ntt_len, NTT1::ModIntType(0));
        std::fill(buffer5.begin() + single_len, buffer5.begin() + ntt_len, NTT2::ModIntType(0));

        std::copy(in1_p, in1_p + single_len, buffer1.begin());
        NTT0::convolutionRecursion_rep(buffer2.data(), buffer1.data(), buffer1.data(), ntt_len);
        std::copy(in1_p, in1_p + single_len, buffer3.begin());
        NTT1::convolutionRecursion_rep(buffer4.data(), buffer3.data(), buffer3.data(), ntt_len);
        std::copy(in1_p, in1_p + single_len, buffer5.begin());
        NTT2::convolutionRecursion_rep(buffer6.data(), buffer5.data(), buffer5.data(), ntt_len);
        carry = 0;
        for (lamp_ui i = 0; i < conv_len; i++) {
            carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
            balance_prod.set(i, lamp_ui(carry));
            carry = carry.rShift64();
        }
        balance_prod.set(conv_len, lamp_ui(carry));
        abs_add_binary_half(balance_prod.data(), balance_len, out + len, len2, out + len);
    }
    if (rem > 0) {
        in1_p = in1 + len;
        std::fill(buffer1.begin() + rem, buffer1.begin() + ntt_len, NTT0::ModIntType(0));
        std::fill(buffer3.begin() + rem, buffer3.begin() + ntt_len, NTT1::ModIntType(0));
        std::fill(buffer5.begin() + rem, buffer5.begin() + ntt_len, NTT2::ModIntType(0));
        std::copy(in1_p, in1_p + rem, buffer1.begin());
        NTT0::convolutionRecursion_rep(buffer2.data(), buffer1.data(), buffer1.data(), ntt_len);
        std::copy(in1_p, in1_p + rem, buffer3.begin());
        NTT1::convolutionRecursion_rep(buffer4.data(), buffer3.data(), buffer3.data(), ntt_len);
        std::copy(in1_p, in1_p + rem, buffer5.begin());
        NTT2::convolutionRecursion_rep(buffer6.data(), buffer5.data(), buffer5.data(), ntt_len);
        carry = 0;
        for (lamp_ui i = 0; i < conv_len; i++) {
            carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
            balance_prod.set(i, lamp_ui(carry));
            carry = carry.rShift64();
        }
        balance_prod.set(conv_len, lamp_ui(carry));
        // 注意这两个加数不可调换，否则越界
        abs_add_binary_half(out + len, len2, balance_prod.data(), len2 + rem, out + len);
    }
}

// NTT square 在 base_num进制下
inline void abs_sqr64_ntt_base(lamp_ptr in, lamp_ui len, lamp_ptr out, const lamp_ui base_num) {
    using namespace lammp::Transform::number_theory;
    if (0 == len || in == nullptr) {
        return;
    }
    lamp_ui out_len = len * 2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::copy(in, in + len, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer1.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer2(ntt_len);
    {
        std::copy(in, in + len, buffer2.begin());
        NTT1::convolutionRecursion(buffer2.data(), buffer2.data(), buffer2.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer3(ntt_len);
    {
        std::copy(in, in + len, buffer3.begin());
        NTT2::convolutionRecursion(buffer3.data(), buffer3.data(), buffer3.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer2[i], buffer3[i]);
        out[i] = carry.self_div_rem(base_num);
    }
    out[conv_len] = carry.self_div_rem(base_num);
}

// NTT multiplication 在 base_num进制下
inline void abs_mul64_ntt_base(lamp_ptr in1, lamp_ui len1,
                               lamp_ptr in2, lamp_ui len2,
                               lamp_ptr out,
                               const lamp_ui base_num) {
    if (0 == len1 || 0 == len2 || in1 == nullptr || in2 == nullptr) {
        return;
    }
    if (in1 == in2) {
        abs_sqr64_ntt_base(in1, len1, out, base_num);  // Square
        return;
    }
    using namespace lammp::Transform::number_theory;
    lamp_ui out_len = len1 + len2, conv_len = out_len - 1;
    lamp_ui ntt_len = int_ceil2(conv_len);
    std::vector<NTT0::ModIntType> buffer1(ntt_len);
    {
        std::vector<NTT0::ModIntType> buffer2(ntt_len);
        std::copy(in2, in2 + len2, buffer2.begin());
        std::copy(in1, in1 + len1, buffer1.begin());
        NTT0::convolutionRecursion(buffer1.data(), buffer2.data(), buffer1.data(), ntt_len);
    };
    std::vector<NTT1::ModIntType> buffer3(ntt_len);
    {
        std::vector<NTT1::ModIntType> buffer4(ntt_len);
        std::copy(in2, in2 + len2, buffer4.begin());
        std::copy(in1, in1 + len1, buffer3.begin());
        NTT1::convolutionRecursion(buffer3.data(), buffer4.data(), buffer3.data(), ntt_len);
    };
    std::vector<NTT2::ModIntType> buffer5(ntt_len);
    {
        std::vector<NTT2::ModIntType> buffer6(ntt_len);
        std::copy(in2, in2 + len2, buffer6.begin());
        std::copy(in1, in1 + len1, buffer5.begin());
        NTT2::convolutionRecursion(buffer5.data(), buffer6.data(), buffer5.data(), ntt_len);
    };
    _uint192 carry = 0;
    for (lamp_ui i = 0; i < conv_len; i++) {
        carry += crt3(buffer1[i], buffer3[i], buffer5[i]);
        out[i] = carry.self_div_rem(base_num);
    }
    out[conv_len] = carry.self_div_rem(base_num);
}

inline void abs_mul64_balanced(lamp_ptr in1, lamp_ui len1,
                               lamp_ptr in2, lamp_ui len2,
                               lamp_ptr out,
                               lamp_ptr work_begin = nullptr,
                               lamp_ptr work_end = nullptr) {
    if (len1 < len2) {
        std::swap(in1, in2);
        std::swap(len1, len2);
    }
    if (len2 <= KARATSUBA_MIN_THRESHOLD) {
        abs_mul64_classic(in1, len1, in2, len2, out, work_begin, work_end);
    } else if (len2 <= KARATSUBA_MAX_THRESHOLD) {
        abs_mul64_karatsuba_buffered(in1, len1, in2, len2, out, work_begin, work_end);
    } else {
        abs_mul64_ntt(in1, len1, in2, len2, out);
    }
}

inline void abs_mul64(lamp_ptr in1, lamp_ui len1,
                      lamp_ptr in2, lamp_ui len2,
                      lamp_ptr out, 
                      lamp_ptr work_begin = nullptr,
                      lamp_ptr work_end = nullptr) {
    if (len1 < len2) {
        std::swap(in1, in2);
        std::swap(len1, len2);
    }
    if (len2 <= KARATSUBA_MIN_THRESHOLD) {
        abs_mul64_classic(in1, len1, in2, len2, out, work_begin, work_end);
        return;
    } else if (len2 >= KARATSUBA_MAX_THRESHOLD) {
        lamp_ui M = len1 / len2;
        if (M >= 3 && M <= 9) {
            abs_mul64_ntt_unbalanced(in1, len1, in2, len2, 0, out);
            return;
        } else if (M > 9) {
            M = std::sqrt(len1 / len2);
            abs_mul64_ntt_unbalanced(in1, len1, in2, len2, M, out);
            return;
        } else {
            abs_mul64_ntt(in1, len1, in2, len2, out);
            return;
        }
    }
    // Get enough work memory
    _internal_buffer<0> work_mem(0);
    const lamp_ui work_size = len2 * 3 + len1;  // len1 + len2 + len2 * 2,存放结果以及平衡乘积
    if (work_begin + work_size > work_end) {
        work_mem.resize(work_size + len2 * 6);  // 为karatsuba做准备
        work_begin = work_mem.data();
        work_end = work_begin + work_mem.capacity();
    } else {
        std::fill_n(work_begin, work_size, lamp_ui(0));
    }
    auto balance_prod = work_begin, total_prod = balance_prod + len2 * 2;
    lamp_ui rem = len1 % len2, i = len2;
    abs_mul64_balanced(in2, len2, in1, len2, total_prod, work_begin + work_size, work_end);
    for (; i < len1 - rem; i += len2) {
        abs_mul64_balanced(in2, len2, in1 + i, len2, balance_prod, work_begin + work_size, work_end);
        abs_add_binary_half(balance_prod, len2 * 2, total_prod + i, len2, total_prod + i);
    }
    if (rem > 0) {
        abs_mul64(in2, len2, in1 + i, rem, balance_prod, work_begin + work_size, work_end);
        // 注意这两个加数不可调换
        abs_add_binary_half(total_prod + i, len2, balance_prod, len2 + rem, total_prod + i);
    }
    std::copy(total_prod, total_prod + len1 + len2, out);
}

template <typename NumTy, typename ProdTy>
class DivSupporter {
   private:
    NumTy divisor = 0;
    NumTy inv = 0;
    int shift = 0, shift1 = 0, shift2 = 0;
    enum : int { NUM_BITS = sizeof(NumTy) * CHAR_BIT };

   public:
    constexpr DivSupporter(NumTy divisor_in) : divisor(divisor_in) {
        inv = getInv(divisor, shift);
        divisor <<= shift;
        shift1 = shift / 2;
        shift2 = shift - shift1;
    }
    // Return dividend / divisor, dividend %= divisor
    NumTy divMod(ProdTy& dividend) const {
        dividend <<= shift;
        NumTy r = NumTy(dividend);
        dividend = (dividend >> NUM_BITS) * inv + dividend;
        NumTy q1 = NumTy(dividend >> NUM_BITS) + 1;
        r -= q1 * divisor;
        if (r > NumTy(dividend)) {
            q1--;
            r += divisor;
        }
        if (r >= divisor) {
            q1++;
            r -= divisor;
        }
        dividend = r >> shift;
        return q1;
    }

    void prodDivMod(NumTy a, NumTy b, NumTy& quot, NumTy& rem) const {
        ProdTy dividend = ProdTy(a << shift1) * (b << shift2);
        rem = NumTy(dividend);
        dividend = (dividend >> NUM_BITS) * inv + dividend;
        quot = NumTy(dividend >> NUM_BITS) + 1;
        rem -= quot * divisor;
        if (rem > NumTy(dividend)) {
            quot--;
            rem += divisor;
        }
        if (rem >= divisor) {
            quot++;
            rem -= divisor;
        }
        rem >>= shift;
    }

    NumTy div(ProdTy dividend) const { return divMod(dividend); }
    NumTy mod(ProdTy dividend) const {
        divMod(dividend);
        return dividend;
    }

    static constexpr NumTy getInv(NumTy divisor, int& leading_zero) {
        constexpr NumTy MAX = all_one<NumTy>(NUM_BITS);
        leading_zero = lammp_clz(divisor);
        divisor <<= leading_zero;
        ProdTy x = ProdTy(MAX - divisor) << NUM_BITS;
        return NumTy((x + MAX) / divisor);
    }
};

/*
 * @brief 将一个表示为64位块数组的大整数除以一个64位除数
 * @param in 表示要被除的大整数的输入数组。数组的每个元素都是该整数的一个64位块
 * @param length 输入数组中64位块的数量
 * @param out 存储除法结果的输出数组。除法后，out将表示商（*this / divisor）
 * @param divisor 用于除大整数的64位数字
 * @return 除法的余数（*this % divisor），为64位值
 * @details
 * 该函数对由多个64位块表示的大整数执行除法操作：
 * 1. 将`remainder_high64bit`初始化为0
 * 2. 对于大整数的每个块，从最高有效块（索引`length-1`）到最低有效块（索引0）：
 *    a. 组合当前块`in[length]`和当前`remainder_high64bit`以形成128位值
 *    b. 对这个128位值调用`selfDivRem(divisor)`：
 *       - 用64位`divisor`除以128位值
 *       - 用商更新`out[llength]`中的当前块
 *       - 用余数更新`remainder_high64bit`
 * 3. 处理完所有块后，`remainder_high64bit`的最终值就是整个除法的余数
 * 4. 返回`remainder_high64bit`
 */
inline lamp_ui abs_div_rem_num64(lamp_ptr in, lamp_ui length, lamp_ptr out, lamp_ui divisor) {
    lamp_ui remainder_high64bit = 0;
    assert(divisor != 0);
    if (divisor == 1) {
        std::copy(in, in + length, out);
        return 0;
    }
    else if (divisor > UINT32_MAX) {
        while (length > 0) {
            length--;
            _uint128 n(in[length], remainder_high64bit);
            remainder_high64bit = n.self_div_rem(divisor);
            out[length] = n;
        }
        return remainder_high64bit;
    }
    else {
        const uint32_t divisor32 = uint32_t(divisor);
        while (length > 0) {
            length--;
            _uint128 n(in[length], remainder_high64bit);
            remainder_high64bit = n.self_div_rem(divisor32);
            out[length] = n;
        }
        return remainder_high64bit;
    }
}

/*
 in 输入会被更改，且输入in的缓冲区长度应为len+1
 */
inline void abs_div_knuth(
    lamp_ptr in,      lamp_ui len,
    lamp_ptr divisor, lamp_ui divisor_len,
    lamp_ptr out,                 // 商的输出数组，长度为len-divisor_len+1
    lamp_ptr remainder = nullptr  // 余数的输出数组，长度为divisor_len
) {
    assert(in != nullptr && len > 0 && divisor != nullptr && divisor_len > 0);
    assert(divisor_len <= len);
    assert(divisor[divisor_len - 1] >= (1ull << 63));

    if (divisor_len == 1) {
        if (remainder != nullptr) {
            remainder[0] = abs_div_rem_num64(in, len, out, divisor[0]);
        } else {
            lamp_ui rem = abs_div_rem_num64(in, len, out, divisor[0]);
            std::fill(in, in + len, 0);
            in[0] = rem;
        }
        return;
    }
    if (len == divisor_len) {
        lamp_si sign = abs_compare(in, len, divisor, divisor_len);
        if (remainder != nullptr) {
            if (sign > 0) {
                out[0] = 1;
                abs_sub_binary(in, len, divisor, divisor_len, remainder);
            } else if (sign == 0) {
                out[0] = 1;
                remainder[0] = 0;
            } else {
                out[0] = 0;
                std::copy(in, in + len, remainder);
            }
        } else {
            if (sign > 0) {
                out[0] = 1;
                abs_sub_binary(in, len, divisor, divisor_len, in);
            } else if (sign == 0) {
                out[0] = 1;
                std::fill(in, in + len, 0);
            } else {
                out[0] = 0;
            }
        }
        return;
    }

    lamp_ui out_len = get_div_len(len, divisor_len);
    lamp_ui _dividend_len = len + 1;  // 由于kunth除法一定高估，所以需要多分配一个元素
    _internal_buffer<0> _dividend(_dividend_len, 0);

    for (lamp_ui i = out_len - 1; i-- != 0;) {
        if (len < divisor_len) {
            break;
        }
        _uint128 _dividend_i(in[len - 2], in[len - 1]);

        _dividend_i.self_div_rem(divisor[divisor_len - 1]);

        lamp_ui q_hat[2] = {_dividend_i.low64(), _dividend_i.high64()};
        lamp_ui q_hat_len = rlz(q_hat, 2);

        abs_mul64_classic(q_hat, q_hat_len, divisor, divisor_len, _dividend.data() + i, nullptr, nullptr);
        _dividend_len = rlz(_dividend.data(), i + get_mul_len(divisor_len, q_hat_len));
        lamp_si sign = abs_compare(_dividend.data(), _dividend_len, in, len);
        if (sign > 0) {
            _dividend_i -= 1;
            lamp_si sub_flag = abs_difference_binary(_dividend.data() + i, _dividend_len - i, divisor, divisor_len,
                                                     _dividend.data() + i);
            assert(sub_flag == 1);
            _dividend_len = rlz(_dividend.data(), _dividend_len);
            sign = abs_compare(_dividend.data(), _dividend_len, in, len);
            if (sign > 0) {
                _dividend_i -= 1;
                sub_flag = abs_difference_binary(_dividend.data() + i, _dividend_len - i, divisor, divisor_len,
                                                 _dividend.data() + i);
                assert(sub_flag == 1);
            }
        }
        lamp_si sus_flag = abs_difference_binary(in + i, len - i, _dividend.data() + i, _dividend_len - i, in + i);
        assert(sus_flag >= 0);
        out[i + 1] = (_dividend_i.high64() == 0) ? out[i + 1] : _dividend_i.high64();
        out[i] = _dividend_i.low64();
        len = rlz(in, len);
        std::fill(_dividend.data() + i, _dividend.data() + _dividend_len, 0);
    }
    if (remainder != nullptr) {
        std::copy(in, in + len, remainder);
    }
}

/*
 当 len == 3时， out 需要额外一个长度
 */
inline lamp_ui barrett_2powN_recursive(lamp_ptr in, lamp_ui len, lamp_ptr out) {
    using _uint192 = lammp::Transform::number_theory::_uint192;
    assert(in != nullptr && len > 0);

    if (len == 1) {
        assert(false && "This function should not be called with len == 1");
        _uint192 n(0, 0, 1);
        n.self_div_rem(in[0]);
        out[0] = n.low64();
        out[1] = n.mid64();
        out[2] = n.high64();
        return rlz(out, 3);
    } else if (len <= 16) {
        lamp_ui _2_powN_len = len * 2 + 1;
        _internal_buffer<0> _2_powN(_2_powN_len + 1, 0);
        lamp_si shift = lammp_clz(in[len - 1]);
        _2_powN.set(_2_powN_len - 1, (1ull << shift));
        _internal_buffer<0> _in(len + 1, 0); /* 此处加一没有用，主要因为lsift强制要求导致的 */
        lshift_in_word(in, len, _in.data(), shift);
        abs_div_knuth(_2_powN.data(), _2_powN_len, _in.data(), len, out, nullptr);
        lamp_ui out_len = rlz(out, len + 1);
        return out_len;
    }

    lamp_ui _len = len / 2 + 1;
    lamp_ui rem_len = len - _len;

    _internal_buffer<0> q_hat(len + 2, 0); /* q_hat 长度应于 out 长度一致，同时必须多分配一个 */
    lamp_ui q_hat_len = barrett_2powN_recursive(in + rem_len, _len, q_hat.data() + rem_len);
    q_hat_len += rem_len;

    lamp_ui _2len = 2 * len, _2_powN_len = _2len + len + 1;
    /* q_hat 多分配一个， 相应的 _2_powN 和 _q_mul_in 也要多分配一个 */
    _internal_buffer<0> _2_powN(_2_powN_len + 1, 0), _q_mul_in(q_hat_len + len + 1, 0);
    _2_powN.set(_2len, 2);

    abs_mul64(q_hat.data(), q_hat_len, in, len, _q_mul_in.data());
    lamp_ui _q_mul_in_len = rlz(_q_mul_in.data(), len + q_hat_len);
    lamp_si suss = abs_difference_binary(_2_powN.data(), _2len + 1, _q_mul_in.data(), _q_mul_in_len, _2_powN.data());
    assert(suss >= 0);
    _2_powN_len = rlz(_2_powN.data(), _2len + 1);
    abs_mul64(q_hat.data(), q_hat_len, _2_powN.data(), _2_powN_len, _2_powN.data());
    _2_powN_len = rlz(_2_powN.data(), get_mul_len(q_hat_len, _2_powN_len));
    std::copy(_2_powN.data() + _2len, _2_powN.data() + _2_powN_len, out);
    return rlz(out, _2_powN_len - _2len);
}
/*
 * @brief 计算 ceil(base^N / in)，使用牛顿迭代法
 * @param N 指数
 * @param in 被除数
 * @param len 被除数的长度
 * @param out 商的输出数组，长度至少 N + 1 - len
 * @return 商的长度
 * @details
 * 该函数计算 base^N / in，其中 base 为 2^64，in 为一个大整数。
 * 主要调用 barrett_2powN_recursive 函数。
 */
inline lamp_ui barrett_2powN(lamp_ui N, lamp_ptr in, lamp_ui len, lamp_ptr out) {
    assert(in != nullptr && len > 0);
    assert(N >= 2 * len);

    lamp_ui carry_flag = in[len - 1] & (in[len - 1] - 1);
    for (lamp_ui i = 0; i < len - 1; i++) {
        carry_flag |= in[i];
    }
    if (carry_flag == 0) {
    /*
     * carry_flag == 0 表示 in 为二的幂
     */    
        lamp_ui in_bits = len * 64 - lammp_clz(in[len - 1]);
        lamp_ui out_bits = 64 * N + 1 - in_bits;
        lamp_ui word_shr = out_bits / 64;
        lamp_ui bit_shr = out_bits % 64;
        std::fill(out, out + word_shr, 0);
        out[word_shr] = 1ull << bit_shr;
        return rlz(out, word_shr + 1);
    }

    lamp_ui offset = N - 2 * len;

    lamp_ui _in_len = len + 2 + offset;
    _internal_buffer<0> _in(_in_len + 1, 0);
    std::copy(in, in + len, _in.data() + 2 + offset);
    _internal_buffer<0> _out(_in_len + 2, 0);
    lamp_ui _out_len = barrett_2powN_recursive(_in.data(), _in_len, _out.data());
    lamp_ui one[1] = {1};
    std::copy(_out.data() + 2, _out.data() + _out_len, out);
    lamp_ui out_len = rlz(out, _out_len - 2);
    abs_add_binary(out, _out_len, one, 1, out);
    return rlz(out, out_len + 1);
}



namespace Numeral {
namespace BaseTable {

// 第一列为基数，第二列为基数的幂次数
constexpr lamp_ui table1[35][2] = {
    {9223372036854775808ull, 63ull},  {12157665459056928801ull, 40ull}, {4611686018427387904ull, 31ull},
    {7450580596923828125ull, 27ull},  {4738381338321616896ull, 24ull},  {3909821048582988049ull, 22ull},
    {9223372036854775808ull, 21ull},  {12157665459056928801ull, 20ull}, {10000000000000000000ull, 19ull},
    {5559917313492231481ull, 18ull},  {2218611106740436992ull, 17ull},  {8650415919381337933ull, 17ull},
    {2177953337809371136ull, 16ull},  {6568408355712890625ull, 16ull},  {1152921504606846976ull, 15ull},
    {2862423051509815793ull, 15ull},  {6746640616477458432ull, 15ull},  {15181127029874798299ull, 15ull},
    {1638400000000000000ull, 14ull},  {3243919932521508681ull, 14ull},  {6221821273427820544ull, 14ull},
    {11592836324538749809ull, 14ull}, {876488338465357824ull, 13ull},   {1490116119384765625ull, 13ull},
    {2481152873203736576ull, 13ull},  {4052555153018976267ull, 13ull},  {6502111422497947648ull, 13ull},
    {10260628712958602189ull, 13ull}, {15943230000000000000ull, 13ull}, {787662783788549761ull, 12ull},
    {1152921504606846976ull, 12ull},  {1667889514952984961ull, 12ull},  {2386420683693101056ull, 12ull},
    {3379220508056640625ull, 12ull},  {4738381338321616896ull, 12ull}};
// 用以计算进制转换后的数据长度，根据对数计算
constexpr double table2[35] = {
    1.015873015873016e+00, 1.009487605714332e+00, 1.032258064516129e+00, 1.020862952470265e+00, 1.031607485958778e+00,
    1.036239089768792e+00, 1.015873015873016e+00, 1.009487605714332e+00, 1.013995774868147e+00, 1.027786049130268e+00,
    1.050138148333665e+00, 1.017367169608733e+00, 1.050598140148774e+00, 1.023832099239262e+00, 1.066666666666667e+00,
    1.043842313037765e+00, 1.023199857357361e+00, 1.004411363697657e+00, 1.057728974444613e+00, 1.040778279757500e+00,
    1.025114624994631e+00, 1.010581620377160e+00, 1.073744206698002e+00, 1.060126912180660e+00, 1.047365186724249e+00,
    1.039781450100194e+00, 1.031607485958778e+00};
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

}  // namespace BaseTable

// 返回逆序的字符串，低位数字在前
// 对于10进制以上的进制，使用大写字母A-Z，A表示10，B表示11，以此类推
inline std::string to_string_base(lamp_ui num, const lamp_ui base, const lamp_ui base_len) {
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
inline lamp_ui num2base_classic(lamp_ptr in, lamp_ui len, const lamp_ui base_num, lamp_ptr res) {
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
inline lamp_ui base2num_classic(lamp_ptr in, lamp_ui len, const lamp_ui base_num, lamp_ptr res) {
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
inline lamp_ui _2_64power_index_classic(const lamp_ui base_num, const lamp_ui index, lamp_ptr res) {
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
inline lamp_ui base_power_index_classic(const lamp_ui base_num, const lamp_ui index, lamp_ptr res) {
    lamp_ui len = index + 1;
    _internal_buffer<0> base_pow_index(len, 0);
    base_pow_index.set(index, 1);
    return base2num_classic(base_pow_index.data(), len, base_num, res);
}

// 进制转换后的长度，额外加一防止溢出
inline lamp_ui get_buffer_size(lamp_ui len, double base_d) { return static_cast<lamp_ui>(std::ceil(base_d * len)) + 1; }

// 计算 in * 2^64，并存储在 in 数组中，in 数组会被修改
// in 数组为 base_num 进制下的数
inline void abs_mul2pow64_base(lamp_ptr in, lamp_ui len, const lamp_ui base_num) {
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
inline void abs_mul_base(lamp_ptr in, lamp_ui len, const lamp_ui base_num) {
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
inline lamp_ui _2_64_power_index(const lamp_ui base_num, const lamp_ui index, lamp_ptr res, lamp_ui res_len) {
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
inline lamp_ui base_power_index(const lamp_ui base_num, const lamp_ui index, lamp_ptr res, lamp_ui res_len) {
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

typedef struct base_index_node {
    lamp_ui index;
    lamp_ui length;
    base_index_node* front;
    base_index_node* back;
    _internal_buffer<0> base_index;

    base_index_node(lamp_ui _index, double base_d) {
        index = _index;
        length = get_buffer_size(_index, base_d);
        base_index.resize(length);
        front = nullptr;
        back = nullptr;
    }

}* _2pow64_index_list;

typedef struct base_index_node* _base_index_list;

constexpr size_t MIN_LEN = 64ull;

// 只能处理 len 为二的次幂的情况，in 会被修改
// 将2^64进制转换为base_num进制，并存储在out数组中
inline lamp_ui num_base_recursive_core(
              lamp_ptr       in,    
              lamp_ui        len,
    const     lamp_ui        base_num,
    const     double         base_d,
              lamp_ptr       out,
    const _2pow64_index_list list
) {
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
inline _2pow64_index_list create_2pow64_index_list(lamp_ui max_index, const lamp_ui base_num, const double base_d) {
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
inline lamp_ui base_num_recursive_core(
               lamp_ptr    in,
               lamp_ui     len,
    const      lamp_ui     base_num,
    const      double      base_d,
               lamp_ptr    out,
    const _base_index_list list
) {
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
inline _base_index_list create_base_index_list(lamp_ui max_index, const lamp_ui base_num, const double base_d) {
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
inline _2pow64_index_list find_head(_2pow64_index_list head, lamp_ui index) {
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
inline lamp_ui num2base(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res) {
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
inline lamp_ui base2num(lamp_ptr in, lamp_ui len, const lamp_ui base, lamp_ptr res) {
    BaseTable::BaseInfo info(base);
    info.base_d_inv();
    const lamp_ui max_len_base_index = 63ull - lammp_clz(len);

    if (len <= 2 * MIN_LEN) {
        return base2num_classic(in, len, info.base_num, res);
    }
    _base_index_list head = create_base_index_list(1ull << max_len_base_index, info.base_num, info.base_d);
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

}  // namespace Numeral

};  // namespace Arithmetic

};  // namespace lammp
#define __LAMMP_HPP__
#endif  // __LAMMP_HPP__