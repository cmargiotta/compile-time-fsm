#ifndef CTFSM_FSM_TRAITS_REMOVE_FINAL_STATES_HPP_
#define CTFSM_FSM_TRAITS_REMOVE_FINAL_STATES_HPP_

#include <tuple>

#include "final_state.hpp"

namespace ctfsm::pvt
{
    // Filter out final states from a tuple of states (final states cannot be instanciated/used)
    template<class states, class result = std::tuple<>>
    struct remove_final_states;

    template<class state, class... states, class... result>
        requires(!final<state>)
    struct remove_final_states<std::tuple<state, states...>, std::tuple<result...>>
        : public remove_final_states<std::tuple<states...>, std::tuple<state, result...>>
    {
    };

    template<final state, class... states, class... result>
    struct remove_final_states<std::tuple<state, states...>, std::tuple<result...>>
        : public remove_final_states<std::tuple<states...>, std::tuple<result...>>
    {
    };

    template<class _result>
    struct remove_final_states<std::tuple<>, _result>
    {
            using states = _result;
    };
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_REMOVE_FINAL_STATES_HPP_*/
