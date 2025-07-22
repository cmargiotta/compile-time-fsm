#ifndef CTFSM_FSM_TRAITS_VARIANT_BUILDER_HPP_
#define CTFSM_FSM_TRAITS_VARIANT_BUILDER_HPP_

#include <tuple>
#include <variant>

namespace ctfsm::pvt
{
    // Build an std::variant type from a tuple of states
    template<class fsms, class states>
    struct variant_builder;

    template<class... states, class... fsms>
    struct variant_builder<std::tuple<fsms...>, std::tuple<states...>>
    {
        public:
            using variant = std::variant<states *..., fsms *...>;
    };

    template<class states, class fsms>
    using variant_builder_t = typename variant_builder<states, fsms>::variant;
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_VARIANT_BUILDER_HPP_*/
