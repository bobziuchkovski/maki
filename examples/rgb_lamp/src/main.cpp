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
        See guards::is_long_push later in the code to see how the event can be
        accessed.
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
    An action class is required to implement the operator()() function described
    below.
    Also, just like state classes, action classes must be constructible with a
    reference to the context.
    */
    struct turn_light_white
    {
        void operator()(const fgfsm::any_cref& /*event*/)
        {
            ctx.led.set_color(rgb_led::color::white);
        }

        context& ctx;
    };

    /*
    We can also define an action with a function, but it must be wrapped into
    an fgfsm::fn to turn it into a class.
    */
    void turn_light_red(context& ctx, const fgfsm::any_cref& /*event*/)
    {
        ctx.led.set_color(rgb_led::color::red);
    }

    void turn_light_green(context& ctx, const fgfsm::any_cref& /*event*/)
    {
        ctx.led.set_color(rgb_led::color::green);
    }

    void turn_light_blue(context& ctx, const fgfsm::any_cref& /*event*/)
    {
        ctx.led.set_color(rgb_led::color::blue);
    }

    void turn_light_off(context& ctx, const fgfsm::any_cref& /*event*/)
    {
        ctx.led.set_color(rgb_led::color::off);
    }
}

/*
Guards are classes.
*/
namespace guards
{
    /*
    An action class is required to implement the operator()() function described
    below.
    Also, just like state classes, action classes must be constructible with a
    reference to the context.
    */
    struct is_long_push
    {
        bool operator()(const fgfsm::any_cref& event)
        {
            auto long_push = false;

            /*
            An fgfsm::any_cref object is usually accessed through the
            fgfsm::visit() function, which takes the fgfsm::any_cref object,
            followed by any number of unary function objects.
            */
            fgfsm::visit
            (
                event,

                /*
                This function object is called if the actual type of the event
                is button::push_event.
                */
                [&](const button::push_event& event)
                {
                    if(event.duration_ms > 1000)
                        long_push = true;
                },

                /*
                This function object is called if the actual type of the event
                is int... which never happens in our example. This is just here
                as an illustration.
                */
                [&](const int /*event*/)
                {
                    //Won't happen.
                }
            );

            return long_push;
        }

        context& ctx;
    };

    using is_short_push = fgfsm::not_<is_long_push>;
}

//Allow shorter names in transition table
using namespace states;
using namespace actions;
using namespace guards;
using push_event = button::push_event;

using transition_table = fgfsm::transition_table
<
    //         start state,    event,      target state,   action,                      guard
    fgfsm::row<off,            push_event, emitting_white, turn_light_white>,
    fgfsm::row<emitting_white, push_event, emitting_red,   fgfsm::fn<turn_light_red>,   is_short_push>,
    fgfsm::row<emitting_red,   push_event, emitting_green, fgfsm::fn<turn_light_green>, is_short_push>,
    fgfsm::row<emitting_green, push_event, emitting_blue,  fgfsm::fn<turn_light_blue>,  is_short_push>,
    fgfsm::row<emitting_blue,  push_event, emitting_white, turn_light_white,            is_short_push>,
    fgfsm::row<fgfsm::any,     push_event, off,            fgfsm::fn<turn_light_off>,   is_long_push>
>;

struct fsm_configuration: fgfsm::fsm_configuration
{
    using transition_table = ::transition_table;
};

using fsm = fgfsm::fsm<fsm_configuration>;

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
