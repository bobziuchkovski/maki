//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_SUBMACHINE_HPP
#define MAKI_DETAIL_SUBMACHINE_HPP

#include "call_member.hpp"
#include "tlu.hpp"
#include "region.hpp"
#include "machine_object_holder.hpp"
#include "context_holder.hpp"
#include "simple_state.hpp"
#include "submachine_fwd.hpp"
#include "tuple.hpp"
#include "../machine_fwd.hpp"
#include "../state_conf.hpp"
#include "../transition_table.hpp"
#include "../region_path.hpp"
#include "../type_patterns.hpp"
#include <type_traits>

namespace maki::detail
{

template<const auto& Conf, class ParentRegion>
struct submachine_context
{
    using conf_type = std::decay_t<decltype(Conf)>;
    using conf_context_type = typename conf_type::context_type;

    /*
    Context type is either (in this order of priority):
    - the one specified in the submachine_opts::context option, if any;
    - a reference to the context type of the parent SM (not necessarily the root
      SM).
    */
    using type = std::conditional_t
    <
        std::is_void_v<conf_context_type>,
        typename ParentRegion::parent_sm_type::context_type&,
        conf_context_type
    >;
};

template<const auto& Conf>
struct submachine_context<Conf, void>
{
    using type = typename std::decay_t<decltype(Conf)>::context_type;
};

template
<
    class ParentSm,
    class RegionIndexSequence
>
struct region_tuple;

template
<
    class ParentSm,
    int... RegionIndexes
>
struct region_tuple
<
    ParentSm,
    std::integer_sequence<int, RegionIndexes...>
>
{
    using type = tuple
    <
        region
        <
            ParentSm,
            RegionIndexes
        >...
    >;
};

template<const auto& Conf, class ParentRegion>
class submachine
{
public:
    using conf_type = std::decay_t<decltype(Conf)>;
    using context_type = typename submachine_context<Conf, ParentRegion>::type;
    using transition_table_type_list = decltype(Conf.transition_tables_);

    template<class Machine, class... ContextArgs>
    submachine(Machine& mach, ContextArgs&&... ctx_args):
        ctx_holder_(mach, std::forward<ContextArgs>(ctx_args)...),
        simple_state_(mach, context()),
        regions_(uniform_construct, mach, *this)
    {
    }

    context_type& context()
    {
        return ctx_holder_.get();
    }

    const context_type& context() const
    {
        return ctx_holder_.get();
    }

    auto& data()
    {
        return simple_state_.data();
    }

    const auto& data() const
    {
        return simple_state_.data();
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    auto& state_data()
    {
        using state_region_path_t = std::decay_t<decltype(StateRegionPath)>;

        static_assert
        (
            same_ref
            (
                detail::tlu::front_t<state_region_path_t>::machine_conf_t,
                Conf
            )
        );

        static constexpr auto region_index = tlu::front_t<state_region_path_t>::region_index;
        static constexpr auto state_region_relative_path = tlu::pop_front_t<state_region_path_t>{};
        return tuple_get<region_index>(regions_).template state_data<state_region_relative_path, StateConf>();
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    const auto& state_data() const
    {
        using state_region_path_t = std::decay_t<decltype(StateRegionPath)>;

        static_assert
        (
            same_ref
            (
                detail::tlu::front_t<state_region_path_t>::machine_conf_t,
                Conf
            )
        );

        static constexpr auto region_index = tlu::front_t<state_region_path_t>::region_index;
        static constexpr auto state_region_relative_path = tlu::pop_front_t<state_region_path_t>{};
        return tuple_get<region_index>(regions_).template state_data<state_region_relative_path, StateConf>();
    }

    template<const auto& StateRegionPath, const auto& StateConf>
    [[nodiscard]] bool is_active_state_def() const
    {
        using state_region_path_t = std::decay_t<decltype(StateRegionPath)>;

        static_assert
        (
            same_ref
            (
                detail::tlu::front_t<state_region_path_t>::machine_conf_t,
                Conf
            )
        );

        static constexpr auto region_index = tlu::front_t<state_region_path_t>::region_index;
        static constexpr auto state_region_relative_path = tlu::pop_front_t<state_region_path_t>{};
        return tuple_get<region_index>(regions_).template is_active_state_def<state_region_relative_path, StateConf>();
    }

    template<const auto& StateConf>
    [[nodiscard]] bool is_active_state_def() const
    {
        static_assert(tlu::size_v<transition_table_type_list> == 1);

        static constexpr auto state_region_relative_path = region_path<>{};
        return tuple_get<0>(regions_).template is_active_state_def<state_region_relative_path, StateConf>();
    }

    template<const auto& RegionPath>
    [[nodiscard]] bool is_running() const
    {
        return !is_active_state_def<RegionPath, states::stopped>();
    }

    [[nodiscard]] bool is_running() const
    {
        return !is_active_state_def<states::stopped>();
    }

    template<class Machine, class Context, class Event>
    void call_entry_action(Machine& mach, Context& ctx, const Event& event)
    {
        simple_state_.call_entry_action
        (
            mach,
            own_context_or(ctx),
            event
        );
        tlu::for_each<region_tuple_type, region_start>(*this, mach, own_context_or(ctx), event);
    }

    template<bool Dry = false, class Machine, class Context, class Event>
    void call_internal_action
    (
        Machine& mach,
        Context& ctx,
        const Event& event
    )
    {
        call_internal_action_2<Dry>(*this, mach, ctx, event);
    }

    template<bool Dry = false, class Machine, class Context, class Event>
    void call_internal_action
    (
        Machine& mach,
        Context& ctx,
        const Event& event,
        bool& processed
    )
    {
        call_internal_action_2<Dry>(*this, mach, ctx, event, processed);
    }

    template<bool Dry = false, class Machine, class Context, class Event>
    void call_internal_action
    (
        Machine& mach,
        Context& ctx,
        const Event& event
    ) const
    {
        call_internal_action_2<Dry>(*this, mach, ctx, event);
    }

    template<bool Dry = false, class Machine, class Context, class Event>
    void call_internal_action
    (
        Machine& mach,
        Context& ctx,
        const Event& event,
        bool& processed
    ) const
    {
        call_internal_action_2<Dry>(*this, mach, ctx, event, processed);
    }

    template<class Machine, class Context, class Event>
    void call_exit_action(Machine& mach, Context& ctx, const Event& event)
    {
        tlu::for_each<region_tuple_type, region_stop>(*this, mach, own_context_or(ctx), event);
        simple_state_.call_exit_action
        (
            mach,
            own_context_or(ctx),
            event
        );
    }

    template<class /*Event*/>
    static constexpr bool has_internal_action_for_event()
    {
        return true;
    }

    static constexpr const auto& conf = Conf;

private:
    using region_tuple_type = typename region_tuple
    <
        submachine,
        std::make_integer_sequence<int, tlu::size_v<transition_table_type_list>>
    >::type;

    //We need a simple_state to manage data and actions
    using simple_state_type = simple_state<Conf>;

    struct region_start
    {
        template<class Region, class Self, class Machine, class Context, class Event>
        static void call(Self& self, Machine& mach, Context& ctx, const Event& event)
        {
            tuple_get<Region>(self.regions_).start(mach, ctx, event);
        }
    };

    template<bool Dry>
    struct region_process_event
    {
        template<class Region, class Self, class Machine, class Context, class Event, class... ExtraArgs>
        static void call(Self& self, Machine& mach, Context& ctx, const Event& event, ExtraArgs&... extra_args)
        {
            tuple_get<Region>(self.regions_).template process_event<Dry>(mach, ctx, event, extra_args...);
        }
    };

    struct region_stop
    {
        template<class Region, class Self, class Machine, class Context, class Event>
        static void call(Self& self, Machine& mach, Context& ctx, const Event& event)
        {
            tuple_get<Region>(self.regions_).stop(mach, ctx, event);
        }
    };

    template<bool Dry, class Self, class Machine, class Context, class Event>
    static void call_internal_action_2
    (
        Self& self,
        Machine& mach,
        Context& ctx,
        const Event& event
    )
    {
        if constexpr(simple_state_type::template has_internal_action_for_event<Event>())
        {
            if constexpr(!Dry)
            {
                self.simple_state_.call_internal_action
                (
                    mach,
                    self.own_context_or(ctx),
                    event
                );
            }
        }

        tlu::for_each<region_tuple_type, region_process_event<Dry>>(self, mach, self.own_context_or(ctx), event);
    }

    template<bool Dry, class Self, class Machine, class Context, class Event>
    static void call_internal_action_2
    (
        Self& self,
        Machine& mach,
        Context& ctx,
        const Event& event,
        bool& processed
    )
    {
        if constexpr(simple_state_type::template has_internal_action_for_event<Event>())
        {
            if constexpr(!Dry)
            {
                self.simple_state_.call_internal_action
                (
                    mach,
                    self.own_context_or(ctx),
                    event
                );
                tlu::for_each<region_tuple_type, region_process_event<Dry>>(self, mach, self.own_context_or(ctx), event);
            }
            processed = true;
        }

        tlu::for_each<region_tuple_type, region_process_event<Dry>>(self, mach, self.own_context_or(ctx), event, processed);
    }

    template<class Context>
    auto& own_context_or(Context& ctx)
    {
        if constexpr(std::is_void_v<typename conf_type::context_type>)
        {
            return ctx;
        }
        else
        {
            return ctx_holder_.get();
        }
    }

    template<class Context>
    const auto& own_context_or(const Context& ctx) const
    {
        if constexpr(std::is_void_v<typename conf_type::context_type>)
        {
            return ctx;
        }
        else
        {
            return ctx_holder_.get();
        }
    }

    context_holder<context_type> ctx_holder_;
    simple_state_type simple_state_;
    region_tuple_type regions_;
};

} //namespace

#endif
