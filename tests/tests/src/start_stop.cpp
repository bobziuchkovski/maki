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
    struct context
    {
        std::string out;
    };

    namespace states
    {
        struct s0
        {
            using conf = awesm::state_conf
                ::on_entry_any
                ::on_exit_any
            ;

            void on_entry()
            {
                ctx.out += "s0::on_entry;";
            }

            void on_exit()
            {
                ctx.out += "s0::on_exit;";
            }

            context& ctx;
        };

        struct s1
        {
            using conf = awesm::state_conf
                ::on_entry_any
                ::on_exit_any
            ;

            void on_entry()
            {
                ctx.out += "s1::on_entry;";
            }

            void on_exit()
            {
                ctx.out += "s1::on_exit;";
            }

            context& ctx;
        };
    }

    namespace events
    {
        struct button_press{};
    }

    using sm_transition_table = awesm::transition_table
        ::add<states::s0, awesm::null,          states::s1>
        ::add<states::s1, events::button_press, states::s0>
    ;

    struct sm_def
    {
        using conf = awesm::sm_conf
            ::transition_tables<sm_transition_table>
            ::context<context>
            ::no_auto_start
        ;
    };

    using sm_t = awesm::sm<sm_def>;
}

TEST_CASE("start_stop")
{
    auto sm = sm_t{};
    auto& ctx = sm.context();

    REQUIRE(!sm.is_running());
    REQUIRE(ctx.out == "");

    sm.start();
    REQUIRE(sm.is_active_state<states::s1>());
    REQUIRE(ctx.out == "s0::on_entry;s0::on_exit;s1::on_entry;");

    ctx.out.clear();
    sm.stop();
    REQUIRE(!sm.is_running());
    REQUIRE(ctx.out == "s1::on_exit;");
}