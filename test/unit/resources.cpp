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

template<
    typename Res,
    typename Arena,
    typename Registered,
    typename Pinned,
    typename Memory>
struct is_same_resource : public std::false_type {};

template<template<typename...> typename R, typename... M>
struct r_t {};

template<
    template<typename...> typename Arena,
    template<typename...> typename Registered,
    template<typename...> typename Pinned,
    template<typename...> typename Memory,
    typename... M0,
    typename... M1,
    typename... M2,
    typename... M3>
struct is_same_resource<
    Arena<Registered<Pinned<Memory<hwmalloc2::res::sentinel,M3...>,M2...>,M1...>,M0...>,
    r_t<Arena, M0...>,
    r_t<Registered, M1...>,
    r_t<Pinned, M2...>,
    r_t<Memory, M3...>>
: public std::true_type {};

template<
    typename Res,
    typename Arena,
    typename Registered,
    typename Pinned,
    typename Memory>
inline constexpr bool is_same_resource_v = is_same_resource<Res, Arena, Registered, Pinned, Memory>::value;

template<
    typename Args,
    typename A0,
    typename A1,
    typename A2,
    typename A3>
struct are_same_args : public std::false_type {};

template<typename... A>
struct a_t {};

template<
    typename... A0,
    typename... A1,
    typename... A2,
    typename... A3>
struct are_same_args<
    std::tuple<std::tuple<A0...>, std::tuple<A1...>, std::tuple<A2...>, std::tuple<A3...>>,
    a_t<A0...>,
    a_t<A1...>,
    a_t<A2...>,
    a_t<A3...>>
: public std::true_type {};

template<
    typename Args,
    typename A0,
    typename A1,
    typename A2,
    typename A3>
inline constexpr bool are_same_args_v = are_same_args<Args, A0, A1, A2, A3>::value;

template<
    typename B,
    typename Arena,
    typename Registered,
    typename Pinned,
    typename Memory,
    typename A0,
    typename A1,
    typename A2,
    typename A3>
inline constexpr bool check_builder_v = std::integral_constant<bool,
    is_same_resource_v<typename B::resource_t, Arena, Registered, Pinned, Memory> &&
    are_same_args_v<typename B::args_t, A0, A1, A2, A3>>::value;

template<
    typename Arena,
    typename Registered,
    typename Pinned,
    typename Memory,
    typename A0,
    typename A1,
    typename A2,
    typename A3,
    typename F>
inline void check_builder(F f) {
    auto b = f();
    static_assert(check_builder_v<
        decltype(b),
        Arena,
        Registered,
        Pinned,
        Memory,
        A0,
        A1,
        A2,
        A3
    >);
}


TEST_CASE( "host memory2", "[concat]" ) {

    test_registry r;

    {
        using namespace hwmalloc2;

        res::host_memory m0{res::sentinel{}, 4096};
        res::pinned      m1{std::move(m0)};
        res::registered  m2{std::move(m1), r};
        res::arena       m3{std::move(m2)};

        static_assert(is_same_resource_v<decltype(m3),
            r_t<res::arena>,
            r_t<res::registered, test_registry>,
            r_t<res::pinned>,
            r_t<res::host_memory>>);

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

        static_assert(is_same_resource_v<decltype(m3),
            r_t<res::not_arena>,
            r_t<res::registered, test_registry>,
            r_t<res::pinned>,
            r_t<res::user_host_memory>>);

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

        check_builder<
            r_t<res::not_arena>,
            r_t<res::not_registered>,
            r_t<res::not_pinned>,
            r_t<res::host_memory>,
            a_t<>,
            a_t<>,
            a_t<>,
            a_t<std::size_t>
        >([](){ return resource_builder().alloc_on_host(4096); });

        check_builder<
            r_t<res::not_arena>,
            r_t<res::not_registered>,
            r_t<res::pinned>,
            r_t<res::not_memory>,
            a_t<>,
            a_t<>,
            a_t<>,
            a_t<>
        >([](){ return resource_builder().pin(); });

        check_builder<
            r_t<res::not_arena>,
            r_t<res::registered, test_registry>,
            r_t<res::not_pinned>,
            r_t<res::not_memory>,
            a_t<>,
            a_t<test_registry&>,
            a_t<>,
            a_t<>
        >([&r](){ return resource_builder().register_memory(r); });

        check_builder<
            r_t<res::arena>,
            r_t<res::not_registered>,
            r_t<res::not_pinned>,
            r_t<res::not_memory>,
            a_t<>,
            a_t<>,
            a_t<>,
            a_t<>
        >([](){ return resource_builder().add_arena(); });
    }

    {
        using namespace hwmalloc2;

        auto m = resource_builder()
            .register_memory(r)
            .build();

        static_assert(is_same_resource_v<decltype(m),
            r_t<res::not_arena>,
            r_t<res::registered, test_registry>,
            r_t<res::not_pinned>,
            r_t<res::not_memory>>);

        void* my_ptr = m.allocate(128);
        auto k = m.get_key(my_ptr, 128);
        m.deallocate(my_ptr, 128);
    }

    {
        using namespace hwmalloc2;

        auto m = host_resource(4096);

        static_assert(is_same_resource_v<decltype(m),
            r_t<res::arena>,
            r_t<res::not_registered>,
            r_t<res::pinned>,
            r_t<res::host_memory>>);

        void* my_ptr = m.allocate(128);
        auto k = m.get_key(my_ptr, 128);
        m.deallocate(my_ptr, 128);
    }
}
