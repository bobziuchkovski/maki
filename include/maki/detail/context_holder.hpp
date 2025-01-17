//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_DETAIL_CONTEXT_HOLDER_HPP
#define MAKI_DETAIL_CONTEXT_HOLDER_HPP

#include "type_traits.hpp"
#include <type_traits>

namespace maki::detail
{

/*
Contexts can be constructed with one of these statements:
    auto obj = context{root_sm, args...};
    auto obj = context{args...};
*/
template<class T>
class context_holder
{
public:
    using context_type = T;

    template
    <
        class RootSm,
        class... Args,
        std::enable_if_t<is_brace_constructible<T, RootSm&, Args...>, bool> = true
    >
    context_holder(RootSm& root_sm, Args&&... args):
        ctx_{root_sm, std::forward<Args>(args)...}
    {
    }

    template
    <
        class RootSm,
        class... Args,
        std::enable_if_t<is_brace_constructible<T, Args...>, bool> = true
    >
    context_holder(RootSm& /*root_sm*/, Args&&... args):
        ctx_{std::forward<Args>(args)...}
    {
    }

    template
    <
        class RootSm,
        class U = T,
        std::enable_if_t<std::is_reference_v<U>, bool> = true
    >
    context_holder(RootSm& /*root_sm*/, T& ctx):
        ctx_{ctx}
    {
    }

    T& get()
    {
        return ctx_;
    }

    const T& get() const
    {
        return ctx_;
    }

private:
    T ctx_;
};

} //namespace

#endif
