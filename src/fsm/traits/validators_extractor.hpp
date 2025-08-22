#ifndef CTFSM_FSM_TRAITS_VALIDATORS_EXTRACTOR_HPP_
#define CTFSM_FSM_TRAITS_VALIDATORS_EXTRACTOR_HPP_

#include <tuple>
#include <utility>

#include "utility/type_map.hpp"

namespace ctfsm::pvt
{
    template<typename State, typename Transition>
    struct node
    {
            using key   = std::pair<State, typename Transition::key>;
            using value = Transition;
    };

    template<typename State, typename Transitions>
    struct transitions_extractor;

    template<typename State, typename... Transitions>
    struct transitions_extractor<State, type_map<Transitions...>>
    {
            using result = type_map<node<State, Transitions>...>;
    };

    template<typename States>
    struct validators_extractor;

    template<typename... States>
    struct validators_extractor<std::tuple<States...>>
    {
            using result = typename merge_type_maps<
                typename transitions_extractor<States, typename States::transitions>::result...>::result;
    };
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_VALIDATORS_EXTRACTOR_HPP_*/
