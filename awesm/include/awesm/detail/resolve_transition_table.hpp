//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_RESOLVE_TRANSITION_TABLE_HPP
#define AWESM_DETAIL_RESOLVE_TRANSITION_TABLE_HPP

#include "alternative.hpp"
#include "tlu.hpp"
#include "../row.hpp"
#include "../type_patterns.hpp"
#include "../transition_table.hpp"

namespace awesm::detail
{

/*
Replaces each pattern-start-state row with equivalent set of n rows, n being the
number of state types that match the pattern.
The list of state types is given as second argument to save some build time.
It's already computed at the time we call resolve_transition_table.

For example, the following resolved_transition_table_t type...:
    using transition_table = awesm::transition_table
    <
        awesm::row<state0,     event0, state1, action0, guard0>,
        awesm::row<state1,     event1, state2, action1, guard1>,
        awesm::row<awesm::any, event2, state3, action2, guard2>
    >;
    using resolved_transition_table_t =
        awesm::detail::resolve_transition_table
        <
            transition_table,
            tuple<state0, state1, state2, state3>
        >
    ;

... is equivalent to this type:
    using resolved_transition_table_t = awesm::transition_table
    <
        awesm::row<state0, event0, state1, action0, guard0>,
        awesm::row<state1, event1, state2, action1, guard1>,
        awesm::row<state0, event2, state3, action2, guard2>,
        awesm::row<state1, event2, state3, action2, guard2>,
        awesm::row<state2, event2, state3, action2, guard2>,
        awesm::row<state3, event2, state3, action2, guard2>
    >;
*/

namespace resolve_transition_table_detail
{
#ifdef _MSC_VER
    template<class RowWithPattern>
    struct add_resolved_row_holder;

    template<class SourceState, class Event, class TargetState, const auto& Action, const auto& Guard>
    struct add_resolved_row_holder<row<SourceState, Event, TargetState, Action, Guard>>
    {
        template<class TransitionTable, class State>
        using type = alternative_t
        <
            SourceState::template matches<State>,
            tlu::push_back_t
            <
                TransitionTable,
                row<State, Event, TargetState, Action, Guard>
            >,
            TransitionTable
        >;
    };
#else
    template<class RowWithPattern>
    struct add_resolved_row_holder
    {
        template<class TransitionTable, class State>
        using type = alternative_t
        <
            RowWithPattern::source_state_type::template matches<State>,
            tlu::push_back_t
            <
                TransitionTable,
                row
                <
                    State,
                    typename RowWithPattern::event_type,
                    typename RowWithPattern::target_state_type,
                    RowWithPattern::get_action(),
                    RowWithPattern::get_guard()
                >
            >,
            TransitionTable
        >;
    };
#endif

    /*
    Return TransitionTable with n new rows, n being the number of matching
    states.
    */
    template<class TransitionTable, class Row, class StateTypeList>
    struct add_row_with_pattern_holder
    {
        template<bool = true> //Dummy template for lazy evaluation
        using type = tlu::left_fold_t
        <
            StateTypeList,
            add_resolved_row_holder<Row>::template type,
            TransitionTable
        >;
    };

    template<class TransitionTable, class Row>
    struct add_row_without_pattern_holder
    {
        template<bool = true> //Dummy template for lazy evaluation
        using type = tlu::push_back_t<TransitionTable, Row>;
    };

    //We need a holder to pass StateTypeList
    template<class StateTypeList>
    struct add_row_holder
    {
        /*
        Return TransitionTable with added rows.
        Added rows are either:
        - Row as is if Row::source_state_type isn't a pattern;
        - n resolved rows otherwise (n being the number of states that match the
          pattern).
        */
        template<class TransitionTable, class Row>
        using type = typename alternative_t
        <
            is_type_pattern_v<typename Row::source_state_type>,
            add_row_with_pattern_holder<TransitionTable, Row, StateTypeList>,
            add_row_without_pattern_holder<TransitionTable, Row>
        >::template type<>;
    };
}

template<class TransitionTable, class StateTypeList, bool HasSourceStatePatterns>
struct resolve_transition_table;

template<class TransitionTable, class StateTypeList>
struct resolve_transition_table<TransitionTable, StateTypeList, true>
{
    using type = tlu::left_fold_t
    <
        TransitionTable,
        resolve_transition_table_detail::add_row_holder<StateTypeList>::template type,
        transition_table_t<>
    >;
};

template<class TransitionTable, class StateTypeList>
struct resolve_transition_table<TransitionTable, StateTypeList, false>
{
    using type = TransitionTable;
};

template<class TransitionTable, class StateTypeList, bool HasSourceStatePatterns>
using resolve_transition_table_t = typename resolve_transition_table
<
    TransitionTable,
    StateTypeList,
    HasSourceStatePatterns
>::type;

} //namespace

#endif
