# Compile time finite state machine

The `fsm` provides an easy way to manage finite state machines, almost without overhead.

This library requires C++20 and is based on meson build system.

To run unit tests:
```bash
$ meson build
$ ninja -C build test
```

## Declaration 

Every state and every handled event is simply a type, with only two mandatory requirements:
- a state **must** provide a public alias, `transitions`, that is a `ctfsm::type_map` of `event`s and their relative target states, basically the edges list of a graph, events must be unique;
- a state **must** be defaultly constructible.

And can, **optionally**: 
- expose a `void on_enter()` function, that will be called every time the fsm enters in this state; optionally it can receive the event instance: `void on_enter(event& e)`;
- expose a `void on_exit()` function, that will be called every time the fsm exits from this state, optionally it can receive the event instance: `void on_exit(event& e)`; 

```cpp
struct on;
struct off;

struct switch_toggle {};
struct blackout {}

struct on
{
	using transitions = ctfsm::type_map<
		std::pair<switch_toggle, off>,
		std::pair<blackout, off>
	>;

	void on_exit(blackout& event)
	{
		...
	}

	void on_exit() 
	{
		...
	}
};

struct off
{
	using transitions = ctfsm::type_map<
		std::pair<switch_toggle, on>
	>;

	void on_enter()
	{
		...
	}
};
```

In this case: 
- `on` handles the event triggered when changing state from `on` with the `blackout` event with a different handler than other exit events; 
- `off` handles every `on_enter` event;

While `switch_toggle` and `blackout` are valid events, it can be useful to include data and event handlers in them: 

```cpp
struct switch_toggle
{
	int data1; 
	std::string data2;

	void on_transit()
	{
		...
	}
};
```

`switch_toggle` provides a payload that will be accessible to states in their event handlers. The on_transit event handler will be triggered every time the state machine handles an event of this type.

Finally, to build an `fsm` from these states, doing: 

```cpp
asters::fsm<on> state_machine; 
```

is enough, all reachable states will be deduced from the provided initial state and `on` will be the starting state.

## Handling events

Given an event instance, 

```cpp
event t; 
```

to trigger a state change in the fsm: 

```cpp
state_machine.handle_event(t); 
```

or 

```cpp
state_machine(t); 
```

This call will trigger, in this order:

1. `current_state.on_exit(event&)` if available, `.on_exit()` otherwise;
2. `t.on_transit()` if available;
3. `next_state.on_enter(event&)` if available, `.on_enter()` otherwise;

If the event is default initializable, it is possible to: 

```cpp
state_machine.handle_event<event>(); 
```