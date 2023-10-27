/*
 * ghex-org
 *
 * Copyright (c) 2014-2023, ETH Zurich
 * All rights reserved.
 *
 * Please, refer to the LICENSE file in the root directory.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <hwmalloc2/concepts.hpp>
#include <hwmalloc2/resource/host_memory.hpp>
#include <hwmalloc2/resource/user_host_memory.hpp>
#include <hwmalloc2/resource/pinned.hpp>
#include <hwmalloc2/resource/registered.hpp>
#include <hwmalloc2/resource/arena.hpp>

#include <vector>

#include <catch2/catch_test_macros.hpp>

struct test_registry {

    struct test_key {
        test_key(void*, std::size_t) {}  
    };

    struct test_region {
        test_region(void*, std::size_t) {}  
        auto get_key(void*, std::size_t) const { return test_key{nullptr, 4096}; }
    };

    auto register_memory(void* ptr, std::size_t s) {
        return test_region{ptr, s};
    }
};

TEST_CASE( "Simple Concept Check2", "[concepts]" ) {
    static_assert(hwmalloc2::Registry<test_registry>);
}

TEST_CASE( "host memory2", "[concat]" ) {

    test_registry r;

    {
        using namespace hwmalloc2;

        res::host_memory m0{4096};
        res::pinned      m1{std::move(m0)};
        res::registered  m2{std::move(m1), r};
        res::arena       m3{std::move(m2)};

        static_assert(
            std::is_same_v<
                decltype(m3),
                res::arena<
                    res::registered<
                        res::pinned<
                            res::host_memory
                        >,
                        test_registry
                    >
                >
            >
        );

        void* my_ptr = m3.allocate(128);
        auto k = m3.get_key(my_ptr, 128);
        m3.deallocate(my_ptr, 128);
    }

    {
        using namespace hwmalloc2;

        std::vector<char>     m0(128);
        res::user_host_memory m1{m0.data(), m0.size()};
        res::pinned           m2{std::move(m1)};
        res::registered       m3{std::move(m2), r};

        void* my_ptr = m3.allocate(128);
        auto k = m3.get_key(my_ptr, 128);
        m3.deallocate(my_ptr, 128);
    }
}
