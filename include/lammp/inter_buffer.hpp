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

#ifndef __LAMMP_INTER_BUFFER_HPP__
#define __LAMMP_INTER_BUFFER_HPP__
#include "alloc.h"
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace lammp {

// 默认分配器
class _default_allocator {
   public:
    static void* allocate(size_t size, size_t alignment) { return LAMMP_ALLOC(alignment, size); }

    static void deallocate(void* ptr) { return LAMMP_FREE(ptr); }
};

// 主模板 - 适用于 StackCapacity > 0 的情况
template <size_t StackCapacity, size_t Alignment = 64, typename Allocator = _default_allocator>
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

};  // namespace lammp
#endif  // __LAMMP_INTER_BUFFER_HPP__
