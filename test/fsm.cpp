#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>

#include <cstddef>
#include <string_view>

#include <ctfsm.hpp>

struct switch_on
{
        bool transited = false;

        void on_transit()
        {
            transited = true;
        }
};

struct switch_off
{
};

struct force
{
};

struct state_off;
struct state_on;

struct state_on
{
        using transitions = ctfsm::transition_map<ctfsm::transition<switch_on, state_on>,
                                                  ctfsm::transition<switch_off, state_off>>;

        static constexpr std::string_view id {"ON"};
        static constinit inline bool      switched_off = false;
        static constinit inline bool      on_entered   = false;
        static constinit inline bool      forced       = false;

        void on_enter(switch_on&)
        {
            on_entered = true;
        }

        void on_enter()
        {
            forced = true;
        }

        void on_exit()
        {
            switched_off = true;
        }

        auto work(auto& fsm) -> bool
        {
            return fsm.template handle_event<switch_off>();
        }
};

struct state_off
{
        using transitions
            = ctfsm::transition_map<ctfsm::transition<switch_on, state_on>, ctfsm::transition<force, state_on>>;

        static constexpr std::string_view id {"OFF"};
        static constinit inline bool      entered = false;
        static constinit inline bool      forced  = false;

        void on_exit(force)
        {
            forced = true;
        }

        void on_enter()
        {
            entered = true;
        }

        auto work(auto& fsm) -> bool
        {
            return fsm.template handle_event<force>();
        }
};

TEST_CASE("FSM basic usage", "[fsm]")
{
    ctfsm::fsm<state_on> fsm;
    switch_on            on;
    switch_off           off;

    static_assert(ctfsm::pvt::valid_fsm<decltype(fsm)>);

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(!state_on::on_entered);

    REQUIRE(fsm.handle_event(on));

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<state_on>());
    REQUIRE(state_on::on_entered);
    REQUIRE(on.transited);

    REQUIRE(fsm.handle_event(switch_off()));

    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<state_off>());
    REQUIRE(state_on::switched_off);

    REQUIRE_FALSE(fsm.handle_event(off));

    REQUIRE(fsm.handle_event<switch_on>());

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<state_on>());

    REQUIRE(fsm(switch_off()));
    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<state_off>());

    REQUIRE(fsm(on));
    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<state_on>());

    REQUIRE(fsm(switch_off()));
    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<state_off>());

    REQUIRE(fsm(force {}));
    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(state_on::forced);
    REQUIRE(fsm.is_current_state<state_on>());

    auto current_id = fsm.invoke_on_current([](auto&& current, auto& fsm) { return current.id; });
    REQUIRE(current_id == "ON");

    fsm.reset();
    REQUIRE(fsm.get_current_state_id() == "ON");
}

struct move_to_switch
{
};

struct robot_idle;

struct robot_discharging
{
        using transitions = ctfsm::transition_map<ctfsm::transition<move_to_switch, robot_idle>>;

        static constexpr std::string_view id {"DISCHARGING"};

        auto work(auto& fsm) -> bool
        {
            return fsm.template handle_event<move_to_switch>();
        }
};

struct robot_idle
{
        using transitions = ctfsm::transition_map<ctfsm::nested<switch_on, state_on, force>,
                                                  ctfsm::transition<force, robot_discharging>>;

        static constexpr std::string_view id {"IDLE"};
        static constinit inline bool      force_detected = false;

        void on_exit(force)
        {
            force_detected = true;
        }

        auto work(auto& fsm) -> bool
        {
            return fsm.template handle_event<switch_on>();
        }
};

TEST_CASE("FSM with nested FSM handling with internal methods", "[fsm]")
{
    ctfsm::fsm<robot_idle> fsm;

    static_assert(ctfsm::pvt::valid_fsm<decltype(fsm)>);

    REQUIRE(fsm.get_current_state_id() == "IDLE");
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& /*fsm*/) { return state.id; }) == "IDLE");

    REQUIRE(fsm.invoke_on_current([](auto& state, auto& fsm) { return state.work(fsm); }));
    // We are in a nested FSM (switch_on), but from outside we are still in IDLE
    REQUIRE(fsm.get_current_state_id() == "IDLE");
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& /*fsm*/) { return state.id; }) == "ON");

    // State on -> state off
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& fsm) { return state.work(fsm); }));
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& /*fsm*/) { return state.id; }) == "OFF");
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    // State off sends force
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& fsm) { return state.work(fsm); }));
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& /*fsm*/) { return state.id; }) == "DISCHARGING");
    REQUIRE(fsm.get_current_state_id() == "DISCHARGING");
}

TEST_CASE("FSM with nested FSM", "[fsm]")
{
    ctfsm::fsm<robot_idle> fsm;

    static_assert(ctfsm::pvt::valid_fsm<decltype(fsm)>);

    REQUIRE(fsm.get_current_state_id() == "IDLE");

    REQUIRE(fsm.handle_event<switch_on>());
    // We are in a nested FSM, but from outside we are still in IDLE
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    REQUIRE_FALSE(fsm.handle_event<force>());
    REQUIRE(fsm.handle_event<switch_off>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    state_off::forced = false;
    REQUIRE(fsm.handle_event<force>());
    REQUIRE(fsm.get_current_state_id() == "DISCHARGING");
    REQUIRE(state_off::forced);

    // Check if the nested FSM state is coherent
    REQUIRE(fsm.handle_event<move_to_switch>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    // Same sequence
    REQUIRE(fsm.handle_event<switch_on>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    REQUIRE_FALSE(fsm.handle_event<force>());
    REQUIRE(fsm.handle_event<switch_off>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");
}
