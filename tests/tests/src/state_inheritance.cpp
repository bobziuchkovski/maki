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

        struct off: on
        {
        };
    }

    namespace events
    {
        struct button_press{};
    }

    constexpr auto transition_table = maki::transition_table_c
        .add<states::off, events::button_press, states::on>
        .add<states::on,  events::button_press, states::off>
    ;

    struct machine_def
    {
        static constexpr auto conf = maki::machine_conf_c
            .set_transition_tables(transition_table)
            .set_context_type<context>()
        ;
    };

    using machine_t = maki::machine<machine_def>;
}

TEST_CASE("state_inheritance")
{
    auto machine = machine_t{};

    REQUIRE(machine.is_active_state<states::off>());

    machine.process_event(events::button_press{});
    REQUIRE(machine.is_active_state<states::on>());

    machine.process_event(events::button_press{});
    REQUIRE(machine.is_active_state<states::off>());
}