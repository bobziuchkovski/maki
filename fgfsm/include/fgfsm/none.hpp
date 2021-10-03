//Copyright Florian Goujeon 2021.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#ifndef FGFSM_NONE_HPP
#define FGFSM_NONE_HPP

namespace fgfsm
{

/*
Represents either:
- a null event (for anonymous transitions);
- a action that does nothing;
- a guard that always returns true.
*/
struct none
{
    template<class... Args>
    none(const Args&...)
    {
    }

    template<class... Args>
    bool operator()(const Args&...)
    {
        return true;
    }
};

} //namespace

#endif
