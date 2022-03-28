//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#ifndef FGFSM_DETAIL_TRANSITION_TABLE_DIGEST_HPP
#define FGFSM_DETAIL_TRANSITION_TABLE_DIGEST_HPP

#include "tlu.hpp"
#include "../any.hpp"
#include "../none.hpp"
#include "../transition_table.hpp"
#include <tuple>

namespace fgfsm::detail
{

/*
Creates a set of tuples containing all the action types, guard types and state
types of a given transition_table.

For example, the following digest type...:
    using transition_table = fgfsm::transition_table
    <
        fgfsm::row<state0, event0, state1>,
        fgfsm::row<state1, event1, state2>,
        fgfsm::row<state2, event2, state3>,
        fgfsm::row<state3, event3, state0>
    >;
    using digest = fgfsm::detail::transition_table_digest<transition_table>;

... is equivalent to this type:
    struct digest
    {
        using state_tuple = std::tuple<state0, state1, state2, state3>;
        using event_tuple = std::tuple<event0, event1, event2, event3>;
    };
*/

namespace transition_table_digest_detail
{
    template<class TList, class U>
    using push_back_unique_if_not_any_or_none = tlu::push_back_if
    <
        TList,
        U,
        !tlu::contains<TList, U> && !std::is_same_v<U, any> && !std::is_same_v<U, none>
    >;

    template<class StateTuple, class EventTuple, class... Rows>
    struct helper;

    template<class StateTuple, class EventTuple, class Row, class... Rows>
    struct helper<StateTuple, EventTuple, Row, Rows...>:
        helper
        <
            push_back_unique_if_not_any_or_none
            <
                push_back_unique_if_not_any_or_none<StateTuple, typename Row::start_state_t>,
                typename Row::target_state_t
            >,
            tlu::push_back_unique<EventTuple, typename Row::event_t>,
            Rows...
        >
    {
    };

    //terminal case
    template<class StateTuple, class EventTuple>
    struct helper<StateTuple, EventTuple>
    {
        using state_tuple = StateTuple;
        using event_tuple = EventTuple;
    };
}

template<class TransitionTable>
struct transition_table_digest;

template<class... Rows>
struct transition_table_digest<transition_table<Rows...>>:
    transition_table_digest_detail::helper
    <
        std::tuple<>,
        std::tuple<>,
        Rows...
    >
{
};

} //namespace

#endif
