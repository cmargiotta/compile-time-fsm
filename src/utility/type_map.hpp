/**
 * @file type_map.hpp
 * @author Carmine Margiotta (car.margiotta@icloud.com)
 *
 * @copyright Copyright (c) 2022
 */

#ifndef TYPE_TRAITS_TYPE_MAP_HPP
#define TYPE_TRAITS_TYPE_MAP_HPP

#include <tuple>
#include <type_traits>

#include "type_set.hpp"

namespace ctfsm
{
    /**
     * @brief Extract an std::tuple of keys from the given type map
     *
     * @tparam map
     */
    template<class map>
    struct extract_keys;

    /**
     * @brief Extract an std::tuple of values from the given type map
     *
     * @tparam map
     */
    template<class map>
    struct extract_values;

    /**
     * @brief An std::tuple of std::pair is considered a type_map, where the pair is composed of
     * <Key, Value>
     *
     * @tparam data
     */
    template<class... data_>
    struct type_map;

    /**
     * @brief Find the value corresponding to the given Key, if it does not exist void is exposed
     *
     * @tparam key
     * @tparam map
     */
    template<class key, class map>
    struct find_by_key;

    template<class key, class current_key, class value, class... data>
    struct find_by_key<key, type_map<std::pair<current_key, value>, data...>>
        : find_by_key<key, type_map<data...>>
    {
            // Key has not been found, keep searching it
    };

    template<class key>
    struct find_by_key<key, type_map<>>
    {
            // Key has not been found, expose void
        public:
            using result = void;
    };

    template<class key, class value, class... data>
    struct find_by_key<key, type_map<std::pair<key, value>, data...>>
    {
            // Key has been found, expose the result
        public:
            using result = value;
    };

    template<class key, class value, class... data>
    struct extract_values<type_map<std::pair<key, value>, data...>>
    {
        public:
            using values = decltype(std::tuple_cat(
                std::declval<std::tuple<value>>(),
                std::declval<typename extract_values<type_map<data...>>::values>()));
    };

    template<>
    struct extract_values<type_map<>>
    {
            // Base case, all values have been extracted, nothing to do
        public:
            using values = std::tuple<>;
    };

    template<class key, class value, class... data>
    struct extract_keys<type_map<std::pair<key, value>, data...>>
    {
            // Iterating through the Map and composing the Keys tuple
        public:
            using keys = decltype(std::tuple_cat(
                std::declval<std::tuple<key>>(),
                std::declval<typename extract_keys<type_map<data...>>::keys>()));
    };

    template<>
    struct extract_keys<type_map<>>
    {
            // Base case, all keys have been extracted, nothing to do
        public:
            using keys = std::tuple<>;
    };

    template<class key, class value, class... data_>
    struct type_map<std::pair<key, value>, data_...>
    {
        private:
            using type = type_map<std::pair<key, value>, data_...>;

        public:
            using data = std::tuple<std::pair<key, value>, data_...>;

            static constexpr bool valid
                = (std::tuple_size<typename type_set<typename extract_keys<type>::keys>::set>::value
                   == std::tuple_size<data>::value);
    };

    template<>
    struct type_map<>
    {
        public:
            using data = std::tuple<>;

            static constexpr bool valid = true;
    };
}// namespace ctfsm

#endif// TYPE_TRAITS_TYPE_MAP_HPP
