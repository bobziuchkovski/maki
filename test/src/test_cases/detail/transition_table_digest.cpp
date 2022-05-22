//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm/row.hpp>
#include <awesm/transition_table.hpp>
#include <awesm/none.hpp>
#include <awesm/detail/transition_table_digest.hpp>
#include <catch2/catch.hpp>

namespace
{
    struct state0{};
    struct state1{};
    struct state2{};
    struct state3{};

    struct event0{};
    struct event1{};
    struct event2{};
    struct event3{};

    struct action0{};
    struct action1{};

    struct guard0{};
    struct guard1{};

    using transition_table = awesm::transition_table
    <
        awesm::row<state0,     event0, state1>,
        awesm::row<state1,     event1, state2, awesm::none, guard0>,
        awesm::row<state2,     event2, state3, action0>,
        awesm::row<state3,     event3, state0, action1,     guard1>,
        awesm::row<awesm::any, event3, state0>
    >;

    using digest = awesm::detail::transition_table_digest<transition_table>;

    using action_type_list = awesm::detail::type_list<action0, action1>;
    using guard_type_list = awesm::detail::type_list<guard0, guard1>;
    using state_type_list = awesm::detail::type_list<state0, state1, state2, state3>;
}

TEST_CASE("detail::transition_table_digest")
{
    REQUIRE(std::is_same_v<digest::action_type_list, action_type_list>);
    REQUIRE(std::is_same_v<digest::guard_type_list, guard_type_list>);
    REQUIRE(std::is_same_v<digest::state_type_list, state_type_list>);
    REQUIRE(digest::has_source_state_patterns);
    REQUIRE(!digest::has_none_events);
}
