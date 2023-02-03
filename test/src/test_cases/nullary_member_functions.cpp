//Copyright Florian Goujeon 2021 - 2022.
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

    namespace events
    {
        struct e1
        {
        };

        struct e2
        {
        };
    }

    namespace states
    {
        EMPTY_STATE(off);

        struct on
        {
            using conf_type = awesm::state_conf
            <
                awesm::state_options::on_entry_any,
                awesm::state_options::on_exit_any
            >;

            void on_entry(const events::e1&)
            {
                ctx.out += "on_entry(e1);";
            }

            void on_entry()
            {
                ctx.out += "on_entry();";
            }

            void on_exit(const events::e1&)
            {
                ctx.out += "on_exit(e1);";
            }

            void on_exit()
            {
                ctx.out += "on_exit();";
            }

            context& ctx;
        };
    }

    constexpr auto action = [](auto& /*sm*/, context& ctx, const auto& event)
    {
        using event_type = std::decay_t<decltype(event)>;
        if constexpr(std::is_same_v<event_type, events::e1>)
        {
            ctx.out += "execute(e1);";
        }
        else
        {
            ctx.out += "execute();";
        }
    };

    constexpr auto guard = [](auto& /*sm*/, context& ctx, const auto& event)
    {
        using event_type = std::decay_t<decltype(event)>;
        if constexpr(std::is_same_v<event_type, events::e1>)
        {
            ctx.out += "check(e1);";
        }
        else
        {
            ctx.out += "check();";
        }
        return true;
    };

    auto sm_transition_table()
    {
        return awesm::transition_table
        <
            awesm::row<states::off, events::e1, states::on,  action, guard>,
            awesm::row<states::off, events::e2, states::on,  action, guard>,
            awesm::row<states::on,  events::e1, states::off, action, guard>,
            awesm::row<states::on,  events::e2, states::off, action, guard>
        >;
    }

    struct sm_def
    {
        using conf_type = awesm::sm_conf<sm_transition_table, context>;
    };

    using sm_t = awesm::sm<sm_def>;
}

TEST_CASE("nullary_member_functions")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    sm.start();

    ctx.out.clear();
    sm.process_event(events::e1{});
    REQUIRE(ctx.out == "check(e1);execute(e1);on_entry(e1);");

    ctx.out.clear();
    sm.process_event(events::e1{});
    REQUIRE(ctx.out == "check(e1);on_exit(e1);execute(e1);");

    ctx.out.clear();
    sm.process_event(events::e2{});
    REQUIRE(ctx.out == "check();execute();on_entry();");

    ctx.out.clear();
    sm.process_event(events::e2{});
    REQUIRE(ctx.out == "check();on_exit();execute();");
}
