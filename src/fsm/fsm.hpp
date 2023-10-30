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

#include "utility/existence_verifier.hpp"
#include "utility/type_map.hpp"
#include "utility/type_set.hpp"

MAKE_EXISTENCE_VERIFIER(on_enter)
MAKE_EXISTENCE_VERIFIER(on_exit)
MAKE_EXISTENCE_VERIFIER(on_transit)
MAKE_EXISTENCE_VERIFIER(id)

#ifdef __EXCEPTIONS
#    define HANDLE_EVENT_RETURN_TYPE void
#else
#    define HANDLE_EVENT_RETURN_TYPE bool
#endif

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
                if constexpr (ctfsm::has_on_exit_method<current_state, _event&>)
                {
                    static_assert(!ctfsm::has_on_exit_method<current_state>,
                                  "States cannot have both on_exit(event&) and on_exit()");

                    current.on_exit(event);
                }
                else if constexpr (ctfsm::has_on_exit_method<current_state>)
                {
                    current.on_exit();
                }
            }

            template<class current_state, class _event>
            constexpr void invoke_on_enter(current_state& current, _event& event) noexcept
            {
                if constexpr (ctfsm::has_on_enter_method<current_state, _event&>)
                {
                    static_assert(!ctfsm::has_on_enter_method<current_state>,
                                  "States cannot have both on_enter(event&) and on_enter()");

                    current.on_enter(event);
                }
                else if constexpr (ctfsm::has_on_enter_method<current_state>)
                {
                    current.on_enter();
                }
            }

            template<class _event>
            constexpr void invoke_on_transit(_event& event) noexcept
            {
                if constexpr (!has_on_transit_method<_event>)
                {
                    return;
                }
                else
                {
                    event.on_transit();
                }
            }

            template<class current_state, class target_state>
            constexpr HANDLE_EVENT_RETURN_TYPE handle_event_(auto&)
                requires(std::is_same_v<target_state, void>)
            {
#ifdef __EXCEPTIONS
                throw std::runtime_error("Unhandled transation");
#else
                return false;
#endif
            }

            template<class current_state, class target_state>
            constexpr HANDLE_EVENT_RETURN_TYPE handle_event_(auto& event) noexcept
            {
                invoke_on_exit(std::get<current_state>(_states), event);
                invoke_on_transit(event);

                _current_state    = &std::get<target_state>(_states);
                _current_state_id = &id_extractor<target_state>::id;

                invoke_on_enter(std::get<target_state>(_states), event);

#ifndef __EXCEPTIONS
                return true;
#endif
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
#ifndef __EXCEPTIONS
            [[nodiscard]]
#endif
            constexpr HANDLE_EVENT_RETURN_TYPE
                handle_event(event_&& event)
            {
                return std::visit(
                    [this, &event](auto&& current)
                    {
                        using current_state
                            = std::remove_pointer_t<std::remove_reference_t<decltype(current)>>;
                        using target_state =
                            typename find_by_key<std::remove_reference_t<event_>,
                                                 typename current_state::transitions>::result;

                        static_assert(current_state::transitions::valid,
                                      "transitions events must be unique");

                        return this->handle_event_<current_state, target_state>(event);
                    },
                    _current_state);
            }

            /**
             * @brief Construct an empty event and handle it
             *
             * @tparam event
             */
            template<std::default_initializable event>
#ifndef __EXCEPTIONS
            [[nodiscard]]
#endif
            constexpr HANDLE_EVENT_RETURN_TYPE
                handle_event()
            {
                return handle_event(event {});
            }

            /**
             * @brief FSM can also be directly invoked with an event
             *
             * @param event
             */
            constexpr void operator()(auto&& event)
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
                return std::holds_alternative<T*>(_current_state);
            }

            /**
             * @brief Invoke the given lambda passing a reference to the current state
             *
             * @param lambda
             * @return the lambda return value
             */
            constexpr auto invoke_on_current(auto lambda) noexcept
            {
                return std::visit([this, lambda](auto current) { return lambda(*current, *this); },
                                  _current_state);
            }
    };
}// namespace ctfsm

#endif// FSM_HPP
