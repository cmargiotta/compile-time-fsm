#ifndef CTFSM_FSM_TRAITS_EXIT_EVENTS_EXTRACTOR_HPP_
#define CTFSM_FSM_TRAITS_EXIT_EVENTS_EXTRACTOR_HPP_

#include <tuple>

#include "final_state.hpp"
#include "utility/type_map.hpp"

namespace ctfsm::pvt
{
    template<class transitions, class result = std::tuple<>>
    struct exit_events_extractor_from_transitions;

    template<class transition, class... transitions, class... results>
        requires(pvt::final<typename transition::value>)
    struct exit_events_extractor_from_transitions<type_map<transition, transitions...>, std::tuple<results...>>
    {
            // Final target state detected, save the event in results
            using result = typename exit_events_extractor_from_transitions<
                type_map<transitions...>,
                std::tuple<typename transition::key, results...>>::result;
    };

    template<class transition, class... transitions, class... results>
        requires(!pvt::final<typename transition::value>)
    struct exit_events_extractor_from_transitions<type_map<transition, transitions...>, std::tuple<results...>>
    {
            using result =
                typename exit_events_extractor_from_transitions<type_map<transitions...>,
                                                                std::tuple<results...>>::result;
    };

    template<class _result>
    struct exit_events_extractor_from_transitions<type_map<>, _result>
    {
            using result = _result;
    };

    // Extract exit events from a tuple of states
    template<class states>
    struct exit_events_extractor;

    template<class t>
        requires(std::same_as<t, std::tuple<>>)
    struct exit_events_extractor<t>
    {
            using result = std::tuple<>;
    };

    template<class state, class... states>
    struct exit_events_extractor<std::tuple<state, states...>>
    {
            using exit_events =
                typename exit_events_extractor_from_transitions<typename state::transitions>::result;

            using result =
                typename type_set_merge<exit_events,
                                        typename exit_events_extractor<std::tuple<states...>>::result>::set;
    };
}// namespace ctfsm::pvt
#endif /* CTFSM_FSM_TRAITS_EXIT_EVENTS_EXTRACTOR_HPP_*/
