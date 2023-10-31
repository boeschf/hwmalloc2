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
struct not_pinned : public Resource {

    not_pinned(Resource&& r) : Resource{std::move(r)} {}

    not_pinned(not_pinned&&) noexcept = default;
};

} // namespace res
} // namespace hwmalloc2

