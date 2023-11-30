/**
 * @file existence_verifier.hpp
 * @author Carmine Margiotta (cmargiotta@posteo.net)
 *
 * @copyright Copyright (c) 2022
 */

#ifndef UTILITY_EXISTENCE_VERIFIER_HPP
#define UTILITY_EXISTENCE_VERIFIER_HPP

#include <type_traits>

/**
 * @brief Build a concept that verifies the existence of the given member.
 */
#define MAKE_EXISTENCE_VERIFIER(member)                                                            \
    namespace ctfsm                                                                                \
    {                                                                                              \
        template<typename T>                                                                       \
        concept has_##member = requires(T instance) { std::declval<T>().member; };                 \
                                                                                                   \
        template<typename T, typename Ret, typename... args>                                       \
        concept has_##member##_method = requires(T instance, args... arguments) {                  \
            {                                                                                      \
                instance.member(arguments...)                                                      \
            } -> std::same_as<Ret>;                                                                \
        } || (sizeof...(args) == 0 && requires(T instance) {                                       \
                                            {                                                      \
                                                instance.member()                                  \
                                            } -> std::same_as<Ret>;                                \
                                        });                                                        \
    }

#endif// UTILITY_EXISTENCE_VERIFIER_HPP
