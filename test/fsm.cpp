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

struct force
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
};

struct state_off
{
        using transitions
            = ctfsm::type_map<std::pair<switch_on, state_on>, std::pair<force, state_on>>;

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
    REQUIRE(fsm.is_current_state<state_on>());
    REQUIRE(state_on::on_entered);
    REQUIRE(on.transited);

    fsm.handle_event(switch_off());

    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<state_off>());
    REQUIRE(state_on::switched_off);

    REQUIRE_THROWS(fsm.handle_event(off));

    fsm.handle_event<switch_on>();

    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<state_on>());

    fsm(switch_off());
    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<state_off>());

    fsm(on);
    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(fsm.is_current_state<state_on>());

    fsm(switch_off());
    REQUIRE(fsm.get_current_state_id() == "OFF");
    REQUIRE(fsm.is_current_state<state_off>());

    fsm(force {});
    REQUIRE(fsm.get_current_state_id() == "ON");
    REQUIRE(state_on::forced);
    REQUIRE(fsm.is_current_state<state_on>());

    auto current_id = fsm.invoke_on_current([](auto&& current, auto& fsm) { return current.id; });
    REQUIRE(current_id == "ON");

    fsm.reset();
    REQUIRE(fsm.get_current_state_id() == "ON");
}
