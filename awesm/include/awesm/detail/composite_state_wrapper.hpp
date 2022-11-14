//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_COMPOSITE_STATE_WRAPPER_HPP
#define AWESM_DETAIL_COMPOSITE_STATE_WRAPPER_HPP

#include "../state_conf.hpp"
#include "region_tuple.hpp"
#include "sm_object_holder.hpp"
#include "sm_path.hpp"

namespace awesm::detail
{

template<class Sm, class RegionPath, class WrappedState>
class composite_state_wrapper
{
    public:
        using conf = state_conf
        <
            state_options::on_entry_any,
            state_options::on_event_any,
            state_options::on_exit_any
        >;

        template<class Context>
        composite_state_wrapper(Sm& mach, Context& ctx):
            state_(mach, ctx),
            region_tuple_(mach, ctx)
        {
        }

        template<class State, int RegionIndex = 0>
        [[nodiscard]] bool is_active_state() const
        {
            return region_tuple_.template is_active_state<State, RegionIndex>();
        }

        [[nodiscard]] bool is_running() const
        {
            return !is_active_state<detail::null_state>();
        }

        template<class Event>
        void on_entry(const Event& event)
        {
            state_.on_entry(event);
            region_tuple_.start(event);
        }

        template<class Event>
        void on_event(const Event& event)
        {
            region_tuple_.process_event(event);
            state_.on_event(event);
        }

        template<class Event>
        void on_exit(const Event& event)
        {
            region_tuple_.stop(event);
            state_.on_exit(event);
        }

    private:
        using conf_t = typename WrappedState::conf;
        using transition_table_list_t = typename conf_t::transition_table_list_t;
        using sm_path_t = detail::sm_path<RegionPath, composite_state_wrapper>;

        detail::sm_object_holder<WrappedState> state_;
        detail::region_tuple<Sm, sm_path_t, transition_table_list_t> region_tuple_;
};

} //namespace

#endif
