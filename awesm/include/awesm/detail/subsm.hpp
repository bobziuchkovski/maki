//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_SUBSM_HPP
#define AWESM_DETAIL_SUBSM_HPP

#include "call_member.hpp"
#include "tlu.hpp"
#include "region.hpp"
#include "region_path_of.hpp"
#include "sm_object_holder.hpp"
#include "context_holder.hpp"
#include "subsm_fwd.hpp"
#include "tuple.hpp"
#include "../sm_fwd.hpp"
#include "../state_conf.hpp"
#include "../transition_table.hpp"
#include "../region_path.hpp"
#include <type_traits>

namespace awesm::detail
{

template<class Def, class ParentRegion>
struct region_path_of<subsm<Def, ParentRegion>>
{
    using type = region_path_of_t<ParentRegion>;
};

template<class Def>
struct region_path_of<subsm<Def, void>>
{
    using type = region_path_tpl<>;
};

template<class Def, class ParentRegion>
struct root_sm_of<subsm<Def, ParentRegion>>
{
    using type = root_sm_of_t<ParentRegion>;

    static type& get(subsm<Def, ParentRegion>& node)
    {
        return node.root_sm();
    }
};

template<class Def>
struct root_sm_of<subsm<Def, void>>
{
    using type = sm<Def>;

    static type& get(subsm<Def, void>& node)
    {
        return node.root_sm();
    }
};

template<class Def, class ParentRegion>
struct subsm_context
{
    /*
    Context type is either (in this order of priority):
    - the one specified in the subsm_opts::context option, if any;
    - a reference to the context type of the parent SM (not necessarily the root
      SM).
    */
    using type = option_or_t
    <
        typename Def::conf,
        option_id::context,
        typename ParentRegion::parent_sm_type::context_type&
    >;
};

template<class Def>
struct subsm_context<Def, void>
{
    using type = option_t<typename Def::conf, option_id::context>;
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

template<class Def, class ParentRegion>
class subsm
{
public:
    using conf = state_conf
        ::on_entry_any
        ::on_event<awesm::any>
        ::on_exit_any
    ;

    using def_type = Def;
    using context_type = typename subsm_context<Def, ParentRegion>::type;
    using root_sm_type = root_sm_of_t<subsm>;

    using transition_table_type_list = option_t<typename Def::conf, option_id::transition_tables>;

    template<class... ContextArgs>
    subsm(root_sm_type& root_sm, ContextArgs&&... ctx_args):
        root_sm_(root_sm),
        ctx_holder_(root_sm, std::forward<ContextArgs>(ctx_args)...),
        def_holder_(root_sm, context()),
        regions_(*this)
    {
    }

    root_sm_type& root_sm()
    {
        return root_sm_;
    }

    context_type& context()
    {
        return ctx_holder_.get();
    }

    Def& def()
    {
        return def_holder_.get();
    }

    template<class StateRegionPath, class StateDef>
    StateDef& state_def()
    {
        static_assert
        (
            std::is_same_v
            <
                typename detail::tlu::front_t<StateRegionPath>::sm_def_type,
                Def
            >
        );

        static constexpr auto region_index = tlu::front_t<StateRegionPath>::region_index;
        return get<region_index>(regions_).template state_def<tlu::pop_front_t<StateRegionPath>, StateDef>();
    }

    template<class StateRegionPath, class StateDef>
    const StateDef& state_def() const
    {
        static_assert
        (
            std::is_same_v
            <
                typename detail::tlu::front_t<StateRegionPath>::sm_def_type,
                Def
            >
        );

        static constexpr auto region_index = tlu::front_t<StateRegionPath>::region_index;
        return get<region_index>(regions_).template state_def<tlu::pop_front_t<StateRegionPath>, StateDef>();
    }

    template<class StateRegionPath, class StateDef>
    [[nodiscard]] bool is_active_state_def() const
    {
        static_assert
        (
            std::is_same_v
            <
                typename detail::tlu::front_t<StateRegionPath>::sm_def_type,
                Def
            >
        );

        static constexpr auto region_index = tlu::front_t<StateRegionPath>::region_index;
        return get<region_index>(regions_).template is_active_state_def<tlu::pop_front_t<StateRegionPath>, StateDef>();
    }

    template<class StateDef>
    [[nodiscard]] bool is_active_state_def() const
    {
        static_assert(tlu::size_v<transition_table_type_list> == 1);

        return get<0>(regions_).template is_active_state_def<region_path_tpl<>, StateDef>();
    }

    template<class RegionPath>
    [[nodiscard]] bool is_running() const
    {
        return !is_active_state_def<RegionPath, states::stopped>();
    }

    [[nodiscard]] bool is_running() const
    {
        return !is_active_state_def<states::stopped>();
    }

    template<class Event>
    void on_entry(const Event& event)
    {
        call_on_entry(def_holder_.get(), root_sm_, event);
        tlu::for_each<region_tuple_type, region_start>(*this, event);
    }

    template<class Event>
    bool on_event(const Event& event)
    {
        if constexpr(state_traits::requires_on_event_v<Def, Event>)
        {
            def_holder_.get().on_event(event);
            tlu::for_each<region_tuple_type, region_process_event>(*this, event);
            return true;
        }
        else
        {
            return static_cast<bool>
            (
                tlu::for_each_plus<region_tuple_type, region_process_event>
                (
                    *this,
                    event
                )
            );
        }
    }

    template<class Event>
    void on_exit(const Event& event)
    {
        tlu::for_each<region_tuple_type, region_stop>(*this, event);
        call_on_exit(def_holder_.get(), root_sm_, event);
    }

private:
    using region_tuple_type = typename region_tuple
    <
        subsm,
        std::make_integer_sequence<int, tlu::size_v<transition_table_type_list>>
    >::type;

    struct region_start
    {
        template<class Region, class Event>
        static void call(subsm& self, const Event& event)
        {
            get<Region>(self.regions_).start(event);
        }
    };

    struct region_process_event
    {
        template<class Region, class Event>
        static int call(subsm& self, const Event& event)
        {
            return static_cast<int>(get<Region>(self.regions_).process_event(event));
        }
    };

    struct region_stop
    {
        template<class Region, class Event>
        static void call(subsm& self, const Event& event)
        {
            get<Region>(self.regions_).stop(event);
        }
    };

    root_sm_type& root_sm_;
    context_holder<context_type> ctx_holder_;
    detail::sm_object_holder<Def> def_holder_;
    region_tuple_type regions_;
};

} //namespace

#endif
