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
#include <any>
#include <memory>

namespace hwmalloc2 {

// type-erases resource and dispatches to allocation function through
// virtual function calls
// the keys are wrapped in a std::any object
class any_resource {

  private:
    struct iface {
        virtual void* allocate(std::size_t, std::size_t) = 0;
        virtual void deallocate(void*, std::size_t, std::size_t) = 0;
        virtual std::any get_key(void*, std::size_t) = 0;
        virtual ~iface() = default;
    };

    template<typename R>
    struct pimpl : public iface {
        R _impl;
        pimpl(R r) noexcept : _impl{std::move(r)} {}
        ~pimpl() override final = default;
        void* allocate(std::size_t s, std::size_t a) override final { return _impl.allocate(s, a); }
        void deallocate(void* p, std::size_t s, std::size_t a) override final { _impl.deallocate(p, s, a); }
        std::any get_key(void* p, std::size_t s) override final { return _impl.get_key(p, s); }
    };

    std::unique_ptr<iface> _r;

  public:
    template<typename R>
    any_resource(R r) : _r{ ::new pimpl<R>{std::move(r)}} {}

    void* allocate(std::size_t s, std::size_t a = alignof(std::max_align_t)) { return _r->allocate(s, a); }

    void deallocate(void* p, std::size_t s, std::size_t a = alignof(std::max_align_t)) { _r->deallocate(p, s, a); }

    std::any get_key(void* p, std::size_t s) { return _r->get_key(p, s); }
};

} // namespace hwmalloc2

