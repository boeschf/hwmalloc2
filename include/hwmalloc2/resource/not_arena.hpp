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

template<typename Resource>
struct not_arena : public Resource {

    not_arena(Resource&& r) : Resource{std::move(r)} {}

    not_arena(not_arena&&) noexcept = default;

    void* allocate(std::size_t s, std::size_t alignment = alignof(std::max_align_t)) {
        if (alignment <= alignof(std::max_align_t)) {
            return (s > this->size()) ? nullptr : this->data();
        }
        else {
            std::size_t space = s + alignment + sizeof(void*) - 1;
            if (space > this->size()) return nullptr;
            std::size_t size = s + sizeof(void*);
            void* ptr = this->data();
            void* aligned_ptr = std::align(alignment, size, ptr, space);
            return aligned_ptr;
        }
    }

    void deallocate(void*, std::size_t, std::size_t = alignof(std::max_align_t)) {
        // do nothing
    }
};

} // namespace res
} // namespace hwmalloc2

