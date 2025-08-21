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

#include "traits/checked_fsm.hpp"
#include "traits/exit_events_extractor.hpp"
#include "traits/id_types_verifier.hpp"
#include "traits/nested_filter.hpp"
#include "traits/remove_final_states.hpp"
#include "traits/state_expander.hpp"
#include "traits/variant_builder.hpp"
#include "utility/existence_verifier.hpp"
#include "utility/type_map.hpp"
#include "utility/type_set.hpp"

MAKE_EXISTENCE_VERIFIER(on_enter)
MAKE_EXISTENCE_VERIFIER(on_exit)
MAKE_EXISTENCE_VERIFIER(on_transit)

namespace ctfsm
{
    /**
     * @brief A finite state machine, it can only handle a specific designed set of states. Only the
     * initial state has to be provided.
     *
     * @tparam state
     */
    template<class _initial_state, class _parent_state = void>
    class fsm
    {
            template<class I, class P>
            friend class fsm;

        public:
            using initial_state = _initial_state;

            using states  = pvt::nested_filter<typename pvt::remove_final_states<
                 typename pvt::state_expander<std::tuple<initial_state>>::states>::states>;
            using id_type = typename pvt::id_types_verifier<typename states::states>::type;

            typename states::states                                                _states;
            typename states::fsms                                                  _nested_fsms;
            pvt::variant_builder_t<typename states::states, typename states::fsms> _current_state;
            const id_type* _current_state_id;

            using exit_events  = typename pvt::exit_events_extractor<decltype(_states)>::result;
            using parent_state = _parent_state;

        private:
            template<class current_state, class _event>
            constexpr void invoke_on_exit(current_state& current, _event& event) noexcept
            {
                if constexpr (pvt::has_on_exit_method<current_state, void, std::decay_t<_event>>)
                {
                    current.on_exit(event);
                }
                else if constexpr (pvt::has_on_exit_method<current_state, void>)
                {
                    current.on_exit();
                }
            }

            template<class current_state, class _event>
            constexpr void invoke_on_enter(current_state& current, _event& event) noexcept
            {
                if constexpr (pvt::has_on_enter_method<current_state, void, std::decay_t<_event>>)
                {
                    current.on_enter(event);
                }
                else if constexpr (pvt::has_on_enter_method<current_state, void>)
                {
                    current.on_enter();
                }
            }

            template<class _event>
            constexpr void invoke_on_transit(_event& event) noexcept
            {
                if constexpr (!pvt::has_on_transit_method<_event, void>)
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

                if constexpr (pvt::contains<event_t, exit_events>::value)
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
                        _current_state_id = &pvt::id_extractor<target_state>::id;

                        _current_state = &std::get<target_state>(_states);

                        invoke_on_enter(std::get<target_state>(_states), event);
                    }
                    else
                    {
                        auto* current = &std::get<target_state>(_nested_fsms);
                        auto& nested_state
                            = std::get<typename target_state::initial_state>(current->_states);

                        current->invoke_on_enter(nested_state, event);

                        _current_state = current;
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
                            auto checked
                                = pvt::checked_fsm<current_state, std::decay_t<decltype(injected_parent)>> {
                                    injected_parent};
                            return lambda(*current, checked);
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
                  _current_state_id(&pvt::id_extractor<initial_state>::id)
            {
                static_assert(pvt::valid_fsm<fsm>);

                if constexpr (std::same_as<_parent_state, void>)
                {
                    // This is not a nested FSM
                    static_assert(std::same_as<exit_events, std::tuple<>>,
                                  "Exit events are admitted only in a nested FSM");
                }
            }

            /**
             * @brief Reset the finite state machine */
            constexpr auto reset() noexcept -> void
            {
                // Initial state is always a simple state, no need to check if it is an FSM
                _current_state    = &std::get<initial_state>(_states);
                _current_state_id = &pvt::id_extractor<initial_state>::id;
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
                            using target_state = typename pvt::parent_injector_single<
                                current_state,
                                typename pvt::find_by_key<event_t, typename current_state::transitions>::result>::result;

                            static_assert(current_state::transitions::valid,
                                          "Transitions events must be unique");

                            return this->handle_event_<current_state, target_state>(event);
                        }
                        else
                        {
                            if constexpr (pvt::contains<event_t, typename current_state::exit_events>::value)
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

    template<typename Event>
    struct exit_transition
    {
            using key   = Event;
            using value = pvt::final_state;
    };

    template<pvt::mappable... data>
    using transition_map = pvt::type_map<data...>;
}// namespace ctfsm

#endif /* CTFSM_FSM_CTFSM_HPP_*/
