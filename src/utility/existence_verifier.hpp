/**
 * @file existence_verifier.hpp
 * @author Carmine Margiotta (car.margiotta@icloud.com)
 *
 * @copyright Copyright (c) 2022
 */

#ifndef UTILITY_EXISTENCE_VERIFIER_HPP
#define UTILITY_EXISTENCE_VERIFIER_HPP

#include <type_traits>

#include "function_signature.hpp"

/**
 * @brief Build a concept that verifies the existence of the given member.
 */
#define MAKE_EXISTENCE_VERIFIER(member)                                                            \
    namespace ctfsm                                                                                \
    {                                                                                              \
        template<typename T>                                                                       \
        concept has_##member = requires(T instance)                                                \
        {                                                                                          \
            std::declval<T>().member;                                                              \
        };                                                                                         \
        template<typename T, typename signature>                                                   \
        concept has_##member##_method = requires(T instance)                                       \
        {                                                                                          \
            static_cast<to_member_signature_t<signature, T>>(&T::member);                          \
        };                                                                                         \
    }

#endif// UTILITY_EXISTENCE_VERIFIER_HPP
