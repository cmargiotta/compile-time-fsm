/**
 * @file function_signature.hpp
 * @author Carmine Margiotta (car.margiotta@icloud.com)
 *
 * @copyright Copyright (c) 2022
 */

#ifndef TYPE_TRAITS_FUNCTION_SIGNATURE_HPP
#define TYPE_TRAITS_FUNCTION_SIGNATURE_HPP

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace ctfsm
{
    /**
     * @brief Get the return type of the given signature
     *
     * @tparam signature
     */
    template<typename signature>
    struct return_type;

    template<typename return_, typename... args>
    struct return_type<return_(args...)>
    {
            using type = return_;
    };

    template<typename return_, typename parent, typename... args>
    struct return_type<return_ (parent::*)(args...)> : return_type<return_(args...)>
    {
    };

    template<typename signature>
    struct return_type<signature*> : return_type<signature>
    {
    };

    template<typename signature>
    using return_type_t = typename return_type<signature>::type;

    /**
     * @brief Get the nth argument type
     *
     * @tparam std::size_t n
     * @tparam signature of the function
     */
    template<std::size_t n, typename signature>
    struct arg_type;

    template<std::size_t n, typename ret, typename... args>
    struct arg_type<n, ret(args...)> : std::tuple_element<n, std::tuple<args...>>
    {
    };

    template<std::size_t n, typename ret, class parent, typename... args>
    struct arg_type<n, ret (parent::*)(args...)> : arg_type<n, ret(args...)>
    {
            // For member functions
    };

    template<std::size_t n, typename signature>
    struct arg_type<n, signature*> : arg_type<n, signature>
    {
            // For function pointers
    };

    template<std::size_t n, typename signature>
    using arg_type_t = typename arg_type<n, signature>::type;

    /**
     * @brief Extract the number of arguments of the given function signature
     *
     * @tparam signature
     */
    template<typename signature>
    struct number_of_args;

    template<typename ret, typename... args>
    struct number_of_args<ret(args...)> : std::integral_constant<std::size_t, sizeof...(args)>
    {
            // Number of arguments of a function
    };

    template<typename ret, class parent, typename... args>
    struct number_of_args<ret (parent::*)(args...)> : number_of_args<ret(args...)>
    {
            // Number of arguments of a member function
    };

    template<typename signature>
    struct number_of_args<signature*> : number_of_args<signature>
    {
            // Number of arguments of a function pointer
    };

    template<typename signature>
    static constexpr auto number_of_args_v = number_of_args<signature>::value;

    template<typename signature>
    struct to_plain_signature
    {
            using type = signature;
    };

    template<typename ret, typename owner, typename... args>
    struct to_plain_signature<ret (owner::*)(args...)> : to_plain_signature<ret(args...)>
    {
    };

    template<typename signature>
    struct to_plain_signature<signature*> : to_plain_signature<signature>
    {
    };

    template<typename signature, typename owner>
    struct to_member_signature;

    template<typename ret, typename... args, typename owner>
    struct to_member_signature<ret(args...), owner>
    {
            using type = ret (owner::*)(args...);
    };

    template<typename signature, typename owner>
    using to_member_signature_t = typename to_member_signature<signature, owner>::type;

}// namespace ctfsm

#endif// TYPE_TRAITS_FUNCTION_SIGNATURE_HPP
