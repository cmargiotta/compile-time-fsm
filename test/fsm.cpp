#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>

#include <cstddef>
#include <string_view>

#include <fsm/fsm.hpp>

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

struct state_off;
struct state_on;

struct state_on
{
        using transitions
            = ctfsm::type_map<std::pair<switch_on, state_on>, std::pair<switch_off, state_off>>;

        static constexpr std::string_view id {"ON"};
        static constinit inline bool      switched_off = false;
        static constinit inline bool      on_entered   = false;

        void on_enter(switch_on&)
        {
            on_entered = true;
        }

        void on_enter()
        {
        }

        void on_exit(switch_off&)
        {
            switched_off = true;
        }

        void on_exit()
        {
        }
};

struct state_off
{
        using transitions = ctfsm::type_map<std::pair<switch_on, state_on>>;

        static constexpr std::string_view id {"OFF"};
        static constinit inline bool      entered = false;

        void on_enter()
        {
            entered = true;
        }
};

TEST_CASE("FSM basic usage", "[fsm]")
{
    ctfsm::fsm<state_on> fsm;
    switch_on            on;
    switch_off           off;

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(!state_on::on_entered);

    fsm.handle_event(on);

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(state_on::on_entered);
    REQUIRE(on.transited);

    fsm.handle_event(switch_off());

    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(state_on::switched_off);

    fsm.handle_event<switch_on>();

    REQUIRE(fsm.get_current_state_id() == "ON");

    fsm(switch_off());
    REQUIRE(fsm.get_current_state_id() == "OFF");

    fsm(on);
    REQUIRE(fsm.get_current_state_id() == "ON");

    BENCHMARK("Changing state 1000 times")
    {
        for (std::size_t i = 0; i < 500; ++i)
        {
            fsm(off);
            fsm(on);
        }
    };
}