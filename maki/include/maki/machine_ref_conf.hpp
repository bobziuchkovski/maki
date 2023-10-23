//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::machine_ref_conf struct template
*/

#ifndef MAKI_MACHINE_REF_CONF_HPP
#define MAKI_MACHINE_REF_CONF_HPP

#include "type_list.hpp"
#include "type.hpp"
#include "detail/tlu.hpp"

namespace maki
{

/**
@brief The configuration for @ref machine_ref
*/
template<class EventTypeList = type_list<>>
struct machine_ref_conf
{
    EventTypeList has_on_event_for;

    template<class... Ts>
    [[nodiscard]] constexpr auto enable_on_event_for() const
    {
        return machine_ref_conf<type_list<Ts...>>{};
    }
};

/**
@related machine_ref_conf
*/
inline constexpr auto default_machine_ref_conf = machine_ref_conf<>{};

} //namespace

#endif
