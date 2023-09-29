//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_MACHINE_CONF_HPP
#define MAKI_MACHINE_CONF_HPP

#include "transition_table.hpp"
#include "type_patterns.hpp"
#include "detail/type_list.hpp"
#include "detail/type.hpp"
#include "detail/tlu.hpp"

namespace maki
{

namespace detail
{
    template<class... Args>
    constexpr auto make_machine_conf(const Args&... args);
}

template
<
    class ContextType = detail::type_t<void>,
    class OnEventTypeList = detail::type_list<>,
    class TransitionTableTypeList = detail::type_list<>
>
struct machine_conf
{
    /**
    @brief Requires the @ref machine to call a user-provided
    `after_state_transition()` member function after any external state
    transition.

    The following expression must be valid, for every possible template argument
    list:
    @code
    machine_def.after_state_transition
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
    struct machine_def
    {
        using conf = maki::machine_conf
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
    bool after_state_transition_enabled = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Prevents the constructor of @ref machine from calling @ref machine::start().
    */
    bool auto_start = true; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Requires the @ref machine to call a user-provided
    `before_state_transition()` member function before any external state
    transition.

    The following expression must be valid, for every possible template argument
    list:
    @code
    machine_def.before_state_transition
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
    struct machine_def
    {
        using conf = maki::machine_conf
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
    bool before_state_transition_enabled = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Specifies the context type.
    */
    ContextType context_type; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Disables run-to-completion.

    This makes the state machine much faster, but you have to make sure you
    <b>never</b> call @ref machine::process_event() recursively.

    Use it at your own risk!
    */
    bool run_to_completion = true; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Requires the @ref machine to call a user-provided `pretty_name()` static
    member function to get the pretty name of the state machine.

    See @ref PrettyPrinting.

    Example:
    @code
    struct machine_def
    {
        using conf = machine_conf
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
    bool pretty_name_fn = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Requires the @ref machine to call a user-provided `on_exception_en()`
    member function whenever it catches an exception.

    The following expression must be valid:
    @code
    machine_def.on_exception_en(std::current_exception());
    @endcode

    If this option isn't set, the @ref machine will send itself a @ref
    events::exception event, like so:
    @code
    process_event(events::exception{std::current_exception()});
    @endcode

    Example:
    @code
    struct machine_def
    {
        using conf = machine_conf
            ::pretty_name
            //...
        ;

        void on_exception_en(const std::exception_ptr& eptr)
        {
            //...
        }

        //...
    };
    @endcode
    */
    bool on_exception_enabled = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Requires the @ref machine to call a user-provided `on_unprocessed_en()`
    member function whenever a call to @ref machine::process_event() doesn't lead to
    any state transition.

    The said member function must have the following form:
    @code
    void on_unprocessed_en(const event_type& event);
    @endcode

    State machine definitions typically define several overloads of this
    function, for all events of interest.

    Example:
    @code
    struct machine_def
    {
        using conf = machine_conf
            ::on_unprocessed_en
            //...
        ;

        void on_unprocessed_en(const some_event_type& event)
        {
            //...
        }

        void on_unprocessed_en(const some_other_event_type& event)
        {
            //...
        }

        //Ignore all other event types
        template<class Event>
        void on_unprocessed_en(const Event&)
        {
            //nothing
        }

        //...
    };
    @endcode

    */
    bool on_unprocessed_enabled = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Maximum object alignment requirement for the run-to-completion event
    queue to enable small object optimization (and thus avoid an extra memory
    allocation).
    */
    std::size_t small_event_max_align = 8; //NOLINT(misc-non-private-member-variables-in-classes, cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    /**
    @brief Maximum object size for the run-to-completion event queue to enable
    small object optimization (and thus avoid an extra memory allocation).
    */
    std::size_t small_event_max_size = 16; //NOLINT(misc-non-private-member-variables-in-classes, cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    /**
    @brief Requires the @ref machine to call a user-provided `on_event()` member
    function whenever it is about to process an event. Run-to-completion
    guarantee applies.
    @tparam EventFilters the list of events for which we want the @ref machine to
    call `on_event()`

    One of these expressions must be valid, for every given event type:
    @code
    machine_def.on_event(fsm, event);
    machine_def.on_event(event);
    machine_def.on_event();
    @endcode

    This hook can be useful when there are certain event types that you always
    want to process the same way, whatever the active state(s) of the state
    machine.

    Example:
    @code
    struct machine_def
    {
        using conf = machine_conf
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
    OnEventTypeList on_event_types; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Behaves like the @ref on_event option, except that the event type
    list is automatically determined by the defined `on_event()` member
    functions.

    In the following example, `on_event()` will only be called for
    `event_type_0` and `event_type_1`:
    @code
    struct machine_def
    {
        using conf = machine_conf
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
    bool on_event_auto = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Requires the @ref machine to call a user-provided `on_entry()` member
    function whenever it starts.

    One of these expressions must be valid, for every given event type:
    @code
    machine_def.on_entry(fsm, event);
    machine_def.on_entry(event);
    machine_def.on_entry();
    @endcode

    Example:
    @code
    struct machine_def
    {
        using conf = machine_conf
            ::enable_on_entry_any
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
    bool on_entry_any = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief Requires the @ref machine to call a user-provided `on_exit()` member
    function whenever it stops.

    One of these expressions must be valid, for every given event type:
    @code
    machine_def.on_exit(fsm, event);
    machine_def.on_exit(event);
    machine_def.on_exit();
    @endcode

    Example:
    @code
    struct machine_def
    {
        using conf = machine_conf
            ::enable_on_exit_any
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
    bool on_exit_any = false; //NOLINT(misc-non-private-member-variables-in-classes)

    /**
    @brief The list of transition table types. One region per transmission table
    is created.
    */
    TransitionTableTypeList transition_table_types; //NOLINT(misc-non-private-member-variables-in-classes)

#define MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(changed_var_name, new_value) /*NOLINT(cppcoreguidelines-macro-usage)*/ \
    [[maybe_unused]] const auto arg_after_state_transition_enabled  = after_state_transition_enabled; \
    [[maybe_unused]] const auto arg_auto_start                      = auto_start; \
    [[maybe_unused]] const auto arg_before_state_transition_enabled = before_state_transition_enabled; \
    [[maybe_unused]] const auto arg_context_type                    = context_type; \
    [[maybe_unused]] const auto arg_run_to_completion               = run_to_completion; \
    [[maybe_unused]] const auto arg_pretty_name_fn                  = pretty_name_fn; \
    [[maybe_unused]] const auto arg_on_exception_enabled            = on_exception_enabled; \
    [[maybe_unused]] const auto arg_on_unprocessed_enabled          = on_unprocessed_enabled; \
    [[maybe_unused]] const auto arg_small_event_max_align           = small_event_max_align; \
    [[maybe_unused]] const auto arg_small_event_max_size            = small_event_max_size; \
    [[maybe_unused]] const auto arg_on_event_types                  = on_event_types; \
    [[maybe_unused]] const auto arg_on_event_auto                   = on_event_auto; \
    [[maybe_unused]] const auto arg_on_entry_any                    = on_entry_any; \
    [[maybe_unused]] const auto arg_on_exit_any                     = on_exit_any; \
    [[maybe_unused]] const auto arg_transition_table_types          = transition_table_types; \
 \
    { \
        const auto arg_##changed_var_name = new_value; \
 \
        return detail::make_machine_conf \
        ( \
            arg_after_state_transition_enabled, \
            arg_auto_start, \
            arg_before_state_transition_enabled, \
            arg_context_type, \
            arg_run_to_completion, \
            arg_pretty_name_fn, \
            arg_on_exception_enabled, \
            arg_on_unprocessed_enabled, \
            arg_small_event_max_align, \
            arg_small_event_max_size, \
            arg_on_event_types, \
            arg_on_event_auto, \
            arg_on_entry_any, \
            arg_on_exit_any, \
            arg_transition_table_types \
        ); \
    }

    [[nodiscard]] constexpr auto after_state_transition() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(after_state_transition_enabled, true)
    }

    [[nodiscard]] constexpr auto disable_auto_start() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(auto_start, false)
    }

    [[nodiscard]] constexpr auto before_state_transition() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(before_state_transition_enabled, true)
    }

    template<class Context>
    [[nodiscard]] constexpr auto set_context_type() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(context_type, detail::type_c<Context>)
    }

    [[nodiscard]] constexpr auto disable_run_to_completion() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(run_to_completion, false)
    }

    [[nodiscard]] constexpr auto enable_pretty_name_fn() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(pretty_name_fn, true)
    }

    [[nodiscard]] constexpr auto on_exception() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(on_exception_enabled, true)
    }

    [[nodiscard]] constexpr auto on_unprocessed() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(on_unprocessed_enabled, true)
    }

    [[nodiscard]] constexpr auto set_small_event_max_align(const std::size_t value) const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(small_event_max_align, value)
    }

    [[nodiscard]] constexpr auto set_small_event_max_size(const std::size_t value) const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(small_event_max_size, value)
    }

    template<class... Ts>
    [[nodiscard]] constexpr auto enable_on_event() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(on_event_types, detail::type_list<Ts...>{})
    }

    [[nodiscard]] constexpr auto enable_on_event_auto() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(on_event_auto, true)
    }

    [[nodiscard]] constexpr auto enable_on_entry_any() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(on_entry_any, true)
    }

    [[nodiscard]] constexpr auto enable_on_exit_any() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(on_exit_any, true)
    }

    template<class... Ts>
    [[nodiscard]] constexpr auto set_transition_tables() const
    {
        MAKI_DETAIL_MAKE_MACHINE_CONF_COPY(transition_table_types, detail::type_list<Ts...>{})
    }

#undef MAKI_DETAIL_MAKE_MACHINE_CONF_COPY
};

inline constexpr auto machine_conf_c = machine_conf<>{};

namespace detail
{
    template<class... Args>
    constexpr auto make_machine_conf(const Args&... args)
    {
        using args_t = detail::type_list<Args...>;
        constexpr auto context_type_arg_index = 3;
        constexpr auto on_event_type_list_arg_index = 10;
        constexpr auto transition_table_list_type_index = 14;
        return machine_conf
        <
            tlu::get_t<args_t, context_type_arg_index>,
            tlu::get_t<args_t, on_event_type_list_arg_index>,
            tlu::get_t<args_t, transition_table_list_type_index>
        >{args...};
    }

    template<class T>
    struct is_root_sm_conf
    {
        static constexpr auto value = false;
    };

    template<class... Options>
    struct is_root_sm_conf<machine_conf<Options...>>
    {
        static constexpr auto value = true;
    };

    template<class T>
    constexpr auto is_root_sm_conf_v = is_root_sm_conf<T>::value;
}

} //namespace

#endif
