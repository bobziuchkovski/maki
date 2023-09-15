//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace
{
    struct context
    {
    };

    namespace states
    {
        EMPTY_STATE(on);
        EMPTY_STATE(off);
    }

    namespace events
    {
        struct button_press{};
    }

    using transition_table_t = maki::transition_table
        ::add<states::off, events::button_press, states::on>
        ::add<states::on,  events::button_press, states::off>
    ;

    struct machine_def
    {
        using conf = maki::machine_conf
            ::transition_tables<transition_table_t>
            ::context<context>
            ::no_auto_start
            ::no_run_to_completion
            ::on_exception
        ;

        void on_exception(const std::exception_ptr& /*eptr*/)
        {
        }
    };

    using machine_t = maki::machine<machine_def>;
}

TEST_CASE("basic_transition")
{
    auto machine = machine_t{};

    REQUIRE(!machine.is_running());

    machine.start();
    REQUIRE(machine.is_active_state(states::off));

    machine.process_event(events::button_press{});
    REQUIRE(machine.is_active_state(states::on));

    machine.process_event(events::button_press{});
    REQUIRE(machine.is_active_state(states::off));
}
