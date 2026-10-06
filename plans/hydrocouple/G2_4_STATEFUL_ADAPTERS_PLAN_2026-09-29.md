# G2.4: gradients through stateful adapters (plan)

**Status:** implemented 2026-10-02, inside the still-unreleased ABI 4 as
decided on 2026-09-29; verified in a Linux container (GCC and Clang,
including the LibTorch suite against a CPU-loaded torch 2.14.1 wheel), not
yet on macOS. Hand-off: `plans/sdk/G2_4_HANDOFF_2026-10-02.md`. Decisions
taken while implementing, recorded in "Outcome" at the end.
Decision (2026-09-29):
build on that round and put the adapter additions into the same unreleased
ABI 4, so there is one breaking change instead of two.
**Parent plan:** `plans/hydrocouple/DIFFERENTIABLE_INTERFACE_ALTERNATIVES_2026-09-28.md` (§7 G2)
**Repos:** HydroCouple (interface and Python mirror), HydroCoupleSDK (engine and adapters)

## Why

The reverse engine refuses any connection through an adapter whose values
depend on its earlier refreshes. The plan's G2 example puts such an
adapter (temporal interpolation) between the two components. This is the
last G2 item that is not implemented.

## What exists, and which adapter this phase covers

- **`RelaxationAdaptedOutput`** computes y_k = (1 − λ) y_{k−1} + λ x_k.
  - `TimeSteppedWorkflow` refreshes it exactly once per step on an acyclic
    connection (inputs read with `getValuesInto`, never `updateValues`).
    So in a stepped composition it is a first-order lag.
  - Its state is y_{k−1}, of fixed shape.
  - It fits the lockstep engine as it stands. **It is this phase's target.**
- **`TemporalInterpolationAdaptedOutput`** is deferred to a multi-rate
  phase, for four reasons:
  1. Its adaptee must be a time-series item whose history grows every step.
  2. Its value is computed at a query time that nothing in
     `TimeSteppedWorkflow` sets (only tests call `setQueryTime`).
  3. Its state, the sample history, changes shape every step.
  4. Differentiating it would need a provider whose time-series history is
     differentiable state, a workflow that drives query times, and state
     items that change shape across a step. None of these exist.

  The G2 example is therefore met with relaxation, the stateful adapter
  that the stepped workflow actually drives. The plan's status table
  should say so.

## Interface additions (inside ABI 4, std-only)

1. **`IDifferentiableAdaptedOutput::differentiableStates()`** returns
   `std::vector<IComponentDataItem *>`. Stateless adapters return `{}`.
   - vjp seeds may include `StateAfter`, and results may include
     `StateBefore`. jvp is the mirror. This is the same shape as
     `IDifferentiableModelComponent`.
   - It replaces the "Stateless adapters only" paragraph.
   - Rule: a state item keeps its shape across a refresh.
2. **`ICheckpointableAdaptedOutput : public virtual IAdaptedOutput`** has
   `saveState`, `restoreState` and `releaseState`, with the component
   interface's semantics (restore makes the adapter behave as if it had
   refreshed its way there; the token is dead after release).
   - It is required of a differentiable adapter with states whenever more
     than one step must be replayed.
3. **Check against the round.** The round adds `IModelComponent::states()`,
   so decide whether adapters get the same `states()` for symmetry. The
   checkpoint captures at least the adapter's states.
4. **Python and gates.** Add Python ABC mirrors. C++ gates check that the
   members exist with the right signatures. The include allowlist is
   unchanged.

## SDK engine (`DifferentiableWorkflow`)

- **Routing rewrite.** Cotangents accumulate per `IOutput`, whether an
  adapter or a root output.
  - In reverse schedule order, before a component's own vjp, its adapter
    trees are processed leaves first.
  - Each adapter's vjp runs once per step, seeded with Output plus
    StateAfter. Its adaptee cotangent accumulates on its parent.
  - This replaces the per-path chain walk.
- **State carry and checkpoints.**
  - State carry is keyed by owner (component or adapter).
  - `saveAll`, `restoreAll` and `releaseAll` also cover the checkpointable
    adapters hanging off managed components' outputs.
  - Rewind, the ladder and bisection then include adapters with no further
    change.
- **Refusals, by name.**
  - A stateful differentiable adapter without `ICheckpointableAdaptedOutput`
    when more than one step is recorded.
  - Stateful adapters inside a cyclic group (already refused as a loop).
- **Gradients.** Adapter initial-state gradients are reported:
  `StateGradient` gains an adapter owner. The state before step 1 is what
  `initialize()` pulled from the provider's prepared output, which the
  engine does not differentiate. The caller chains that gradient, and a
  gate does so analytically.
- **`RelaxationAdaptedOutput`** implements both interfaces.
  - Adjoint: with t = ȳ_out + ȳ_state, the results are x̄ = λ t,
    ȳ_{k−1} = (1 − λ) t and λ̄ = Σ t (x_k − y_{k−1}).
  - The first refresh after `initialize()` passes x through.
  - The token holds the values and the history flag.
  - Read λ at each refresh instead of latching it at `initialize()`. Then
    `TorchComposition` no longer has to re-initialize it, which would reset
    its state.

## Gates and falsifiers

- **Relaxation adjoint:** vjp vs FD, and adjointness.
- **Composition A → relax → B, A → C:** every gradient vs FD of full
  forward runs, including λ, with the adapter's initial-state gradient
  chained through `prepare()`.
- **Ladder:** gradients identical to EveryStep.
- **Tokens:** release counts include adapter tokens, and none is released
  twice.
- **Refusals:** a stateful adapter without checkpointing is refused by name.
- **`TorchComposition`:** a training loop recovers a known λ.
- **Falsifiers:** a row for each: state carry dropped, adapter vjp called
  per path instead of once, adapter not restored, adapter token not
  released, and λ̄ at the wrong linearization point.

## Order of work, once the round is committed

0. Port the SDK to ABI 4 (the round's own follow-up).
1. Interface additions.
2. Engine routing rewrite, with the existing 29 + 20 gates green.
3. Relaxation adjoint and checkpoint.
4. Gates, falsifiers, and the Mac hand-off in `plans/sdk/`.

## Outcome (2026-10-02)

- **Item 3 decided: adapters get `states()`.** `IAdaptedOutput::states()`
  (pure virtual, empty for a stateless adapter) mirrors
  `IModelComponent::states()`, and `differentiableStates()` is a subset of
  it. It has a consumer from day one: an engine that replays must return
  every stateful adapter to the earlier step, *whether or not a derivative
  crosses it*, and without `states()` it cannot tell a stateful adapter
  from a stateless one. The engine refuses replay through a stateful adapter
  that is not checkpointable, on or off the differentiated path.
- `ICheckpointableAdaptedOutput` has the component's three members and
  semantics; adapters have no status, so all three are legal after
  `initialize()`. Adapters have no capability set: their optional interfaces
  are discovered by casting the adapter (now stated under `Capability`), and
  their `bool` + `message` methods report through the message alone (stated
  in the error-channel convention).
- `TemporalInterpolationAdaptedOutput` lists its sample record (a read-only
  `[samples, 2]` item) as its state, and is neither differentiable nor
  checkpointable, as this plan deferred.
- `RelaxationAdaptedOutput` lists itself as its state; its token is the
  state (values, and whether there is a history), so another process can
  restore it. λ is read at every refresh.
- Engine: the routing rewrite as planned; checkpoints carry an adapter token
  per checkpointable adapter, keyed by adapter; a restore refuses up front if
  a stateful adapter would be left out (not checkpointable, or joined after
  the checkpoint). `StateGradient` gained `owner` / `adapter`, and
  `Gradients::findState()` takes any owner.
- `TorchComposition` re-initializes only stateless adapters after writing
  their arguments; a stateful one keeps the state the rewind restored.
- An independent review (a separate agent, not shown the work as it was
  built) found what the gates did not, and each is fixed with a gate of its
  own: an adapter between a provider outside the workflow and a managed
  consumer had lost its argument gradients in the routing rewrite (a
  regression); `TorchComposition` re-initializing a stateless adapter
  refreshed a stateful one below it, so runs drifted -- the linear transform
  now reads its coefficients at every refresh and `TorchComposition`
  initializes no adapter; a component reading its own output through a
  relaxation (a lagged self-coupling) was accepted with a wrong gradient --
  now refused; the relaxation's token threw on a negative count and lost
  infinities/NaNs (now bit patterns, validated); after a restore, stateless
  adapters showed the last replayed step (now refreshed); seeds on a
  non-differentiable adapter were silently zero (now refused); checkpoint
  tokens are matched to adapters by id as well as address.
- Gates: 17 new SDK tests and the two that asserted stateful adapters are
  refused rewritten; 2 new Torch tests; 2 interface tests; 1 binding test
  and the parity test's new ABC. Falsifier rows S1–S20 and L4, with
  E1/E4/E5/E8/T11 rewritten for the new routing.
