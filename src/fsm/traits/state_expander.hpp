#ifndef CTFSM_FSM_TRAITS_STATE_EXPANDER_HPP_
#define CTFSM_FSM_TRAITS_STATE_EXPANDER_HPP_

#include <tuple>

#include "parent_injector.hpp"
#include "utility/type_map.hpp"
#include "utility/type_set.hpp"
#include "valid_fsm.hpp"

namespace ctfsm::pvt
{
    /**
     * Explore reachable states
     */
    template<class states, class set = std::tuple<>>
    struct state_expander;

    template<class... set>
    struct state_expander<std::tuple<>, std::tuple<set...>>
    {
            // Base case, there are no more nodes to expand. This handles
            // eventual cycles in the graph too.
        public:
            using states = std::tuple<set...>;
    };

    template<class state, class set>
    struct state_expander<std::tuple<state>, set>
    {
            // Expand a single state and every reachable state from that
        public:
            // Adding State and the reachable nodes from it to set
            using state_set = type_set_merge<
                typename parent_injector<state, typename extract_values<typename state::transitions>::values>::result,
                typename type_set_merge<std::tuple<state>, set>::set>;

            // Expanding the nodes that were not already in set
            using states =
                typename state_expander<typename state_set::delta, typename state_set::set>::states;
    };

    template<class state, class set>
        requires(pvt::valid_fsm<state>)
    struct state_expander<std::tuple<state>, set>
    {
            // State is a nested FSM, no need to expand it
        public:
            // Adding State to set
            using state_set = type_set_merge<std::tuple<state>, set>;

            // Expanding the nodes that were not already in set
            using states =
                typename state_expander<typename state_set::delta, typename state_set::set>::states;
    };

    template<class state, class... states_, class set>
    struct state_expander<std::tuple<state, states_...>, set>
    {
            // Iterate through the states list and expand every state
            using states =
                typename type_set_merge<typename state_expander<std::tuple<state>, set>::states,
                                        typename state_expander<std::tuple<states_...>, set>::states>::set;
    };
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_STATE_EXPANDER_HPP_*/
