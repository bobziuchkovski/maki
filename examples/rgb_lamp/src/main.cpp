//Copyright Florian Goujeon 2021 - 2022.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/fgfsm

#include <fgfsm.hpp>
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
            //Low-level stuff...
        }

    private:
        //Low-level stuff...
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
            //Low-level stuff...
        }

    private:
        color color_ = color::off;
        //Low-level stuff...
};

/*
An instance of this class is shared by all the states, actions and guards of the
FSM.
*/
struct context
{
    rgb_led led;
};

/*
States are classes.
*/
namespace states
{
    /*
    A state class is required to implement the on_entry(), on_event() and
    on_exit() functions described below.
    Also, it must be constructible with a reference to the context. Since FGFSM
    instantiates its states using aggregate initialization, an explicit
    constructor isn't necessary. Declaring a public member variable like below
    is enough.
    */
    struct off
    {
        /*
        This function is called whenever the FSM enters this state.
        The event that caused the state transition is given as argument. The
        event is wrapped into an fgfsm::any_cref object.
        fgfsm::any_cref is a std::any-like container that stores a reference
        of an object. FGFSM provides visitation functions to access
        fgfsm::any_cref objects is a safe and concise way.
        */
        void on_entry(const fgfsm::any_cref& /*event*/)
        {
        }

        /*
        This function is called whenever fgfsm::fsm::process_event() is called,
        provided this state is active.
        */
        void on_event(const fgfsm::any_cref& /*event*/)
        {
        }

        /*
        This function is called whenever the FSM exits this state.
        */
        void on_exit(const fgfsm::any_cref& /*event*/)
        {
        }

        context& ctx;
    };

    /*
    In this example, since we don't need entry/exit actions or internal
    transitions, we can declare our states with FGFSM_SIMPLE_STATE.
    */
    FGFSM_SIMPLE_STATE(emitting_white);
    FGFSM_SIMPLE_STATE(emitting_red);
    FGFSM_SIMPLE_STATE(emitting_green);
    FGFSM_SIMPLE_STATE(emitting_blue);
}

/*
Actions are classes.
*/
namespace actions
{
    /*
    An action class is required to implement the execute() function described
    below.
    Also, just like state classes, action classes must be constructible with a
    reference to the context.
    */
    struct turn_light_white
    {
        void execute(const fgfsm::any_cref& /*event*/)
        {
            ctx.led.set_color(rgb_led::color::white);
        }

        context& ctx;
    };

    struct turn_light_red
    {
        void execute(const fgfsm::any_cref& /*event*/)
        {
            ctx.led.set_color(rgb_led::color::red);
        }

        context& ctx;
    };

    struct turn_light_green
    {
        void execute(const fgfsm::any_cref& /*event*/)
        {
            ctx.led.set_color(rgb_led::color::green);
        }

        context& ctx;
    };

    struct turn_light_blue
    {
        void execute(const fgfsm::any_cref& /*event*/)
        {
            ctx.led.set_color(rgb_led::color::blue);
        }

        context& ctx;
    };

    struct turn_light_off
    {
        void execute(const fgfsm::any_cref& /*event*/)
        {
            ctx.led.set_color(rgb_led::color::off);
        }

        context& ctx;
    };
}

/*
Guards are classes.
*/
namespace guards
{
    /*
    A guard class is required to implement the check() function described
    below.
    Also, just like state classes, guard classes must be constructible with a
    reference to the context.
    */
    struct is_long_push
    {
        bool check(const fgfsm::any_cref& event)
        {
            /*
            fgfsm::any_cref objects are usually accessed through visitation
            functions like this one.
            */
            return fgfsm::visit_or_false
            (
                event,
                [](const button::push_event& event)
                {
                    return event.duration_ms > 1000;
                }
            );
        }

        context& ctx;
    };

    //You can use guard operators to combine your guards.
    using is_short_push = fgfsm::not_<is_long_push>;
}

//Allow shorter names in transition table
using namespace states;
using namespace actions;
using namespace guards;
using push_event = button::push_event;

using transition_table = fgfsm::transition_table
<
    //         start state,    event,      target state,   action,            guard
    fgfsm::row<off,            push_event, emitting_white, turn_light_white>,
    fgfsm::row<emitting_white, push_event, emitting_red,   turn_light_red,    is_short_push>,
    fgfsm::row<emitting_red,   push_event, emitting_green, turn_light_green,  is_short_push>,
    fgfsm::row<emitting_green, push_event, emitting_blue,  turn_light_blue,   is_short_push>,
    fgfsm::row<emitting_blue,  push_event, emitting_white, turn_light_white,  is_short_push>,
    fgfsm::row<fgfsm::any,     push_event, off,            turn_light_off,    is_long_push>
>;

using fsm = fgfsm::fsm<transition_table>;

int main()
{
    auto ctx = context{};
    auto sm = fsm{ctx};

#if TESTING
    auto simulate_push = [&](const int duration_ms)
    {
        sm.process_event(button::push_event{duration_ms});
    };

    auto check = [](const bool b)
    {
        if(!b)
        {
            std::cerr << "Test failed\n";
            exit(1);
        }
    };

    check(sm.is_active_state<states::off>());
    check(ctx.led.get_color() == rgb_led::color::off);

    simulate_push(200);
    check(sm.is_active_state<states::emitting_white>());
    check(ctx.led.get_color() == rgb_led::color::white);

    simulate_push(200);
    check(sm.is_active_state<states::emitting_red>());
    check(ctx.led.get_color() == rgb_led::color::red);

    simulate_push(200);
    check(sm.is_active_state<states::emitting_green>());
    check(ctx.led.get_color() == rgb_led::color::green);

    simulate_push(200);
    check(sm.is_active_state<states::emitting_blue>());
    check(ctx.led.get_color() == rgb_led::color::blue);

    simulate_push(200);
    check(sm.is_active_state<states::emitting_white>());
    check(ctx.led.get_color() == rgb_led::color::white);

    simulate_push(1500);
    check(sm.is_active_state<states::off>());
    check(ctx.led.get_color() == rgb_led::color::off);

    std::cout << "Test succeeded\n";

    return 0;
#else
    auto btn = button
    {
        [&](const auto& event)
        {
            sm.process_event(event);
        }
    };

    while(true){}
#endif
}
