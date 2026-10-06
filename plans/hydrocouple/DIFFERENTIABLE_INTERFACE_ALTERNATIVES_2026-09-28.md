# Gradients across the HydroCouple interface — alternatives for review

**Status:** rev 3 — G0, G1, G3-lite and G2.1–G2.4 implemented (see the status table) · **Date:** 2026-09-28 · **Author:** Caleb Buahin
**Scope:** the interface (`HydroCouple`), its Python bindings (`HydroCouple/python`), and the SDK that implements both
**Reads with:** `HydroCouple/docs/python_bindings_strategy.md`, `HydroCouple/python/UPDATE_PLAN.md`, `HydroCoupleSDK/include/hydrocouplesdk/device/devicebackend.h`

**Rev 2 changes.** Adds Alt 8 (*LibTorch as the autograd engine, never as the
interface's type*) and makes it the recommendation in place of the SDK-owned
tape of Alt 3; states the **header-only, dependency-free invariant** the
interface must keep under any alternative (the two invariants below, and
§6); rewrites phases G2–G3
around it. Alt 3's analysis of adapter adjoints and iterative coupling
carries over unchanged, because those problems do not go away when the tape
is Torch's.

**Rev 3: what implementing G0, G1 and G3-lite changed.** Six corrections,
each forced by the code rather than chosen:

1. *The contract needed state (§2 Alt 2, §6).* A VJP over inputs and
   arguments alone cannot carry a gradient through time inside a
   component. The implemented contract lists `differentiableStates()` —
   ordinary data items — and tags every buffer with a `DifferentialRole`
   (`Input`, `Argument`, `Output`, `StateBefore`, `StateAfter`), because the
   same state item sits on both sides of a step. `CotangentSet`/`TangentSet`
   became one aggregate, `DifferentialEntry`, and one view,
   `DifferentialSet = std::span<const DifferentialEntry>`.
2. *Results are written, not accumulated.* Accumulation belongs to
   whoever composes the derivative; a component that accumulates forces
   every caller to zero its buffers first.
3. *The linearization point is the most recent `update()`.* Earlier steps
   are replayed by the orchestrator (restore the checkpoint saved before
   the step, re-supply its state, inputs and arguments, update). A
   component without `Checkpointing` can have only its last step
   differentiated, and the overlays say so.
4. *There is no `__dlpack__` on a data item (§5.2).* The data plane is
   copy-into: `getValuesInto` fills a caller's buffer, and an item never
   exposes its own storage. So DLPack enters where buffers do — the
   data-plane methods accept any DLPack producer, and a C++ engine's device
   descriptor reaches a Python item as a `BufferView` that produces DLPack.
5. *JAX arrays are refused as destinations.* JAX exports writable-looking
   capsules for arrays it treats as immutable (and may share).
6. *`IDifferentiableAdaptedOutput` moves to G2*, where the SDK's adapters
   give it something to be tested against.

| Phase | State | Evidence |
|---|---|---|
| G0 DLPack, host and device, both directions | **implemented**; verified on Linux and macOS (clang/libc++, CPython 3.13); a real Metal tensor reaches C++ as `MemorySpace::Device` with its own pointer; CUDA rows and the stream question pending a CUDA machine | 53 gates in `python/tests/test_dlpack.py` (3 device rows skip without CUDA) |
| G1 the contract | **implemented** in `hydrocouple.h`, std-only | C++ `DifferentialTest.*`; one-step VJP/JVP vs finite differences, adjointness `<u,Jv> = <Jᵀu,v>`, write-not-accumulate |
| G3-lite torch and JAX overlays over C++ components | **implemented** | torch net → C++ → C++ → torch head, six steps: every gradient equals FD; JAX equals PyTorch to 1e-12; `jit` equals eager |
| G2.1 the SDK reverse engine, Torch-free: `IDifferentiableAdaptedOutput` (interface), the linear-transform adapter's adjoint, `DifferentiableWorkflow` (checkpoint, replay, route through adapters, sum at fan-out, carry state, restore) | **implemented** | A → unit-conversion adapter → B, A → C, eight steps, driven from C++: all 17 gradients (three components' parameters, the adapter's multiplier and offset, two initial states) equal FD of full forward runs; 16/16 falsifiers caught. Hand-off: `HydroCouple/plans/sdk/G2_1_HANDOFF_2026-09-28.md` |
| G2.2 logarithmic checkpointing: an online dyadic ladder during forward, recursive bisection during backward | **implemented** | N = 10/100/1000: 5/8/11 checkpoints held (vs N), peak ≤ held + ⌈log₂N⌉ + 1, replays ≤ N⌈log₂N⌉ (5 028 at N = 1000); gradients bit-identical to every-step; 23/23 falsifiers. Hand-off: `plans/sdk/G2_2_HANDOFF_2026-09-29.md`. Exposed a gap: `ICheckpointableModelComponent` had no way to give a saved state back, so a file-backed token leaked disk as rungs were dropped |
| releaseState: `ICheckpointableModelComponent::releaseState` (interface, ABI 2 → 3), bindings, overlays, SDK engine | **implemented** | every checkpoint released exactly once (rungs dropped, temporaries, final state, clear/finish); SDK 29/29 and G0/G1 24/24 falsifiers. **Mac-verified** (146 C++, 185 + 3 skipped Python, 24/24; record in `HydroCouple/verification/g0g1/`, fada009). Hand-off: `plans/hydrocouple/RELEASE_STATE_HANDOFF_2026-09-29.md` |
| G2.3 LibTorch layer (`HYDROCOUPLESDK_WITH_TORCH`, separate `HydroCoupleSDKTorch` library): `AtenModelComponent` (physics in ATen ops; vjp from `torch::autograd::grad`, jvp by double backward, state-in-token checkpoints) and `TorchComposition` (a recorded run as one autograd node over the G2.1 engine); `DifferentiableWorkflow::rewind()` | **implemented** | ATen reservoir's vjp equals the hand-written adjoint (1e-13) and FD; all-ATen composition gives the hand-written 17 gradients; mixed ATen/hand-written/adapter composition equals FD; `TorchComposition` gradients equal the engine's; an Adam loop recovers a known parameter; the core SDK links no LibTorch; 46/46 falsifiers. **Mac-verified** (358 + 1 skip lean, 20 Torch, 32/32 and 46/46; plain `find_package(Torch)` works with the pip wheel; record in `HydroCoupleSDK/verification/g2/`, f16a307). The Mac run exposed a race in the falsifier, not the code: two targets in one parallel `make` rebuilt the shared libraries concurrently (fixed in b84bca7: one target per build). Design change from rev 2: the SDK engine stays the tape and Torch sees the whole run as one `Function` (not one per step). Hand-off: `plans/sdk/G2_3_HANDOFF_2026-09-29.md` |
| G2.4 gradients through stateful adapters (relaxation; temporal interpolation deferred to multi-rate stepping) | **implemented** inside ABI 4 (2026-10-02): `IAdaptedOutput::states()`, `IDifferentiableAdaptedOutput::differentiableStates()`, `ICheckpointableAdaptedOutput`; engine routes cotangents per output and reverses each adapter once per step; relaxation differentiable and checkpointable | A → relax → B, A → C and A → relax → {B, C}: every gradient (λ, the adapter's initial state chained through `prepare()`) equals FD of full forward runs; the ladder gives bit-identical gradients; adapter tokens released exactly once; a `TorchComposition` training loop recovers λ; 21 new falsifier rows (S1–S20, L4), 67/67 caught; independently reviewed. Container-verified (GCC, Clang, LibTorch CPU); not yet on macOS. Plan and outcome: `plans/hydrocouple/G2_4_STATEFUL_ADAPTERS_PLAN_2026-09-29.md`; hand-off `plans/sdk/G2_4_HANDOFF_2026-10-02.md` |
| G2b; G3 proper; G4; G5; device (CUDA) rows; installing the Torch layer | not started | — |

Hand-off: `plans/hydrocouple/G0_G1_HANDOFF_2026-09-28.md`; Mac verification:
`HydroCouple/verification/g0g1/MAC_VERIFICATION_2026-09-28.md` (182 passed / 3 skipped,
144 C++ passed, 23/23 falsifiers caught, cross-image `dynamic_cast` to
`IDifferentiableModelComponent` holds on clang/libc++).

---

## Two invariants this plan must not break

Stated up front because the review question that prompted rev 2 —
*"can I just adapt it to use PyTorch's C++ implementation?"* — has a yes and
a no in it, and the line between them is exactly these two invariants.

1. **The interface stays header-only and depends on nothing.** Today
   `HydroCouple` is a CMake `INTERFACE` library of seven headers whose only
   includes are the C++ standard library (`<cstdint>`, `<span>`, `<vector>`,
   `<functional>`, …). No third-party header is reachable from
   `hydrocouple.h`, and every addition in this plan keeps it that way:
   **no `torch/torch.h`, no `dlpack.h`, nothing beyond `<...>`**. A
   component author who never heard of PyTorch must be able to implement
   `IDifferentiableModelComponent` with the standard library alone, and a
   C-ABI shim must be able to carry every type the contract names.

2. **The interface carries no executable code.** Pure abstract classes and
   plain aggregates; anything that *does* something — a conversion, a
   finite-difference loop, a tape — lives in the SDK or the bindings.
   (Rev 1 put two helpers in `hydrocouplehelpers.h`; rev 2 moves them out,
   §6.)

Everything below that says *Torch* is on the far side of that line.

---

## 0. The question, sharpened

The ask is to let gradients flow through a HydroCouple composition — across
components written in C++ and Python, on host and device — so that machine
learning can be part of model development rather than bolted on beside it.

That single sentence hides four different jobs, and they want different
machinery. Naming them first, because every alternative below serves some
of them well and others badly:

| | Job | Direction | Shape | Who wants it |
|---|---|---|---|---|
| **A** | **Calibration.** Gradient of a scalar misfit with respect to a component's *arguments* (parameters), through the whole composition | reverse | many parameters, one loss | every physics user |
| **B** | **Hybrid physics–ML.** A neural component coupled to a physics component, trained end to end; gradients cross the physics component's *exchange items* | reverse | tensors in, tensors out, through a black box | the differentiable-physics user |
| **C** | **Surrogates and emulators.** A component that *is* a network: inference inside a composition, training data extracted from one | none at run time | device tensors, zero copies | the operational user |
| **D** | **Sensitivity and UQ.** Directional derivatives of many outputs with respect to few inputs | forward | Jacobian-vector products | the analyst |

C needs tensors to cross the boundary without copies, and needs nothing
else. A and B need a *derivative contract* — some way for a component to
say what it does to a cotangent. D needs the same contract's forward twin.
Everything in this document is one of those two things: **move tensors**,
or **compose derivatives**.

## 1. What the interface already gives us

This is not greenfield. Four things in the v2 interface were put there for
other reasons and turn out to be most of the foundation.

**`BufferDescriptor` is already a `DLTensor`.** The interface's sole
currency of field exchange is a plain aggregate: `data`, `kind`, `rank`,
`shape`, `stridesBytes`, `space` (Host / HostPinned / Device / Unified) and
`deviceId`. DLPack's `DLTensor` is `data`, `device{type, id}`, `ndim`,
`dtype`, `shape`, `strides`, `byte_offset`. The header says so itself —
"the layout model follows the DLPack/NumPy buffer protocol" — and the
mapping is field-for-field, with one unit conversion (element strides to
byte strides). Nothing has to be *marshalled* to hand a HydroCouple buffer
to PyTorch, JAX, CuPy or NumPy: a descriptor is *built*, and the framework's
`from_dlpack` reads it. Device buffers included.

**`capabilities()` is how a component says what it can do.** The enum
already carries `DeviceBuffers`, `Checkpointing`, `Cloneable`,
`DistributedExecution`. A component that can produce derivatives is one
more entry and one more optional interface, in exactly the pattern
`ICheckpointableModelComponent` established.

**`ICheckpointableModelComponent` is a reverse-mode checkpoint.** Reverse
mode through a time-stepping model needs the forward trajectory, and
storing every step is impossible for any run worth calibrating. The
standard answer — Griewank's Revolve, binomial checkpointing — needs
exactly two things from a model: *save your state as a token* and *restore
it and carry on*. Both exist, verbatim, for a different reason (walltime
limits on HPC). The adjoint scheduler can be written against them today.

**The Python bindings already do zero-copy in both directions.**
`_hydrocouple/_core.pyx` builds a `BufferDescriptor` as a view of an
ndarray outbound, and wraps a descriptor's memory as an ndarray inbound —
"no element copies ever cross the boundary". Host only; the update plan
defers `MemorySpace.Device` to "a later phase" and names DLPack as the
mechanism. That later phase is Phase G0 below.

And one governing constraint that shapes every alternative: **the interface
carries no executable code.** It is pure abstract classes plus
non-normative helpers, and components are opaque objects across a shared
library boundary — or, under `ITransport`, across a process boundary. There
is no way to *trace* through a component. Whatever the interface offers for
gradients, it is a contract a component fulfils, not a tape the runtime
records inside it.

## 2. The alternatives

Eight. Alts 1 and 2 are the interface half of the recommendation; Alt 8 is
the engine half and supersedes Alt 3 as the tape (Alt 3 is kept because its
analysis of adapters and iterative coupling still governs what Alt 8 has
to do); Alt 6 is the Python surface; Alt 7 is the necessary fallback; Alts
4 and 5 are rejected as *interface* mechanisms with reasons, and Alt 5 is
kept as an implementation route.

### Alt 1 — Tensors cross the boundary; no derivative contract

Bind `BufferDescriptor` to the DLPack protocol: every data-item wrapper
gains `__dlpack__()` and `__dlpack_device__()`, and `set_values_from`
accepts anything with `__dlpack__`. Gradients then work wherever the *whole
chain* lives inside one framework: a Python component whose inputs and
outputs are `torch.Tensor`s can be trained by `torch.autograd` because
nothing left the tape.

*Serves:* C entirely; B only when the physics component is *also* written
in the framework. *Does not serve:* A or B across a C++ component — the
tape stops at the boundary and gradients are zero on the far side, which is
worse than an error because it looks like a result.

*Cost:* small. One protocol on one wrapper class, plus stream and lifetime
handling (§5). *Verdict:* **necessary and insufficient** — it is the
substrate every other alternative stands on, and it is where the work
starts.

### Alt 2 — A derivative contract on the component: `IDifferentiableModelComponent`

The interface-level answer. A component declares `Capability::Differentiable`
and implements one more optional interface:

```cpp
// As implemented in hydrocouple.h (rev 3).
enum class DifferentialRole : uint8_t
{ Input, Argument, Output, StateBefore, StateAfter };

struct DifferentialEntry                 // a plain aggregate, like BufferDescriptor
{
  const IComponentDataItem *item = nullptr;
  DifferentialRole role = DifferentialRole::Input;
  BufferDescriptor value;                // spans the item's whole shape
};
using DifferentialSet = std::span<const DifferentialEntry>;

class IDifferentiableModelComponent : public virtual IModelComponent
{
public:
  virtual std::vector<IInput *>             differentiableInputs()    const = 0;
  virtual std::vector<IArgument *>          differentiableArguments() const = 0;
  virtual std::vector<IOutput *>            differentiableOutputs()   const = 0;
  virtual std::vector<IComponentDataItem *> differentiableStates()    const = 0;

  //! Of the most recent update(). Seeds: Output, StateAfter cotangents.
  //! Results (overwritten): Input, Argument, StateBefore cotangents.
  virtual bool vjp(DifferentialSet seeds, DifferentialSet results,
                   std::string *message = nullptr) = 0;
  //! Seeds: Input, Argument, StateBefore tangents. Results: Output, StateAfter.
  virtual bool jvp(DifferentialSet seeds, DifferentialSet results,
                   std::string *message = nullptr) = 0;
};
```

The state items are what let a gradient flow through time inside a
component: a `StateBefore` cotangent from step *n* is the `StateAfter`
seed of step *n − 1*. An absent seed is zero; results are written, never
accumulated; an entry naming an item or a role the component does not
differentiate is refused with a message.

**How a component fulfils it is the component's business**, and that is
the point. A C++ component may hand-write its adjoint (the only option for
legacy kernels, and the one with the best performance); use an
operator-overloading tool such as CoDiPack or Adept inside its own
translation units; use Enzyme at the LLVM level (Alt 5); or, for a Python
component, let `torch.autograd` do it — a Python `DifferentiableModelComponent`
gets `vjp` for free from the framework it was written in (Alt 6). The
interface does not care which. It asks for a VJP and receives one.

*Serves:* A, B and D, framework-neutrally, across the ABI, across
processes. *Does not serve:* anything on its own — a VJP per component is
useless without something that composes them, which is Alt 3.

*Cost to a component author:* the whole cost of the design lands here, and
it should be stated plainly: **writing an adjoint is as much work as
writing the model.** The interface cannot make that cheaper. What it can do
is make the adjoint *composable* once written, and make the fallback (Alt 7)
automatic where it is not.

### Alt 3 — The coupler owns the tape: a reverse pass over the exchange graph

The SDK's workflow already knows the graph: which output feeds which input,
through which adapted outputs, in which order the scheduler ran the
components (A8: "the schedule follows adapted connections to their roots").
The reverse pass walks that graph backwards, per time step, calling each
component's `vjp` (Alt 2) and each adapted output's adjoint, and
accumulating cotangents at every input.

Three things this needs that Alt 2 does not provide:

*Adjoints for the adapted outputs.* Every adapter the SDK ships — unit
conversion, temporal interpolation (`TemporalInterpolationAdaptedOutput`),
spatial regridding, time slicing — is a linear operator or nearly so, and
its adjoint is the transpose. These are the SDK's to write once, not the
component author's: `IDifferentiableAdaptedOutput` with `vjp`/`jvp`, and
the SDK's factory produces differentiable adapters wherever it produces
adapters. A user-supplied adapter without an adjoint breaks the chain, and
the tape must say so at *construction* rather than return zeros.

*The trajectory.* Reverse mode replays the forward run backwards. For a
composition of `N` steps the tape needs each component's state at each
step, which is exactly `ICheckpointableModelComponent::saveState` — and the
adjoint scheduler implements Revolve over it: `O(log N)` checkpoints,
`O(N log N)` recomputation. A component without `Checkpointing` forces the
tape to store every step; that should work, and warn.

*Iterative coupling.* Where two components iterate to a fixed point within
a step (`IMultiInput`, feedback loops), unrolling the iteration in reverse
is wrong-headed — memory grows with iteration count and the derivative of a
converged fixed point does not depend on the path taken to it. The
implicit-function theorem gives the adjoint as the solution of a *linear*
fixed point in the cotangent, using the same components' VJPs. The tape
should detect a cycle in the exchange graph and use that rather than
unrolling. This is the single hardest piece of Alt 3 and is a phase of its
own.

*Serves:* A and B, end to end. *Does not serve:* a composition that
contains one component without a VJP — hence Alt 7.

*Where it lives:* in the SDK, as executable code, never in the interface.

*Rev 2 verdict:* **superseded as a tape by Alt 8.** Writing and verifying
a reverse-mode tape with checkpoint scheduling is the most expensive item
in rev 1, and LibTorch already is one. The three sub-problems above do not
disappear, though: the adapter adjoints are still the SDK's to write (as
`backward`s now), the trajectory still has to be checkpointed (LibTorch's
C++ API has no `torch.utils.checkpoint`; Alt 8 says how), and iterative
coupling still needs the implicit-function treatment. Alt 3 stays in the
document as the specification of what the engine must do.

### Alt 4 — Derivative types in the data plane: extend `DataKind`

Add `Float64Dual` (forward mode) and an activity flag to `DataKind`, so that
tangents travel *inside* the buffers and every arithmetic operation a
component performs on them propagates derivatives by operator overloading.

**Rejected as the interface mechanism**, for three reasons that do not
weaken with more thought.

It breaks the load-bearing property of §1: a buffer of duals is not a
`DLTensor`, and every framework's `from_dlpack` would refuse it. The
zero-copy path to PyTorch dies.

It is maximally invasive. Every component would have to be *templated on
its scalar type* to propagate anything — which is precisely the rewrite of
every legacy kernel that this design exists to avoid. A component compiled
for `double` would silently strip the derivative.

It only gives forward mode natively. Reverse mode by operator overloading
needs a tape *inside the component*, which is a component-internal choice
(CoDiPack does it well) and belongs behind Alt 2's `vjp`, not in the data
plane.

Kept for one purpose: **as a component-internal implementation strategy**
for fulfilling Alt 2. A component whose kernels are templated may use
CoDiPack to produce its VJP with no hand-written adjoint. That is the
component's decision and the interface never sees the dual type.

### Alt 5 — Source transformation for C++ components: Enzyme

Enzyme differentiates LLVM IR, so an existing C++ kernel compiled with
Clang can yield a gradient function with no source change. It is the
cheapest route to fulfilling Alt 2 for a *legacy* C++ component, and the
one to try first on any kernel that is a pure function of its buffers.

The constraints are real and should be in the plan rather than discovered:
Enzyme cannot see through a virtual call it cannot devirtualise, cannot
cross a shared-library boundary, and treats memory it does not understand
(a `std::map` grown during the step, an I/O call) as opaque. A component's
`update()` as written — virtual, calling into exchange items through
pointers, touching the SDK — is not what Enzyme differentiates. What it
differentiates is the *kernel* the component calls: a free function over
raw buffers. Components whose kernels are already factored that way
(FVQual's transport kernel is; see §7) get Enzyme-derived VJPs cheaply.
Components whose physics is tangled through their object graph do not, and
for those Alt 5 is not a shortcut.

*Verdict:* **an implementation route for Alt 2, not an alternative to it.**
Worth a reference implementation in Phase G4 so the plan knows, rather than
supposes, how far it reaches.

### Alt 6 — The framework as coupler: `torch.autograd.Function` / `jax.custom_vjp` per component

From Python, drive the composition through the framework: wrap each C++
component's step as a custom autograd op whose `forward` calls `update()`
and whose `backward` calls `vjp()` (Alt 2). The framework's own autograd
then *is* the tape (Alt 3), and a Python-authored neural component sits in
the same graph as a native one with no adaptation at all.

This is the **Python surface** of Alt 2, and it is where Job B's user
actually lives. It is not the whole engine, because a composition driven
from C++ (the Composer, a batch run, an HPC job) never enters Python; that
side is Alt 8b, which uses the same LibTorch from C++. The two entry
points must agree, which is what the gradient-check gates in §8 are for.

Two specifics for the bindings:

*PyTorch.* `torch.autograd.Function` with `ctx.save_for_backward` holding
whatever the component's `vjp` needs — and here the component's own
checkpoint token (Alt 3) is the right thing to save, so that the framework
tape and the SDK tape share one notion of "the state at this step".

*JAX.* `jax.custom_vjp` around a `jax.pure_callback`, since a C++ component
is opaque to tracing. Batching (`vmap`) over a callback needs the component
to be `Cloneable` — one clone per batch element — which is the third
existing capability doing a job it was not designed for.

### Alt 7 — Fallback: finite differences and surrogate adjoints

A composition with one component that has no `vjp` must still train, or
the whole design is only useful once every component in the room has been
rewritten. Two fallbacks, both automatic, both loud:

*Finite differences* over `ICloneableModelComponent`: clone the component,
perturb one input element, step, difference. Cost is one forward step per
input element — unusable for a field of a million cells, fine for a scalar
parameter. The tape should use it for *arguments* with a warning and refuse
it for *fields* unless told otherwise.

*Surrogate adjoints*: a trained network standing in for the component's
VJP. This is Job C feeding Job A, and it belongs in the plan as a documented
pattern rather than as machinery.

*Verdict:* **required**, as the degradation path. A design that has no
answer for the undifferentiable component is a design for a world that
does not exist.

### Alt 8 — LibTorch as the engine, never as the type

The question rev 2 answers. "Adopt PyTorch's C++ implementation" can mean
two different things, and they have opposite verdicts.

**8a — Torch as the data type: rejected.** Make `at::Tensor` the currency
of exchange — `IComponentDataItem` hands out tensors, every component links
LibTorch, autograd tracks everything. Rejected on both invariants at once:
`torch/torch.h` in `hydrocouple.h` ends the header-only, standard-library-
only interface, and an `at::Tensor` cannot cross a C ABI, so a SWMM or a
Fortran groundwater model can no longer implement the interface behind a
shim. It also does not buy the hard part: autograd tracks *ATen ops*, so a
legacy solver doing its arithmetic in its own loops gets no gradient from
Torch anyway and still has to hand it a `backward` — which is Alt 2's
`vjp` under another name. And it picks a winner: Torch's graph and JAX's
tracer do not interoperate, so a Torch-typed interface excludes the JAX
user, whereas a `BufferDescriptor` plus a `vjp` serves both.

**8b — Torch as the autograd engine: recommended.** Keep the interface
exactly as Alt 2 leaves it (a capability, `vjp`/`jvp`, buffers). Inside the
**SDK**, behind a build option, implement the reverse pass of Alt 3 as
LibTorch autograd rather than as a tape of the SDK's own:

- Every component step is a `torch::autograd::Function` whose `forward`
  wraps the component's inputs as tensors by DLPack (zero copy, Alt 1),
  runs `update()`, and wraps the outputs; whose `backward` calls the
  component's `vjp` (Alt 2) with the incoming cotangents as
  `BufferDescriptor`s and returns what it accumulates. The coupling graph
  *is* the autograd graph; nothing walks the exchange graph in reverse
  because Torch already does.
- Every adapted output the SDK ships becomes a `Function` too, with the
  transpose as its `backward` — the same work Alt 3 named, in a shape
  Torch composes.
- A component written *natively in ATen* — a new C++ component whose
  kernels are Torch ops — needs no `vjp` at all: it declares
  `Differentiable`, its `vjp` is implemented by the SDK bridge as
  `torch::autograd::grad` over its own forward, and it is differentiable
  for free. That is the real prize of 8b: **new physics written against
  ATen is differentiable by construction**, while old physics keeps the
  contract.
- Checkpointing: LibTorch's C++ API has no `torch.utils.checkpoint`, so the
  SDK provides one `CheckpointedStep` `Function` that saves the component's
  `ICheckpointable::saveState` token in `ctx` and re-runs the forward
  inside `backward`. Revolve then becomes a *policy* — which steps keep
  their token, which recompute — applied over Torch's graph, rather than a
  tape of its own. `O(log N)` memory is preserved and the §8 gate still
  measures it.
- Device: a component on the SDK's Kokkos backend hands its buffer to Torch
  by DLPack on the same CUDA context; the SDK synchronises the component's
  stream against the tensor's before `forward` returns. No copy, but this
  is the seam where a silently stale tensor comes from, and G0's stream
  gate is written for it.

*What it costs.* LibTorch becomes a dependency **of the SDK, optionally**
(`HYDROCOUPLESDK_WITH_TORCH`), never of the interface and never of a
component that does not choose it. The dependency is large (hundreds of MB
of shared libraries, an ABI that must match the Python `torch` wheel in the
same process) and the SDK pins a version. An SDK built without it still
supports everything single-component: `Differentiable` components declare
and answer `vjp`, the finite-difference fallback and the G1 gradient checks
work, and Alt 6's Python wrappers work because they use Python's `torch`.
Only the *C++-driven composed* reverse pass (Composer, batch runs) needs
the option on.

*JAX has no C++ implementation to adopt.* JAX is a Python tracer over XLA;
C++ enters it only as XLA FFI custom calls (`jax.ffi`) with `custom_vjp`
registered in Python, and `jit` requires pure functions with static
shapes. A stateful, time-stepping component can only ever be a *consumer*
of the contract from JAX's side — a callback per step with its `vjp` as
the custom backward — which is Alt 6 as already written. There is no 8b
for JAX, and that is fine: the contract is the same shape as
`custom_vjp`'s `bwd`, so nothing in the interface prefers Torch.

*Serves:* A and B end to end, C, and D through `torch::autograd::forward_ad`
where ATen-native and through `jvp` otherwise. *Does not serve:* a
composition driven from C++ on an SDK built without Torch — by design.

## 3. Comparison

| | 1 DLPack | 2 VJP contract | 3 SDK tape | 4 Dual kinds | 5 Enzyme | 6 Framework coupler | 7 Fallback | 8a Torch type | 8b Torch engine |
|---|---|---|---|---|---|---|---|---|---|
| Serves A (calibration) | — | with 3 or 8b | with 2 | forward only | via 2 | with 2 | slowly | yes | **with 2** |
| Serves B (hybrid) | in-framework only | with 3 or 8b | with 2 | no | via 2 | **yes** | slowly | yes | **yes** |
| Serves C (surrogates) | **yes** | — | — | breaks it | — | yes | — | yes | yes |
| Serves D (sensitivity) | — | `jvp` | with 2 | **yes** | via 2 | yes | yes | yes | yes |
| Framework-neutral | yes | **yes** | **yes** | yes | yes | no | yes | **no** | interface yes, SDK no |
| Crosses the ABI / process boundary | yes | **yes** | **yes** | no | no | via 2 | yes | **no** | via 2 |
| Device buffers | **yes** | yes | yes | no | partial | yes | yes | yes | yes |
| Interface stays header-only, std-only | yes | **yes** | yes | yes | yes | yes | yes | **no** | **yes** |
| Change to the interface | none | one capability, two interfaces | none | `DataKind` | none | none | none | everything | none |
| New physics differentiable for free | no | no | no | if templated | if factored | Python only | no | yes | **yes, in ATen** |
| Cost to a component author | none | **the adjoint** | none | template everything | factor the kernel | none | none | link Torch, still the adjoint for legacy kernels | none beyond 2 |
| Cost to the SDK | small | small | **large** | large | small | medium | medium | rewrite | medium, optional dep |

## 4. Recommendation

**Alts 1 + 2 in the interface; 8b as the engine in the SDK; surfaced
through 6; degraded through 7.** In one sentence: *tensors cross the
boundary by DLPack; components declare and deliver their own derivatives
through one optional, standard-library-only interface; the SDK composes
those derivatives with LibTorch's autograd as an optional build, where
new ATen-native physics is differentiable by construction and old physics
keeps the contract; the Python bindings expose every component as a native
`torch.autograd.Function` or `jax.custom_vjp`; and anything without an
adjoint falls back to finite differences with a warning.*

Alt 3 is retained as the specification of what the engine must do
(adapter adjoints, checkpointing, iterative coupling) and dropped as an
implementation. Alt 4 is rejected for the interface and kept as a
component-internal option. Alt 5 is an implementation route, to be
measured rather than assumed. Alt 8a is rejected.

The reasons, briefly, for a *contract* in the interface and a *framework*
underneath it:

- The interface's own rules — header-only, standard library only, no
  executable code — forbid both a tape and a tensor type in it. A
  framework can only ever sit behind the contract.
- Components are black boxes across an ABI. The only derivative a black box
  can offer is the one it computes itself; a framework cannot see inside,
  so the framework does not replace the adjoint, it composes it.
- Framework neutrality is not optional for a *standard*, but it is
  optional for an *implementation*. The SDK may pin Torch; the interface
  may not. The contract has the same shape as `torch::autograd::Function::
  backward` and `jax.custom_vjp`'s `bwd`, which is what makes 8b a thin
  layer and what keeps JAX users whole.
- The contract is the same shape as the capabilities already there, and
  reuses two of them (`Checkpointing`, `Cloneable`) for jobs they were not
  designed for but fit exactly.
- Where the ML work will actually happen — new components, hybrid models —
  8b removes the adjoint cost entirely for anyone willing to write against
  ATen. That is the strongest argument in this document and it costs the
  interface nothing.

## 5. The Python bindings, specifically

Everything the user of Job B or C touches is here, so it gets its own
section.

### 5.1 Keep Cython for the interface; the question is the SDK

The interface bindings are Cython, the decision is documented, and the
two-way bridge (`py_component_bridge.h` — a Python ABC visible to C++ as a
real `IModelComponent*`) is, in the update plan's words, the crown jewel.
Nothing here argues for changing that.

The SDK has no bindings yet, and that is where a real choice exists.
**nanobind** deserves consideration for the SDK layer specifically because
its `nb::ndarray<>` type *is* a DLPack adapter — it accepts and returns
NumPy, PyTorch, JAX, CuPy and TensorFlow arrays natively, on any device,
with framework and device selectable in the type. The autograd wrappers
(Alt 6) and the Python face of the Torch engine (8b) would be written
against exactly that type. Two binding technologies in one project is a
cost — two build systems, two sets of idioms — and the plan should weigh
it in Phase G3 rather than decide it here. What it must not do is bind the
SDK in a way that makes DLPack an afterthought.

One thing 8b settles: the SDK's Torch engine and Python's `torch` must be
**the same LibTorch** when both are loaded in one process, or the two
autograd graphs cannot share tensors. The SDK's Torch build therefore
links against the LibTorch that ships inside the `torch` wheel (its
`TORCH_CMAKE_PREFIX_PATH`), not a separately downloaded one, whenever the
Python bindings are built.

### 5.2 DLPack on the data-item wrapper

> **Rev 3 correction.** As built, there is no `__dlpack__` on a data item:
> the interface's data plane is copy-into, and an item never exposes its
> own storage, so there is nothing for a capsule to point at. DLPack
> enters where buffers do. `get_values_into` / `set_values_from` accept
> any DLPack producer and lend its memory to C++ as a `BufferDescriptor`;
> in the other direction a C++ device descriptor reaches a Python item as
> a `hydrocouple.dlpack.BufferView`, which *is* a producer (and is revoked
> when the call returns). The lifetime, stream and mutability analysis
> below applies unchanged to those two objects.

```python
class ComponentDataItem:
    def __dlpack__(self, *, stream=None, max_version=None,
                   dl_device=None, copy=None) -> PyCapsule: ...
    def __dlpack_device__(self) -> tuple[DLDeviceType, int]: ...
```

`__dlpack__` builds a `DLManagedTensor` over the item's *current* buffer.
Three things it has to get right:

*Lifetime.* A `BufferDescriptor` does not own its memory; a `DLManagedTensor`
must keep it alive until the consumer's deleter runs. The capsule's
`manager_ctx` holds a reference to the item (and, for a Python component,
to the ndarray or tensor behind it). The failure mode otherwise is a tensor
that reads freed memory some frames later, which is the kind of bug that
survives for months.

*Streams.* On a device, the consumer passes the stream it will read on;
the producer must synchronise its own writes to that stream before
returning. `DeviceBackend::synchronize` is the seam. Getting this wrong is
silent — a tensor of last step's values.

*Mutability.* A DLPack tensor is a *view*. A framework that writes into it
writes into the component. That is either the fastest possible
`set_values_from` or a corruption bug, depending on whether it was meant;
the wrapper should hand out read-only views by default and a writable one
through an explicit method.

Inbound (`set_values_from(tensor)`), the wrapper calls the framework's
`__dlpack__`, reads the `DLTensor` into a `BufferDescriptor` — field for
field — and calls `setValuesFrom`. `MemorySpace` comes from
`DLDeviceType`; a device the SDK has no backend for is an error at that
point, not a copy to host.

### 5.3 The autograd surface

Rev 1 sketched a `hydrocouple.autograd` tape API of its own. Rev 2 drops
it: with 8b the tape is Torch's, and the Python user should see nothing
but Torch.

```python
import torch
import hydrocouple.torch as hct

roughness = catchment.argument("roughness").as_tensor(requires_grad=True)
step = hct.Step(workflow)            # one torch.autograd.Function per step
for t in schedule:
    outputs = step(t, roughness, net.parameters())
loss = misfit(outputs[gauge], observed)
loss.backward()                      # no HydroCouple-specific call
```

`hct.Step` is the `torch.autograd.Function` of Alt 6: `forward` drives the
SDK's workflow one step (which, on an SDK built with Torch, is itself
recording the same graph — the two are one graph, not two that must
agree), and `backward` calls each component's `vjp`. A JAX user gets the
same through `hydrocouple.jax.step`, a `jax.custom_vjp` around a
`jax.pure_callback`, as Alt 6 describes.

A Python component becomes differentiable by subclassing
`hydrocouple.abc.DifferentiableModelComponent` and doing nothing else *if*
its `update` is written in Torch: the bridge records the forward call
under Torch's tape and answers `vjp` from it, so a C++-driven reverse pass
(Composer, batch) gets a correct adjoint from a Python neural component
without that component knowing it was asked. A Python component written
in plain NumPy gets Alt 7.

### 5.4 The GIL

A C++-driven reverse pass calls `vjp` on Python components; the bridge must
acquire the GIL around each call and release it around every C++ `vjp`, or
a composition of one Python and ten C++ components runs its whole backward
pass serialised on one lock. The existing bridge already does this for
`update()`; `vjp` follows the same pattern and the plan should say so
rather than rediscover it.

### 5.5 Ownership across the bridge, both directions

A Python component's outputs, seen from C++, are ndarrays or tensors
wrapped as descriptors; a C++ component's outputs, seen from Python, are
descriptors wrapped as tensors. In both directions the wrapped object must
outlive every descriptor or tensor made from it. The bridge already solves
this for host ndarrays; device tensors add the stream question above and
nothing else.

## 6. What the interface change actually is

Small, additive, and **confined to `hydrocouple.h` with no new includes.**
One line in an enum, two optional interfaces, two plain aggregates — in
the same style as `ICheckpointableModelComponent` and its
`Capability::Checkpointing`.

```
hydrocouple.h                                  (std-only; no new #include)
  enum class Capability { ..., Differentiable }        -- appended: value 7
  enum class DifferentialRole                          -- which side of the step
  struct DifferentialEntry                             -- item, role, BufferDescriptor
  using DifferentialSet = std::span<const DifferentialEntry>
  class IDifferentiableModelComponent                  -- vjp, jvp, and what they reach
  (class IDifferentiableAdaptedOutput                  -- deferred to G2)
```

The aggregate is as plain as `BufferDescriptor` itself — aggregate,
standard-layout, trivially copyable, trivially destructible, all asserted
by `DifferentialTest.EntryIsAPlainAggregateACAbiCanCarry` — so that a
C-ABI shim can carry it. `HYDROCOUPLE_ABI_VERSION` stays 2: the change is
a new side interface and an appended enum value; no existing vtable moves.

No `std::map`, no callbacks, no ownership: the caller owns every buffer,
exactly as on the data plane.

**What is deliberately *not* in the interface**, and where it goes
instead — this is the rev 2 correction to rev 1, which had put the first
two in `hydrocouplehelpers.h`:

| Rev 1 placed in the interface | Rev 2 places in | Why |
|---|---|---|
| descriptor ⇄ `DLTensor` conversion | the Cython bindings and the SDK (`hydrocouplesdk/device/dlpack.h`) | it needs `dlpack.h`; the mapping is field-for-field and any consumer that has `dlpack.h` can write it in twenty lines |
| finite-difference VJP over `ICloneable` | the SDK (`hydrocouplesdk/autograd/finitedifference.h`) | it is executable code |
| the tape / reverse pass | the SDK, as LibTorch autograd, behind `HYDROCOUPLESDK_WITH_TORCH` | 8b |
| `torch::autograd::Function` wrappers per component and adapter | the SDK (`hydrocouplesdk/autograd/torch/`) | needs `torch/torch.h` |
| `CheckpointedStep` + Revolve policy | the SDK, same directory | executable |
| Python `hct.Step`, `hydrocouple.jax.step` | `HydroCouple/python` (interface-level, uses Python `torch`/`jax`) and SDK bindings | Python |

The test that the invariant held is mechanical and should be a gate in
`HydroCouple/tests`: every `#include` line in `include/*.h` is either a
standard header or a sibling —

```sh
grep -hE '^#include' include/*.h \
  | grep -vE '^#include (<[a-z_]+>|"hydrocouple[a-z]*\.h"|"version\.h")' \
  && { echo "third-party include in the interface"; exit 1; }
```

— and a translation unit that includes `hydrocouple.h` alone compiles with
`-I include` and nothing else, which is what the syntax harness already
checks for every other change. A falsifier adds `#include <dlpack/dlpack.h>`
to `hydrocouple.h` and requires the gate to fail.

Nothing existing changes meaning. A component that never declares
`Differentiable` is exactly as it was, and an SDK built without Torch is
exactly as it was.

## 7. Phases

Each phase is meant to be **tangible and evaluable on its own** — something
that can be run, and whose gradients can be *checked* rather than admired.

**G0 — Tensors cross the boundary.** DLPack on the data-item wrapper, host
and device, both directions; stream and lifetime handling; `MemorySpace.Device`
finally bound. *Tangible:* a `torch.cuda` tensor flows into a C++ component
and its output flows back into PyTorch, both without a copy, measured by
pointer equality. Serves Job C entirely and is the substrate for the rest.
Reads `UPDATE_PLAN.md`'s deferred phase and does it.

**G1 — The contract.** The interface additions of §6 (std-only, gated by
the include allowlist), their Python ABC mirrors, and the
finite-difference helper *in the SDK*. *Tangible:* one component with a
hand-written `vjp` passes a gradient check against the helper's finite
differences on every input and argument, to a tolerance the check reports;
the interface's include gate passes and its falsifier is caught. No Torch
anywhere yet — G1 proves the contract stands on its own.

*Rev 3 split G2 into three slices, in dependency order: **G2.1** the
Torch-free engine (every piece the Torch layer needs — reverse schedule,
routing through adapters, fan-out sums, state carry, replay — exists
without Torch, so an SDK built without it can still differentiate an
acyclic composition: the §9 "Torch-free floor" question, answered yes
unless the review objects); **G2.2** Revolve; **G2.3** the LibTorch layer
below.*

**G2 — The Torch engine.** `HYDROCOUPLESDK_WITH_TORCH`: a
`torch::autograd::Function` per component step and per SDK adapter, with
`backward` calling `vjp` or the adapter's transpose; DLPack views over
`BufferDescriptor` in both directions (from G0, now on the C++ side);
`CheckpointedStep` over `ICheckpointable::saveState` with a Revolve
policy; a cycle detector that refuses (for now) to differentiate an
iterative coupling rather than unrolling it silently; and the ATen-native
path, where a component whose kernels are Torch ops declares
`Differentiable` and gets its `vjp` from `torch::autograd::grad` without
writing one. *Tangible:* a two-component composition with a
temporal-interpolation adapter between them, driven from C++, yields an
end-to-end gradient that matches finite differences through the whole
chain; one of the two components is ATen-native and has no hand-written
adjoint; and memory stays `O(log N)` in the step count — measured. An SDK
configured without the option still passes every G1 gate.

**G2b — Iterative coupling.** The implicit-function adjoint for a converged
fixed point, as a `Function` whose `backward` solves the linear cotangent
fixed point with the components' VJPs. *Tangible:* a feedback
composition's gradient matches a brute-force unrolled adjoint to tolerance,
at a fraction of its memory. Split out because it is the hardest piece and
should not hold G2 hostage.

**G3 — The Python surface.** `hydrocouple.torch.Step` and
`hydrocouple.jax.step`, the `DifferentiableModelComponent` ABC with
Torch-derived `vjp`, the same-LibTorch build arrangement of §5.1, and the
nanobind-versus-Cython decision for the SDK bindings. *Tangible:* a
PyTorch network coupled to a C++ physics component trains — loss falls
over epochs — and the gradient from `loss.backward()` in Python agrees
with the gradient from the C++-driven G2 engine on the same composition
to tolerance. This is Job B, working.

**G4 — Reference adjoints.** One Enzyme-derived VJP and one hand-written
one for real components, to learn what Alt 5 reaches. The obvious flagship
is FVQual's groundwater transport kernel — factored, pure over its buffers,
and the thing a calibration user actually wants a gradient of. *Tangible:*
a calibration run recovers a known dispersion coefficient from synthetic
observations.

**G5 — Fallback and degradation.** Alt 7 wired into the tape with its
warnings; a composition containing one undifferentiable component still
trains. *Tangible:* the same G3 composition with the physics component's
`vjp` removed still converges, slower, and says why.

## 8. Verification: the falsifier for gradients is finite differences

Every phase's gates share one shape, because AD has a universal oracle:
**a VJP that disagrees with central finite differences is wrong**, and the
disagreement is a number. Each gate is a gradient check with a stated
tolerance, and each falsifier mutates a sign or a transpose in an adjoint
and requires the check to fail. Mutation survivors in this domain are
almost always a gate with too loose a tolerance, and that is what the
falsifier is for.

Three gates that are not gradient checks and matter as much:

- **Zero is not a gradient.** A boundary that drops the tape returns zeros,
  which look like "no sensitivity" and are the worst possible failure. A
  gate asserts that a known-sensitive parameter's gradient is non-zero
  across every boundary the composition crosses.
- **The two drivers agree.** The C++-driven engine (G2) and the
  Python-driven `loss.backward()` (G3) must match on the same composition,
  or Job A and Job B users are calibrating different models. With 8b these
  are one autograd library entered from two sides, so a disagreement
  points at the DLPack seam or the GIL, not at the mathematics.
- **Memory is logarithmic.** Revolve's whole point; a gate that steps
  `N = 10, 100, 1000` and asserts checkpoint count grows as `log N`.
- **The interface stayed clean.** The include allowlist of §6, run on
  every change; and a build of the SDK with `HYDROCOUPLESDK_WITH_TORCH=OFF`
  that passes every non-composed gate.

## 9. Risks and the questions this review should settle

**The adjoint is the cost, and the interface cannot hide it.** Every
physics component wanting Job A or B needs a VJP, which is the model's
work over again. The plan's honest position: make it composable once
written, automate it where kernels are factored (Enzyme), and degrade
gracefully where it is not. *Question:* which components are candidates
for a hand-written adjoint in the next year, and which will only ever have
the fallback? That list decides whether G4's reference implementations are
one or five.

**Discontinuities are the physics', not the interface's.** Wet/dry
transitions, thresholds, `max(0, ·)` — a hydrological model is piecewise,
and its gradient is piecewise too, with discontinuities the optimiser will
find. The interface should carry derivatives faithfully and say nothing
about smoothing; smoothing is a modelling choice. *Question:* does the
plan need a `Capability` distinguishing "differentiable" from
"differentiable and smooth"?

**Iterative coupling is genuinely hard** (G2b), and the wrong shortcut —
unrolling — works on small examples and fails at scale. The plan isolates
it. *Question:* how common is fixed-point coupling in the compositions
that matter? If rare, G2b waits.

**Two binding technologies.** §5.1's nanobind question. *Question for
review:* is DLPack-first for the SDK bindings worth a second toolchain, or
should the Cython layer grow a DLPack adapter and stay single?

**LibTorch as an SDK dependency.** 8b makes the SDK's composed reverse
pass depend on a large library with a pinned ABI, optional at build time.
Three consequences the review should accept or reject explicitly: the
Composer, to differentiate a composition it drives, must ship with a
Torch-enabled SDK; the SDK's Torch must be the same LibTorch as the `torch`
wheel when both are in one process (§5.1); and the JAX user gets the
contract but no engine on the C++ side. *Question:* is an SDK build that
cannot differentiate a composition without Torch acceptable, or does the
review want a minimal SDK-own tape (rev 1's Alt 3, reduced to
non-checkpointed, acyclic compositions) as a Torch-free floor?

**ATen-native components.** The strongest argument for 8b is that new
physics written against ATen is differentiable for free. *Question:* will
new HydroCouple components actually be written that way — with
`at::Tensor` kernels and Torch's device management — or will they keep
using the SDK's Kokkos backend, in which case 8b's free path is unused and
the adjoint cost stays where rev 1 put it? The answer decides whether the
Kokkos backend and Torch need to share an allocator, which is a further
phase.

**Stream semantics on device** are the silent-failure risk of G0: a
gradient of last step's values, correct in shape, wrong in every element.
The G0 gate must include a device test with an explicit stream, or G0 is
not done. *Rev 3:* implementing G0 showed that the gap is in the
interface, not the bindings — `BufferDescriptor` has no stream or event,
and `getValuesInto` does not say when device work is complete. The
bindings adopt a host-synchronous rule (values are in place when the call
returns; a producer is handed `stream=None`, i.e. the legacy default
stream, unless the caller passes one). *Question:* should the interface
say so normatively, or carry a stream/event handle so device items can
stay asynchronous?

**Distributed compositions** (`ITransport`) are out of scope for G0–G5.
The contract crosses a process boundary fine — a VJP is just a call — but
the tape's reverse pass over a distributed schedule is a further phase and
this plan does not pretend otherwise.

---

*Written against the interface at v2.0 (header-only `INTERFACE` target,
seven headers, standard-library includes only) and the bindings as of
`UPDATE_PLAN.md` (2026-08-22). Rev 2 folds in the LibTorch-as-engine
question. Nothing in this document is implemented; it is a plan for
review, and §9's questions are what the review should answer.*
