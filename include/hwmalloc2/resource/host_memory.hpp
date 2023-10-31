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
struct host_memory : public Resource {

    std::unique_ptr<std::byte[]> _mem;
    std::size_t _size;

    host_memory(Resource&& r, std::size_t s) : Resource{std::move(r)}, _mem{ new std::byte[s] }, _size{s} {}

    host_memory(host_memory&&) noexcept = default;

    inline void* data() const noexcept { return _mem.get(); }

    inline auto size() const noexcept { return _size; }

    inline operator bool() const noexcept { return (bool)_mem; }

};

} // namespace res
} // namespace hwmalloc2

