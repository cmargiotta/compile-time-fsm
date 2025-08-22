#ifndef CTFSM_UTILITY_TYPE_MAP_HPP_
#define CTFSM_UTILITY_TYPE_MAP_HPP_

/**
 * @file type_map.hpp
 * @author Carmine Margiotta (email@cmargiotta.net)
 *
 * @copyright Copyright (c) 2025
 */

#include <concepts>
#include <tuple>
#include <type_traits>

#include "type_set.hpp"

namespace ctfsm::pvt
{
    /**
     * @brief A type indexable by a type_map
     */
    template<class T>
    concept mappable = requires() {
        typename T::key;
        typename T::value;
    };

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
     * @brief An std::tuple of mappable is considered a type_map
     *
     * @tparam data
     */
    template<mappable... data_>
    struct type_map;

    /**
     * @brief Find the value corresponding to the given Key, if it does not exist void is exposed
     *
     * @tparam key
     * @tparam map
     */
    template<class key, class map>
    struct find_by_key;

    /**
     * @brief Merge type maps
     *
     * @tparam data
     */
    template<typename... maps>
    struct merge_type_maps;

    template<typename... Elements1, typename... Elements2, typename... Tail>
    struct merge_type_maps<type_map<Elements1...>, type_map<Elements2...>, Tail...>
    {
            using result =
                typename merge_type_maps<type_map<Elements1..., Elements2...>, Tail...>::result;
    };

    template<typename... Elements>
    struct merge_type_maps<type_map<Elements...>>
    {
            using result = type_map<Elements...>;
    };

    template<>
    struct merge_type_maps<>
    {
            using result = type_map<>;
    };

    template<class key, mappable current, class... data>
    struct find_by_key<key, type_map<current, data...>> : find_by_key<key, type_map<data...>>
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

    template<class key, mappable current, class... data>
        requires(std::same_as<key, typename current::key>)
    struct find_by_key<key, type_map<current, data...>>
    {
            // Key has been found, expose the result
        public:
            using result = typename current::value;
    };

    template<mappable current, mappable... data>
    struct extract_values<type_map<current, data...>>
    {
        public:
            using values = decltype(std::tuple_cat(
                std::declval<std::tuple<typename current::value>>(),
                std::declval<typename extract_values<type_map<data...>>::values>()));
    };

    template<>
    struct extract_values<type_map<>>
    {
            // Base case, all values have been extracted, nothing to do
        public:
            using values = std::tuple<>;
    };

    template<mappable current, class... data>
    struct extract_keys<type_map<current, data...>>
    {
            // Iterating through the Map and composing the Keys tuple
        public:
            using keys = decltype(std::tuple_cat(
                std::declval<std::tuple<typename current::key>>(),
                std::declval<typename extract_keys<type_map<data...>>::keys>()));
    };

    template<>
    struct extract_keys<type_map<>>
    {
            // Base case, all keys have been extracted, nothing to do
        public:
            using keys = std::tuple<>;
    };

    template<mappable current, mappable... data_>
    struct type_map<current, data_...>
    {
        private:
            using type = type_map<current, data_...>;

        public:
            using data = std::tuple<current, data_...>;

            // Check if every key is unique
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
}// namespace ctfsm::pvt

#endif /* CTFSM_UTILITY_TYPE_MAP_HPP_*/
