#ifndef CTFSM_FSM_CTFSM_HPP_
#define CTFSM_FSM_CTFSM_HPP_

/**
 * @file fsm.hpp
 * @author Carmine Margiotta (email@cmargiotta.net)
 *
 * @copyright Copyright (c) 2025
 */

#include <concepts>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <variant>

#include "utility/existence_verifier.hpp"
#include "utility/type_map.hpp"
#include "utility/type_set.hpp"

MAKE_EXISTENCE_VERIFIER(on_enter)
MAKE_EXISTENCE_VERIFIER(on_exit)
MAKE_EXISTENCE_VERIFIER(on_transit)
MAKE_EXISTENCE_VERIFIER(id)

namespace ctfsm
{
    namespace pvt
    {
        template<class T>
        concept valid_fsm = requires(T fsm) {
            typename T::exit_events;
            typename T::parent_state;

            {
                fsm.invoke_on_current([](auto s, auto & fsm) { return true; })
            } -> std::same_as<bool>;
        };
    }// namespace pvt

    /**
     * @brief A finite state machine, it can only handle a specific designed set of states. Only the
     * initial state has to be provided.
     *
     * @tparam state
     */
    template<class initial_state, class _exit_events = std::tuple<>, class _parent_state = void>
    class fsm
    {
            template<class I, class E, class P>
            friend class fsm;

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

            // Inject the _parent_state parameter to a child
            template<class parent, class child>
            struct parent_injector_single
            {
                    using result = child;
            };

            template<class parent, class i, class e, class p>
            struct parent_injector_single<parent, fsm<i, e, p>>
            {
                    // Current is a nested FSM, injecting parent
                    using result = fsm<i, e, parent>;
            };

            // Inject the _parent_state parameter to nested fsms
            template<class parent, class children_tuple, class result = std::tuple<>>
            struct parent_injector;

            template<class parent, class i, class e, class p, class... children, class... result>
            struct parent_injector<parent, std::tuple<fsm<i, e, p>, children...>, std::tuple<result...>>
                : parent_injector<parent, std::tuple<children...>, std::tuple<fsm<i, e, parent>, result...>>
            {
                    // Current is a nested FSM, injecting parent
            };

            template<class parent, class current, class... children, class... result>
            struct parent_injector<parent, std::tuple<current, children...>, std::tuple<result...>>
                : parent_injector<parent, std::tuple<children...>, std::tuple<current, result...>>
            {
                    // Current is a normal state, nothing to do
            };

            template<class parent, class _result>
            struct parent_injector<parent, std::tuple<>, _result>
            {
                    // Base case, injection done
                    using result = _result;
            };

            template<class state, class set>
            struct state_expander<std::tuple<state>, set>
            {
                    // Expand a single state and every reachable state from that
                public:
                    // Adding State and the reachable nodes from it to set
                    using state_set = ctfsm::type_set_merge<
                        typename parent_injector<state, typename ctfsm::extract_values<typename state::transitions>::values>::result,
                        typename ctfsm::type_set_merge<std::tuple<state>, set>::set>;

                    // Expanding the nodes that were not already in set
                    using states =
                        typename state_expander<typename state_set::delta, typename state_set::set>::states;
            };

            template<class state, class set>
                requires(pvt::valid_fsm<state>)
            struct state_expander<std::tuple<state>, set>
            {
                    // State is a nested FSM, no need to expand it
                public:
                    // Adding State to set
                    using state_set = ctfsm::type_set_merge<std::tuple<state>, set>;

                    // Expanding the nodes that were not already in set
                    using states =
                        typename state_expander<typename state_set::delta, typename state_set::set>::states;
            };

            template<class state, class... states_, class set>
            struct state_expander<std::tuple<state, states_...>, set>
            {
                    // Iterate through the states list and expand every state
                    using states = typename ctfsm::type_set_merge<
                        typename state_expander<std::tuple<state>, set>::states,
                        typename state_expander<std::tuple<states_...>, set>::states>::set;
            };

            // Filter nested fsms from states
            template<class raw_states, class fsms = std::tuple<>, class states = std::tuple<>>
            struct nested_filter;

            template<pvt::valid_fsm current, class... tail, class... fsms, class states>
            struct nested_filter<std::tuple<current, tail...>, std::tuple<fsms...>, states>
                : nested_filter<std::tuple<tail...>, std::tuple<current, fsms...>, states>
            {
                    // A nested FSM has been found
            };

            template<class current, class... tail, class fsms, class... states>
                requires(!pvt::valid_fsm<current>)
            struct nested_filter<std::tuple<current, tail...>, fsms, std::tuple<states...>>
                : nested_filter<std::tuple<tail...>, fsms, std::tuple<current, states...>>
            {
                    // A simple state has been found
            };

            template<class _fsms, class _states>
            struct nested_filter<std::tuple<>, _fsms, _states>
            {
                    // Base case, no more raw states to iter
                    using states = _states;
                    using fsms   = _fsms;
            };

            template<class fsms, class states>
            struct variant_builder;

            template<class... states, class... fsms>
            struct variant_builder<std::tuple<fsms...>, std::tuple<states...>>
            {
                public:
                    using variant = std::variant<states*..., fsms*...>;
            };

            template<class states, class fsms>
            using variant_builder_t = typename variant_builder<states, fsms>::variant;

            template<class states>
            struct id_types_verifier;

            template<ctfsm::has_id state, ctfsm::has_id... states>
            struct id_types_verifier<std::tuple<state, states...>>
            {
                public:
                    static_assert(
                        std::conjunction_v<std::is_same<decltype(state::id), decltype(states::id)>...>,
                        "Every state ID must be of the same type");

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
            using states = nested_filter<typename state_expander<std::tuple<initial_state>>::states>;
            using id_type = typename id_types_verifier<typename states::states>::type;

            typename states::states                                           _states;
            typename states::fsms                                             _nested_fsms;
            variant_builder_t<typename states::states, typename states::fsms> _current_state;
            const id_type*                                                    _current_state_id;

            using exit_events  = _exit_events;
            using parent_state = _parent_state;

        private:
            template<class current_state, class _event>
            constexpr void invoke_on_exit(current_state& current, _event& event) noexcept
            {
                if constexpr (ctfsm::has_on_exit_method<current_state, void, std::decay_t<_event>>)
                {
                    current.on_exit(event);
                }
                else if constexpr (ctfsm::has_on_exit_method<current_state, void>)
                {
                    current.on_exit();
                }
            }

            template<class current_state, class _event>
            constexpr void invoke_on_enter(current_state& current, _event& event) noexcept
            {
                if constexpr (ctfsm::has_on_enter_method<current_state, void, std::decay_t<_event>>)
                {
                    current.on_enter(event);
                }
                else if constexpr (ctfsm::has_on_enter_method<current_state, void>)
                {
                    current.on_enter();
                }
            }

            template<class _event>
            constexpr void invoke_on_transit(_event& event) noexcept
            {
                if constexpr (!has_on_transit_method<_event, void>)
                {
                    return;
                }
                else
                {
                    event.on_transit();
                }
            }

            template<class current_state, class target_state>
            constexpr auto handle_event_(auto&) -> bool
                requires(std::is_same_v<target_state, void>)
            {
                return false;
            }

            template<class current_state, class target_state>
            constexpr auto handle_event_(auto& event) noexcept -> bool
            {
                using event_t = std::decay_t<decltype(event)>;

                // On exit will always be invoked
                invoke_on_exit(std::get<current_state>(_states), event);

                if constexpr (ctfsm::contains<event_t, exit_events>::value)
                {
                    // This is an exit event, reset the fsm and return
                    reset();
                }
                else
                {
                    invoke_on_transit(event);

                    if constexpr (!pvt::valid_fsm<target_state>)
                    {
                        // Update current state ID only if this is not a nested fsm
                        _current_state_id = &id_extractor<target_state>::id;

                        _current_state = &std::get<target_state>(_states);

                        invoke_on_enter(std::get<target_state>(_states), event);
                    }
                    else
                    {
                        _current_state = &std::get<target_state>(_nested_fsms);
                    }
                }

                return true;
            }

            constexpr auto invoke_on_current(auto lambda, auto& injected_parent) noexcept
            {
                return std::visit(
                    [this, lambda, &injected_parent](auto current) mutable
                    {
                        using current_state = std::remove_pointer_t<std::decay_t<decltype(current)>>;

                        if constexpr (pvt::valid_fsm<current_state>)
                        {
                            // Current is a nested FSM
                            return current->invoke_on_current(lambda, injected_parent);
                        }
                        else
                        {
                            return lambda(*current, injected_parent);
                        }
                    },
                    _current_state);
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
                static_assert(pvt::valid_fsm<fsm>);
            }

            /**
             * @brief Reset the finite state machine */
            constexpr auto reset() noexcept -> void
            {
                // Initial state is always a simple state, no need to check if it is an FSM
                _current_state    = &std::get<initial_state>(_states);
                _current_state_id = &id_extractor<initial_state>::id;
            }

            /**
             * @brief Handles an incoming event, as mapped in current_state::transitions
             *
             * @param event
             */
            [[nodiscard]] constexpr auto handle_event(auto& event) -> bool
            {
                using event_t = std::decay_t<decltype(event)>;

                return std::visit(
                    [this, &event](auto* current)
                    {
                        using current_state = std::remove_pointer_t<decltype(current)>;

                        if constexpr (!pvt::valid_fsm<current_state>)
                        {
                            // Current state is not a nested FSM

                            // Inject parent to target state, if it is a nested fsm
                            using target_state = typename parent_injector_single<
                                current_state,
                                typename find_by_key<event_t, typename current_state::transitions>::result>::result;

                            static_assert(current_state::transitions::valid,
                                          "Transitions events must be unique");

                            return this->handle_event_<current_state, target_state>(event);
                        }
                        else
                        {
                            if constexpr (ctfsm::contains<event_t, typename current_state::exit_events>::value)
                            {
                                // This event is an exit event for the nested fsm

                                if (!current->handle_event(event))
                                {
                                    // Transition not accepted by the nested fsm
                                    return false;
                                }

                                // Return to parent state
                                _current_state
                                    = &std::get<typename current_state::parent_state>(_states);

                                // Try to pass event to current state
                                if (handle_event(event))
                                {
                                    // Event has been accepted!
                                    return true;
                                }

                                // Parent state does not care about this event
                                return true;
                            }
                            else
                            {
                                // Directly pass the event to the nested FSM
                                return current->handle_event(event);
                            }
                        }
                    },
                    _current_state);
            }

            constexpr auto handle_event(auto&& event) -> bool
            {
                return handle_event(event);
            }

            /**
             * @brief Construct an empty event and handle it
             *
             * @tparam event
             */
            template<std::default_initializable event>
            [[nodiscard]] constexpr auto handle_event() -> bool
            {
                return handle_event(event {});
            }

            /**
             * @brief FSM can also be directly invoked with an event
             *
             * @param event
             */
            [[nodiscard]] constexpr auto operator()(auto&& event) -> bool
            {
                return handle_event(event);
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
            constexpr auto is_current_state() const noexcept -> bool
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
                return invoke_on_current(std::move(lambda), *this);
            }
    };

    template<typename Event, typename Target>
    struct transition
    {
            using key   = Event;
            using value = Target;
    };

    template<typename Event, typename Nested_Fsm_Initial_State, typename... Exit_Events>
    struct nested
    {
            using key   = Event;
            using value = fsm<Nested_Fsm_Initial_State, std::tuple<Exit_Events...>>;
    };

    template<mappable... data>
    using transition_map = type_map<data...>;
}// namespace ctfsm

#endif /* CTFSM_FSM_CTFSM_HPP_*/
