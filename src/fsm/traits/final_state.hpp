#ifndef CTFSM_FSM_TRAITS_FINAL_STATE_HPP_
#define CTFSM_FSM_TRAITS_FINAL_STATE_HPP_

#include <type_traits>

#include "utility/type_map.hpp"

namespace ctfsm::pvt
{
    struct final_state
    {
            using transitions = type_map<>;
    };

    template<typename T>
    struct final_state_checker : public std::false_type
    {
    };

    template<>
    struct final_state_checker<final_state> : public std::true_type
    {
    };

    template<typename T>
    concept final = final_state_checker<T>::value;
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_FINAL_STATE_HPP_*/
