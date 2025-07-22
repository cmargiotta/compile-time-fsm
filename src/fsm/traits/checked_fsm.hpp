#ifndef CTFSM_FSM_TRAITS_CHECKED_FSM_HPP_
#define CTFSM_FSM_TRAITS_CHECKED_FSM_HPP_

#include <concepts>

#include "utility/type_map.hpp"

namespace ctfsm::pvt
{
    template<class current_state, class fsm>
    class checked_fsm
    {
            // Current state is available at compile time, this can be used to validate
            // transitions

        private:
            fsm& instance;

        public:
            constexpr checked_fsm(fsm& instance): instance {instance}
            {
            }

            constexpr auto handle_event(auto& event) -> bool
            {
                using event_t = std::decay_t<decltype(event)>;

                static_assert(
                    !std::same_as<typename find_by_key<event_t, typename current_state::transitions>::result, void>,
                    "Transition not admissible");

                return instance.handle_event(event);
            }

            constexpr auto handle_event(auto&& event) -> bool
            {
                return handle_event(event);
            }

            template<std::default_initializable event>
            constexpr auto handle_event() -> bool
            {
                return handle_event(event {});
            }
    };
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_CHECKED_FSM_HPP_*/
