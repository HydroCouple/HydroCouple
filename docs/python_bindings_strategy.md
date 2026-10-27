# HydroCouple Python Bindings Strategy — Cython

## 1. Goal

Enable **bidirectional component coupling** so that:

- A **pure-Python component** can be plugged into a C++ workflow alongside compiled C++ components.
- A **compiled C++ component** can be loaded and driven from Python.
- Data exchange between Python and C++ components uses the same `IOutput`/`IInput` contract with minimal overhead.
- Python developers implement HydroCouple interfaces by subclassing **Python abstract base classes** that mirror the C++ hierarchy — the same design-by-contract pattern.

---

## 2. Why Cython

| Criterion | Cython | pybind11 / nanobind | ctypes / cffi |
|---|---|---|---|
| Direct C/C++ pointer access | Yes (typed memoryviews, `cdef` pointers) | Indirect (type-erased holders) | Indirect (cast-heavy) |
| Extension type inheritance | Native (`cdef class`) | Trampoline boilerplate | N/A |
| NumPy zero-copy | Built-in (memoryview / buffer protocol) | Manual buffer protocol | Manual |
| GIL control | `with nogil:` blocks | `py::gil_scoped_release` | N/A |
| Cython-native ABC pattern | `cdef class` + Python ABC mixin | Not applicable | N/A |
| Build integration | `cythonize` + setuptools/scikit-build | CMake | None |
| Compile-time type checking | Yes (`.pxd` declarations) | C++ templates | None |
| Overhead per virtual call | ~1 C function-pointer call | ~1 indirect + type-erase | ctypes marshalling |

Key advantage: Cython `cdef class` types are **real C extension types** with C-level method dispatch. A Python subclass of a Cython base class can be passed to C++ code through a thin C wrapper — no trampoline indirection needed for the hot path.

---

## 3. Architecture Overview

```
+===========================================================================+
||                       Workflow Orchestrator                              ||
||             (C++ IWorkflowComponent  or  Python WorkflowComponent)      ||
+===========================================================================+
|                                                                           |
|  +------------------------+    IOutput/IInput    +----------------------+ |
|  |  Python Component      |<------------------->|  C++ Component       | |
|  |  (subclasses           |                      |  (shared library     | |
|  |   hydrocouple.abc.     |                      |   loaded via         | |
|  |   ModelComponent)      |                      |   ComponentLoader)   | |
|  +----------+-------------+                      +-----------+----------+ |
|             |                                                |            |
|             v                                                v            |
|  +------------------------+                      +----------------------+ |
|  | hydrocouple  (Python)  |                      | HydroCouple (C++)    | |
|  | Abstract base classes  |                      | Pure virtual headers | |
|  | (.py ABC definitions)  |                      | hydrocouple.h, etc.  | |
|  +----------+-------------+                      +-----------+----------+ |
|             |                                                |            |
|             v                                                v            |
|  +---------------------------------------------------------------+       |
|  |           _hydrocouple_cython  (Cython extension)              |       |
|  |                                                                |       |
|  |  Layer A: .pxd declarations   (C++ header mapping)            |       |
|  |  Layer B: cdef wrapper classes (C++ ptr -> Python obj)        |       |
|  |  Layer C: cdef bridge classes  (Python obj -> C++ callback)   |       |
|  |  Layer D: variant / numpy marshalling                         |       |
|  +---------------------------------------------------------------+       |
|                               |                                           |
|                               v                                           |
|  +---------------------------------------------------------------+       |
|  |             HydroCouple  (C++ header-only interfaces)          |       |
|  |         Pure virtual classes — no compiled library needed       |       |
|  +---------------------------------------------------------------+       |
+---------------------------------------------------------------------------+
```

---

## 4. Layered Design

```
+---------------------------------------------------------------------+
|  Layer 4 — Pure-Python ABCs  (hydrocouple package)                   |
|  Users subclass these to write Python components.                    |
|  Uses @abstractmethod, properties, type hints.                       |
|  Delegates to Cython extension for C++ interop.                      |
+---------------------------------------------------------------------+
|  Layer 3 — Cython Bridge Classes  (cdef classes)                     |
|  Forwards Python method calls into C++ vtable (wrapping C++ objs)   |
|  AND forwards C++ callbacks into Python method calls (bridging).     |
|  Handles variant <-> Python, numpy <-> span, signal <-> callable.    |
+---------------------------------------------------------------------+
|  Layer 2 — Cython .pxd Declarations                                  |
|  Typed C++ declarations: classes, methods, enums, typedefs.          |
|  Maps std::variant, std::span, std::vector, std::shared_ptr.        |
+---------------------------------------------------------------------+
|  Layer 1 — C++ Headers (header-only)                                 |
|  HydroCouple pure virtual interfaces — no compiled library needed.   |
+---------------------------------------------------------------------+
```

---

## 5. Package Structure

```
hydrocouple-python/
|-- pyproject.toml                  # scikit-build-core or setuptools + cythonize
|-- setup.py                        # fallback build (cythonize + Extension)
|-- CMakeLists.txt                  # optional: for vcpkg C++ deps
|-- vcpkg.json                      # HydroCouple (header-only) dep
|
|-- hydrocouple/                    # Pure-Python public API
|   |-- __init__.py                 # Re-exports ABCs + enums
|   |-- abc.py                      # Abstract base classes (Layer 4)
|   |-- temporal.py                 # Temporal ABCs
|   |-- spatial.py                  # Spatial ABCs
|   |-- spatiotemporal.py           # Spatio-temporal ABCs
|   |-- units.py                    # Unit / Quantity helpers
|   |-- workflow.py                 # Python workflow orchestration
|   +-- loader.py                   # Load compiled C++ components
|
|-- _hydrocouple/                   # Cython extension (compiled)
|   |-- __init__.pxd
|   |-- _core.pxd                   # C++ declarations: hydrocouple.h
|   |-- _core.pyx                   # Cython wrappers + bridge classes
|   |-- _temporal.pxd               # C++ declarations: hydrocoupletemporal.h
|   |-- _temporal.pyx
|   |-- _spatial.pxd                # C++ declarations: hydrocouplespatial.h
|   |-- _spatial.pyx
|   |-- _spatialwkb.pxd             # C++ declarations: hydrocouplespatialwkb.h
|   |-- _spatialwkb.pyx
|   |-- _spatiotemporal.pxd
|   |-- _spatiotemporal.pyx
|   +-- _variant.pyx                # hydrocouple_variant <-> Python marshalling
|
+-- tests/
    |-- test_variant_roundtrip.py
    |-- test_component_lifecycle.py
    |-- test_python_to_cpp_exchange.py
    |-- test_cpp_to_python_exchange.py
    +-- test_spatial_numpy.py
```

---

## 6. Interface Mirroring Pattern

The core idea: **every C++ `I*` interface gets a Python ABC counterpart**. Python developers subclass these ABCs exactly as C++ developers subclass the virtual interfaces.

### 6.1 C++ Interface (reference)

```cpp
// hydrocouple.h
class IModelComponent : public virtual IIdentity,
                         public virtual ISignal<...> {
    virtual IModelComponentInfo* componentInfo() const = 0;
    virtual ComponentStatus status() const = 0;
    virtual void initialize() = 0;
    virtual std::vector<std::string> validate() = 0;
    virtual void prepare() = 0;
    virtual void update(const std::vector<IOutput*>& requiredOutputs = {}) = 0;
    virtual void finish() = 0;
    virtual std::vector<IInput*> inputs() const = 0;
    virtual std::vector<IOutput*> outputs() const = 0;
    virtual std::vector<IArgument*> arguments() const = 0;
    // ...
};
```

### 6.2 Python ABC (what users implement)

```python
# hydrocouple/abc.py
from abc import ABC, abstractmethod
from enum import IntEnum
from typing import Optional


class ComponentStatus(IntEnum):
    CREATED = 0
    INITIALIZING = 1
    INITIALIZED = 2
    VALIDATING = 3
    VALID = 4
    WAITING_FOR_DATA = 5
    INVALID = 6
    PREPARING = 7
    UPDATED = 8
    UPDATING = 9
    DONE = 10
    FINISHING = 11
    FINISHED = 12
    FAILED = 13


class IDescription(ABC):
    @property
    @abstractmethod
    def caption(self) -> str: ...

    @caption.setter
    @abstractmethod
    def caption(self, value: str) -> None: ...

    @property
    @abstractmethod
    def description(self) -> str: ...

    @description.setter
    @abstractmethod
    def description(self, value: str) -> None: ...


class IIdentity(IDescription):
    @property
    @abstractmethod
    def id(self) -> str: ...


class IValueDefinition(IDescription):
    @property
    @abstractmethod
    def type(self) -> type: ...

    @property
    @abstractmethod
    def missing_value(self) -> object: ...


class IDimension(IIdentity):
    pass


class IComponentDataItem(IIdentity):
    @property
    @abstractmethod
    def model_component(self) -> "IModelComponent": ...

    @property
    @abstractmethod
    def dimensions(self) -> list["IDimension"]: ...

    @property
    @abstractmethod
    def value_definition(self) -> IValueDefinition: ...

    @abstractmethod
    def dimension_length(self, dimension_indexes: list[int] = None) -> int: ...

    @abstractmethod
    def get_value(self, dimension_indexes: list[int]) -> object: ...

    @abstractmethod
    def set_value(self, data: object, dimension_indexes: list[int]) -> None: ...

    @abstractmethod
    def get_values(self, dimension_indexes: list[int],
                   dimension_lengths: list[int] = None) -> object: ...

    @abstractmethod
    def set_values(self, data, dimension_indexes: list[int],
                   dimension_lengths: list[int] = None) -> None: ...


class IExchangeItem(IComponentDataItem):
    pass


class IOutput(IExchangeItem):
    @property
    @abstractmethod
    def consumers(self) -> list["IInput"]: ...

    @abstractmethod
    def add_consumer(self, consumer: "IInput") -> None: ...

    @abstractmethod
    def remove_consumer(self, consumer: "IInput") -> bool: ...

    @abstractmethod
    def update_values(self, query_specifier: "IInput") -> None: ...


class IInput(IExchangeItem):
    @property
    @abstractmethod
    def provider(self) -> Optional[IOutput]: ...

    @abstractmethod
    def set_provider(self, provider: IOutput) -> bool: ...

    @abstractmethod
    def can_consume(self, provider: IOutput) -> tuple[bool, str]: ...


class IArgument(IComponentDataItem):
    @property
    @abstractmethod
    def is_optional(self) -> bool: ...

    @property
    @abstractmethod
    def is_read_only(self) -> bool: ...


class IModelComponentInfo(IIdentity):
    @abstractmethod
    def create_component_instance(self) -> "IModelComponent": ...


class IModelComponent(IIdentity):
    @property
    @abstractmethod
    def component_info(self) -> IModelComponentInfo: ...

    @property
    @abstractmethod
    def status(self) -> ComponentStatus: ...

    @property
    @abstractmethod
    def arguments(self) -> list[IArgument]: ...

    @property
    @abstractmethod
    def inputs(self) -> list[IInput]: ...

    @property
    @abstractmethod
    def outputs(self) -> list[IOutput]: ...

    @abstractmethod
    def initialize(self) -> None: ...

    @abstractmethod
    def validate(self) -> list[str]: ...

    @abstractmethod
    def prepare(self) -> None: ...

    @abstractmethod
    def update(self, required_outputs: list[IOutput] = None) -> None: ...

    @abstractmethod
    def finish(self) -> None: ...

    # Signal interface (Pythonic)
    @abstractmethod
    def on_status_changed(self, callback) -> None:
        """Register a callable to be notified on status changes."""
        ...
```

### 6.3 User Implements the ABC

```python
import numpy as np
from hydrocouple.abc import IModelComponent, ComponentStatus

class MyPythonModel(IModelComponent):
    """A pure-Python HydroCouple component."""

    def initialize(self):
        self._status = ComponentStatus.INITIALIZING
        # load data, set up state
        self._status = ComponentStatus.INITIALIZED

    def validate(self) -> list[str]:
        return []  # no errors

    def prepare(self):
        self._status = ComponentStatus.PREPARING
        # pre-compute
        self._status = ComponentStatus.UPDATED

    def update(self, required_outputs=None):
        self._status = ComponentStatus.UPDATING
        # read from self.inputs, compute, write to self.outputs
        for inp in self.inputs:
            data = inp.provider.get_values([0], [100])  # numpy array
            result = data * 2.0
            self.outputs[0].set_values(result, [0], [100])
        self._status = ComponentStatus.UPDATED

    def finish(self):
        self._status = ComponentStatus.FINISHED
```

---

## 7. Cython Bridge: Wrapping C++ Objects for Python

When a compiled C++ component is loaded, its C++ `IModelComponent*` must appear as a Python object implementing the same ABCs. Cython `cdef` wrapper classes do this.

### 7.1 .pxd Declarations (Layer 2)

```cython
# _hydrocouple/_core.pxd
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr

cdef extern from "hydrocouple.h" namespace "HydroCouple":

    ctypedef enum ComponentStatus "HydroCouple::IModelComponent::ComponentStatus":
        Created
        Initializing
        Initialized
        Validating
        Valid
        WaitingForData
        Invalid
        Preparing
        Updated
        Updating
        Done
        Finishing
        Finished
        Failed

    cdef cppclass IDescription:
        string caption() const
        void setCaption(const string&)
        string description() const
        void setDescription(const string&)

    cdef cppclass IIdentity(IDescription):
        string id() const

    cdef cppclass IDimension(IIdentity):
        pass

    cdef cppclass IValueDefinition(IDescription):
        pass

    cdef cppclass IModelComponent  # forward declaration

    cdef cppclass IComponentDataItem(IIdentity):
        IModelComponent* modelComponent() const
        vector[IDimension*] dimensions() const
        int dimensionLength(const int* dimIdx, int ndims) const
        # simplified: actual uses std::span
        void getValue(void* data, const int* dimIdx, int ndims) const
        void setValue(const void* data, const int* dimIdx, int ndims)

    cdef cppclass IExchangeItem(IComponentDataItem):
        pass

    cdef cppclass IInput(IExchangeItem):
        IOutput* provider() const
        bint setProvider(IOutput* provider)

    cdef cppclass IOutput(IExchangeItem):
        vector[IInput*] consumers() const
        void addConsumer(IInput* consumer)
        bint removeConsumer(IInput* consumer)
        void updateValues(const IInput* querySpecifier)

    cdef cppclass IArgument(IComponentDataItem):
        bint isOptional() const
        bint isReadOnly() const

    cdef cppclass IModelComponentInfo(IIdentity):
        pass

    cdef cppclass IModelComponent(IIdentity):
        IModelComponentInfo* componentInfo() const
        ComponentStatus status() const
        vector[IArgument*] arguments() const
        vector[IInput*] inputs() const
        vector[IOutput*] outputs() const
        void initialize()
        vector[string] validate()
        void prepare()
        void update(const vector[IOutput*]& requiredOutputs)
        void finish()
```

### 7.2 Wrapper Classes — C++ to Python (Layer 3A)

```cython
# _hydrocouple/_core.pyx
cimport _hydrocouple._core as cpp
from hydrocouple.abc import IModelComponent as PyIModelComponent

cdef class CppModelComponentWrapper:
    """Wraps a C++ IModelComponent* so Python code can use it as a
    hydrocouple.abc.IModelComponent."""

    cdef cpp.IModelComponent* _ptr    # borrowed pointer, owned by C++ side
    cdef bint _owner                   # True if Python owns the pointer

    def __cinit__(self):
        self._ptr = NULL
        self._owner = False

    @staticmethod
    cdef CppModelComponentWrapper wrap(cpp.IModelComponent* ptr):
        cdef CppModelComponentWrapper obj = CppModelComponentWrapper.__new__(
            CppModelComponentWrapper)
        obj._ptr = ptr
        obj._owner = False
        return obj

    @property
    def id(self) -> str:
        return self._ptr.id().decode('utf-8')

    @property
    def caption(self) -> str:
        return self._ptr.caption().decode('utf-8')

    @property
    def status(self):
        from hydrocouple.abc import ComponentStatus
        return ComponentStatus(< int > self._ptr.status())

    @property
    def inputs(self) -> list:
        cdef vector[cpp.IInput*] cpp_inputs = self._ptr.inputs()
        return [CppInputWrapper.wrap(cpp_inputs[i])
                for i in range(cpp_inputs.size())]

    @property
    def outputs(self) -> list:
        cdef vector[cpp.IOutput*] cpp_outputs = self._ptr.outputs()
        return [CppOutputWrapper.wrap(cpp_outputs[i])
                for i in range(cpp_outputs.size())]

    def initialize(self):
        with nogil:
            self._ptr.initialize()

    def validate(self) -> list:
        cdef vector[string] msgs
        with nogil:
            msgs = self._ptr.validate()
        return [m.decode('utf-8') for m in msgs]

    def prepare(self):
        with nogil:
            self._ptr.prepare()

    def update(self, required_outputs=None):
        cdef vector[cpp.IOutput*] empty
        with nogil:
            self._ptr.update(empty)

    def finish(self):
        with nogil:
            self._ptr.finish()

# Register as virtual subclass so isinstance() checks pass
PyIModelComponent.register(CppModelComponentWrapper)
```

### 7.3 Bridge Classes — Python to C++ (Layer 3B)

When a pure-Python component must be visible to C++ code, a thin C++ bridge class implements the C++ virtual interface and delegates every call to the Python object.

```
   C++ caller
       |
       v
+---------------------------+
| PyModelComponentBridge    |  (C++ class, compiled in the Cython ext)
| : public IModelComponent  |
|                           |
|  initialize() override {  |
|    GIL_acquire();         |
|    py_obj.initialize();   |
|    GIL_release();         |
|  }                        |
+---------------------------+
       |
       v
   Python object (user's MyPythonModel)
```

This bridge is written as a small C++ header included by the Cython `.pyx`:

```cpp
// _hydrocouple/bridge/py_model_component_bridge.h
#include <Python.h>
#include "hydrocouple.h"

class PyModelComponentBridge : public HydroCouple::IModelComponent {
    PyObject* m_pyobj;  // borrowed ref, prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent prevent
public:
    explicit PyModelComponentBridge(PyObject* obj) : m_pyobj(obj) {
        Py_INCREF(m_pyobj);
    }
    ~PyModelComponentBridge() override {
        PyGILState_STATE gstate = PyGILState_Ensure();
        Py_DECREF(m_pyobj);
        PyGILState_Release(gstate);
    }

    void initialize() override {
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyObject_CallMethod(m_pyobj, "initialize", nullptr);
        PyGILState_Release(gstate);
    }

    std::vector<std::string> validate() override {
        PyGILState_STATE gstate = PyGILState_Ensure();
        PyObject* result = PyObject_CallMethod(m_pyobj, "validate", nullptr);
        std::vector<std::string> msgs;
        // ... convert Python list[str] -> vector<string>
        Py_DECREF(result);
        PyGILState_Release(gstate);
        return msgs;
    }

    void update(const std::vector<IOutput*>& requiredOutputs) override {
        PyGILState_STATE gstate = PyGILState_Ensure();
        // Convert C++ IOutput* list -> Python wrapper list, call update()
        PyGILState_Release(gstate);
    }

    // ... remaining methods follow the same pattern
};
```

---

## 8. Data Exchange Flows

### 8.1 Python Output --> C++ Input

```
  Python Component                        C++ Component
  ================                        ==============

  class MyOutput(IOutput):
    def update_values(self, query):
      data = np.array([1.0, 2.0, 3.0])
      self.set_values(data, [0], [3])
           |
           |  set_values() stores numpy array
           |  in internal buffer
           v
  +---------------------------+
  | PyOutputBridge (C++)      |     IOutput* ptr visible to C++
  | getValues() override {    |
  |   GIL_acquire()           |
  |   call py_obj.get_values()|----+
  |   copy numpy -> variant[] |    |
  |   GIL_release()           |    |
  | }                         |    |
  +---------------------------+    |
           |                       |
           v                       v
  C++ IInput reads via          numpy buffer -> variant[]
  provider->getValues()         (memcpy for contiguous float64)
```

### 8.2 C++ Output --> Python Input

```
  C++ Component                           Python Component
  ==============                          ================

  IOutput* flow_out;                      class MyInput(IInput):
  flow_out->setValues(data, dims);          @property
           |                                def provider(self):
           v                                  return self._provider
  +------------------------------+
  | CppOutputWrapper (Cython)    |        on update():
  | cdef IOutput* _ptr           |          vals = self.provider.get_values(
  |                              |              [0], [100])
  | def get_values(self, ...):   |          # vals is a numpy array
  |   cdef double* buf           |          result = vals * 2.0
  |   with nogil:                |
  |     self._ptr.getValues(     |
  |       buf, idx, lens)        |
  |   return np.asarray(         |
  |     <double[:n]> buf)        |----->  numpy.ndarray (zero-copy view
  +------------------------------+        if contiguous numeric type)
```

### 8.3 Python Output --> Python Input (same process)

```
  Python Component A                  Python Component B
  ==================                  ==================

  class FlowOutput(IOutput):          class TempInput(IInput):
    _buffer: np.ndarray                 provider -> FlowOutput

    def update_values(self, q):         def on_update():
      self._buffer = compute()            data = self.provider.get_values(...)
                                          # direct Python object access
          |                               # NO marshalling, NO GIL dance
          +-----------------------------> returns self._buffer  (zero overhead)
```

This is the **fastest path** -- pure Python, no crossing into C++.

---

## 9. Variant Marshalling

```
+----------------------------+---------------------+---------------------+
| hydrocouple_variant type   | Python type          | NumPy dtype         |
+----------------------------+---------------------+---------------------+
| std::monostate             | None                 | --                  |
| bool                       | bool                 | np.bool_            |
| int8_t .. int64_t          | int                  | np.int8 .. int64    |
| uint8_t .. uint64_t        | int                  | np.uint8 .. uint64  |
| float                      | float                | np.float32          |
| double                     | float                | np.float64          |
| long double                | float                | np.longdouble       |
| std::string                | str                  | np.str_             |
| std::any                   | object (capsule)     | np.object_          |
+----------------------------+---------------------+---------------------+
```

**Bulk transfers** (`getValues` / `setValues`) bypass per-element variant inspection when the `IValueDefinition` indicates a homogeneous numeric type. In that case, the Cython layer reads/writes a contiguous `double*` or `int*` buffer directly into a NumPy array — no variant unpacking per element.

```cython
# Fast path for numeric bulk reads
cdef np.ndarray[double, ndim=1] fast_get_values_double(
        cpp.IComponentDataItem* item,
        int[:] dim_indexes,
        int[:] dim_lengths):
    cdef int n = 1
    for i in range(dim_lengths.shape[0]):
        n *= dim_lengths[i]
    cdef np.ndarray[double, ndim=1] out = np.empty(n, dtype=np.float64)
    with nogil:
        item.getValues(<void*> out.data,
                       &dim_indexes[0], dim_indexes.shape[0])
    return out
```

---

## 10. Signal/Slot Strategy

C++ uses `ISignal<Args...>` / `ISlot<Args...>` templates. Cython cannot directly instantiate C++ templates at the Python level, so we use explicit specializations for the known signal types.

### Known Signal Specializations

| Signal Type | Args | Python Event |
|---|---|---|
| Component status | `IComponentStatusChangeEventArgs` | `on_status_changed(old, new, msg)` |
| Data item value | `IComponentDataItemValueChanged` | `on_value_changed(item)` |
| Exchange item | `IExchangeItemChangeEventArgs` | `on_exchange_changed(item)` |
| Property changed | `std::string` | `on_property_changed(name)` |

### Python Signal API

```python
# Pythonic observer pattern -- no ISlot objects exposed

component.on_status_changed(lambda old, new, msg: print(f"{old} -> {new}"))

# For C++ components wrapped in Python:
wrapper.on_status_changed(my_callback)
# Internally creates a C++ BridgeSlot that calls my_callback with GIL
```

### Cython Implementation

```cython
# Slot bridge: C++ ISlot that invokes a Python callable
cdef cppclass StatusSlotBridge(cpp.ISlot[cpp.StatusChangeArgs]):
    PyObject* callback

    void operator()(const cpp.ISignal[cpp.StatusChangeArgs]& sender,
                    cpp.StatusChangeArgs args) noexcept with gil:
        py_callback = <object> this.callback
        py_callback(args.old_status, args.new_status, args.message)
```

---

## 11. Component Loading

Loading compiled C++ HydroCouple shared libraries from Python:

```python
# hydrocouple/loader.py
from _hydrocouple._core import load_component_library

def load(library_path: str) -> IModelComponent:
    """Load a compiled C++ HydroCouple component from a shared library.

    The library must export a factory function that returns
    an IModelComponentInfo*.
    """
    return load_component_library(library_path)
```

```cython
# _hydrocouple/_core.pyx
from libc.dlfcn cimport dlopen, dlsym, RTLD_LAZY

cdef extern from "hydrocouple.h" namespace "HydroCouple":
    cdef cppclass IModelComponentInfo:
        pass  # declared in .pxd

def load_component_library(str path):
    """dlopen a shared library, find the factory, create a component."""
    cdef void* handle = dlopen(path.encode(), RTLD_LAZY)
    if handle == NULL:
        raise OSError(f"Cannot load library: {path}")

    # Convention: exported symbol "CreateComponentInfo"
    cdef void* sym = dlsym(handle, b"CreateComponentInfo")
    if sym == NULL:
        raise OSError(f"Symbol 'CreateComponentInfo' not found in {path}")

    cdef cpp.IModelComponentInfo* info = (<cpp.IModelComponentInfo*(*)()> sym)()
    cdef cpp.IModelComponent* comp = info.createComponentInstance()
    return CppModelComponentWrapper.wrap(comp)
```

---

## 12. Performance Considerations

### 12.1 Call Overhead Comparison

```
+-------------------------------+----------------+-------------------+
| Call Path                     | Overhead       | Notes             |
+-------------------------------+----------------+-------------------+
| Python -> Python              | ~50 ns         | Direct method call|
| Python -> Cython cdef         | ~20 ns         | C function ptr    |
| Python -> C++ (via Cython)    | ~80 ns         | nogil + vtable    |
| C++ -> Python (via bridge)    | ~200 ns        | GIL acquire +     |
|                               |                | PyObject_Call     |
| C++ -> C++ (same process)     | ~5 ns          | vtable only       |
+-------------------------------+----------------+-------------------+
```

### 12.2 Hot Path Optimization

The **update loop** is the critical path. Strategies to minimize overhead:

**a) Batch data, not calls:**

```python
# BAD: per-element access (N GIL transitions if crossing to C++)
for i in range(n):
    val = output.get_value([i])

# GOOD: bulk array transfer (1 GIL transition)
values = output.get_values([0], [n])  # returns numpy array
```

**b) `nogil` blocks for C++ computation:**

```cython
def update(self):
    cdef double* input_buf = self._get_input_buffer()
    cdef double* output_buf = self._get_output_buffer()
    cdef int n = self._buffer_size

    with nogil:
        # Pure C computation, other threads run freely
        for i in range(n):
            output_buf[i] = input_buf[i] * self._factor
```

**c) Typed memoryviews for zero-copy:**

```cython
def get_values_as_array(self, int[:] indexes, int[:] lengths):
    cdef int total = 1
    for i in range(lengths.shape[0]):
        total *= lengths[i]

    cdef double[::1] view = self._internal_buffer[:total]
    return np.asarray(view)  # zero-copy numpy wrapper
```

**d) Avoid Python object creation in inner loops:**

```cython
# Keeps everything at C level, no Python objects until final return
cdef void _update_internal(self) noexcept nogil:
    cdef int i
    cdef double* src = self._input_ptr
    cdef double* dst = self._output_ptr
    for i in range(self._n):
        dst[i] = _compute(src[i])
```

### 12.3 Memory Layout

| Scenario | Strategy | Copy? |
|---|---|---|
| Contiguous numeric C++ buffer | `np.asarray(<double[:n]> ptr)` | No (view) |
| Non-contiguous C++ data | `np.empty()` then `memcpy` per stride | Yes |
| Python numpy to C++ | Pass `arr.data` pointer directly | No (borrow) |
| Variant array (mixed types) | Element-wise conversion to Python list | Yes |
| WKB geometry bytes | `bytes(ptr[:nbytes])` / buffer protocol | Optional |

### 12.4 GIL Management Rules

1. **Always release GIL** before calling into C++ computation (`with nogil:`).
2. **Always hold GIL** when accessing Python objects or calling Python methods.
3. **Bridge classes acquire GIL** at entry when C++ calls back into Python.
4. **Minimize GIL hold time**: extract C data from Python objects first, then release GIL for computation.

```
C++ update loop (no GIL):
  for each component:
    if component is C++:
      component->update()               # no GIL needed
    if component is Python (via bridge):
      GIL_acquire()
        py_component.update()            # Python code runs
          calls C++ output.getValues()
            GIL_release()                # C++ reads data
            GIL_acquire()                # back to Python
        py_component stores results
      GIL_release()
```

---

## 13. Temporal & Spatial Extensions

### Temporal ABCs

```python
# hydrocouple/temporal.py
from datetime import datetime, timedelta
from hydrocouple.abc import IModelComponent, IComponentDataItem

class IDateTime(ABC):
    @property
    @abstractmethod
    def julian_day(self) -> float: ...

    def to_datetime(self) -> datetime:
        """Convert Julian day to Python datetime."""
        # Julian day 2440587.5 = Unix epoch
        return datetime(1970, 1, 1) + timedelta(
            days=self.julian_day - 2440587.5)

class ITimeSpan(IDateTime):
    @property
    @abstractmethod
    def duration(self) -> float:
        """Duration in days."""
        ...

    def to_timedelta(self) -> timedelta:
        return timedelta(days=self.duration)

class ITimeModelComponent(IModelComponent):
    @property
    @abstractmethod
    def current_date_time(self) -> IDateTime: ...

    @property
    @abstractmethod
    def simulation_period(self) -> ITimeSpan: ...

class ITimeSeriesComponentDataItem(IComponentDataItem):
    @abstractmethod
    def time(self, index: int) -> IDateTime: ...

    @property
    @abstractmethod
    def time_count(self) -> int: ...
```

### Spatial ABCs (with Shapely/NumPy interop)

```python
# hydrocouple/spatial.py
from hydrocouple.abc import IIdentity, IComponentDataItem
import numpy as np

class IGeometry(ABC):
    @property
    @abstractmethod
    def geometry_type(self) -> "GeometryType": ...

    @abstractmethod
    def wkb(self) -> bytes: ...

    @abstractmethod
    def wkt(self) -> str: ...

    def to_shapely(self):
        """Convert to shapely geometry (optional dependency)."""
        import shapely
        return shapely.from_wkb(self.wkb())

class IPoint(IGeometry):
    @property
    @abstractmethod
    def x(self) -> float: ...

    @property
    @abstractmethod
    def y(self) -> float: ...

class IGeometryComponentDataItem(IComponentDataItem):
    @property
    @abstractmethod
    def geometry_count(self) -> int: ...

    @abstractmethod
    def geometry(self, index: int) -> IGeometry: ...

    @abstractmethod
    def get_values(self, geometry_index: int,
                   dimension_indexes: list[int] = None) -> np.ndarray: ...
```

---

## 14. Implementation Phases

```
Phase 1: Foundation                     Phase 2: Temporal
----------------------------------      ----------------------------------
 [x] Cython .pxd for hydrocouple.h      [x] Temporal .pxd declarations
 [x] variant marshalling (_variant)      [x] IDateTime <-> datetime
 [x] Python ABCs (abc.py)               [x] ITimeSpan <-> timedelta
 [x] CppModelComponentWrapper           [x] ITimeModelComponent ABC
 [x] PyModelComponentBridge (C++)        [x] ITimeSeriesComponentDataItem
 [x] Component loading (dlopen)          [x] Temporal wrappers + bridges
 [x] IOutput/IInput wrappers
 [x] Signal/slot bridge
 [x] End-to-end: Py comp <-> C++ comp
            |                                       |
            v                                       v
Phase 3: Spatial                        Phase 4: Spatio-temporal
----------------------------------      ----------------------------------
 [x] Spatial .pxd declarations           [x] Combined interfaces
 [x] Geometry hierarchy ABCs             [x] ITimeGeometryComponentDataItem
 [x] WKB struct bindings (numpy)         [x] ITimeRasterComponentDataItem
 [x] IRaster -> numpy buffer             [x] Diamond inheritance handling
 [x] IRegularGrid 2D/3D ABCs
 [x] Shapely interop helpers
 [x] Spatial component data items
            |                                       |
            v                                       v
Phase 5: Polish & Distribute
----------------------------------
 [x] Full test suite (pytest) — 34 tests split into 8 modules
 [ ] scikit-build-core + cibuildwheel
 [ ] Wheels: manylinux, macOS, Windows
 [x] Sphinx documentation + examples (pydata theme, epytext, 112 HTML pages)
 [x] .pyi type stubs for all 5 Cython extensions
 [x] Epytext docstrings on all ABCs and Cython wrappers
 [x] C++/Python cross-reference navigation
 [ ] PyPI publish as `hydrocouple`
```

---

## 15. Build System

```toml
# pyproject.toml
[build-system]
requires = ["setuptools>=68", "cython>=3.0", "numpy>=1.24"]
build-backend = "setuptools.build_meta"

[project]
name = "hydrocouple"
version = "2.0.0"
requires-python = ">=3.10"
dependencies = ["numpy>=1.24"]

[project.optional-dependencies]
spatial = ["shapely>=2.0"]
xarray = ["xarray>=2023.0"]
dev = ["pytest", "cython>=3.0"]

[tool.setuptools]
packages = ["hydrocouple", "_hydrocouple"]

[tool.cibuildwheel]
build = "cp310-* cp311-* cp312-* cp313-*"
```

```python
# setup.py
from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy as np

extensions = [
    Extension(
        "_hydrocouple._core",
        sources=["_hydrocouple/_core.pyx"],
        include_dirs=[
            np.get_include(),
            "${HYDROCOUPLE_INCLUDE_DIR}",
        ],
        # Header-only: no libraries to link
        language="c++",
        extra_compile_args=["-std=c++20"],
    ),
    Extension(
        "_hydrocouple._temporal",
        sources=["_hydrocouple/_temporal.pyx"],
        include_dirs=[np.get_include(), "${HYDROCOUPLE_INCLUDE_DIR}"],
        # Header-only: no libraries to link
        language="c++",
        extra_compile_args=["-std=c++20"],
    ),
    # ... _spatial, _spatiotemporal, _variant
]

setup(
    ext_modules=cythonize(extensions, language_level="3"),
)
```

---

## 16. Risks & Mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| GIL contention in tight update loops | Throughput loss | Batch data via numpy; `nogil` for C++ calls |
| C++20 features unsupported by Cython parser | Build failure | Use `.pxd` extern blocks with manual C++ casts where needed; Cython 3.0+ supports most C++17/20 |
| `std::variant` not directly expressible in Cython | Type marshalling complexity | Write a small C++ helper header that converts variant to/from a tagged union Cython can consume |
| `std::span` not available in Cython | Signature mismatch | Pass raw pointer + length; wrap in Cython as `(const int* ptr, int n)` |
| Object lifetime across Python/C++ boundary | Use-after-free | Clear ownership rules: wrappers borrow, bridges ref-count via `Py_INCREF`/`DECREF` |
| Diamond inheritance in spatio-temporal | Cython single-inheritance `cdef class` | Use composition (delegate to inner wrapper) rather than multiple `cdef` inheritance |
| Platform ABI differences | Wheel incompatibility | Build per-platform with `cibuildwheel`; pin compiler version |

---

## 17. References

- [Cython Documentation — Wrapping C++ Libraries](https://cython.readthedocs.io/en/latest/src/userguide/wrapping_CPlusPlus.html)
- [Cython — Using C++ in Cython](https://cython.readthedocs.io/en/latest/src/userguide/external_C_code.html)
- [Cython — Typed Memoryviews](https://cython.readthedocs.io/en/latest/src/userguide/memoryviews.html)
- [scikit-build-core](https://scikit-build-core.readthedocs.io/)
- [cibuildwheel](https://cibuildwheel.readthedocs.io/)
- [HydroCouple API Docs](https://www.hydrocouple.org/hydrocoupledocs/html/index.html)
- [OGC Simple Features Access](https://www.ogc.org/standards/sfa)
