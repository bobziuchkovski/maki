//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common.hpp"

namespace pretty_name_ns
{
    struct test{};

    template<class T, class U>
    class templ{};

    struct state
    {
        using conf = awesm::state_conf
            ::pretty_name
        ;

        static const char* pretty_name()
        {
            return "my_state";
        }
    };

    using subsm_transition_table = awesm::transition_table
        ::add<state, awesm::null, awesm::null>
    ;

    struct subsm
    {
        using conf = awesm::subsm_conf
            ::transition_tables<subsm_transition_table>
            ::pretty_name
        ;

        static const char* pretty_name()
        {
            return "my_subsm";
        }
    };

    struct context
    {
    };

    using sm_transition_table = awesm::transition_table
        ::add<state, awesm::null, awesm::null>
    ;

    struct sm_def
    {
        using conf = awesm::subsm_conf
            ::transition_tables<sm_transition_table>
            ::context<context>
            ::pretty_name
        ;

        static const char* pretty_name()
        {
            return "my_sm";
        }
    };

    using sm_t = awesm::sm<sm_def>;

    struct region_path{};
}

TEST_CASE("pretty_name")
{
    REQUIRE
    (
        awesm::pretty_name<pretty_name_ns::test>() ==
        std::string_view{"test"}
    );

    REQUIRE
    (
        awesm::pretty_name<pretty_name_ns::templ<int, pretty_name_ns::test>>() ==
        std::string_view{"templ"}
    );

    REQUIRE
    (
        awesm::pretty_name<pretty_name_ns::state>() ==
        std::string_view{"my_state"}
    );

    REQUIRE
    (
        awesm::pretty_name<pretty_name_ns::subsm>() ==
        std::string_view{"my_subsm"}
    );
}