//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_COMPOSITE_STATE_CONF_HPP
#define AWESM_COMPOSITE_STATE_CONF_HPP

#include "state_conf.hpp"
#include "pretty_name.hpp"
#include "detail/type_list.hpp"

namespace awesm
{

namespace composite_state_options
{
    using on_entry_any = state_options::on_entry_any;

    using on_exit_any = state_options::on_exit_any;

    using get_pretty_name = detail::get_pretty_name_option;
}

template<const auto& TransitionTableFn, class... Options>
struct composite_state_conf
{
    using option_type_list = detail::type_list<Options...>;
    static constexpr auto is_composite = true;
    static constexpr auto transition_table_fn = TransitionTableFn;
};

} //namespace

#endif
