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

namespace hwmalloc2 {
namespace res {

template<typename Memory>
struct arena : public Memory {

    arena(Memory&& mem)
    : Memory{std::move(mem)}
    {
        // arena construction
    }

    arena(arena&&) noexcept = default;

    ~arena() {
        // arena cleanup
    }

    template<std::size_t Alignment = alignof(std::max_align_t)>
    void* allocate(std::size_t s) {
        if constexpr (Alignment <= alignof(std::max_align_t)) {
            // void* ptr = arena.allocate(s); // allocate `s` from arena
            return this->data();
        }
        else {
            // std::size_t space = s + Alignment + sizeof(void*) - 1;
            // std::size_t size = s + sizeof(void*);
            // void* ptr = ...; // allocate `space` from arena
            // void* orig_ptr = ptr;
            // void* aligned_ptr = std::align(Alignment, size, ptr, space);
            // ::new((unsigned char*)aligned_ptr + s) void*{orig_ptr};
            // return aligned_ptr;
            return this->data();
        }
    }

    template<std::size_t Alignment = alignof(std::max_align_t)>
    void deallocate(void* ptr, std::size_t s) {
        if constexpr (Alignment <= alignof(std::max_align_t)) {
            // arena.deallocate(ptr, s); // deallocate `s` from arena
        }
        else {
            // std::size_t space = s + Alignment + sizeof(void*) - 1;
            // void* orig_ptr = *reinterpret_cast<void**>(reinterpret_cast<unsigned char*>(ptr)+s);
            // arena.deallocate(orig_ptr, space); // dallocate `space` from arena
        }
    }
};

} // namespace res
} // namespace hwmalloc2

