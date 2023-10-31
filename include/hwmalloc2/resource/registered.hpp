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

#include <hwmalloc2/concepts.hpp>

namespace hwmalloc2 {
namespace res {

template<typename Resource, Registry R>
struct registered : public Resource {

    using region = std::decay_t<decltype(std::declval<R>().register_memory(nullptr, 0u))>;
    using key = std::decay_t<decltype(std::declval<region>().get_key(nullptr, 0u))>;

    region _region;

    registered(Resource&& r, R& registry)
    : Resource{std::move(r)}
    , _region{registry.register_memory(this->data(), this->size())}
    {}

    registered(registered&&) noexcept = default;

    // deregister: implicitely done in region destructor
    //~registered() {}
    
    key get_key(void* ptr, std::size_t s) const { return _region.get_key(ptr, s); }
};

} // namespace res
} // namespace hwmalloc2

