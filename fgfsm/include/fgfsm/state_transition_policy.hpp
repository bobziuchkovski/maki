//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#ifndef FGFSM_STATE_TRANSITION_POLICY_HPP
#define FGFSM_STATE_TRANSITION_POLICY_HPP

#include "any_copy.hpp"
#include "none.hpp"
#include <type_traits>

namespace fgfsm
{

template
<
    class StartState,
    class TargetState,
    class Action,
    class Guard
>
class state_transition_policy_helper
{
    private:
        template<class TransitionTable, class Configuration>
        friend class fsm;

        using voidp = void*;
        using voidpp = void**;

        state_transition_policy_helper
        (
            StartState& start_state,
            const any_cref& event,
            TargetState& target_state,
            Action& action,
            Guard& guard,
            bool& processed,
            int& active_state_index,
            const int target_state_index,
            const voidpp ppactive_state_on_event_fn,
            const voidp ptarget_state_on_event_fn
        ):
            start_state_(start_state),
            evt_(event),
            target_state_(target_state),
            action_(action),
            guard_(guard),
            processed_(processed),
            active_state_index_(active_state_index),
            target_state_index_(target_state_index),
            ppactive_state_on_event_fn_(ppactive_state_on_event_fn),
            ptarget_state_on_event_fn_(ptarget_state_on_event_fn)
        {
        }

    public:
        bool check_guard() const
        {
            return guard_(start_state_, evt_, target_state_);
        }

        void invoke_start_state_on_exit()
        {
            if constexpr(!std::is_same_v<TargetState, none>)
                start_state_.on_exit(evt_);
        }

        void activate_target_state()
        {
            if constexpr(!std::is_same_v<TargetState, none>)
            {
                active_state_index_ = target_state_index_;
                *ppactive_state_on_event_fn_ = ptarget_state_on_event_fn_;
            }
            processed_ = true;
        }

        void execute_action()
        {
            action_(start_state_, evt_, target_state_);
        }

        void invoke_target_state_on_entry()
        {
            if constexpr(!std::is_same_v<TargetState, none>)
                target_state_.on_entry(evt_);
        }

    private:
        StartState& start_state_;
        const any_cref& evt_;
        TargetState& target_state_;
        Action& action_;
        Guard& guard_;
        bool& processed_;
        int& active_state_index_;
        const int target_state_index_;
        const voidpp ppactive_state_on_event_fn_;
        const voidp ptarget_state_on_event_fn_;
};

struct fast_state_transition_policy
{
    template<class Context>
    fast_state_transition_policy(const Context&)
    {
    }

    template
    <
        class StartState,
        class TargetState,
        class Action,
        class Guard
    >
    void operator()
    (
        state_transition_policy_helper
        <
            StartState,
            TargetState,
            Action,
            Guard
        >& helper
    )
    {
        if(helper.check_guard())
        {
            helper.invoke_start_state_on_exit();
            helper.activate_target_state();
            helper.execute_action();
            helper.invoke_target_state_on_entry();
        }
    }
};

} //namespace

#endif
