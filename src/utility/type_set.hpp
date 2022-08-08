#ifndef UTILITY_TYPE_SET_HPP
#define UTILITY_TYPE_SET_HPP

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace ctfsm
{
    /**
     * @brief Checks if the Element type is in the given std::tuple
     *
     * @tparam element
     * @tparam containers
     */
    template<class element, class container>
    struct contains : contains<element, typename container::Set>
    {
    };

    template<class element, class... set>
    struct contains<element, std::tuple<set...>> : std::disjunction<std::is_same<element, set>...>
    {
    };

    template<class element, class set, bool contained = contains<element, set>::value>
    struct type_set_insert;

    template<class element, class... set_>
    struct type_set_insert<element, std::tuple<set_...>, true>
    {
            // Element is already in Set_, nothing to do, Delta is empty
        public:
            using set   = std::tuple<set_...>;
            using delta = std::tuple<>;
    };

    template<class element, class... set_>
    struct type_set_insert<element, std::tuple<set_...>, false>
    {
            // Element is not in set_, append it to the set and report it in Delta
        public:
            using set   = std::tuple<element, set_...>;
            using delta = std::tuple<element>;
    };

    /**
     * @brief Given an std::tuple and a type set, type_set_merge<...>::Set will be the given set
     * with appended types from the tuple. Sorting is not guaranteed, unicity is guaranteed.
     *
     * @tparam elements_tuple
     * @tparam base_set the set to be updated
     */
    template<class elements_tuple, class base_set>
    struct type_set_merge;

    template<class element, class... elements, class... set_>
    struct type_set_merge<std::tuple<element, elements...>, std::tuple<set_...>>
    {
        private:
            // Recursive call (until elements is not empty)
            using tail_set = type_set_merge<std::tuple<elements...>, std::tuple<set_...>>;
            // type_set_insert call
            using current = type_set_insert<element, typename tail_set::set>;

        public:
            using set   = typename current::set;
            using delta = decltype(std::tuple_cat(std::declval<typename current::delta>(),
                                                  std::declval<typename tail_set::delta>()));
    };

    template<class... elements>
    struct type_set_merge<std::tuple<>, std::tuple<elements...>>
    {
            // Base case, no more elements to insert, nothing to do
        public:
            using set   = std::tuple<elements...>;
            using delta = std::tuple<>;
    };

    /**
     * @brief Build a type_set from the given parameter pack of types, removing repetitions
     *
     * @tparam elements
     */
    template<class... elements>
    struct type_set : type_set_merge<std::tuple<elements...>, std::tuple<>>
    {
    };

    /**
     * @brief Build a type_set from the given std::tuple, removing repetitions
     *
     * @tparam elements
     */
    template<class... elements>
    struct type_set<std::tuple<elements...>> : type_set<elements...>
    {
    };

    /**
     * @brief Obtain the nth element of the given type set
     *
     * @tparam n
     * @tparam set
     */
    template<std::size_t n, class set>
    using nth_type_set_element = std::tuple_element<n, set>;

    template<class element, class set, std::size_t index = 0>
    struct type_set_find_element_helper;

    template<typename element, class head, class... tail, std::size_t index_>
    struct type_set_find_element_helper<element, std::tuple<head, tail...>, index_>
    {
            // Exploring the types list
        public:
            static constexpr std::size_t index
                = type_set_find_element_helper<element, std::tuple<tail...>, index_ + 1>::index;
    };

    template<class element, class... tail, std::size_t index_>
    struct type_set_find_element_helper<element, std::tuple<element, tail...>, index_>
    {
            // Element found, exposing index
        public:
            static constexpr std::size_t index = index_;
    };

    /**
     * @brief Find an element in the given set and expose its index
     *
     * @tparam element
     * @tparam set
     */
    template<class element, class set>
    using type_set_find_element = type_set_find_element_helper<element, set, 0>;
}// namespace ctfsm

#endif// UTILITY_TYPE_SET_HPP
