//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_DETAIL_TLU_FRONT_HPP
#define AWESM_DETAIL_TLU_FRONT_HPP

#include "at.hpp"

namespace awesm::detail::tlu
{

template<class TList>
using front_t = at_t<TList, 0>;

} //namespace

#endif
