[![Windows Build status](https://ci.appveyor.com/api/projects/status/2vc1wsgwg7bo9y45/branch/main?svg=true)](https://ci.appveyor.com/project/fgoujeon/maki/branch/main)

[![Linux build status](https://github.com/fgoujeon/maki/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/fgoujeon/maki/actions/workflows/ci.yml)

---

# Maki
Maki is a C++17 finite-state machine library.

**This library is still in early development stage: it is functional and tested, but its API is subject to change.**

## Features
Maki implements the following key features:

* **transition tables**, featuring:
  * **actions**;
  * **guards**;
  * **internal transitions**, aka transitions to `maki::null` state;
  * **completion transitions**, aka anonymous transitions, aka transitions through `maki::null` event;
  * **type patterns**, aka `maki::any`, `maki::any_of`, `maki::any_but`, `maki::any_if` and `maki::any_if_not` for source states and events;
* **states as classes**, featuring:
  * **entry/exit actions**, aka `on_entry()` and `on_exit()` member functions;
  * **internal transition actions**, aka `on_event()` member function;
* **run-to-completion**, the guarantee that the processing of an event won't be interrupted, even if we ask to handle other events in the process;
* **orthogonal regions**;
* **submachines**.

Besides its features, Maki:

* **has excellent performance**, both at build time and runtime (see [benchmark](https://github.com/fgoujeon/fsm-benchmark));
* **doesn't depend on any library** other than the C++ standard library;
* **doesn't rely on exceptions**, while still allowing you to be exception-safe;
* **doesn't rely on RTTI**;
* is licensed under the terms of the very permissive **Boost Software License**, allowing you to use the library in any kind of free or proprietary software or firmware.

What is *not* implemented (yet):

* elaborate ways to enter and exit a submachine (e.g. forks, history and exit points);
* event deferral;
* optional thread safety with mutexes.

## Documentation
You can access the full documentation [here](https://fgoujeon.github.io/maki/doc/v1).

## Example
The following example is firmware for an RGB lamp. This lamp has a single button and an LED that can emit white, red, green or blue.

The expected behavior is:

* when the lamp is off, pushing the button turns it on in white color;
* then, briefly pushing the button changes the color of the LED, following this order: white -> red -> green -> blue -> white -> etc.;
* finally, whenever the user presses the button more than one second, the lamp turns off.

This behavior can be expressed with the following transition table:
```c++
constexpr auto transition_table = maki::empty_transition_table
    //     source_state,   event,       target_state,   action,           guard
    .add_c<off,            button_push, emitting_white, turn_light_white>
    .add_c<emitting_white, button_push, emitting_red,   turn_light_red,   is_short_push>
    .add_c<emitting_red,   button_push, emitting_green, turn_light_green, is_short_push>
    .add_c<emitting_green, button_push, emitting_blue,  turn_light_blue,  is_short_push>
    .add_c<emitting_blue,  button_push, emitting_white, turn_light_white, is_short_push>
    .add_c<any_but<off>,   button_push, off,            turn_light_off,   is_long_push>
;
```

Here is the full program:
```c++
#include <maki.hpp>
#include <functional>
#include <iostream>

/*
This class monitors the button. It sends an event of type push_event whenever
the user pushes the button.
*/
class button
{
public:
    struct push_event
    {
        //Push duration, in milliseconds
        int duration_ms = 0;
    };

    using event_handler = std::function<void(const push_event&)>;

    button(const event_handler& /*eh*/)
    {
        //Implementation detail...
    }

private:
    //Implementation detail...
};

//This class drives the RGB LED.
class rgb_led
{
public:
    enum class color
    {
        off,
        white,
        red,
        green,
        blue
    };

    color get_color() const
    {
        return color_;
    }

    void set_color(const color c)
    {
        color_ = c;
        //Implementation detail...
    }

private:
    color color_ = color::off;
    //Implementation detail...
};

/*
An instance of this class is shared by all the states, actions and guards of the
state machine.
*/
struct context
{
    rgb_led led;
};

/*
States are represented by classes.
*/
namespace states
{
    /*
    A state class must be constructible with one of the following expressions:
        auto state = state_type{machine, context};
        auto state = state_type{context};
        auto state = state_type{};
    */
    struct off
    {
        /*
        A state class must define a conf variable.
        */
        static constexpr auto conf = maki::default_state_conf
            /*
            With this option, we require the state machine to call an on_entry()
            function whenever it enters our state.
            One of these expressions must be valid:
                state.on_entry(event);
                state.on_entry();
            Where `event` is the event that caused the state transition.
            */
            .enable_on_entry()

            /*
            Here, we require the state machine to call an on_event() function
            whenever it processed an event while our state is active. We also
            indicate that we want it to do so only when the type of the said
            event is button::push_event.
            This expression must be valid:
                state.on_event(event);
            */
            .enable_on_event_for<button::push_event>()

            /*
            Finally, we want the state machine to call on_exit() whenever it
            exits our state.
            One of these expressions must be valid:
                state.on_exit(event);
                state.on_exit();
            Where `event` is the event that caused the state transition.
            */
            .enable_on_exit()
        ;

        void on_entry(const button::push_event& event)
        {
            std::cout << "Turned off after a ";
            std::cout << event.duration_ms << " millisecond push\n";
        }

        void on_entry(const maki::events::start& /*event*/)
        {
            std::cout << "Started state machine\n";
        }

        void on_event(const button::push_event& event)
        {
            std::cout << "Received a ";
            std::cout << event.duration_ms;
            std::cout << " millisecond push in off state\n";
        }

        void on_exit()
        {
            std::cout << "Turned on\n";
        }

        context& ctx;
    };

    /*
    These are minimal valid state classes.
    */
    struct emitting_white { static constexpr auto conf = maki::default_state_conf; };
    struct emitting_red { static constexpr auto conf = maki::default_state_conf; };
    struct emitting_green { static constexpr auto conf = maki::default_state_conf; };
    struct emitting_blue { static constexpr auto conf = maki::default_state_conf; };
}

/*
An action is a function (or lambda) called when a state transition is performed.
*/
namespace actions
{
    /*
    One of the following expressions must be valid:
        action(machine, context, event);
        action(context, event);
        action(context);
    */
    void turn_light_off(context& ctx)
    {
        ctx.led.set_color(rgb_led::color::off);
    }

    //We can of course factorize with a template.
    template<auto Color>
    void turn_light_tpl(context& ctx)
    {
        ctx.led.set_color(Color);
    }
    constexpr auto turn_light_white = turn_light_tpl<rgb_led::color::white>;
    constexpr auto turn_light_red   = turn_light_tpl<rgb_led::color::red>;
    constexpr auto turn_light_green = turn_light_tpl<rgb_led::color::green>;
    constexpr auto turn_light_blue  = turn_light_tpl<rgb_led::color::blue>;
}

/*
A guard is a function (or lambda) called to check that a state transition can
be performed.
*/
namespace guards
{
    /*
    One of the following expressions must be valid:
        guard(machine, context, event);
        guard(context, event);
        guard(context);
    */
    bool is_long_push(context& /*ctx*/, const button::push_event& event)
    {
        return event.duration_ms > 1000;
    }

    //We can use maki::guard and boolean operators to compose guards.
    constexpr auto is_short_push = !maki::guard_c<is_long_push>;
}

using namespace states;
using namespace actions;
using namespace guards;
using button_push = button::push_event;
using maki::any_but;

/*
This is the transition table. This is where we define the actions that must be
executed depending on the active state and the event we receive.
Basically, whenever maki::machine::process_event() is called, Maki iterates
over the transitions of this table until it finds a match, i.e. when:
- 'source_state' is the currently active state (or is maki::any);
- 'event' is the type of the processed event;
- and the 'guard' returns true (or is void).
When a match is found, Maki:
- exits 'source_state';
- marks 'target_state' as the new active state;
- executes the 'action';
- enters 'target_state'.
The initial active state of the state machine is the first state encountered in
the transition table ('off', is our case).
*/
constexpr auto transition_table = maki::empty_transition_table
    //     source_state,   event,       target_state,   action,           guard
    .add_c<off,            button_push, emitting_white, turn_light_white>
    .add_c<emitting_white, button_push, emitting_red,   turn_light_red,   is_short_push>
    .add_c<emitting_red,   button_push, emitting_green, turn_light_green, is_short_push>
    .add_c<emitting_green, button_push, emitting_blue,  turn_light_blue,  is_short_push>
    .add_c<emitting_blue,  button_push, emitting_white, turn_light_white, is_short_push>
    .add_c<any_but<off>,   button_push, off,            turn_light_off,   is_long_push>
;

/*
We have to define this struct to define our state machine. Here we just specify
the transition table, but we can put many options in it.
*/
struct machine_def
{
    static constexpr auto conf = maki::default_machine_conf
        .set_transition_tables(transition_table)
        .set_context_type<context>()
    ;
};

/*
We finally have our state machine.
Note that we can pass a configuration struct as second template argument to fine
tune the behavior of our state machine.
*/
using machine_t = maki::machine<machine_def>;

int main()
{
    /*
    When we instantiate the state machine, we also instantiate:
    - a context;
    - the states mentionned in the transition table.
    Note that the states are instantiated once and for all: no construction or
    destruction happens during state transitions.
    */
    auto machine = machine_t{};
    auto& ctx = machine.context();

#if TESTING
    auto simulate_push = [&](const int duration_ms)
    {
        std::cout << "Button push (" << duration_ms << " ms)\n";
        machine.process_event(button::push_event{duration_ms});
    };

    auto check = [](const bool b)
    {
        if(!b)
        {
            std::cerr << "NOK\n";
            std::cerr << "Test failed\n";
            exit(1);
        }

        std::cout << "OK\n";
    };

    check(machine.is_active_state<states::off>());
    check(ctx.led.get_color() == rgb_led::color::off);

    simulate_push(200);
    check(machine.is_active_state<states::emitting_white>());
    check(ctx.led.get_color() == rgb_led::color::white);

    simulate_push(200);
    check(machine.is_active_state<states::emitting_red>());
    check(ctx.led.get_color() == rgb_led::color::red);

    simulate_push(200);
    check(machine.is_active_state<states::emitting_green>());
    check(ctx.led.get_color() == rgb_led::color::green);

    simulate_push(200);
    check(machine.is_active_state<states::emitting_blue>());
    check(ctx.led.get_color() == rgb_led::color::blue);

    simulate_push(200);
    check(machine.is_active_state<states::emitting_white>());
    check(ctx.led.get_color() == rgb_led::color::white);

    simulate_push(1500);
    check(machine.is_active_state<states::off>());
    check(ctx.led.get_color() == rgb_led::color::off);

    std::cout << "Test succeeded\n";

    return 0;
#else
    auto btn = button
    {
        [&](const auto& event)
        {
            machine.process_event(event);
        }
    };

    while(true){}
#endif
}
```

## Acknowledgements
Maki is greatly inspired by Boost.MSM. Actually, Maki was born because Boost.MSM was too slow to build large state machines (which is expected for a library that has been written in a time when variadic templates weren't supported by the language). Thank you Christophe Henry for your work.
