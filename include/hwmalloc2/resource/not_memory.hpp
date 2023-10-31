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

namespace hwmalloc2 {
namespace res {

template<typename Resource>
struct not_memory : public Resource {

    not_memory(Resource&& r) : Resource{std::move(r)} {}

    not_memory(not_memory&&) noexcept = default;

    inline void* data() const noexcept { return nullptr; }

    inline std::size_t size() const noexcept { return 0u; }

    inline operator bool() const noexcept { return false; }

};

} // namespace res
} // namespace hwmalloc2

