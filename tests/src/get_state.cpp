//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common.hpp"
#include <string>

namespace
{
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
            using conf = awesm::state_conf_tpl
            <
                awesm::state_opts::on_entry
            >;

            void on_entry()
            {
                ctx.current_led_color = led_color::red;
            }

            context& ctx;
            led_color color = led_color::red;
        };

        struct emitting_green
        {
            using conf = awesm::state_conf_tpl
            <
                awesm::state_opts::on_entry
            >;

            void on_entry()
            {
                ctx.current_led_color = led_color::green;
            }

            context& ctx;
            led_color color = led_color::green;
        };

        struct emitting_blue
        {
            using conf = awesm::state_conf_tpl
            <
                awesm::state_opts::on_entry
            >;

            void on_entry()
            {
                ctx.current_led_color = led_color::blue;
            }

            context& ctx;
            led_color color = led_color::blue;
        };

        using on_transition_table = awesm::transition_table
            ::add<states::emitting_red,   events::color_button_press, states::emitting_green>
            ::add<states::emitting_green, events::color_button_press, states::emitting_blue>
            ::add<states::emitting_blue,  events::color_button_press, states::emitting_red>
        ;

        struct on
        {
            using conf = awesm::subsm_conf_tpl
            <
                awesm::subsm_opts::transition_tables<on_transition_table>,
                awesm::subsm_opts::on_exit
            >;

            void on_exit()
            {
                ctx.current_led_color = led_color::off;
            }

            context& ctx;
            bool is_on_state = true;
        };
    }

    using sm_transition_table = awesm::transition_table
        ::add<states::off, events::power_button_press, states::on>
        ::add<states::on,  events::power_button_press, states::off>
    ;

    struct sm_def
    {
        using conf = awesm::sm_conf_tpl
        <
            awesm::sm_opts::transition_tables<sm_transition_table>,
            awesm::sm_opts::context<context>
        >;
    };

    using sm_t = awesm::sm<sm_def>;
}

TEST_CASE("state")
{
    auto sm = sm_t{};
    const auto& const_sm = sm;

    using root_region_path = awesm::region_path<sm_def>;
    using on_region_path = root_region_path::add<states::on>;

    auto& red_state = sm.state<on_region_path, states::emitting_red>();
    REQUIRE(red_state.color == led_color::red);

    const auto& green_state = const_sm.state<on_region_path, states::emitting_green>();
    REQUIRE(green_state.color == led_color::green);

    auto& blue_state = sm.state<on_region_path, states::emitting_blue>();
    REQUIRE(blue_state.color == led_color::blue);

    auto& on_state = sm.state<root_region_path, states::on>();
    REQUIRE(on_state.is_on_state);
}
