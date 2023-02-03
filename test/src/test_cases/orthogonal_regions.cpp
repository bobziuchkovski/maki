//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#include <awesm.hpp>
#include "common.hpp"

namespace
{
    struct context
    {
        std::string out;
    };

    namespace events
    {
        struct button_press
        {
        };

        struct exception_request
        {
        };
    }

    namespace states
    {
        EMPTY_STATE(off0);
        EMPTY_STATE(off1);
        EMPTY_STATE(on0);
        struct on1
        {
            using conf_type = awesm::state_conf
            <
                awesm::state_options::on_event_any_of<events::exception_request>
            >;

            void on_event(const events::exception_request&)
            {
                throw std::runtime_error{"exception"};
            }
        };
    }

    auto region_0_transition_table()
    {
        return awesm::transition_table
        <
            awesm::row<states::off0, events::button_press, states::on0>
        >;
    }

    auto region_1_transition_table()
    {
        return awesm::transition_table
        <
            awesm::row<states::off1, events::button_press, states::on1>
        >;
    }

    struct sm_def
    {
        using conf_type = awesm::sm_conf
        <
            awesm::transition_table_list
            <
                region_0_transition_table,
                region_1_transition_table
            >,
            context,
            awesm::sm_options::on_exception,
            awesm::sm_options::before_state_transition,
            awesm::sm_options::after_state_transition
        >;

        template<class RegionPath, class SourceState, class Event, class TargetState>
        void before_state_transition(const Event& /*event*/)
        {
            constexpr auto region_index = awesm::detail::tlu::at_t<RegionPath, 0>::region_index;
            ctx.out += "before_state_transition[" + std::to_string(region_index) + "];";
        }

        template<class RegionPath, class SourceState, class Event, class TargetState>
        void after_state_transition(const Event& /*event*/)
        {
            constexpr auto region_index = awesm::detail::tlu::at_t<RegionPath, 0>::region_index;
            ctx.out += "after_state_transition[" + std::to_string(region_index) + "];";
        }

        void on_exception(const std::exception_ptr& eptr)
        {
            try
            {
                std::rethrow_exception(eptr);
            }
            catch(const std::exception& e)
            {
                ctx.out += std::string{"on_exception:"} + e.what() + ";";
            }
        }

        context& ctx;
    };

    using sm_t = awesm::sm<sm_def>;
}

TEST_CASE("orthogonal_regions")
{
    auto ctx = context{};
    auto sm = sm_t{ctx};

    using sm_region_0_path = awesm::make_region_path<sm_t, 0>;
    using sm_region_1_path = awesm::make_region_path<sm_t, 1>;

    sm.start();
    REQUIRE(sm.is_active_state<sm_region_0_path, states::off0>());
    REQUIRE(sm.is_active_state<sm_region_1_path, states::off1>());
    REQUIRE(ctx.out == "before_state_transition[0];after_state_transition[0];before_state_transition[1];after_state_transition[1];");

    ctx.out.clear();
    sm.process_event(events::button_press{});
    REQUIRE(sm.is_active_state<sm_region_0_path, states::on0>());
    REQUIRE(sm.is_active_state<sm_region_1_path, states::on1>());
    REQUIRE(ctx.out == "before_state_transition[0];after_state_transition[0];before_state_transition[1];after_state_transition[1];");

    ctx.out.clear();
    sm.process_event(events::exception_request{});
    REQUIRE(ctx.out == "on_exception:exception;");
}
