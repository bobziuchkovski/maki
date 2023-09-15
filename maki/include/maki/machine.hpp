//Copyright Florian Goujeon 2021 - 2023.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/maki

#ifndef MAKI_MACHINE_HPP
#define MAKI_MACHINE_HPP

#include "machine_conf.hpp"
#include "region_path.hpp"
#include "detail/noinline.hpp"
#include "detail/submachine.hpp"
#include "detail/function_queue.hpp"
#include "detail/tlu.hpp"
#include "detail/type.hpp"
#include "detail/overload_priority.hpp"
#include <type_traits>

namespace maki
{

namespace detail
{
    enum class machine_operation
    {
        start,
        stop,
        process_event
    };
}

/**
@brief The state machine implementation template.
@tparam Def the state machine definition, a class that must at least define a
`using conf` to an @ref machine_conf_tpl instance whose `transition_tables` and
`context` options are set

Here is an example of valid state machine definition, where:
- `transition_table_t` is a user-provided `using` of a @ref
transition_table_tpl instance;
- `context` is a user-provided class.

@snippet lamp/src/main.cpp machine-def

The state machine type itself can then be defined like so:
@snippet lamp/src/main.cpp machine
*/
template<class Def>
class machine
{
public:
    /**
    @brief The state machine definition type.
    */
    using def_type = Def;

    /**
    @brief The state machine configuration type.
    */
    using conf = typename Def::conf;

    /**
    @brief The state machine context type.
    */
    using context_type = detail::option_t<conf, detail::option_id::context>;

    static_assert(detail::is_root_sm_conf_v<conf>, "The root state machine definition must include a using conf = machine_conf::...");

    /**
    @brief The constructor.
    @param ctx_args the arguments to be passed to the context constructor

    The constructor first instantiates all the state machine objects (i.e. the
    state machine context, the state machine definition, the region contexts (if
    any) and the states). All state machine objects are constructed with one of
    these statements:
    @code
    auto obj = object_type{*this, context()};
    auto obj = object_type{context()};
    auto obj = object_type{};
    @endcode

    Finally, unless the `no_auto_start` option is defined, `start()` is called.
    */
    template<class... ContextArgs>
    explicit machine(ContextArgs&&... ctx_args):
        submachine_(*this, std::forward<ContextArgs>(ctx_args)...)
    {
        if constexpr(detail::option_v<conf, detail::option_id::auto_start>)
        {
            //start
            execute_operation_now<detail::machine_operation::start>(events::start{});
        }
    }

    machine(const machine&) = delete;
    machine(machine&&) = delete;
    machine& operator=(const machine&) = delete;
    machine& operator=(machine&&) = delete;
    ~machine() = default;

    /**
    @brief Returns the definition instantiated at construction.
    */
    Def& def()
    {
        return submachine_.def();
    }

    /**
    @brief Returns the definition instantiated at construction.
    */
    const Def& def() const
    {
        return submachine_.def();
    }

    /**
    @brief Returns the context instantiated at construction.
    */
    context_type& context()
    {
        return submachine_.context();
    }

    /**
    @brief Returns the context instantiated at construction.
    */
    const context_type& context() const
    {
        return submachine_.context();
    }

    /**
    @brief Returns the state of type `State` instantiated by the region
    indicated by `RegionPath`.
    @tparam RegionPath an instance of @ref region_path_tpl pointing to the
    region of interest (see @ref RegionPath)
    @tparam State the state type
    */
    template<class RegionPath, class State>
    State& state()
    {
        return submachine_.template state_def<RegionPath, State>();
    }

    /**
    @brief Returns the state of type `State` instantiated by the region
    indicated by `RegionPath`.
    @tparam RegionPath an instance of @ref region_path_tpl pointing to the
    region of interest (see @ref RegionPath)
    @tparam State the state type
    */
    template<class RegionPath, class State>
    const State& state() const
    {
        return submachine_.template state_def<RegionPath, State>();
    }

    /**
    @brief Returns whether the region indicated by `RegionPath` is running.
    @tparam RegionPath an instance of @ref region_path_tpl pointing to the
    region of interest (see @ref RegionPath)
    */
    template<class RegionPath>
    [[nodiscard]] bool is_running() const
    {
        return submachine_.template is_running<RegionPath>();
    }

    /**
    @brief Returns whether the single region of the state machine in running.
    This function can only be called if the state machine contains a single
    region.
    */
    [[nodiscard]] bool is_running() const
    {
        return submachine_.is_running();
    }

    /**
    @brief Returns whether `State` is active in the region indicated by
    `RegionPath`.
    @tparam RegionPath an instance of @ref region_path_tpl pointing to the
    region of interest (see @ref RegionPath)
    @tparam State the state type
    */
    template<class RegionPath, class State>
    [[nodiscard]] bool is_active_state(const State& stt) const
    {
        return submachine_.template is_active_state_def<RegionPath>(stt);
    }

    /**
    @brief Returns whether `State` is active in the single region of the state
    machine. This function can only be called if the state machine contains a
    single region.
    @tparam State the state type
    */
    template<class State>
    [[nodiscard]] bool is_active_state(const State& stt) const
    {
        return submachine_.is_active_state_def(stt);
    }

    /**
    @brief Starts the state machine
    @param event the event to be passed to the event hooks, mainly the
    `on_entry()` function of the initial state(s)

    Concretely, if @ref states::stopped is the active state, exits @ref
    states::stopped and enters the initial state.

    Reminder: There's no need to call this function after the construction,
    unless the `no_auto_start` option is set.
    */
    template<class Event = events::start>
    void start(const Event& event = {})
    {
        execute_operation<detail::machine_operation::start>(event);
    }

    /**
    @brief Stops the state machine
    @param event the event to be passed to the event hooks, mainly the
    `on_exit()` function of the active state(s)

    Concretely, if @ref states::stopped is not the active state, exits the
    active state and enters @ref states::stopped.
    */
    template<class Event = events::stop>
    void stop(const Event& event = {})
    {
        execute_operation<detail::machine_operation::stop>(event);
    }

    /**
    @brief Processes the given event
    @param event the event to be processed

    It's hard to describe all the things this function does, as it is the point
    of the whole library, but let's try to list the basic stuff with the
    following pseudocode:
    @code
    //Run-to-completion: Don't let potential recursive calls interrupt the
    //current processing
    if(processing_event)
    {
        enqueue_event(event);
    }

    //Process the event
    FOR_EACH_REGION()
    {
        bool state_transition_happened = false;

        //Process event in transition table
        FOR_EACH_TRANSITION_IN_REGION_TRANSITION_TABLE()
        {
            if
            (
                IS_ACTIVE_STATE(transition_t::source_state_type) &&
                SAME_TYPE(Event, transition_t::event_type) &&
                CALL_GUARD(transition_t)
            )
            {
                CALL_ON_EXIT(transition_t::source_state_type);
                active_state_id = transition_t::target_state_id;
                CALL_ACTION(transition_t);
                CALL_ON_ENTRY(transition_t::target_state_type);

                state_transition_happened = true;
                break;
            }
        }

        //Process event in active state
        if(!state_transition_happened)
        {
            CALL_ON_EVENT(active_state);
        }
    }

    //Run-to-completion: Process enqueued events the same way
    process_enqueued_events();
    @endcode
    */
    template<class Event>
    void process_event(const Event& event)
    {
        execute_operation<detail::machine_operation::process_event>(event);
    }

    /**
    @brief Like process_event(), but doesn't check if an event is being
    processed.
    @param event the event to be processed

    <b>USE WITH CAUTION!</b>

    You can call this function if you're **absolutely** sure that you're not
    calling this function while process_event() is being called. Otherwise,
    <b>run-to-completion will be broken</b>.

    Compared to process_event(), process_event_now() is:
    - faster to build, because the enqueue_event() function template won't be
    instantiated;
    - faster to run, because an `if` statement is skipped.
    */
    template<class Event>
    void process_event_now(const Event& event)
    {
        execute_operation_now<detail::machine_operation::process_event>(event);
    }

    /**
    @brief Enqueues event for later processing
    @param event the event to be processed

    You can call this function instead of doing a recursive call to @ref
    process_event().

    This function is slightly faster than @ref process_event(), but if you're
    not sure what you're doing, just call @ref process_event() instead.
    */
    template<class Event>
    MAKI_NOINLINE void enqueue_event(const Event& event)
    {
        static_assert(detail::option_v<conf, detail::option_id::run_to_completion>);
        try
        {
            enqueue_event_impl<detail::machine_operation::process_event>(event);
        }
        catch(...)
        {
            process_exception(std::current_exception());
        }
    }

    /**
    @brief Processes events that have been enqueued by the run-to-completion
    mechanism.

    Calling this function is only relevant when managing an exception thrown by
    user code and caught by the state machine.
    */
    void process_enqueued_events()
    {
        if(!executing_operation_)
        {
            auto grd = executing_operation_guard{*this};
            try
            {
                operation_queue_.invoke_and_pop_all(*this);
            }
            catch(...)
            {
                process_exception(std::current_exception());
            }
        }
    }

private:
    class executing_operation_guard
    {
    public:
        executing_operation_guard(machine& self):
            self_(self)
        {
            self_.executing_operation_ = true;
        }

        executing_operation_guard(const executing_operation_guard&) = delete;
        executing_operation_guard(executing_operation_guard&&) = delete;
        executing_operation_guard& operator=(const executing_operation_guard&) = delete;
        executing_operation_guard& operator=(executing_operation_guard&&) = delete;

        ~executing_operation_guard()
        {
            self_.executing_operation_ = false;
        }

    private:
        machine& self_;
    };

    struct real_operation_queue_holder
    {
        template<bool = true> //Dummy template for lazy evaluation
        using type = detail::function_queue
        <
            machine&,
            detail::option_v<conf, detail::option_id::small_event_max_size>,
            detail::option_v<conf, detail::option_id::small_event_max_align>
        >;
    };
    struct empty_holder
    {
        template<bool = true> //Dummy template for lazy evaluation
        struct type{};
    };
    using operation_queue_type = typename std::conditional_t
    <
        detail::option_v<conf, detail::option_id::run_to_completion>,
        real_operation_queue_holder,
        empty_holder
    >::template type<>;

    template<detail::machine_operation Operation, class Event>
    void execute_operation(const Event& event)
    {
        try
        {
            if constexpr(detail::option_v<conf, detail::option_id::run_to_completion>)
            {
                if(!executing_operation_) //If call is not recursive
                {
                    execute_operation_now<Operation>(event);
                }
                else
                {
                    //Enqueue event in case of recursive call
                    enqueue_event_impl<Operation>(event);
                }
            }
            else
            {
                execute_one_operation<Operation>(event);
            }
        }
        catch(...)
        {
            process_exception(std::current_exception());
        }
    }

    template<detail::machine_operation Operation, class Event>
    void execute_operation_now(const Event& event)
    {
        if constexpr(detail::option_v<conf, detail::option_id::run_to_completion>)
        {
            auto grd = executing_operation_guard{*this};

            execute_one_operation<Operation>(event);

            //Process enqueued events, if any
            operation_queue_.invoke_and_pop_all(*this);
        }
        else
        {
            execute_one_operation<Operation>(event);
        }
    }

    template<detail::machine_operation Operation, class Event>
    void enqueue_event_impl(const Event& event)
    {
        operation_queue_.template push<any_event_visitor<Operation>>(event);
    }

    template<detail::machine_operation Operation>
    struct any_event_visitor
    {
        template<class Event>
        static void call(const Event& event, machine& self)
        {
            self.execute_one_operation<Operation>(event);
        }
    };

    void process_exception(const std::exception_ptr& eptr)
    {
        if constexpr(detail::option_v<conf, detail::option_id::on_exception>)
        {
            def().on_exception(eptr);
        }
        else
        {
            process_event(events::exception{eptr});
        }
    }

    template<detail::machine_operation Operation, class Event>
    void execute_one_operation(const Event& event)
    {
        if constexpr(Operation == detail::machine_operation::start)
        {
            submachine_.on_entry(event);
        }
        else if constexpr(Operation == detail::machine_operation::stop)
        {
            submachine_.on_exit(event);
        }
        else
        {
            if constexpr(detail::option_v<conf, detail::option_id::on_unprocessed>)
            {
                auto processed = false;
                submachine_.on_event(event, processed);
                if(!processed)
                {
                    def().on_unprocessed(event);
                }
            }
            else
            {
                submachine_.on_event(event);
            }
        }
    }

    detail::submachine<Def, void> submachine_;
    bool executing_operation_ = false;
    operation_queue_type operation_queue_;
};

} //namespace

#endif
