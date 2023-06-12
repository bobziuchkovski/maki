# Guard {#concepts-guard}

## Definition

A guard is associated with a transition. It is a condition for the transition to occur.

On a UML state diagram, it is represented between square brackets in the transition label:

@startuml{guard.png} "A guard"
hide empty description
[*] --> state0
state0 -> state1 : event / action [guard]
@enduml

In the above example, the state machine transitions from `state0` to `state1` and executes `action` *provided `guard` is true*. If `guard` is false, nothing happens.

Guards can be based on system-wide data (e.g. current temperature) or event data (e.g. button press duration).

## When to use a guard

You need guards to express the equivalent of `if`/`else`, `switch`, `for` or `while` statements in your state machine.

For example, let's modelize the state machine of a 3-speed fan that remembers its last speed and automatically sets it at startup. Its components include:

* an internal memory (to save last set speed);
* a minus button, to lower the speed;
* a plus button, to raise the speed.

The state machine defines:

* 4 states:
    * `reading memory`, the initial state, in which the fan reads the last saved speed and waits for the `memory read` event;
    * `spinning low`;
    * `spinning med`;
    * `spinning high`;
* 3 events:
    * `memory read`, emitted when the memory is read, which carries the speed value;
    * `minus button press`;
    * `plus button press`;
* 3 actions:
    * `set speed low`;
    * `set speed med`;
    * `set speed high`.

What about transitions? Transitions between `spinning *` states should be obvious: we just use the button press events. But how do we get from the `reading memory` state to these `spinning *` states?

This is where we need guards. Starting from `reading memory`, at `memory read`:

* if the guard `memory speed = low`, execute the action `set speed low` and transition to the `spinning low` state;
* if the guard `memory speed = med`, execute the action `set speed med` and transition to the `spinning med` state;
* if the guard `memory speed = high`, execute the action `set speed high` and transition to the `spinning high` state.

Here is the state machine diagram of our fan:

@startuml{fan.png} "Fan"
hide empty description

state "reading memory" as reading_memory
state "spinning low" as spinning_low
state "spinning med" as spinning_med
state "spinning high" as spinning_high

reading_memory --> spinning_low : memory read / set speed low [memory speed = low]
reading_memory --> spinning_med : memory read / set speed med [memory speed = med]
reading_memory --> spinning_high : memory read / set speed high [memory speed = high]

spinning_low -> spinning_med : plus button press / set speed med
spinning_med -> spinning_high : plus button press / set speed high
spinning_med -> spinning_low : minus button press / set speed low
spinning_high -> spinning_med : minus button press / set speed med
@enduml

## How to use guards within AweSM

Within AweSM, guards are simply functions, preferably without side effect, that return a `bool`. They can express conditions based on the arguments that the @ref sm gives to them:

* a reference to the @ref sm itself;
* a reference to the context;
* a reference to the event that could lead to the transition;

Guards are free to take (or not) any of these arguments; the @ref sm tries the following signatures, in this order of priority:

@code
bool(sm_type& mach, context_type& ctx, const event_type& evt);
bool(context_type& ctx, const event_type& evt);
bool(context_type& ctx);
bool();
@endcode

As allowed by C++, the @ref sm also transparently accepts `constexpr` callables.

Once you've defined your guard, you just have to pass the function name as the fifth argument of the transition of your choice, like so:

@code
using sm_transition_table_t = awesm::transition_table
    ::add<source_state_type, event_type, target_state_type, action, guard>
    //...
;
@endcode

### Example

Let's implement the state machine of our 3-speed fan.

Our event types are the following. Notice that `memory_read` carries the fan speed that is read from the fan internal memory:

@snippet fan/src/main.cpp events-and-datatypes

Our guards need to check the value of the speed carried by the `memory_read` event. Thus, we must use one of the signatures that take the event. As there's no signature that only takes the event, let's use the second one and just ignore the `context_type` parameter. These are our guards:

@snippet fan/src/main.cpp guards

Finally, we pass each guard to the transition they're associated with. Once again, our transition table is a direct representation of the state diagram:

@snippet fan/src/main.cpp transition-table

Here is a test program for our state machine:

@snippet fan/src/main.cpp all

The output of this program is the following:

@include fan/expected-output.txt