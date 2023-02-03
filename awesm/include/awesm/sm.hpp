//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_HPP
#define AWESM_SM_HPP

#include "sm_conf.hpp"
#include "null.hpp"
#include "detail/sm_conf_traits.hpp"
#include "detail/composite_state_wrapper.hpp"
#include "detail/region_tuple.hpp"
#include "detail/alternative.hpp"
#include "detail/any_container.hpp"
#include "detail/sm_path.hpp"
#include "detail/tlu.hpp"
#include "detail/type_tag.hpp"
#include "detail/overload_priority.hpp"
#include <queue>
#include <type_traits>

namespace awesm
{

namespace detail
{
    enum class sm_operation
    {
        start,
        stop,
        process_event
    };

    template<class SmConf>
    constexpr size_t get_small_event_max_size(overload_priority::low /*ignored*/)
    {
        constexpr auto default_max = 16;
        return default_max;
    }

    template<class SmConf>
    constexpr size_t get_small_event_max_alignment_requirement(overload_priority::low /*ignored*/)
    {
        constexpr auto default_max = 8;
        return default_max;
    }

    template<class SmConf>
    constexpr auto get_small_event_max_size(overload_priority::high /*ignored*/) ->
        decltype(SmConf::get_small_event_max_size())
    {
        return SmConf::get_small_event_max_size();
    }

    template<class SmConf>
    constexpr auto get_small_event_max_alignment_requirement(overload_priority::high /*ignored*/) ->
        decltype(SmConf::get_small_event_max_alignment_requirement())
    {
        return SmConf::get_small_event_max_alignment_requirement();
    }
}

template<class Def>
class sm
{
    public:
        using conf_type = typename Def::conf_type;
        using context_type = typename conf_type::context_type;

        explicit sm(context_type& context):
            ctx_(context),
            def_(*this, context),
            region_tuple_(*this)
        {
        }

        sm(const sm&) = delete;
        sm(sm&&) = delete;
        sm& operator=(const sm&) = delete;
        sm& operator=(sm&&) = delete;
        ~sm() = default;

        context_type& get_context()
        {
            return ctx_;
        }

        const context_type& get_context() const
        {
            return ctx_;
        }

        template<class RegionPath>
        [[nodiscard]] bool is_running() const
        {
            return !is_active_state<RegionPath, states::stopped>();
        }

        template<bool Dummy = true>
        [[nodiscard]] bool is_running() const
        {
            return !is_active_state<states::stopped>();
        }

        template<class StateRegionPath, class State>
        [[nodiscard]] bool is_active_state() const
        {
            static_assert
            (
                std::is_same_v
                <
                    typename detail::tlu::front_t<StateRegionPath>::sm_type,
                    sm
                >
            );
            return region_tuple_.template is_active_state<StateRegionPath, State>();
        }

        template<class State>
        [[nodiscard]] bool is_active_state() const
        {
            return region_tuple_.template is_active_state<State>();
        }

        template<class Event = events::start>
        void start(const Event& event = {})
        {
            process_event_2<detail::sm_operation::start>(event);
        }

        template<class Event = events::stop>
        void stop(const Event& event = {})
        {
            process_event_2<detail::sm_operation::stop>(event);
        }

        template<class Event>
        void process_event(const Event& event)
        {
            process_event_2<detail::sm_operation::process_event>(event);
        }

        static decltype(auto) get_pretty_name()
        {
            return awesm::get_pretty_name<Def>();
        }

    private:
        //Let regions access definition and option list
        template<class RegionPath, auto TransitionTableFnList>
        friend class detail::region;

        using option_type_list = typename conf_type::option_type_list;

        using region_tuple_type = detail::region_tuple
        <
            detail::sm_path<region_path<>, sm>,
            detail::sm_conf_traits::transition_table_fn_list_t<conf_type>
        >;

        struct any_event_queue_holder
        {
            using any_event_type = detail::any_container
            <
                sm&,
                detail::get_small_event_max_size<conf_type>(detail::overload_priority::probe),
                detail::get_small_event_max_alignment_requirement<conf_type>(detail::overload_priority::probe)
            >;

            template<bool = true> //Dummy template for lazy evaluation
            using type = std::queue<any_event_type>;
        };
        struct empty_holder
        {
            template<bool = true> //Dummy template for lazy evaluation
            struct type{};
        };
        using any_event_queue_type = typename detail::alternative_t
        <
            detail::tlu::contains_v<option_type_list, sm_options::disable_run_to_completion>,
            empty_holder,
            any_event_queue_holder
        >::template type<>;

        template<detail::sm_operation Operation, class Event>
        void process_event_2(const Event& event)
        {
            if constexpr(!detail::tlu::contains_v<option_type_list, sm_options::disable_run_to_completion>)
            {
                if(!processing_event_) //If call is not recursive
                {
                    processing_event_ = true;

                    process_event_once<Operation>(event);

                    //Process queued events, if any
                    while(!event_queue_.empty())
                    {
                        event_queue_.front().visit(*this);
                        event_queue_.pop();
                    }

                    processing_event_ = false;
                }
                else
                {
                    //Queue event in case of recursive call
                    if constexpr(std::is_nothrow_copy_constructible_v<Event>)
                    {
                        queue_event<Operation>(event);
                    }
                    else
                    {
                        try
                        {
                            queue_event<Operation>(event);
                        }
                        catch(...)
                        {
                            process_exception(std::current_exception());
                        }
                    }
                }
            }
            else
            {
                process_event_once<Operation>(event);
            }
        }

        template<detail::sm_operation Operation, class Event>
        void queue_event(const Event& event)
        {
            event_queue_.emplace
            (
                event,
                detail::type_tag<any_event_visitor<Operation>>{}
            );
        }

        template<detail::sm_operation Operation>
        struct any_event_visitor
        {
            template<class Event>
            static void call(const Event& event, sm& self)
            {
                self.process_event_once<Operation>(event);
            }
        };

        void process_exception(const std::exception_ptr& eptr)
        {
            if constexpr(detail::tlu::contains_v<option_type_list, sm_options::on_exception>)
            {
                def_.on_exception(eptr);
            }
            else
            {
                process_event(eptr);
            }
        }

        template<detail::sm_operation Operation, class Event>
        void process_event_once(const Event& event)
        {
            try
            {
                if constexpr
                (
                    Operation == detail::sm_operation::process_event &&
                    detail::tlu::contains_v<option_type_list, sm_options::on_event>
                )
                {
                    def_.on_event(event);
                }

                if constexpr(Operation == detail::sm_operation::start)
                {
                    region_tuple_.start(event);
                }
                else if constexpr(Operation == detail::sm_operation::stop)
                {
                    region_tuple_.stop(event);
                }
                else
                {
                    region_tuple_.process_event(event);
                }
            }
            catch(...)
            {
                process_exception(std::current_exception());
            }
        }

        context_type& ctx_;
        detail::sm_object_holder<Def> def_;
        region_tuple_type region_tuple_;
        bool processing_event_ = false;
        any_event_queue_type event_queue_;
};

} //namespace

#endif
