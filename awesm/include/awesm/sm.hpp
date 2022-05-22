//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_HPP
#define AWESM_SM_HPP

#include "sm_configuration.hpp"
#include "internal_transition_policy_helper.hpp"
#include "state_transition_policy_helper.hpp"
#include "none.hpp"
#include "detail/sm_object.hpp"
#include "detail/resolve_transition_table.hpp"
#include "detail/transition_table_digest.hpp"
#include "detail/alternative_lazy.hpp"
#include "detail/any_container.hpp"
#include "detail/ignore_unused.hpp"
#include "detail/tlu/apply.hpp"
#include <queue>
#include <type_traits>

namespace awesm
{

namespace detail
{
    class false_at_destruction_setter
    {
        public:
            false_at_destruction_setter(bool& b):
                b_(b)
            {
            }

            false_at_destruction_setter(const false_at_destruction_setter&) = delete;
            false_at_destruction_setter(false_at_destruction_setter&&) = delete;
            false_at_destruction_setter& operator=(const false_at_destruction_setter&) = delete;
            false_at_destruction_setter& operator=(false_at_destruction_setter&&) = delete;

            ~false_at_destruction_setter()
            {
                b_ = false;
            }

        private:
            bool& b_;
    };
}

template<class Configuration>
class sm
{
    public:
        static_assert
        (
            std::is_base_of_v<sm_configuration, Configuration>,
            "Given configuration type must inherit from awesm::sm_configuration."
        );

        template<class Context>
        explicit sm(Context& context):
            pre_transition_event_handler_{context, *this},
            internal_transition_policy_{context, *this},
            state_transition_policy_{context, *this}
        {
            //Instantiate SM objects
            detail::tlu::apply<state_type_list, object_maker>::make(context, *this);
            detail::tlu::apply<action_type_list, object_maker>::make(context, *this);
            detail::tlu::apply<guard_type_list, object_maker>::make(context, *this);
        }

        sm(const sm&) = delete;
        sm(sm&&) = delete;

        ~sm()
        {
            //Destroy SM objects
            detail::tlu::apply<state_type_list, object_unmaker>::unmake();
            detail::tlu::apply<action_type_list, object_unmaker>::unmake();
            detail::tlu::apply<guard_type_list, object_unmaker>::unmake();
        }

        sm& operator=(const sm&) = delete;
        sm& operator=(sm&&) = delete;

        //Check whether the given State is the active state type
        template<class State>
        [[nodiscard]] bool is_active_state() const
        {
            constexpr auto given_state_index = detail::tlu::get_index
            <
                state_type_list,
                State
            >;
            return given_state_index == active_state_index_;
        }

        template<class Event>
        void process_event(const Event& event)
        {
            if constexpr(Configuration::enable_run_to_completion)
            {
                //Queue event processing in case of recursive call
                if(processing_event_)
                {
                    queued_event_processings_.emplace(*this, event);
                    return;
                }

                processing_event_ = true;
                auto _ = detail::false_at_destruction_setter{processing_event_};

                process_event_once(event);

                //Process deferred event processings
                while(!queued_event_processings_.empty())
                {
                    queued_event_processings_.front()();
                    queued_event_processings_.pop();
                }
            }
            else
            {
                process_event_once(event);
            }
        }

    private:
        using unresolved_transition_table_t =
            typename Configuration::transition_table
        ;
        using pre_transition_event_handler_t =
            typename Configuration::pre_transition_event_handler
        ;
        using internal_transition_policy_t =
            typename Configuration::template internal_transition_policy<sm>
        ;
        using state_transition_policy_t =
            typename Configuration::template state_transition_policy<sm>
        ;

        using transition_table_digest_t =
            detail::transition_table_digest<unresolved_transition_table_t>
        ;
        using state_type_list = typename transition_table_digest_t::state_type_list;
        using action_type_list = typename transition_table_digest_t::action_type_list;
        using guard_type_list = typename transition_table_digest_t::guard_type_list;

        template<class... Objects>
        struct object_maker
        {
            template<class Context>
            static void make(Context& ctx, sm& machine)
            {
                (detail::make_sm_object<Objects>(ctx, machine), ...);
            }
        };

        template<class... Objects>
        struct object_unmaker
        {
            static void unmake()
            {
                (detail::unmake_sm_object<Objects>(), ...);
            }
        };

        class event_processing
        {
            public:
                template<class Event>
                event_processing(sm& machine, const Event& event):
                    sm_(machine),
                    event_(event),
                    pprocess_event_
                    (
                        [](sm& machine, const event_storage_t& event)
                        {
                            machine.process_event_once(event.get<Event>());
                        }
                    )
                {
                }

                event_processing(const event_processing&) = delete;
                event_processing(event_processing&&) = delete;
                ~event_processing() = default;
                event_processing& operator=(const event_processing&) = delete;
                event_processing& operator=(event_processing&&) = delete;

                void operator()()
                {
                    (*pprocess_event_)(sm_, event_);
                }

            private:
                static constexpr auto small_event_size = 16;
                using event_storage_t = detail::any_container<small_event_size>;

                sm& sm_;
                event_storage_t event_;
                void(*pprocess_event_)(sm&, const event_storage_t&) = nullptr;
        };

        /*
        Calling detail::resolve_transition_table<> isn't free. We need
        alternative_lazy to avoid the call when it's unnecessary (i.e. when
        there's no pattern-start-state in the transition table).
        */
        struct unresolved_transition_table_holder
        {
            template<class = void>
            using type = unresolved_transition_table_t;
        };
        struct resolved_transition_table_holder
        {
            template<class = void>
            using type = detail::resolve_transition_table
            <
                unresolved_transition_table_t,
                state_type_list
            >;
        };
        using transition_table_t = detail::alternative_lazy
        <
            transition_table_digest_t::has_source_state_patterns,
            resolved_transition_table_holder,
            unresolved_transition_table_holder
        >;

        struct function_queue_holder
        {
            template<class = void>
            using type = std::queue<event_processing>;
        };
        struct empty_holder
        {
            template<class = void>
            struct type{};
        };
        using queued_event_processing_storage_t = detail::alternative_lazy
        <
            Configuration::enable_run_to_completion,
            function_queue_holder,
            empty_holder
        >;

        template
        <
            class Sm,
            class SourceState,
            class Event,
            class TargetState,
            class Action,
            class Guard
        >
        friend class state_transition_policy_helper;

        template<class Object>
        Object& get_object()
        {
            return detail::get_sm_object<Object>();
        }

        template<class Event>
        void process_event_once(const Event& event)
        {
            detail::call_on_event(&pre_transition_event_handler_, &event, 0);

            if constexpr(Configuration::enable_in_state_internal_transitions)
            {
                process_event_in_active_state(event);
            }

            const bool processed = process_event_in_transition_table_once(event);

            //Anonymous transitions
            if constexpr(transition_table_digest_t::has_none_events)
            {
                if(processed)
                {
                    while(process_event_in_transition_table_once(none{})){}
                }
            }
            else
            {
                detail::ignore_unused(processed);
            }
        }

        //Try and trigger one transition
        template<class Event>
        bool process_event_in_transition_table_once(const Event& event)
        {
            return detail::tlu::apply
            <
                transition_table_t,
                transition_table_event_processor
            >::process(*this, event);
        }

        template<class... Rows>
        struct transition_table_event_processor
        {
            template<class Event>
            static bool process(sm& machine, const Event& event)
            {
                return (transition_table_row_event_processor<Rows>::process(machine, &event) || ...);
            }
        };

        //Processes events against one row of the (resolved) transition table
        template<class Row>
        struct transition_table_row_event_processor
        {
            using transition_source_state = typename Row::source_state_type;
            using transition_event        = typename Row::event_type;
            using transition_target_state = typename Row::target_state_type;
            using transition_action       = typename Row::action_type;
            using transition_guard        = typename Row::guard_type;

            static bool process(sm& machine, const transition_event* const pevent)
            {
                //Make sure the transition source state is the active state
                if(!machine.is_active_state<transition_source_state>())
                {
                    return false;
                }

                //Perform the transition
                using helper_t = state_transition_policy_helper
                <
                    sm,
                    transition_source_state,
                    transition_event,
                    transition_target_state,
                    transition_action,
                    transition_guard
                >;
                auto helper = helper_t{machine, *pevent};
                return machine.state_transition_policy_.do_transition(helper);
            }

            /*
            This void* trick allows for shorter build times.

            This alternative implementation takes more time to build:
                template<class Event>
                static bool process(sm&, const Event&)
                {
                    return false;
                }

            Using an if-constexpr in the process() function above is worse as
            well.
            */
            static bool process(sm& /*machine*/, const void* /*pevent*/)
            {
                return false;
            }
        };

        /*
        Call active_state.on_event(event)
        */
        template<class Event>
        void process_event_in_active_state(const Event& event)
        {
            return detail::tlu::apply
            <
                state_type_list,
                active_state_event_processor
            >::process(*this, event);
        }

        //Processes internal events against all states
        template<class... States>
        struct active_state_event_processor
        {
            template<class Event>
            static void process(sm& machine, const Event& event)
            {
                (machine.process_event_in_state<States>(&event) || ...);
            }
        };

        //Processes internal events against one state
        template<class State, class Event>
        auto process_event_in_state
        (
            const Event* pevent
        ) -> decltype(std::declval<State>().on_event(*pevent), bool())
        {
            if(is_active_state<State>())
            {
                auto& state = get_object<State>();
                auto helper = internal_transition_policy_helper
                <
                    State,
                    Event
                >{state, *pevent};
                internal_transition_policy_.do_transition(helper);
                return true;
            }
            return false;
        }

        template<class State>
        bool process_event_in_state
        (
            const void* /*pevent*/
        )
        {
            return false;
        }

        pre_transition_event_handler_t pre_transition_event_handler_;
        internal_transition_policy_t internal_transition_policy_;
        state_transition_policy_t state_transition_policy_;

        int active_state_index_ = 0;
        bool processing_event_ = false;
        queued_event_processing_storage_t queued_event_processings_;
};

} //namespace

#endif
