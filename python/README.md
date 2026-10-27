# hydrocouple — Python bindings for the HydroCouple interface standard

Python bindings for the HydroCouple v2.0.0 component-based modeling
framework. Two layers:

- **`hydrocouple`** (pure Python): abstract base classes mirroring the
  C++20 header-only interfaces 1:1. Python component developers subclass
  these ABCs exactly as C++ developers implement the pure-virtual
  interfaces. No dependencies beyond NumPy.
- **`_hydrocouple`** (Cython): the bridge to compiled C++ components —
  in both directions. `hydrocouple.loader.load()` brings a compiled
  component into Python; `PyComponentBridge` makes a Python component
  visible to C++ workflows as a real `IModelComponent*`.

## The data plane: NumPy is the BufferDescriptor

Field data moves through the typed hyperslab API. An ndarray carries
exactly what the C++ `BufferDescriptor` encodes — data pointer, dtype,
shape, byte strides — so the bridge marshals **zero-copy** in both
directions and releases the GIL around C++ compute:

```python
import numpy as np
from hydrocouple.loader import load

component, info, handle = load("./libMyModel.so")
component.initialize()
component.validate()
component.prepare()

output = component.outputs[0]
values = np.empty(output.shape, dtype=np.float64)
for _ in range(100):
    component.update()                      # GIL released while C++ runs
    ok, msg = output.get_values_into(
        values, start=(0,) * len(output.shape), count=output.shape)
    assert ok, msg

component.finish()
```

Selections are HDF5-style hyperslabs (`start`/`count` per dimension), and
destinations may be non-contiguous views — a pitched or transposed slice
works without copies. Dimension orderings are canonical per data-item
type (time is always dimension 0; see each ABC's docstring).

## Tensors from any framework, on any device (DLPack)

`get_values_into` / `set_values_from` accept not only ndarrays but any
object implementing the DLPack protocol — a `torch.Tensor`, a CuPy array,
a JAX array (as a source; JAX arrays are immutable, so never as a
destination) — on the host or a device. The C++ item receives a
`BufferDescriptor` over the tensor's own memory (`MemorySpace.Device` for
a GPU tensor); nothing is staged through the host behind your back:

```python
t = torch.empty(output.shape, dtype=torch.float64, device="cuda")
ok, msg = output.get_values_into(t, (0,) * t.ndim, t.shape)
```

In the other direction, a C++ engine calling a *Python* item with device
memory hands it a `hydrocouple.dlpack.BufferView`; `torch.from_dlpack(view)`
aliases the engine's buffer. See `hydrocouple.dlpack` for the device
mapping (`set_accelerator` on ROCm/oneAPI/Metal). DLPack lives in the
bindings only: the interface headers depend on the standard library alone.

## Gradients across model boundaries (PyTorch, JAX)

A component that declares `Capability.Differentiable` implements
`IDifferentiableModelComponent` (`vjp` / `jvp` of its most recent step,
with its state listed so gradients flow through time). The overlays turn
it into a native operation of either framework:

```python
import hydrocouple.torch as hct

reservoir = hct.Step(component)          # a C++ model with an adjoint
state = reservoir.initial_state()
loss = 0
for x, observed in data:
    (outflow,), state = reservoir(state, [net(x)], [k])
    loss = loss + ((outflow - observed) ** 2).sum()
loss.backward()     # reaches net.parameters(), k, and the initial state
```

`hydrocouple.jax.Step` is the same as a `jax.custom_vjp` function
(`jax.grad`, `jax.value_and_grad`, `jax.jit`). Earlier steps are replayed
from checkpoints during backward, so a multi-step component should also
declare `Capability.Checkpointing`. `tests/test_gradients.py` proves the
chain torch → C++ → C++ → torch against finite differences, and that
PyTorch and JAX agree.

## Working with compiled objects: wrappers and views

Every C++ object the bindings hand out is wrapped in a class registered as
the ABC it implements, so `isinstance(output, IOutput)` holds and code
written against the ABCs runs unchanged on a compiled component.
`tests/test_wrapper_conformance.py` holds each registration to its ABC:
every abstract member present, a property where the ABC declares a
property, a method where it declares a method (`ABC.register()` itself
checks none of this).

A wrapper starts as the general interface (`component.outputs` yields
`CppOutputWrapper`s). Ask for a more specific interface with a view
function; each returns `None` when the C++ object does not implement it:

```python
from _hydrocouple._spatial import as_layered, as_spatial
from _hydrocouple._spatiotemporal import as_spatiotemporal, as_time_layered
from _hydrocouple._temporal import as_time_model_component, as_time_series

series = as_time_series(output)          # ITimeSeriesComponentDataItem
print(series.time_kind, series.time_interpolation)
grid = as_spatial(output)                # the most specific spatial item
mesh = as_layered(output)                # ILayeredMeshComponentDataItem, ...
timed = as_spatiotemporal(output)        # e.g. ITimeSeriesRasterComponentDataItem
clock = as_time_model_component(component)
```

Wrappers follow three rules:

- **Equality is identity of the C++ object.** Two wrappers are equal (and
  hash alike) when they wrap the same object, so a view equals the wrapper
  it came from and items can key a dict.
- **A child keeps its parent alive.** A view, an item, a geometry or a
  band holds a reference to the wrapper it came from, which holds the
  component, the component info and the loaded library. Objects C++ creates
  for the caller — a buffered geometry, a new component instance, a new
  adapted output — are owned by their wrapper and destroyed with it.
- **`connect(slot)` reaches the signal the ABC documents.** On a component
  the slot receives an `IComponentStatusChangeEventArgs`; on a workflow an
  `IWorkflowComponentStatusChangeEventArgs`; on a data item an
  `IComponentDataItemValueChanged`; on anything else the property name.
  Connecting the same slot twice connects it once, and `disconnect(slot)`
  works from any wrapper of the same object. The `on_status_changed`,
  `on_value_changed` and `on_property_changed` methods return a handle to
  disconnect instead, with the raw signal arguments.

## Implementing a component in Python

Subclass the ABCs, then hand the component to C++ through the bridge:

```python
from hydrocouple.core import ComponentStatus, IModelComponent
from _hydrocouple._core import PyComponentBridge

class MyModel(IModelComponent):
    ...  # implement the lifecycle, arguments, inputs/outputs,
         # capabilities(), errors()

bridge = PyComponentBridge(MyModel())
# The bridge holds a C++ IModelComponent* delegating every virtual call
# (including capabilities() and the errors() diagnostic queue) to Python.
```

## Helpers

`hydrocouple.helpers` mirrors the C++ `hydrocouplehelpers.h` — the single
sanctioned exception to the standard's no-implementation rule:
`DataKind ⇄ np.dtype` maps, `is_valid_component_status_transition()`
(the lifecycle state machine), typed scalar/slab conveniences with
raising variants, and Julian-day conversions matching the `IDateTime`
convention. `hydrocouple.distributed` mirrors `hydrocoupledistributed.h`
(transports, proxies, partitioned data items with virtual/halo entities);
transport implementations are SDK territory.

## Building

Requires a C++20 compiler, Python ≥ 3.10, NumPy, and Cython ≥ 3.0. The
package version is single-sourced from the repository's
`include/version.h`.

```bash
cd python
pip install .            # or: python setup.py build_ext --inplace
python -m pytest         # includes parity checks that parse the C++
                         # headers (enums, transition tables, every ABC's
                         # members) so the mirror cannot silently drift
                         # from the standard, and runs the examples
```

See `examples/` for a runnable Python component (`sine_wave_component.py`),
a script driving a compiled component through the typed data plane
(`drive_cpp_component.py`), and two Python components coupled to each other
with no C++ involved (`coupled_python_models.py`).
