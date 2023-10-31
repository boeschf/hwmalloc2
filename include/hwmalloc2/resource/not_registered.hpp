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


namespace hwmalloc2 {
namespace res {

template<typename Resource>
struct not_registered : public Resource {

    struct key {
        void* ptr;
        std::size_t size;
    };

    not_registered(Resource&& r) : Resource{std::move(r)} {}

    not_registered(not_registered&&) noexcept = default;

    key get_key(void* ptr, std::size_t s) const { return {ptr, s}; }
};

} // namespace res
} // namespace hwmalloc2

