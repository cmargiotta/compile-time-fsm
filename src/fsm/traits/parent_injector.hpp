#ifndef CTFSM_FSM_TRAITS_PARENT_INJECTOR_HPP_
#define CTFSM_FSM_TRAITS_PARENT_INJECTOR_HPP_

#include <tuple>

namespace ctfsm
{
    template<class I, class P>
    class fsm;
}

namespace ctfsm::pvt
{
    // Inject the _parent_state parameter to a child
    template<class parent, class child>
    struct parent_injector_single
    {
            using result = child;
    };

    template<class parent, class i, class p>
    struct parent_injector_single<parent, fsm<i, p>>
    {
            // Current is a nested FSM, injecting parent
            using result = fsm<i, parent>;
    };

    // Inject the _parent_state parameter to nested fsms
    template<class parent, class children_tuple, class result = std::tuple<>>
    struct parent_injector;

    template<class parent, class i, class p, class... children, class... result>
    struct parent_injector<parent, std::tuple<fsm<i, p>, children...>, std::tuple<result...>>
        : parent_injector<parent, std::tuple<children...>, std::tuple<fsm<i, parent>, result...>>
    {
            // Current is a nested FSM, injecting parent
    };

    template<class parent, class current, class... children, class... result>
    struct parent_injector<parent, std::tuple<current, children...>, std::tuple<result...>>
        : parent_injector<parent, std::tuple<children...>, std::tuple<current, result...>>
    {
            // Current is a normal state, nothing to do
    };

    template<class parent, class _result>
    struct parent_injector<parent, std::tuple<>, _result>
    {
            // Base case, injection done
            using result = _result;
    };
}// namespace ctfsm::pvt

#endif /* CTFSM_FSM_TRAITS_PARENT_INJECTOR_HPP_*/
