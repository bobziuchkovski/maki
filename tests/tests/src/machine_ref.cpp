//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"
#include <memory>

namespace
{
    struct context
    {
    };

    namespace states
    {
        EMPTY_STATE(on)
        EMPTY_STATE(off)
    }

    namespace events
    {
        struct on_button_press{};
        struct off_button_press{};
    }

    constexpr auto transition_table_t = maki::transition_table
        .add_c<states::off, events::on_button_press,  states::on>
        .add_c<states::on,  events::off_button_press, states::off>
    ;

    struct machine_def
    {
        static constexpr auto conf = maki::machine_conf
            .transition_tables(transition_table_t)
            .context<context>()
            .run_to_completion(false)
            .exception_action_me
            (
                [](auto& /*mach*/, const std::exception_ptr& /*eptr*/)
                {
                }
            )
        ;
    };

    using machine_t = maki::machine<machine_def>;
}

TEST_CASE("machine_ref_e")
{
    using machine_ref_e_t =
        maki::machine_ref_e<events::on_button_press, events::off_button_press>
    ;

    auto machine = machine_t{};
    auto pmachine_ref_e_temp = std::make_unique<machine_ref_e_t>(machine); //test ref of ref
    auto machine_ref_e = machine_ref_e_t{*pmachine_ref_e_temp};
    pmachine_ref_e_temp.reset();

    machine.start();

    REQUIRE(machine.is_active_state<states::off>());

    machine_ref_e.process_event(events::on_button_press{});
    REQUIRE(machine.is_active_state<states::on>());

    machine_ref_e.process_event(events::off_button_press{});
    REQUIRE(machine.is_active_state<states::off>());
}
