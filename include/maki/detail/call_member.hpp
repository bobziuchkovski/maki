//Copyright Florian Goujeon 2021.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_without_event.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_CALL_MEMBER_HPP
#define MAKI_DETAIL_CALL_MEMBER_HPP

#include "type_traits.hpp"
#include "state_traits.hpp"
#include "submachine_fwd.hpp"
#include <functional>
#include <type_traits>
#include <utility>

namespace maki::detail
{

template<class Sm, class Context, class StateData, class Event, class EventAction, class... EventActions>
void call_event_action
(
    [[maybe_unused]] Sm& mach,
    [[maybe_unused]] Context& ctx,
    [[maybe_unused]] StateData& state_data,
    [[maybe_unused]] const Event& event,
    [[maybe_unused]] const EventAction& event_action,
    [[maybe_unused]] const EventActions&... event_actions
)
{
    using event_type_filter = typename EventAction::event_type_filter;
    if constexpr(matches_pattern_v<Event, event_type_filter>)
    {
        if constexpr(EventAction::sig == event_action_signature::v)
        {
            std::invoke(event_action.action);
        }
        else if constexpr(EventAction::sig == event_action_signature::m)
        {
            std::invoke(event_action.action, mach);
        }
        else if constexpr(EventAction::sig == event_action_signature::me)
        {
            std::invoke(event_action.action, mach, event);
        }
        else if constexpr(EventAction::sig == event_action_signature::c)
        {
            std::invoke(event_action.action, ctx);
        }
        else if constexpr(EventAction::sig == event_action_signature::ce)
        {
            std::invoke(event_action.action, ctx, event);
        }
        else if constexpr(EventAction::sig == event_action_signature::d)
        {
            std::invoke(event_action.action, state_data);
        }
        else if constexpr(EventAction::sig == event_action_signature::de)
        {
            std::invoke(event_action.action, state_data, event);
        }
        else if constexpr(EventAction::sig == event_action_signature::e)
        {
            std::invoke(event_action.action, event);
        }
        else
        {
            constexpr auto is_false = sizeof(Sm) == 0;
            static_assert(is_false, "Unsupported event_action_signature value");
        }
    }
    else
    {
        static_assert(sizeof...(EventActions) != 0, "No event action found for this state and event");
        call_event_action(mach, ctx, state_data, event, event_actions...);
    }
}

template
<
    class ActionTuple,
    class Sm,
    class Context,
    class StateData,
    class Event
>
void call_state_action
(
    const ActionTuple& actions,
    Sm& mach,
    Context& ctx,
    StateData& state_data,
    const Event& event
)
{
    if constexpr(!tlu::empty_v<ActionTuple>)
    {
        tuple_apply
        (
            actions,
            [](auto&&... args)
            {
                call_event_action(std::forward<decltype(args)>(args)...);
            },
            mach,
            ctx,
            state_data,
            event
        );
    }
}

template
<
    class State,
    class ActionTuple,
    class Sm,
    class Context,
    class StateData,
    class Event
>
void call_state_action_old
(
    [[maybe_unused]] State& state,
    ActionTuple& actions,
    Sm& mach,
    Context& ctx,
    StateData& state_data,
    const Event& event
)
{
    call_state_action
    (
        actions,
        mach,
        ctx,
        state_data,
        event
    );
}

template
<
    class State,
    class ActionTuple,
    class Sm,
    class Context,
    class StateData,
    class Event
>
void call_state_action_old
(
    State& state,
    ActionTuple& actions,
    Sm& mach,
    Context& ctx,
    StateData& state_data,
    const Event& event,
    bool& processed
)
{
    if constexpr(state_traits::is_submachine_v<State>)
    {
        state.on_event(event, processed);
    }
    else
    {
        call_state_action(actions, mach, ctx, state_data, event);
        processed = true;
    }
}

template<const auto& Fn, class Sm, class Context, class Event>
auto call_action_or_guard
(
    [[maybe_unused]] Sm& mach,
    [[maybe_unused]] Context& ctx,
    [[maybe_unused]] const Event& event
)
{
    using fn_t = std::decay_t<decltype(Fn)>;

    if constexpr(std::is_invocable_v<fn_t, Sm&, Context&, const Event&>)
    {
        return Fn(mach, ctx, event);
    }
    else if constexpr(std::is_invocable_v<fn_t, Context&, const Event&>)
    {
        return Fn(ctx, event);
    }
    else if constexpr(std::is_invocable_v<fn_t, Context&>)
    {
        return Fn(ctx);
    }
    else if constexpr(is_nullary_v<fn_t>)
    {
        return Fn();
    }
    else
    {
        constexpr auto is_false = sizeof(Sm) == 0;
        static_assert(is_false, "No valid signature found for action/guard");
    }
}

} //namespace

#endif
