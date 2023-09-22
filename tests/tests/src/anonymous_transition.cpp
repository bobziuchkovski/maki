//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace
{
    struct context{};

    namespace events
    {
        struct go_on{};
    }

    namespace states
    {
        EMPTY_STATE(s0);
        EMPTY_STATE(s1);
        EMPTY_STATE(s2);
        EMPTY_STATE(s3);
        EMPTY_STATE(s4);
    }

    using transition_table_t = maki::transition_table
        ::add<states::s0, events::go_on, states::s1>
        ::add<states::s1, maki::null_t,  states::s2>
        ::add<states::s2, events::go_on, states::s3>
        ::add<states::s3, maki::null_t,  states::s4>
        ::add<states::s4, maki::null_t,  states::s0>
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

TEST_CASE("anonymous transition")
{
    auto machine = machine_t{};

    machine.start();

    machine.process_event(events::go_on{});
    REQUIRE(machine.is_active_state(states::s2));

    machine.process_event(events::go_on{});
    REQUIRE(machine.is_active_state(states::s0));
}
