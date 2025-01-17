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
        struct idle
        {
            static constexpr auto conf = maki::default_state_conf
                .enable_on_entry()
                .enable_on_event_for<maki::any>()
                .enable_on_exit()
            ;

            void on_entry()
            {
                ctx.out += "idle::on_entry;";
            }

            template<class Event>
            void on_event(const Event&)
            {
                ctx.out += "idle::on_event;";
            }

            void on_exit()
            {
                ctx.out += "idle::on_exit;";
            }

            context& ctx;
        };

        struct running
        {
            static constexpr auto conf = maki::default_state_conf
                .enable_on_entry()
                .enable_on_event_for<maki::any>()
                .enable_on_exit()
            ;

            void on_entry()
            {
                ctx.out += "running::on_entry;";
            }

            template<class Event>
            void on_event(const Event&)
            {
                ctx.out += "running::on_event;";
            }

            void on_exit()
            {
                ctx.out += "running::on_exit;";
            }

            context& ctx;
        };
    }

    namespace actions
    {
        void beep(context& ctx)
        {
            ctx.out += "beep;";
        }
    }

    constexpr auto transition_table = maki::empty_transition_table
        .add_c<states::idle,    events::power_button_press, states::running>
        .add_c<states::running, events::power_button_press, states::idle>
        .add_c<states::running, events::beep_button_press,  maki::null, actions::beep>
    ;

    struct machine_def
    {
        static constexpr auto conf = maki::default_machine_conf
            .set_transition_tables(transition_table)
            .set_context<context>()
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
