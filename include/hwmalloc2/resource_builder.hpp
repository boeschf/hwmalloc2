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
#include <hwmalloc2/concepts.hpp>
#include <hwmalloc2/resource/sentinel.hpp>
#include <hwmalloc2/resource/not_memory.hpp>
#include <hwmalloc2/resource/host_memory.hpp>
#include <hwmalloc2/resource/user_host_memory.hpp>
#include <hwmalloc2/resource/pinned.hpp>
#include <hwmalloc2/resource/not_pinned.hpp>
#include <hwmalloc2/resource/registered.hpp>
#include <hwmalloc2/resource/not_registered.hpp>
#include <hwmalloc2/resource/arena.hpp>
#include <hwmalloc2/resource/not_arena.hpp>
#include <hwmalloc2/any_resource.hpp>

#include <tuple>

namespace hwmalloc2 {

namespace detail {

// default resource nesting for resource builder
using default_resource =
    res::not_arena<
        res::not_registered<
            res::not_pinned<
                res::not_memory<
                    res::sentinel
                >
            >
        >
    >;

// default arguments for resource builder
using default_args = std::tuple<
    std::tuple<>,
    std::tuple<>,
    std::tuple<>,
    std::tuple<>>;


// replace a resource class template at postion I in a nested resource type
// example:
// let res_orig = res0<res1<res2<res3<...>, ...>, ...>, ...>
// let MyReplacementResource = MyReplacementResource<NestedResource, U1, U2, ...>
// where U1, U2, ... are additional template arguments
// then
// replace_resource_t<2, res_orig, MyReplacementResource, U1, U2, ...> -> res_new
// res_new == res0<res1<MyReplacementResource<res3<...>, U1, U2, ...>, ...>, ...>

// primary class template declaration
template <std::size_t I, typename Nested, template<typename...> typename R, typename... M>
struct replace_resource;

// partial specialization: Nested is a class template with template paramters Inner, More...
template <std::size_t I, template <typename...> typename Nested, template<typename...> typename R, typename Inner, typename... More, typename... M>
struct replace_resource<I, Nested<Inner, More...>, R, M...> {
    // compute type recursively by decrementing I and use Inner as new Nested class template
    using type = Nested<typename replace_resource<I-1, Inner, R, M...>::type, More...>;
};

// partial specialization for I==0 (recursion end point)
template <template <typename...> typename Nested, template<typename...> typename R, typename Inner, typename... More, typename... M>
struct replace_resource<0, Nested<Inner, More...>, R, M...> {
    // inject the class template R at this point instead of Nested but keep Inner
    using type = R<Inner, M...>;
};

// helper alias to extract the member typedef `type` from the `replace_resource` struct
template <std::size_t I, typename Nested, template<typename...> typename R, typename... M>
using replace_resource_t = typename replace_resource<I, Nested, R, M...>::type;


// replace the tuple at postion I in a tuple of tuples (which stores arguments to build the nested resource)
// example:
// let tuple_orig = {{a_00, a_01, ...}, {a_10, a_11, ...}, {a_20, a_21, ...}, ...}
// let replacement_tuple = {b0, b1, ...}
// then
// replace_arg<1>(std::move(tuple_orig), std::move(replacement_tuple)) -> tuple_new
// tuple_new == {{a_00, a_01, ...}, {b0, b1, ...}, {a_20, a_21, ...}, ...}

// helper function with indices to look up elements within the tuple `args`
template<std::size_t I, typename... Ts, typename... Us, std::size_t... Is, std::size_t... Js>
constexpr inline auto replace_arg(std::tuple<Ts...>&& args, std::tuple<Us...>&& arg, std::index_sequence<Is...>, std::index_sequence<Js...>) {
    // take items 0, 1, ..., I-1, I+1, I+2, ... from `args` and replace item I with `arg`
    return std::make_tuple(std::move(std::get<Is>(args))..., std::move(arg), std::move(std::get<I+1+Js>(args))...);
}

// replace element at position I of `args` with `arg`
template<std::size_t I, typename... Ts, typename... Us>
constexpr inline auto replace_arg(std::tuple<Ts...> args, std::tuple<Us...> arg) {
    // dispatch to helper function by additionally passing indices
    return replace_arg<I>(std::move(args), std::move(arg), std::make_index_sequence<I>{}, std::make_index_sequence<sizeof...(Ts) - 1 - I>{});
}

// instantiate a neested resource from arguments in the form of a tuple of tuples 
// example:
// let nested = res0<res1<res2<...>, ...>, ...>
// let args   = {{a_00, a_01, ...}, {a_10, a_11, ...}, {a_20, a_21, ...}, ...}
// then
// nested_resource<nested>::instantiate(std::move(args)) evaluates to a concatenation of constructors:
// return 
//     res0<res1<res2<...>,...>{
//         res1<res2<...>,...>{
//             res2<...>{
//                 ...,
//                 a_20, a_21, ...},
//             a_10, a_11, ...},
//         a_00, a_01, ...};

// primary class template declaration with Index I defaulted to 0
template <typename N, std::size_t I = 0>
struct nested_resource;

// partial specialization: N is a class template with template paramters Inner, More...
template <template<typename...> typename N, typename Inner, typename... More, std::size_t I>
struct nested_resource<N<Inner, More...>, I> {

    // instantiate N<Inner, More...> with the element at postion I of the tuple
    template<typename... Ts>
    static constexpr auto instantiate(const std::tuple<Ts...>& args) {
        // dispatch to helper function by additionally passing indices enumerating the arguments within the tuple at postion I in `args`
        return instantiate(args, std::make_index_sequence<std::tuple_size_v<std::tuple_element_t<I, std::tuple<Ts...>>>>{});
    }

    // helper function with indices to look up elements within tuple extracted with std::get<I>(args)
    template<typename... Ts, std::size_t... Is>
    static constexpr auto instantiate(const std::tuple<Ts...>& args, std::index_sequence<Is...>) {
        auto arg = std::get<I>(args);
        return N<Inner, More...>{
            // first constructor argument is the next nested resource (recurse)
            nested_resource<Inner, I+1>::instantiate(std::move(args)),
            // further constructor arguments are tagen from tuple of tuples
            //std::get<Is>(std::get<I>(std::move(args)))...
            std::get<Is>(std::move(arg))...
        };
    }
};

// partial specialization when the resource is the sentinel (recursion end point)
template<std::size_t I>
struct nested_resource<::hwmalloc2::res::sentinel, I> {

    // return a default constructed sentinel
    template<typename... Ts>
    static constexpr auto instantiate(const std::tuple<Ts...>& args) {
        return ::hwmalloc2::res::sentinel{};
    }
};

} // namespace detail


// resource_builder class template
// template type arguments:
//   - Resource: the type of the resource (nested chain of resources)
//   - Args: type of arguments to construct the nested resource (tuple of tuples)
// member functions (apart from build())
//   - return a new instance of the resource_builder class template with potentially altered template type arguments
//   - which holds an updated argument tuple
// the build() member function
//   - returns a nested resource
//   - which is constructed from the `args` tuple of tuples
template<typename Resource = detail::default_resource, typename Args = detail::default_args>
struct _resource_builder {

    using resource_t = Resource;
    using args_t = Args;

    const args_t args;

    constexpr _resource_builder() noexcept = default;
    constexpr _resource_builder(args_t&& a) : args{std::move(a)} {}
    constexpr _resource_builder(const _resource_builder&) = default;
    constexpr _resource_builder(_resource_builder&&) = default;

    constexpr auto add_arena() const {
        // arena resources are stored at position 0 in the resource nest
        return updated<0, res::arena>(std::tuple<>{});
    }

    template<Registry R>
    constexpr auto register_memory(R& registry) const {
        // registered resources are stored at position 1 in the resource nest
        return updated<1, res::registered, R>(std::tuple<R&>{registry});
    }

    constexpr auto pin() const {
        // pinned resources are stored at position 2 in the resource nest
        return updated<2, res::pinned>(std::tuple<>{});
    }

    constexpr auto alloc_on_host(std::size_t s) const {
        // memory resources are stored at position 3 in the resource nest
        return updated<3, res::host_memory>(std::make_tuple(s));
    }

    constexpr auto use_host_memory(void* p, std::size_t s) const {
        // memory resources are stored at position 3 in the resource nest
        return updated<3, res::user_host_memory>(std::make_tuple(p, s));
    }

    constexpr auto build() const { return detail::nested_resource<resource_t>::instantiate(args); }

    constexpr auto build_any() const { return any_resource{build()}; }

  private:
    template<std::size_t I, template<typename...> typename R, typename... M, typename Arg>
    constexpr auto updated(Arg arg) const {
        // create a new nested resource type by replacing the old resource class template
        using R_new = detail::replace_resource_t<I, resource_t, R, M...>;
        // create new arguments by replacing the old argument tuple
        auto args_new = detail::replace_arg<I>(args, arg);
        // return new _resource_builder class template instantiation
        return _resource_builder<R_new, decltype(args_new)>{std::move(args_new)};
    }
};

inline constexpr auto resource_builder() { return _resource_builder<>{}; }

inline auto host_resource(std::size_t s) {
    static constexpr auto b = resource_builder().pin().add_arena();
    return b.alloc_on_host(4096).build();
}

inline auto host_resource(void* p, std::size_t s) {
    static constexpr auto b = resource_builder().pin();
    return b.use_host_memory(p, s).build();
};

} // namespace hwmalloc2

