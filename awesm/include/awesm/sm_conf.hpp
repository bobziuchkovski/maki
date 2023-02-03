//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_SM_CONF_HPP
#define AWESM_SM_CONF_HPP

#include "detail/type_list.hpp"
#include "transition_table.hpp"
#include "pretty_name.hpp"

namespace awesm
{

namespace sm_options
{
    struct after_state_transition{};
    struct before_entry{};
    struct before_state_transition{};
    struct disable_run_to_completion{};
    struct on_event{};
    struct on_exception{};

    template<size_t Value>
    struct small_event_max_size
    {
        static constexpr size_t get_small_event_max_size()
        {
            return Value;
        }
    };

    template<size_t Value>
    struct small_event_max_align
    {
        static constexpr size_t get_small_event_max_alignment_requirement()
        {
            return Value;
        }
    };

    using get_pretty_name = detail::get_pretty_name_option;
}

template<const auto& TransitionTableFn, class Context, class... Options>
struct sm_conf: Options...
{
    using context_type = Context;
    using option_type_list = detail::type_list<Options...>;
    static constexpr auto transition_table_fn = TransitionTableFn;
};

} //namespace

#endif
