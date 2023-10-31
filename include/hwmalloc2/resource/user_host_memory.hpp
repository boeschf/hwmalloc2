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

template<typename Resource>
struct user_host_memory : public Resource {

    void* _mem;
    std::size_t _size;

    user_host_memory(Resource&& r, void* ptr, std::size_t s) : Resource{std::move(r)}, _mem{ptr}, _size{s} {}

    user_host_memory(user_host_memory&& other) noexcept
    : Resource{std::move(other)}
    , _mem{std::exchange(other._mem, nullptr)}
    , _size{std::exchange(other._size, 0u)}
    {}

    inline void* data() const noexcept { return _mem; }

    inline auto size() const noexcept { return _size; }

    inline operator bool() const noexcept { return (bool)_mem; }
};

} // namespace res
} // namespace hwmalloc2

