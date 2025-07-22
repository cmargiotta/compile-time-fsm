#ifndef CTFSM_FSM_TRAITS_NESTED_FILTER_HPP_
#define CTFSM_FSM_TRAITS_NESTED_FILTER_HPP_

#include <tuple>

#include "valid_fsm.hpp"

namespace ctfsm::pvt
{
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
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_NESTED_FILTER_HPP_*/
