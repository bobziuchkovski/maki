//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

/**
@file
@brief Defines the maki::states namespace and its types
*/

#ifndef MAKI_DETAIL_STATES_HPP
#define MAKI_DETAIL_STATES_HPP

#include "state_conf.hpp"

/**
@brief Some predefined states used by Maki itself
*/
namespace maki::states
{

/**
@brief Represents the state of any region before @ref machine::start() is called and
after @ref machine::stop() is called.
*/
struct stopped
{
    static constexpr const auto& conf = default_state_conf;
};

} //namespace

#endif
