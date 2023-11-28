//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#include <maki.hpp>
#include "common.hpp"

namespace pretty_name_ns
{
    struct test{};

    template<class T, class U>
    class templ{};

    constexpr auto state = maki::state_conf
        .pretty_name("my_state")
    ;

    constexpr auto submachine_transition_table = maki::empty_transition_table
        .add_c<state, maki::null, maki::null_c>
    ;

    constexpr auto submachine = maki::submachine_conf
        .transition_tables(submachine_transition_table)
        .pretty_name("my_submachine")
    ;

    struct context
    {
    };

    constexpr auto transition_table = maki::empty_transition_table
        .add_c<state, maki::null, maki::null_c>
    ;

    struct machine_def
    {
        static constexpr auto conf = maki::submachine_conf
            .transition_tables(transition_table)
            .context<context>()
            .pretty_name("my_sm")
        ;
    };

    using machine_t = maki::machine<machine_def>;

    struct region_path{};
}

TEST_CASE("pretty_name")
{
    REQUIRE
    (
        maki::detail::decayed_type_name<pretty_name_ns::test>() ==
        std::string_view{"test"}
    );

    REQUIRE
    (
        maki::detail::decayed_type_name<pretty_name_ns::templ<int, pretty_name_ns::test>>() ==
        std::string_view{"templ"}
    );

    REQUIRE
    (
        maki::pretty_name<pretty_name_ns::machine_def>() ==
        std::string_view{"my_sm"}
    );

    REQUIRE
    (
        maki::pretty_name<pretty_name_ns::state>() ==
        std::string_view{"my_state"}
    );

    REQUIRE
    (
        maki::pretty_name<pretty_name_ns::submachine>() ==
        std::string_view{"my_submachine"}
    );
}
