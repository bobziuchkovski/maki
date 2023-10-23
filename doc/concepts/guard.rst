Guard
=====

Definition
----------

A guard is associated with a transition. It is a condition for the transition to occur.

On a UML state diagram, it is represented between square brackets in the transition label:

.. uml::

    hide empty description
    [*] --> state0
    state0 -> state1 : event / action [guard]

In the above example, the state machine transitions from ``state0`` to ``state1`` and executes ``action`` *provided* ``guard`` *is true*. If ``guard`` is false, nothing happens.

Guards can be based on system-wide data (e.g. current temperature) or event data (e.g. button press duration).

When to use a guard
-------------------

You need guards to express the equivalent of ``if``/``else``, ``switch``, ``for`` or ``while`` statements in your state machine.

For example, let's modelize the state machine of a 3-speed fan that remembers its last speed and automatically sets it at startup. Its components include:

* an internal memory (to save last set speed);
* a minus button, to lower the speed;
* a plus button, to raise the speed.

The state machine defines:

* 4 states:
    * ``reading memory``, the initial state, in which the fan reads the last saved speed and waits for the ``memory read`` event;
    * ``spinning low``;
    * ``spinning med``;
    * ``spinning high``;
* 3 events:
    * ``memory read``, emitted when the memory is read, which carries the speed value;
    * ``minus button press``;
    * ``plus button press``;
* 3 actions:
    * ``set speed low``;
    * ``set speed med``;
    * ``set speed high``.

What about transitions? Transitions between ``spinning *`` states should be obvious: we just use the button press events. But how do we get from the ``reading memory`` state to these ``spinning *`` states?

This is where we need guards. Starting from ``reading memory``, at ``memory read``:

* if the guard ``memory speed = low``, transition to the ``spinning low`` state;
* if the guard ``memory speed = med``, transition to the ``spinning med`` state;
* if the guard ``memory speed = high``, transition to the ``spinning high`` state.

Here is the state machine diagram of our fan:

.. uml::

    hide empty description

    state "reading memory" as reading_memory

    state "spinning low" as spinning_low
    spinning_low : entry / set speed low

    state "spinning med" as spinning_med
    spinning_med : entry / set speed med

    state "spinning high" as spinning_high
    spinning_high : entry / set speed high

    reading_memory --> spinning_low : memory read [memory speed = low]
    reading_memory --> spinning_med : memory read [memory speed = med]
    reading_memory --> spinning_high : memory read [memory speed = high]

    spinning_low -> spinning_med : plus button press
    spinning_med -> spinning_high : plus button press
    spinning_med -> spinning_low : minus button press
    spinning_high -> spinning_med : minus button press

How to use guards within Maki
-----------------------------

Within Maki, guards are non-member functions, preferably without side effect, that return a ``bool``. Maki accepts the following signatures, in this order of priority:

.. code-block:: cpp

    bool(machine_type& mach, context_type& ctx, const event_type& evt);
    bool(context_type& ctx, const event_type& evt);
    bool(context_type& ctx);
    bool();

Maki also accepts ``constexpr`` callables with the same signatures for ``operator()()``.

Once you've defined your guard, you just have to pass the function name as the fifth argument of the transition of your choice, like so:

.. code-block:: cpp

    using transition_table_t = maki::empty_transition_table
        .add_c<source_state_type, event_type, target_state_type, action, guard>
        //...
    ;

Example
^^^^^^^

Let's implement the state machine of our 3-speed fan.

Our event types are the following. Notice that ``memory_read`` carries the fan speed that is read from the fan internal memory:

.. literalinclude:: /../examples/fan/src/main.cpp
   :language: cpp
   :start-after: //! [events-and-datatypes]
   :end-before: //! [events-and-datatypes]

Our guards need to check the value of the speed carried by the ``memory_read`` event. Thus, we must use one of the signatures that take the event. As there's no signature that only takes the event, let's use the second one and just ignore the ``context_type`` parameter. These are our guards:

.. literalinclude:: /../examples/fan/src/main.cpp
   :language: cpp
   :start-after: //! [guards]
   :end-before: //! [guards]

Finally, we pass each guard to the transition they're associated with. Once again, our transition table is a direct representation of the state diagram:

.. literalinclude:: /../examples/fan/src/main.cpp
   :language: cpp
   :start-after: //! [transition-table]
   :end-before: //! [transition-table]

Here is a test program for our state machine:

.. literalinclude:: /../examples/fan/src/main.cpp
   :language: cpp
   :start-after: //! [all]
   :end-before: //! [all]

The output of this program is the following:

.. literalinclude:: /../examples/fan/expected-output.txt
   :language: none
