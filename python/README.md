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
python -m pytest         # 88 tests, including enum-parity checks that
                         # parse the C++ headers so the bindings cannot
                         # silently drift from the standard
```

See `examples/` for a runnable Python component and a script driving the
typed data plane.
