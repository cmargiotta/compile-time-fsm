#ifndef CTFSM_FSM_TRAITS_VALID_FSM_HPP_
#define CTFSM_FSM_TRAITS_VALID_FSM_HPP_

#include <concepts>

namespace ctfsm::pvt
{
    template<class T>
    concept valid_fsm = requires(T fsm) {
        typename T::exit_events;
        typename T::parent_state;

        {
            fsm.invoke_on_current([](const auto & s, auto & fsm) { return true; })
        } -> std::same_as<bool>;
    };
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_VALID_FSM_HPP_*/
