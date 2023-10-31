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
#include <hwmalloc2/resource/sentinel.hpp>
#include <hwmalloc2/resource/host_memory.hpp>
#include <hwmalloc2/resource/user_host_memory.hpp>
#include <hwmalloc2/resource/pinned.hpp>
#include <hwmalloc2/resource/not_pinned.hpp>
#include <hwmalloc2/resource/registered.hpp>
#include <hwmalloc2/resource/not_registered.hpp>
#include <hwmalloc2/resource/arena.hpp>
#include <hwmalloc2/resource/not_arena.hpp>
#include <hwmalloc2/any_resource.hpp>

#include <hwmalloc2/resource_builder.hpp>

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

        res::host_memory m0{res::sentinel{}, 4096};
        res::pinned      m1{std::move(m0)};
        res::registered  m2{std::move(m1), r};
        res::arena       m3{std::move(m2)};

        static_assert(
            std::is_same_v<
                decltype(m3),
                res::arena<
                    res::registered<
                        res::pinned<
                            res::host_memory<
                                res::sentinel
                            >
                        >,
                        test_registry
                    >
                >
            >
        );

        void* my_ptr = m3.allocate(128);
        auto k = m3.get_key(my_ptr, 128);
        m3.deallocate(my_ptr, 128);

        any_resource m4{std::move(m3)};

        void* my_ptr2 = m4.allocate(128);
        auto k2 = m4.get_key(my_ptr2, 128);
        m4.deallocate(my_ptr2, 128);
    }

    {
        using namespace hwmalloc2;

        std::vector<char>     v(128);
        res::user_host_memory m0{res::sentinel{}, v.data(), v.size()};
        res::pinned           m1{std::move(m0)};
        res::registered       m2{std::move(m1), r};
        res::not_arena        m3{std::move(m2)};

        void* my_ptr = m3.allocate(128);
        auto k = m3.get_key(my_ptr, 128);
        m3.deallocate(my_ptr, 128);

        any_resource m4{std::move(m3)};

        void* my_ptr2 = m4.allocate(128);
        auto k2 = m4.get_key(my_ptr2, 128);
        m4.deallocate(my_ptr2, 128);
    }

    {
        using namespace hwmalloc2;

        auto b = resource_builder();

        auto b1 = b.alloc_on_host(4096);
        static_assert(
            std::is_same_v<
                decltype(b1)::resource_t,
                res::not_arena<
                    res::not_registered<
                        res::not_pinned<
                            res::host_memory<
                                res::sentinel
                            >
                        >
                    >
                >
            >
        );
        static_assert(
            std::is_same_v<
                decltype(b1)::args_t,
                std::tuple<
                    std::tuple<>,
                    std::tuple<>,
                    std::tuple<>,
                    std::tuple<std::size_t>
                >
            >
        );

        auto b2 = b.pin();
        static_assert(
            std::is_same_v<
                decltype(b2)::resource_t,
                res::not_arena<
                    res::not_registered<
                        res::pinned<
                            res::not_memory<
                                res::sentinel
                            >
                        >
                    >
                >
            >
        );
        static_assert(
            std::is_same_v<
                decltype(b2)::args_t,
                std::tuple<
                    std::tuple<>,
                    std::tuple<>,
                    std::tuple<>,
                    std::tuple<>
                >
            >
        );

        auto b3 = b.register_memory(r);
        static_assert(
            std::is_same_v<
                decltype(b3)::resource_t,
                res::not_arena<
                    res::registered<
                        res::not_pinned<
                            res::not_memory<
                                res::sentinel
                            >
                        >,
                        test_registry
                    >
                >
            >
        );
        static_assert(
            std::is_same_v<
                decltype(b3)::args_t,
                std::tuple<
                    std::tuple<>,
                    std::tuple<test_registry&>,
                    std::tuple<>,
                    std::tuple<>
                >
            >
        );

        auto b4 = b.add_arena();
        static_assert(
            std::is_same_v<
                decltype(b4)::resource_t,
                res::arena<
                    res::not_registered<
                        res::not_pinned<
                            res::not_memory<
                                res::sentinel
                            >
                        >
                    >
                >
            >
        );
        static_assert(
            std::is_same_v<
                decltype(b4)::args_t,
                std::tuple<
                    std::tuple<>,
                    std::tuple<>,
                    std::tuple<>,
                    std::tuple<>
                >
            >
        );
    }

    {
        using namespace hwmalloc2;

        auto m = resource_builder()
            .register_memory(r)
            .build();

        static_assert(
            std::is_same_v<
                decltype(m),
                res::not_arena<
                    res::registered<
                        res::not_pinned<
                            res::not_memory<
                                res::sentinel
                            >
                        >,
                        test_registry
                    >
                >
            >
        );

        void* my_ptr = m.allocate(128);
        auto k = m.get_key(my_ptr, 128);
        m.deallocate(my_ptr, 128);
    }
}
