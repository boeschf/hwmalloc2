/*
 * ghex-org
 *
 * Copyright (c) 2014-2023, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <cstddef>
#include <memory>
#include <utility>

namespace hwmalloc2 {
namespace res {

struct user_host_memory {

    void* _mem;
    std::size_t _size;

    user_host_memory(void* ptr, std::size_t s) : _mem{ptr}, _size{s} {}

    user_host_memory(user_host_memory&& other) noexcept
    : _mem{std::exchange(other._mem, nullptr)}
    , _size{std::exchange(other._size, 0u)}
    {}

    inline void* data() const noexcept { return _mem; }

    inline auto size() const noexcept { return _size; }

    inline operator bool() const noexcept { return (bool)_mem; }

    template<std::size_t Alignment = alignof(std::max_align_t)>
    void* allocate(std::size_t s) {
        if constexpr (Alignment <= alignof(std::max_align_t)) {
            return (s > _size) ? nullptr : _mem;
        }
        else {
            std::size_t space = s + Alignment + sizeof(void*) - 1;
            if (space > _size) return nullptr;
            std::size_t size = s + sizeof(void*);
            void* ptr = _mem;
            void* aligned_ptr = std::align(Alignment, size, ptr, space);
            return aligned_ptr;
        }
    }

    template<std::size_t Alignment = alignof(std::max_align_t)>
    void deallocate(void*, std::size_t) {
        // do nothing
    }

};

} // namespace res
} // namespace hwmalloc2

