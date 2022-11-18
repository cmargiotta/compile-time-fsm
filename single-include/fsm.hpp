/**
 * @file fsm.hpp
 * @author Carmine Margiotta (cmargiotta@posteo.net)
 *
 * @copyright Copyright (c) 2022
 */

#ifndef FSM_HPP
#define FSM_HPP

#include <concepts>
#include <exception>
#include <stdexcept>
#include <tuple>
#include <variant>

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
 * @file function_signature.hpp
 * @author Carmine Margiotta (cmargiotta@posteo.net)
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

/**
 * @file type_map.hpp
 * @author Carmine Margiotta (cmargiotta@posteo.net)
 *
 * @copyright Copyright (c) 2022
 */

#ifndef TYPE_TRAITS_TYPE_MAP_HPP
#define TYPE_TRAITS_TYPE_MAP_HPP

#include <tuple>
#include <type_traits>

/**
 * @file type_set.hpp
 * @author Carmine Margiotta (cmargiotta@posteo.net)
 *
 * @copyright Copyright (c) 2022
 */

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

MAKE_EXISTENCE_VERIFIER(on_enter)
MAKE_EXISTENCE_VERIFIER(on_exit)
MAKE_EXISTENCE_VERIFIER(on_transit)
MAKE_EXISTENCE_VERIFIER(id)

namespace ctfsm
{
    /**
     * @brief A finite state machine, it can only handle a specific designed set of states. Only the
     * initial state has to be provided.
     *
     * @tparam state
     */
    template<class initial_state>
    class fsm
    {
        private:
            template<class states, class set = std::tuple<>>
            struct state_expander;

            template<class... set>
            struct state_expander<std::tuple<>, std::tuple<set...>>
            {
                    // Base case, there are no more nodes to expand. This handles eventual cycles in
                    // the graph too.
                public:
                    using states = std::tuple<set...>;
            };

            template<class state, class set>
            struct state_expander<std::tuple<state>, set>
            {
                    // Expand a single state and every reachable state from that
                public:
                    // Adding State and the reachable nodes from it to set
                    using state_set = ctfsm::type_set_merge<
                        typename ctfsm::extract_values<typename state::transitions>::values,
                        typename ctfsm::type_set_merge<std::tuple<state>, set>::set>;

                    // Expanding the nodes that were not already in set
                    using states =
                        typename state_expander<typename state_set::delta, typename state_set::set>::states;
            };

            template<class state, class... states_, class set>
            struct state_expander<std::tuple<state, states_...>, set>
            {
                    // Iterate through the states list and expand every state
                private:
                    using state_set = ctfsm::type_set_merge<
                        typename ctfsm::extract_values<typename state::transitions>::Values,
                        typename ctfsm::type_set_merge<std::tuple<state>, set>::set>;

                public:
                    using states = typename ctfsm::type_set_merge<
                        typename state_expander<std::tuple<state>, set>::states,
                        typename state_expander<std::tuple<states_...>, set>::states>::set;
            };

            template<class elements>
            struct variant_builder;

            template<class... elements>
            struct variant_builder<std::tuple<elements...>>
            {
                public:
                    using variant = std::variant<elements*...>;
            };

            template<class... elements>
            using variant_builder_t = typename variant_builder<elements...>::variant;

            template<class states>
            struct id_types_verifier;

            template<ctfsm::has_id state, ctfsm::has_id... states>
            struct id_types_verifier<std::tuple<state, states...>>
            {
                public:
                    static_assert(
                        std::conjunction_v<std::is_same<decltype(state::id), decltype(states::id)>...>,
                        "Every state ID is of the same type");

                    using type = typename std::remove_cv<decltype(state::id)>::type;
            };

            template<class state, class... states>
            struct id_types_verifier<std::tuple<state, states...>>
            {
                public:
                    using type = decltype(typeid(state).name());
            };

            template<class state>
            struct id_extractor
            {
                    static const inline auto id = typeid(state).name();
            };

            template<ctfsm::has_id state>
            struct id_extractor<state>
            {
                    static constexpr auto id = state::id;
            };

        public:
            using states  = typename state_expander<std::tuple<initial_state>>::states;
            using id_type = typename id_types_verifier<states>::type;

            states                    _states;
            variant_builder_t<states> _current_state;
            const id_type*            _current_state_id;

        private:
            template<class current_state, class _event>
            constexpr void invoke_on_exit(current_state& current, _event& event) noexcept
            {
                if constexpr (ctfsm::has_on_exit_method<current_state, void(_event&)>)
                {
                    current.on_exit(event);
                }
                else if constexpr (ctfsm::has_on_exit_method<current_state, void()>)
                {
                    current.on_exit();
                }
            }

            template<class current_state, class _event>
            constexpr void invoke_on_enter(current_state& current, _event& event) noexcept
            {
                if constexpr (ctfsm::has_on_enter_method<current_state, void(_event&)>)
                {
                    current.on_enter(event);
                }
                else if constexpr (ctfsm::has_on_enter_method<current_state, void()>)
                {
                    current.on_enter();
                }
            }

            template<class _event>
            constexpr void invoke_on_transit(_event& event) noexcept
            {
                if constexpr (!has_on_transit_method<_event, void()>)
                {
                    return;
                }
                else
                {
                    event.on_transit();
                }
            }

            template<class current_state, class target_state>
            constexpr void handle_event_(auto&) requires(std::is_same_v<target_state, void>)
            {
                throw std::runtime_error("Unhandled transation");
            }

            template<class current_state, class target_state>
            constexpr void handle_event_(auto& event) noexcept
            {
                invoke_on_exit(std::get<current_state>(_states), event);
                invoke_on_transit(event);

                _current_state    = &std::get<target_state>(_states);
                _current_state_id = &id_extractor<target_state>::id;

                invoke_on_enter(std::get<target_state>(_states), event);
            }

        public:
            /**
             * @brief Default constructor, defaulty constructs every state and sets the initial
             * state as current
             */
            constexpr fsm() noexcept
                : _current_state(&std::get<initial_state>(_states)),
                  _current_state_id(&id_extractor<initial_state>::id)
            {
            }

            /**
             * @brief Handles an incoming event, as mapped in current_state::transitions
             *
             * @param event
             */
            template<typename event_>
            constexpr void handle_event(event_&& event) noexcept
            {
                std::visit(
                    [this, &event](auto&& current)
                    {
                        using current_state
                            = std::remove_pointer_t<std::remove_reference_t<decltype(current)>>;
                        using target_state =
                            typename find_by_key<std::remove_reference_t<event_>,
                                                 typename current_state::transitions>::result;

                        static_assert(current_state::transitions::valid,
                                      "transitions events must be unique");

                        this->handle_event_<current_state, target_state>(event);
                    },
                    _current_state);
            }

            /**
             * @brief Construct an empty event and handle it
             *
             * @tparam event
             */
            template<std::default_initializable event>
            constexpr void handle_event() noexcept
            {
                handle_event(event {});
            }

            /**
             * @brief FSM can also be directly invoked with an event
             *
             * @param event
             */
            constexpr void operator()(auto&& event) noexcept
            {
                handle_event(event);
            }

            /**
             * @brief Get current state id
             *
             * @return constexpr const id_type&
             */
            constexpr const id_type& get_current_state_id() const noexcept
            {
                return *_current_state_id;
            }

            /**
             * @brief Returns true if T is the current state
             */
            template<typename T>
            constexpr bool is_current_state() const noexcept
            {
                return std::holds_alternative<T>(_current_state);
            }
    };
}// namespace ctfsm

#endif// FSM_HPP
