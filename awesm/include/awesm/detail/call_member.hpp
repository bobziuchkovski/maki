//Copyright Florian Goujeon 2021.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_without_event.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_CALL_MEMBER_HPP
#define AWESM_DETAIL_CALL_MEMBER_HPP

#include "state_traits.hpp"
#include <type_traits>
#include <utility>

namespace awesm::detail
{

template<class State, class Sm, class Event>
auto call_on_entry_impl(State& state, Sm* pmach, const Event* pevent) ->
    decltype(state.on_entry(*pmach, *pevent))
{
    state.on_entry(*pmach, *pevent);
}

template<class State, class Event>
auto call_on_entry_impl(State& state, void* /*pmach*/, const Event* pevent) ->
    decltype(state.on_entry(*pevent))
{
    state.on_entry(*pevent);
}

template<class State>
auto call_on_entry_impl(State& state, void* /*pmach*/, const void* /*pevent*/) ->
    decltype(state.on_entry())
{
    state.on_entry();
}

template<class State, class Sm, class Event>
void call_on_entry
(
    [[maybe_unused]] State& state,
    [[maybe_unused]] Sm& mach,
    [[maybe_unused]] const Event& event
)
{
    if constexpr(state_traits::requires_on_entry_v<State>)
    {
        call_on_entry_impl(state, &mach, &event);
    }
}

template<class State, class Event>
void call_on_event
(
    [[maybe_unused]] State& state,
    [[maybe_unused]] const Event& event
)
{
    if constexpr(state_traits::requires_on_event_v<State, Event>)
    {
        state.on_event(event);
    }
}

template<class State, class Sm, class Event>
auto call_on_exit_impl(State& state, Sm* pmach, const Event* pevent) ->
    decltype(state.on_exit(*pmach, *pevent))
{
    state.on_exit(*pmach, *pevent);
}

template<class State, class Event>
auto call_on_exit_impl(State& state, void* /*pmach*/, const Event* pevent) ->
    decltype(state.on_exit(*pevent))
{
    state.on_exit(*pevent);
}

template<class State>
auto call_on_exit_impl(State& state, void* /*pmach*/, const void* /*pevent*/) ->
    decltype(state.on_exit())
{
    state.on_exit();
}

template<class State, class Sm, class Event>
void call_on_exit
(
    [[maybe_unused]] State& state,
    [[maybe_unused]] Sm& mach,
    [[maybe_unused]] const Event& event
)
{
    if constexpr(state_traits::requires_on_exit_v<State>)
    {
        call_on_exit_impl(state, &mach, &event);
    }
}

template<class Fn, class Sm, class Context, class Event>
auto call_action_or_guard(const Fn& fun, Sm* pmach, Context& ctx, const Event* pevent) ->
    decltype(fun(*pmach, ctx, *pevent))
{
    return fun(*pmach, ctx, *pevent);
}

template<class Fn, class Context, class Event>
auto call_action_or_guard(const Fn& fun, void* /*pmach*/, Context& ctx, const Event* pevent) ->
    decltype(fun(ctx, *pevent))
{
    return fun(ctx, *pevent);
}

template<class Fn, class Context>
auto call_action_or_guard(const Fn& fun, void* /*pmach*/, Context& ctx, const void* /*pevent*/) ->
    decltype(fun(ctx))
{
    return fun(ctx);
}

} //namespace

#endif
