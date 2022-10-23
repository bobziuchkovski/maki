//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_REGION_TUPLE_HPP
#define AWESM_DETAIL_REGION_TUPLE_HPP

#include "region.hpp"
#include "../transition_table_list.hpp"
#include <type_traits>

namespace awesm::detail
{

template<class SmConfiguration, class RegionIndexSequence, class... TransitionTables>
struct region_holder_tuple;

template<class SmConfiguration, int... RegionIndexes, class... TransitionTables>
struct region_holder_tuple<SmConfiguration, std::integer_sequence<int, RegionIndexes...>, TransitionTables...>
{
    using type = sm_object_holder_tuple<region<SmConfiguration, RegionIndexes, TransitionTables>...>;
};

template<class SmConfiguration, class RegionIndexSequence, class... TransitionTables>
using region_holder_tuple_t =
    typename region_holder_tuple<SmConfiguration, RegionIndexSequence, TransitionTables...>::type
;

template<class SmConfiguration, class TransitionTableList>
class region_tuple;

template<class SmConfiguration, class... TransitionTables>
class region_tuple<SmConfiguration, transition_table_list<TransitionTables...>>
{
    public:
        template<class Sm, class Context>
        explicit region_tuple(Sm& top_level_sm, Context& context):
            regions_{top_level_sm, context}
        {
        }

        region_tuple(const region_tuple&) = delete;
        region_tuple(region_tuple&&) = delete;
        region_tuple& operator=(const region_tuple&) = delete;
        region_tuple& operator=(region_tuple&&) = delete;
        ~region_tuple() = default;

        template<int RegionIndex>
        const auto& get_region() const
        {
            return get<RegionIndex>(regions_);
        }

        template<class State, int RegionIndex>
        const auto& get_state() const
        {
            return get_region<RegionIndex>().template get_state<State>();
        }

        template<class State, int RegionIndex>
        [[nodiscard]] bool is_active_state() const
        {
            return get<RegionIndex>(regions_).template is_active_state<State>();
        }

        template<class Sm, class Event = none>
        void start(Sm& mach, const Event& event = {})
        {
            for_each
            (
                regions_,
                [&](auto& reg)
                {
                    reg.start(mach, event);
                }
            );
        }

        template<class Sm, class Event = none>
        void stop(Sm& mach, const Event& event = {})
        {
            for_each
            (
                regions_,
                [&](auto& reg)
                {
                    reg.stop(mach, event);
                }
            );
        }

        template<class Sm, class Event>
        void process_event(Sm& mach, const Event& event)
        {
            for_each
            (
                regions_,
                [&](auto& reg)
                {
                    reg.process_event(mach, event);
                }
            );
        }

    private:
        using region_indexes = std::make_integer_sequence<int, sizeof...(TransitionTables)>;
        region_holder_tuple_t<SmConfiguration, region_indexes, TransitionTables...> regions_;
};

} //namespace

#endif
