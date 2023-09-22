//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"
#include <string>

namespace
{
    struct context
    {
        std::string out;
    };

    namespace events
    {
        struct power_button_press{};
        struct beep_button_press{};
    }

    namespace states
    {
        constexpr auto idle = maki::state_c
            .set_on_entry([](auto& mach, const auto& /*event*/)
            {
                mach.context().out += "idle::on_entry;";
            })
            .set_on_event([](auto& mach, const auto& /*event*/)
            {
                mach.context().out += "idle::on_event;";
            })
            .set_on_exit([](auto& mach, const auto& /*event*/)
            {
                mach.context().out += "idle::on_exit;";
            })
        ;

        constexpr auto running = maki::state_c
            .set_on_entry([](auto& mach, const auto& /*event*/)
            {
                mach.context().out += "running::on_entry;";
            })
            .set_on_event([](auto& mach, const auto& /*event*/)
            {
                mach.context().out += "running::on_event;";
            })
            .set_on_exit([](auto& mach, const auto& /*event*/)
            {
                mach.context().out += "running::on_exit;";
            })
        ;
    }

    namespace actions
    {
        void beep(context& ctx)
        {
            ctx.out += "beep;";
        }
    }

    using transition_table_t = maki::transition_table
        ::add<states::idle,    events::power_button_press, states::running>
        ::add<states::running, events::power_button_press, states::idle>
        ::add<states::running, events::beep_button_press,  maki::null, actions::beep>
    ;

    struct machine_def
    {
        using conf = maki::machine_conf
            ::transition_tables<transition_table_t>
            ::context<context>
        ;
    };

    using machine_t = maki::machine<machine_def>;
}

TEST_CASE("internal_transition_in_transition_table")
{
    auto machine = machine_t{};
    auto& ctx = machine.context();

    machine.start();
    REQUIRE(ctx.out == "idle::on_entry;");

    ctx.out.clear();
    machine.process_event(events::power_button_press{});
    REQUIRE(ctx.out == "idle::on_exit;running::on_entry;");

    ctx.out.clear();
    machine.process_event(events::beep_button_press{});
    REQUIRE(ctx.out == "beep;");
}
