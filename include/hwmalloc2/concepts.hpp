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

#include <hwmalloc2/config.hpp>

#include <concepts>

namespace hwmalloc2 {

template<typename T>
concept Key = std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;

template<typename T>
concept Region = requires (T const& r, void* ptr, std::size_t s) {
    requires std::movable<T>;
    {r.get_key(ptr, s)} -> Key;
};

template<typename T>
concept Registry = requires (T& r, void* ptr, std::size_t s) {
    {r.register_memory(ptr, s)} -> Region;
};

} // namespace hwmalloc2
