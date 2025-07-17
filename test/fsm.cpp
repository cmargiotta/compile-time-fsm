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

struct explode
{
};

struct off;
struct on;

struct on
{
        using transitions
            = ctfsm::transition_map<ctfsm::transition<switch_on, on>, ctfsm::transition<switch_off, off>>;

        static constexpr std::string_view id {"ON"};
        static constinit inline bool      switched_off = false;
        static constinit inline bool      on_entered   = false;
        static constinit inline bool      forced       = false;

        constexpr on() = default;
        // States can be non-copyable
        on(const on&) = delete;

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

struct off
{
        using transitions = ctfsm::transition_map<ctfsm::transition<switch_on, on>>;

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
            return true;
        }
};

TEST_CASE("FSM basic usage", "[fsm]")
{
    ctfsm::fsm<on> fsm;
    switch_on      on;
    switch_off     off;

    static_assert(ctfsm::pvt::valid_fsm<decltype(fsm)>);

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(!on::on_entered);

    REQUIRE(fsm.handle_event(on));

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<::on>());
    REQUIRE(on::on_entered);
    REQUIRE(on.transited);

    REQUIRE(fsm.handle_event(switch_off()));

    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<::off>());
    REQUIRE(on::switched_off);

    REQUIRE_FALSE(fsm.handle_event(off));

    REQUIRE(fsm.handle_event<switch_on>());

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<::on>());

    REQUIRE(fsm(switch_off()));
    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<::off>());

    REQUIRE(fsm(on));
    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<::on>());

    REQUIRE(fsm(switch_off()));
    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<::off>());

    fsm.reset();
    REQUIRE(fsm.get_current_state_id() == "ON");
}

namespace nested_test
{
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

            constexpr state_on() = default;
            // States can be non-copyable
            state_on(const state_on&) = delete;

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
            using transitions = ctfsm::transition_map<ctfsm::transition<switch_on, state_on>,
                                                      ctfsm::exit_transition<force>,
                                                      ctfsm::exit_transition<explode>>;

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
                return fsm.template handle_event<explode>();
            }
    };

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
            using transitions = ctfsm::transition_map<ctfsm::nested<switch_on, state_on>,
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
}// namespace nested_test

TEST_CASE("FSM with nested FSM handling with internal methods", "[fsm]")
{
    ctfsm::fsm<nested_test::robot_idle> fsm;

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

    // Send explode
    REQUIRE(fsm.handle_event<explode>());
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& /*fsm*/) { return state.id; }) == "IDLE");
    REQUIRE(fsm.get_current_state_id() == "IDLE");
}

TEST_CASE("FSM with nested FSM", "[fsm]")
{
    ctfsm::fsm<nested_test::robot_idle> fsm;

    static_assert(ctfsm::pvt::valid_fsm<decltype(fsm)>);

    REQUIRE(fsm.get_current_state_id() == "IDLE");

    REQUIRE(fsm.handle_event<switch_on>());
    // We are in a nested FSM, but from outside we are still in IDLE
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    REQUIRE_FALSE(fsm.handle_event<force>());
    REQUIRE(fsm.handle_event<switch_off>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");
    REQUIRE(fsm.invoke_on_current([](auto& state, auto& /*fsm*/) { return state.id; }) == "OFF");

    nested_test::state_off::forced = false;
    REQUIRE(fsm.handle_event<force>());
    REQUIRE(fsm.get_current_state_id() == "DISCHARGING");
    REQUIRE(nested_test::state_off::forced);

    // Check if the nested FSM state is coherent
    REQUIRE(fsm.handle_event<nested_test::move_to_switch>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    // Same sequence
    REQUIRE(fsm.handle_event<switch_on>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");

    REQUIRE_FALSE(fsm.handle_event<force>());
    REQUIRE(fsm.handle_event<switch_off>());
    REQUIRE(fsm.get_current_state_id() == "IDLE");
}
