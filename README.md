# Compile time finite state machine

![Alla pugna!](https://img.shields.io/badge/ALLA-PUGNA-F70808?style=for-the-badge)

The `fsm` provides an easy way to manage finite state machines, almost without overhead.

This library requires C++20 and is based on meson build system.

To run unit tests:
```bash
$ meson build
$ ninja -C build test
```

Or, to be more concise:

``` bash
$ make test
```

To generate the single-include header:
```bash
$ make single_include
```

## Usage
### Declaration

Every state and every handled event is simply a type, with only two mandatory requirements:
- a state **must** provide a public alias, `transitions`, that is a `ctfsm::type_map` of `event`s and their relative target states, basically the edges list of a graph, events must be unique; a transition **can** be:
  - a `ctfsm::transition<E, T>`, where `E` is an event and `T` is the target state, `T` will be considered reachable from this state;
  - a `ctfsm::nested<E, I>`, where `E` is an event and `I` is the initial state of a nested FSM;
  - a `ctfsm::exit_transition<E>`, where `E` is an exit event for this FSM, this kind of transition is allowed **only** inside a nested state machine (this is enforced by a static assertion);
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
	using transitions = ctfsm::transition_map<
		ctfsm::transition<switch_toggle, off>,
		ctfsm::transition<blackout, off>
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
	using transitions = ctfsm::transition_map<
		ctfsm::transition<switch_toggle, on>
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
ctfsm::fsm<on> state_machine;
```

is enough, all reachable states will be deduced from the provided initial state and `on` will be the starting state.

### Nested state machines

A `transition_map` can contain a mixed list of `ctfm::transition`, as shown in the previous section, and `ctfsm::nested`.

A nested FSM can be specified using:

``` cpp
ctfsm::nested<Event, Init>
```

Where:
- `Event` is the specific event that triggers the parent FSM to transition into this nested FSM;
- `Init` is the initial state of the nested FSM;

The list of exit events will be extracted from the nested FSM: when an exit events is accepted by the nested FSM, the parent state becomes the current state; if that event is accepted by the parent state too, it is automatically forwarded to it via an `handle_event`.

When an event causes the nested FSM to reach one of its designated exit states, the following sequence of actions occurs automatically:
- `handle_event` for the event is invoked on the **nested FSM**;
- `reset` is invoked on the nested FSM, preparing it for future use;
- The parent FSM's current state reverts to the state that owns the nested FSM;
- `handle_event` with the event is invoked on the current state, this allows the parent FSM to react to the nested FSM's completion.

### Lifetimes

Every state is default-constructed when the fsm is constructed.
Their duration is exactly the same of the fsm that owns them.

### Handling events

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

### Talking with state instances

Sometimes, it is necessary to invoke a method on the current state to update its data members or apply specific application logic. The invoke_on_current function is provided to facilitate this type of operation.

```cpp
fsm.invoke_on_current([](auto& current_state, auto& fsm) {
  current_state.work(fsm);
});
```

This example implies that every state of `fsm` provides a function `.work` that takes a reference to the fsm itself.
This allows to update the fsm state inside an `invoke_on_current` execution. The operation sequence in this case is:
1. `invoke_on_current` is invoked;
2. inside it, an `fsm.handle_event` is triggered;
3. `on_exit` of the current state is invoked, if present;
4. `on_transit` of the event is invoked, if present;
5. `on_enter` of the next state is invoked, if present;
6. `invoke_on_current` execution continues.

#### Transition validation

When events are processed directly inside states, `ctfsm` can make strong assumptions at compile time about the current state: this allows a transparent validation of **every** transition that happens inside an `invoke_on_current` call.

If an invalid transition is detected, a static assertion with message `"Transition not admissible"` fails.

`handle_event` in this context are no more `[[nodiscard]]` and their return value will always be `true` because they cannot fail.

## API documentation

### FSM
| Function      | Description   |
|---            |---            |
| `constexpr bool handle_event(event)` | The event is delivered to states and the current state is updated if a valid transaction for this event is found. If this event cannot be handled by the current state, the function returns `false`.|
| `constexpr bool handle_event<event_t>()` | The event of type `event_t` is default-constructed and delivered to states and the current state is updated if a valid transaction for this event is found. If this event cannot be handled by the current state, the function returns `false`.|
| `constexpr const id_type& get_current_state_id() const noexcept` | The ID of the current state is returned: if every state has a member named `id` of the same type, the `id` of the current state is returned, otherwise `typeid(current_state_t).name()` is returned. |
| `constexpr bool is_current_state<T>() const noexcept` | Returns `true` if `T` is the type of the current state. |
| `constexpr auto invoke_on_current(auto lambda) noexcept` | Lambda must be invocable with a reference to the current state instance and the fsm itself. The value returned by lambda is forwarded to the caller. |

### State
- A state class must provide a `transitions` alias, a `ctfs::type_map` composed by `ctfsm::transition`s, `ctfsm::exit_transition`s or `ctfsm::nested`s. If no `transitions` alias is provided, the state will be a sink state.
- Each state can provide a publicly accessible `id` field. The library will use this field only if every state of the FSM provides an `id` field of the same type.
- A state can handle `exit` events, which are triggered when the FSM changes state while it is the current state. An unlimited number of `void on_exit(E& event)` overloads and a `void on_exit()` method can be provided. The no-argument handler will be invoked only for events that do not have a specific overload. A state without `exit` handlers is also completely valid.
- A state can handle `enter` events, which are triggered when the FSM changes state while it is the target state. An unlimited number of `void on_enter(E& event)` overloads and a `void on_enter()` method can be provided. The no-argument handler will be invoked only for events that do not have a specific overload. A state without `enter` handlers is also completely valid.
