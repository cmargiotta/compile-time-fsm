#ifndef CTFSM_FSM_TRAITS_ID_TYPES_VERIFIER_HPP_
#define CTFSM_FSM_TRAITS_ID_TYPES_VERIFIER_HPP_

#include <tuple>
#include <typeinfo>

#include "utility/existence_verifier.hpp"

MAKE_EXISTENCE_VERIFIER(id)

namespace ctfsm::pvt
{
    template<class states>
    struct id_types_verifier;

    template<has_id state, has_id... states>
    struct id_types_verifier<std::tuple<state, states...>>
    {
        public:
            static_assert(std::conjunction_v<std::is_same<decltype(state::id), decltype(states::id)>...>,
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

    template<has_id state>
    struct id_extractor<state>
    {
            static constexpr auto id = state::id;
    };
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_ID_TYPES_VERIFIER_HPP_*/
