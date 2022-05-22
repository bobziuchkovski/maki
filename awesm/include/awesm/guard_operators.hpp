//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_GUARD_OPERATORS_HPP
#define AWESM_GUARD_OPERATORS_HPP

#include "detail/sm_object_holder.hpp"
#include "detail/call_member.hpp"

namespace awesm
{

namespace detail
{
    struct and_
    {
        static bool test(const bool l, const bool r)
        {
            return l && r;
        }
    };

    struct or_
    {
        static bool test(const bool l, const bool r)
        {
            return l || r;
        }
    };

    struct xor_
    {
        static bool test(const bool l, const bool r)
        {
            return l != r;
        }
    };

    template<class Lhs, class Rhs, class Operator>
    class binary_operator_guard
    {
        public:
            template<class Context, class Sm>
            binary_operator_guard(Context& context, Sm& machine):
                lhs_{context, machine},
                rhs_{context, machine}
            {
            }

            template<class SourceState, class Event, class TargetState>
            bool check
            (
                SourceState& source_state,
                const Event& event,
                TargetState& target_state
            )
            {
                return Operator::test
                (
                    detail::call_check
                    (
                        &lhs_.object,
                        &source_state,
                        &event,
                        &target_state
                    ),
                    detail::call_check
                    (
                        &rhs_.object,
                        &source_state,
                        &event,
                        &target_state
                    )
                );
            }

        private:
            detail::sm_object_holder<Lhs> lhs_;
            detail::sm_object_holder<Rhs> rhs_;
    };
}

template<class T>
class not_
{
    public:
        template<class Context, class Sm>
        not_(Context& context, Sm& machine):
            guard_{context, machine}
        {
        }

        template<class SourceState, class Event, class TargetState>
        bool check
        (
            SourceState& source_state,
            const Event& event,
            TargetState& target_state
        )
        {
            return !detail::call_check
            (
                &guard_.object,
                &source_state,
                &event,
                &target_state
            );
        }

    private:
        detail::sm_object_holder<T> guard_;
};

template<class L, class R>
using and_ = detail::binary_operator_guard<L, R, detail::and_>;

template<class L, class R>
using or_ = detail::binary_operator_guard<L, R, detail::or_>;

template<class L, class R>
using xor_ = detail::binary_operator_guard<L, R, detail::xor_>;

} //namespace

#endif