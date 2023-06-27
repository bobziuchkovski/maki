//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/awesm

#ifndef AWESM_ROOT_SM_CONF_HPP
#define AWESM_ROOT_SM_CONF_HPP

#include "transition_table.hpp"
#include "type_patterns.hpp"
#include "detail/constant.hpp"
#include "detail/tlu.hpp"
#include "detail/conf.hpp"

namespace awesm
{

/**
@brief The configuration type template for an @ref sm.

State machine definitions must expose a `conf` type alias to an instance of this
template. To instantiate this template, you have to write a chain of subtypes,
starting from `sm_conf` (a type alias of `sm_conf_tpl<>`), where each subtype
sets/activates an option.

Example:
@code
using conf = awesm::sm_conf
    ::transition_tables<sm_transition_table_t> //sets the transition_tables option
    ::context<context> //sets the context option
    ::pretty_name //activates the pretty_name option
;
@endcode
*/
template<class... Options>
struct sm_conf_tpl
{
    /**
    @brief Requires the @ref sm to call a user-provided
    `after_state_transition()` member function after any external state
    transition.

    The following expression must be valid, for every possible template argument
    list:
    @code
    sm_def.after_state_transition
    <
        region_path_type,
        source_state_type,
        event_type,
        target_state_type
    >(event);
    @endcode

    This hook can be useful for logging state transitions, for example.

    Example:
    @code
    struct sm_def
    {
        using conf = awesm::sm_conf
            ::after_state_transition
            //...
        ;

        template<class RegionPath, class SourceState, class Event, class TargetState>
        void after_state_transition(const Event& event)
        {
            //...
        }

        //...
    };
    @endcode
    */
    using after_state_transition = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::after_state_transition
#endif
    >;

    /**
    @brief Prevents the constructor of @ref sm from calling @ref sm::start().
    */
    using no_auto_start = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::no_auto_start
#endif
    >;

    /**
    @brief Requires the @ref sm to call a user-provided
    `before_state_transition()` member function before any external state
    transition.

    The following expression must be valid, for every possible template argument
    list:
    @code
    sm_def.before_state_transition
    <
        region_path_type,
        source_state_type,
        event_type,
        target_state_type
    >(event);
    @endcode

    This hook can be useful for logging state transitions, for example.

    Example:
    @code
    struct sm_def
    {
        using conf = awesm::sm_conf
            ::before_state_transition
            //...
        ;

        template<class RegionPath, class SourceState, class Event, class TargetState>
        void before_state_transition(const Event& event)
        {
            //...
        }

        //...
    };
    @endcode
    */
    using before_state_transition = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::before_state_transition
#endif
    >;

    /**
    @brief Specifies the context type.
    */
    template<class Context>
    using context = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::context<Context>
#endif
    >;

    /**
    @brief Disables run-to-completion.

    This makes the state machine much faster, but you have to make sure you
    <b>never</b> call @ref sm::process_event() recursively.

    Use it at your own risk!
    */
    using no_run_to_completion = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::no_run_to_completion
#endif
    >;

    /**
    @brief Requires the @ref sm to call a user-provided `pretty_name()` static
    member function to get the pretty name of the state machine.

    See @ref PrettyPrinting.

    Example:
    @code
    struct sm_def
    {
        using conf = sm_conf
            ::pretty_name
            //...
        ;

        static auto pretty_name()
        {
            return "Main FSM";
        }

        //...
    };
    @endcode
    */
    using pretty_name = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::pretty_name
#endif
    >;

    /**
    @brief Requires the @ref sm to call a user-provided `on_exception()`
    member function whenever it catches an exception.

    The following expression must be valid:
    @code
    sm_def.on_exception(std::current_exception());
    @endcode

    If this option isn't set, the @ref sm will send itself a @ref
    events::exception event, like so:
    @code
    process_event(events::exception{std::current_exception()});
    @endcode

    Example:
    @code
    struct sm_def
    {
        using conf = sm_conf
            ::pretty_name
            //...
        ;

        void on_exception(const std::exception_ptr& eptr)
        {
            //...
        }

        //...
    };
    @endcode
    */
    using on_exception = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::on_exception
#endif
    >;

    /**
    @brief Requires the @ref sm to call a user-provided `on_unprocessed()`
    member function whenever a call to @ref sm::process_event() doesn't lead to
    any state transition.

    The said member function must have the following form:
    @code
    void on_unprocessed(const event_type& event);
    @endcode

    State machine definitions typically define several overloads of this
    function, for all events of interest.

    Example:
    @code
    struct sm_def
    {
        using conf = sm_conf
            ::on_unprocessed
            //...
        ;

        void on_unprocessed(const some_event_type& event)
        {
            //...
        }

        void on_unprocessed(const some_other_event_type& event)
        {
            //...
        }

        //Ignore all other event types
        template<class Event>
        void on_unprocessed(const Event&)
        {
            //nothing
        }

        //...
    };
    @endcode

    */
    using on_unprocessed = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::on_unprocessed
#endif
    >;

    /**
    @brief Maximum object alignment requirement for the run-to-completion event
    queue to enable small object optimization (and thus avoid an extra memory
    allocation).
    */
    template<std::size_t Value>
    using small_event_max_align = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::small_event_max_align<Value>
#endif
    >;

    /**
    @brief Maximum object size for the run-to-completion event queue to enable
    small object optimization (and thus avoid an extra memory allocation).
    */
    template<std::size_t Value>
    using small_event_max_size = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::small_event_max_size<Value>
#endif
    >;

    /**
    @brief Requires the @ref sm to call a user-provided `on_event()` member
    function whenever it is about to process an event. Run-to-completion
    guarantee applies.
    @tparam EventFilters the list of events for which we want the @ref sm to
    call `on_event()`

    One of these expressions must be valid, for every given event type:
    @code
    sm_def.on_event(fsm, event);
    sm_def.on_event(event);
    sm_def.on_event();
    @endcode

    This hook can be useful when there are certain event types that you always
    want to process the same way, whatever the active state(s) of the state
    machine.

    Example:
    @code
    struct sm_def
    {
        using conf = sm_conf
            ::on_event<event_type_0, event_type_1>
            //...
        ;

        void on_event(const event_type_0& event)
        {
            //...
        }

        template<class Sm>
        void on_event(Sm& fsm, const event_type_1& event)
        {
            //...
        }

        //...
    };
    @endcode

    If manually listing all the event type you're insterested in is too
    inconvenient, you can use @ref on_event_auto.
    */
    template<class... EventFilters>
    using on_event = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::on_event<EventFilters...>
#endif
    >;

    /**
    @brief Behaves like the @ref on_event option, except that the event type
    list is automatically determined by the defined `on_event()` member
    functions.

    In the following example, `on_event()` will only be called for
    `event_type_0` and `event_type_1`:
    @code
    struct sm_def
    {
        using conf = sm_conf
            ::on_event_auto
            //...
        ;

        void on_event(const event_type_0& event)
        {
            //...
        }

        template<class Sm>
        void on_event(Sm& fsm, const event_type_1& event)
        {
            //...
        }

        //...
    };
    @endcode
    */
    using on_event_auto = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::on_event_auto
#endif
    >;

    /**
    @brief Requires the @ref sm to call a user-provided `on_entry()` member
    function whenever it starts.

    One of these expressions must be valid, for every given event type:
    @code
    sm_def.on_entry(fsm, event);
    sm_def.on_entry(event);
    sm_def.on_entry();
    @endcode

    Example:
    @code
    struct sm_def
    {
        using conf = sm_conf
            ::on_entry_any
            //...
        ;

        void on_entry(const event_type_0& event)
        {
            //...
        }

        template<class Sm>
        void on_entry(Sm& fsm, const event_type_1& event)
        {
            //...
        }

        //For all other event types
        void on_entry()
        {
            //...
        }

        //...
    };
    @endcode
    */
    using on_entry_any = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::on_entry_any
#endif
    >;

    /**
    @brief Requires the @ref sm to call a user-provided `on_exit()` member
    function whenever it stops.

    One of these expressions must be valid, for every given event type:
    @code
    sm_def.on_exit(fsm, event);
    sm_def.on_exit(event);
    sm_def.on_exit();
    @endcode

    Example:
    @code
    struct sm_def
    {
        using conf = sm_conf
            ::on_exit_any
            //...
        ;

        void on_exit(const event_type_0& event)
        {
            //...
        }

        template<class Sm>
        void on_exit(Sm& fsm, const event_type_1& event)
        {
            //...
        }

        //For all other event types
        void on_exit()
        {
            //...
        }

        //...
    };
    @endcode
    */
    using on_exit_any = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::on_exit_any
#endif
    >;

    /**
    @brief The list of transition table types. One region per transmission table
    is created.
    */
    template<class... Ts>
    using transition_tables = sm_conf_tpl
    <
        Options...,
#ifdef DOXYGEN
        implementation-detail
#else
        detail::options::transition_tables<Ts...>
#endif
    >;

#ifndef DOXYGEN
    using fast_unsafe_run_to_completion = sm_conf_tpl
    <
        Options...,
        detail::options::fast_unsafe_run_to_completion
    >;
#endif
};

/**
@brief Handy type alias to an empty (i.e. with default options) @ref
sm_conf_tpl.
*/
using sm_conf = sm_conf_tpl<>;

namespace detail
{
    template<class T>
    struct is_root_sm_conf
    {
        static constexpr auto value = false;
    };

    template<class... Options>
    struct is_root_sm_conf<sm_conf_tpl<Options...>>
    {
        static constexpr auto value = true;
    };

    template<class T>
    constexpr auto is_root_sm_conf_v = is_root_sm_conf<T>::value;
}

} //namespace

#endif
