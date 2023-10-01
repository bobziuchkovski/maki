//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace
{
    struct machine_def;
    using machine_t = maki::machine<machine_def>;

    enum class led_color
    {
        off,
        red,
        green,
        blue
    };

    struct context
    {
        led_color current_led_color = led_color::off;
    };

    namespace events
    {
        struct power_button_press{};
        struct color_button_press{};
    }

    namespace states
    {
        EMPTY_STATE(off);

        struct emitting_red
        {
            static constexpr auto conf = maki::state_conf_c
                .enable_on_entry()
            ;

            void on_entry()
            {
                ctx.current_led_color = led_color::red;
            }

            machine_t& machine;
            context& ctx;
        };

        struct emitting_green
        {
            static constexpr auto conf = maki::state_conf_c
                .enable_on_entry()
            ;

            void on_entry()
            {
                ctx.current_led_color = led_color::green;
            }

            machine_t& machine;
            context& ctx;
        };

        struct emitting_blue
        {
            static constexpr auto conf = maki::state_conf_c
                .enable_on_entry()
            ;

            void on_entry()
            {
                ctx.current_led_color = led_color::blue;
            }

            machine_t& machine;
            context& ctx;
        };

        constexpr auto on_transition_table = maki::transition_table_c
            .add<states::emitting_red,   events::color_button_press, states::emitting_green>
            .add<states::emitting_green, events::color_button_press, states::emitting_blue>
            .add<states::emitting_blue,  events::color_button_press, states::emitting_red>
        ;

        struct on
        {
            static constexpr auto conf = maki::submachine_conf_c
                .set_transition_tables(on_transition_table)
                .enable_on_exit()
            ;

            void on_exit()
            {
                ctx.current_led_color = led_color::off;
            }

            context& ctx;
        };
    }

    constexpr auto transition_table = maki::transition_table_c
        .add<states::off, events::power_button_press, states::on>
        .add<states::on,  events::power_button_press, states::off>
    ;

    struct machine_def
    {
        static constexpr auto conf = maki::machine_conf_c
            .set_transition_tables(transition_table)
            .set_context_type<context>()
        ;
    };
}

TEST_CASE("submachine")
{
    using machine_on_region_path_t = maki::region_path<machine_def>::add<states::on>;

    auto machine = machine_t{};
    auto& ctx = machine.context();

    machine.start();

    REQUIRE(machine.is_active_state<states::off>());
    REQUIRE(!machine.is_running<machine_on_region_path_t>());
    REQUIRE(ctx.current_led_color == led_color::off);

    machine.process_event(events::power_button_press{});
    REQUIRE(machine.is_active_state<states::on>());
    REQUIRE(machine.is_active_state<machine_on_region_path_t, states::emitting_red>());
    REQUIRE(ctx.current_led_color == led_color::red);

    machine.process_event(events::color_button_press{});
    REQUIRE(machine.is_active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::green);

    machine.process_event(events::color_button_press{});
    REQUIRE(machine.is_active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::blue);

    machine.process_event(events::power_button_press{});
    REQUIRE(machine.is_active_state<states::off>());
    REQUIRE(ctx.current_led_color == led_color::off);

    machine.process_event(events::power_button_press{});
    REQUIRE(machine.is_active_state<states::on>());
    REQUIRE(ctx.current_led_color == led_color::red);
}
