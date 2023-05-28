//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common.hpp"

namespace
{
    struct context{};

#define STATE_OPTIONS \
    X(pretty_name) \
    X(on_event<>) \
    X(on_event_auto) \
    X(on_entry_any) \
    X(on_exit_any)

#define SM_OPTIONS \
    STATE_OPTIONS \
    X(context<context>) \
    X(transition_tables<>)

#define ROOT_SM_OPTIONS \
    SM_OPTIONS \
    X(after_state_transition) \
    X(no_auto_start) \
    X(before_entry) \
    X(before_state_transition) \
    X(no_run_to_completion) \
    X(on_exception) \
    X(on_unprocessed) \
    X(small_event_max_align<0>) \
    X(small_event_max_size<1>)

#define X(option) awesm::detail::options::option,
    using state_conf_tpl_1_t = awesm::state_conf_tpl
    <
        STATE_OPTIONS
        awesm::detail::options::pretty_name
    >;
#undef X

#define X(option) ::option
    using state_conf_tpl_2_t = awesm::state_conf
        STATE_OPTIONS
        ::pretty_name
    ;
#undef X

#define X(option) awesm::detail::options::option,
    using subsm_conf_tpl_1_t = awesm::subsm_conf_tpl
    <
        SM_OPTIONS
        awesm::detail::options::pretty_name
    >;
#undef X

#define X(option) ::option
    using subsm_conf_tpl_2_t = awesm::subsm_conf
        SM_OPTIONS
        ::pretty_name
    ;
#undef X

#define X(option) awesm::detail::options::option,
    using sm_conf_tpl_1_t = awesm::sm_conf_tpl
    <
        ROOT_SM_OPTIONS
        awesm::detail::options::pretty_name
    >;
#undef X

#define X(option) ::option
    using sm_conf_tpl_2_t = awesm::sm_conf
        ROOT_SM_OPTIONS
        ::pretty_name
    ;
#undef X

#undef ROOT_SM_OPTIONS
#undef SM_OPTIONS
#undef STATE_OPTIONS
}

TEST_CASE("conf_subtype_chaining")
{
    REQUIRE(std::is_same_v<state_conf_tpl_1_t, state_conf_tpl_2_t>);
    REQUIRE(std::is_same_v<subsm_conf_tpl_1_t, subsm_conf_tpl_2_t>);
    REQUIRE(std::is_same_v<sm_conf_tpl_1_t, sm_conf_tpl_2_t>);
}