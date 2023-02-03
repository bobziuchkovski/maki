//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common.hpp"

namespace
{
    struct context
    {
        bool can_access_state0_0 = false;
        bool can_access_state0_1 = false;

        bool can_access_state1_0 = false;
        bool can_access_state1_1 = false;

        bool can_access_state2_0 = false;
        bool can_access_state2_1 = false;

        bool cant_access_state3 = true;
    };

    namespace states
    {
        EMPTY_STATE(idle);
        EMPTY_STATE(state0);
        EMPTY_STATE(state1);
        EMPTY_STATE(state2);
        EMPTY_STATE(state3);
    }

    namespace events
    {
        struct start{};
        struct stop{};
    }

    namespace guards
    {

#define GUARD(NAME) \
    bool NAME(context& ctx) \
    { \
        return ctx.NAME; \
    }

        GUARD(can_access_state0_0)
        GUARD(can_access_state0_1)
        GUARD(can_access_state1_0)
        GUARD(can_access_state1_1)
        GUARD(can_access_state2_0)
        GUARD(can_access_state2_1)
        GUARD(cant_access_state3)

#undef GUARD

        constexpr auto can_access_state0 = awesm::and_<guards::can_access_state0_0, guards::can_access_state0_1>;
        constexpr auto can_access_state1 = awesm::or_<guards::can_access_state1_0, guards::can_access_state1_1>;
        constexpr auto can_access_state2 = awesm::xor_<guards::can_access_state2_0, guards::can_access_state2_1>;
        constexpr auto can_access_state3 = awesm::not_<guards::cant_access_state3>;
    }

    auto sm_transition_table()
    {
        return awesm::transition_table
        <
            awesm::row<states::idle, events::start, states::state0, awesm::noop, guards::can_access_state0>,
            awesm::row<states::idle, events::start, states::state1, awesm::noop, guards::can_access_state1>,
            awesm::row<states::idle, events::start, states::state2, awesm::noop, guards::can_access_state2>,
            awesm::row<states::idle, events::start, states::state3, awesm::noop, guards::can_access_state3>,

            awesm::row<states::state0, events::stop, states::idle>,
            awesm::row<states::state1, events::stop, states::idle>,
            awesm::row<states::state2, events::stop, states::idle>,
            awesm::row<states::state3, events::stop, states::idle>
        >;
    }

    struct sm_def
    {
        using conf_type = awesm::sm_conf<sm_transition_table, context>;
    };

    using sm_t = awesm::sm<sm_def>;
}

TEST_CASE("guard operators")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    sm.start();

    SECTION("and")
    {
       sm.process_event(events::stop{});
       ctx.can_access_state0_0 = false;
       ctx.can_access_state0_1 = false;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::idle>());

       sm.process_event(events::stop{});
       ctx.can_access_state0_0 = false;
       ctx.can_access_state0_1 = true;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::idle>());

       sm.process_event(events::stop{});
       ctx.can_access_state0_0 = true;
       ctx.can_access_state0_1 = false;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::idle>());

       sm.process_event(events::stop{});
       ctx.can_access_state0_0 = true;
       ctx.can_access_state0_1 = true;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::state0>());
    }

    SECTION("or")
    {
       sm.process_event(events::stop{});
       ctx.can_access_state1_0 = false;
       ctx.can_access_state1_1 = false;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::idle>());

       sm.process_event(events::stop{});
       ctx.can_access_state1_0 = false;
       ctx.can_access_state1_1 = true;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::state1>());

       sm.process_event(events::stop{});
       ctx.can_access_state1_0 = true;
       ctx.can_access_state1_1 = false;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::state1>());

       sm.process_event(events::stop{});
       ctx.can_access_state1_0 = true;
       ctx.can_access_state1_1 = true;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::state1>());
    }

    SECTION("xor")
    {
       sm.process_event(events::stop{});
       ctx.can_access_state2_0 = false;
       ctx.can_access_state2_1 = false;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::idle>());

       sm.process_event(events::stop{});
       ctx.can_access_state2_0 = false;
       ctx.can_access_state2_1 = true;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::state2>());

       sm.process_event(events::stop{});
       ctx.can_access_state2_0 = true;
       ctx.can_access_state2_1 = false;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::state2>());

       sm.process_event(events::stop{});
       ctx.can_access_state2_0 = true;
       ctx.can_access_state2_1 = true;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::idle>());
    }

    SECTION("not")
    {
       sm.process_event(events::stop{});
       ctx.cant_access_state3 = true;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::idle>());

       sm.process_event(events::stop{});
       ctx.cant_access_state3 = false;
       sm.process_event(events::start{});
       REQUIRE(sm.is_active_state<states::state3>());
    }
}
