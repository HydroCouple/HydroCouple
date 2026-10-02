# distutils: language = c++
# cython: language_level = 3
"""
Cython wrapper classes for the core HydroCouple v2.0.0 C++ interfaces.

Each ``Cpp*Wrapper`` class holds a borrowed C++ pointer and exposes a
Pythonic API. After construction the wrappers are registered as virtual
subclasses of the corresponding Python ABCs so that ``isinstance`` checks
work transparently.

Data plane: ``get_values_into`` / ``set_values_from`` convert NumPy arrays
— and, through the DLPack protocol, any array that implements
``__dlpack__`` (PyTorch, JAX, CuPy, ...) on any device — to C++
``BufferDescriptor`` views **zero-copy**: the descriptor carries the
array's data pointer, DataKind (from dtype), shape, byte strides and memory
space, and the GIL is released around the C++ virtual call.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.set cimport set as cppset
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.typeinfo cimport type_info
from libc.stdint cimport int32_t, int64_t, uint64_t, uintptr_t
from cpython.ref cimport PyObject, Py_DECREF

cimport _hydrocouple._core as cpp
cimport _hydrocouple._signal as bridge
cimport _hydrocouple._dlpack as dl

import sys

import numpy as np
cimport numpy as cnp

cnp.import_array()


# ---------------------------------------------------------------------------
# ndarray <-> BufferDescriptor marshalling (zero-copy)
# ---------------------------------------------------------------------------

cdef object _DATA_KIND_OF = None


cdef cpp.DataKind _kind_of_dtype(object dtype) except *:
    """Map a NumPy dtype to a DataKind via hydrocouple.helpers."""
    global _DATA_KIND_OF
    if _DATA_KIND_OF is None:
        from hydrocouple.helpers import data_kind_of
        _DATA_KIND_OF = data_kind_of
    return <cpp.DataKind><int>_DATA_KIND_OF(dtype)


cdef int _fill_descriptor(cnp.ndarray arr,
                          cpp.BufferDescriptor* d,
                          vector[int64_t]* shape_buf,
                          vector[int64_t]* strides_buf,
                          bint writable) except -1:
    """Populate a BufferDescriptor as a zero-copy view of ``arr``.

    ``shape_buf``/``strides_buf`` own the shape/stride storage and must
    outlive the descriptor use.
    """
    if writable and not cnp.PyArray_ISWRITEABLE(arr):
        raise ValueError("destination array is not writable")
    cdef int nd = cnp.PyArray_NDIM(arr)
    cdef cnp.npy_intp* dims = cnp.PyArray_DIMS(arr)
    cdef cnp.npy_intp* strides = cnp.PyArray_STRIDES(arr)
    shape_buf.clear()
    strides_buf.clear()
    cdef int k
    for k in range(nd):
        shape_buf.push_back(<int64_t>dims[k])
        strides_buf.push_back(<int64_t>strides[k])
    d.data = cnp.PyArray_DATA(arr)
    d.kind = _kind_of_dtype(arr.dtype)
    d.rank = nd
    d.shape = shape_buf.data() if nd > 0 else NULL
    d.stridesBytes = strides_buf.data() if nd > 0 else NULL
    d.space = cpp.MemorySpace.Host
    d.deviceId = 0
    return 0


cdef int fill_host_descriptor(object array, cpp.BufferDescriptor* descriptor,
                              vector[int64_t]* shape_buf,
                              vector[int64_t]* strides_buf,
                              bint writable) except -1:
    """A host descriptor over an ndarray, for the other binding modules."""
    if type(array) is not np.ndarray:
        raise TypeError(f"expected a numpy.ndarray, got {type(array).__name__}")
    return _fill_descriptor(<cnp.ndarray>array, descriptor, shape_buf,
                            strides_buf, writable)


cdef int _fill_span_buf(object indices, vector[int64_t]* buf) except -1:
    """Copy a Python int sequence into an int64 vector for span use."""
    buf.clear()
    for v in indices:
        buf.push_back(<int64_t>v)
    return 0


cdef inline cpp.const_int64_span _as_span(vector[int64_t]* buf):
    if buf.size() == 0:
        return cpp.const_int64_span()
    return cpp.const_int64_span(buf.data(), buf.size())


# ---------------------------------------------------------------------------
# DLPack producer -> BufferDescriptor (zero-copy, any device)
# ---------------------------------------------------------------------------

cdef object _dlpack_capsule_of(object producer, object stream):
    """Ask a ``__dlpack__`` producer for a capsule, newest protocol first.

    ``stream`` is the consumer stream handed to the producer (DLPack /
    array-API semantics): ``None`` lets a CUDA/ROCm producer assume the
    legacy default stream; a CPU producer always receives ``None``.
    """
    get_device = getattr(producer, "__dlpack_device__", None)
    if get_device is not None:
        device_type = int(get_device()[0])
        if device_type == dl.kDLCPU:
            stream = None
    try:
        return producer.__dlpack__(
            stream=stream,
            max_version=(dl.DLPACK_MAJOR_VERSION, dl.DLPACK_MINOR_VERSION))
    except TypeError:
        # A pre-1.0 producer: no max_version keyword, legacy capsule.
        pass
    if stream is None:
        return producer.__dlpack__()
    return producer.__dlpack__(stream=stream)


cdef tuple _dlpack_data_plane(cpp.IComponentDataItem* ptr, object producer,
                              object start, object count, bint writable,
                              object stream):
    """Run getValuesInto (writable) or setValuesFrom over a DLPack tensor.

    The producer's tensor is borrowed for the duration of the C++ call and
    its deleter runs when this function returns — exactly once.
    """
    capsule = _dlpack_capsule_of(producer, stream)
    cdef dl.DLPackBorrow borrow
    cdef cpp.BufferDescriptor d
    cdef string msg
    if not dl.borrowCapsule(<PyObject*>capsule, borrow, msg):
        raise BufferError(msg.decode("utf-8"))
    if not dl.descriptorFromBorrow(borrow, writable, d, msg):
        raise ValueError(msg.decode("utf-8"))
    cdef vector[int64_t] start_buf, count_buf
    _fill_span_buf(start, &start_buf)
    _fill_span_buf(count, &count_buf)
    cdef bint ok
    cdef cpp.const_int64_span s = _as_span(&start_buf)
    cdef cpp.const_int64_span c = _as_span(&count_buf)
    msg.clear()
    if writable:
        with nogil:
            ok = ptr.getValuesInto(d, s, c, &msg)
    else:
        with nogil:
            ok = ptr.setValuesFrom(d, s, c, &msg)
    borrow.release()
    return bool(ok), msg.decode("utf-8")


cdef int _refuse_immutable_destination(object destination) except -1:
    """Refuse destinations whose framework promises they never change.

    A JAX array exports writable-looking DLPack capsules (no READ_ONLY
    flag), but JAX treats every array as immutable and may share its
    buffer between arrays; writing into one would silently change others.
    """
    jax = sys.modules.get("jax")
    if jax is not None and isinstance(destination, jax.Array):
        raise ValueError(
            "JAX arrays are immutable and cannot receive values; read into "
            "a NumPy array (or torch tensor) and pass it to jnp.asarray, or "
            "use hydrocouple.jax")
    return 0


cdef tuple _get_values_into(cpp.IComponentDataItem* ptr, object destination,
                            object start, object count, object stream=None):
    """Shared implementation of the typed hyperslab read."""
    if type(destination) is not np.ndarray:
        _refuse_immutable_destination(destination)
        if hasattr(destination, "__dlpack__"):
            return _dlpack_data_plane(ptr, destination, start, count,
                                      True, stream)
        raise TypeError("destination must be a numpy.ndarray or implement "
                        "the DLPack protocol (__dlpack__)")
    cdef cnp.ndarray arr = destination
    cdef cpp.BufferDescriptor d
    cdef vector[int64_t] shape_buf, strides_buf, start_buf, count_buf
    _fill_descriptor(arr, &d, &shape_buf, &strides_buf, True)
    _fill_span_buf(start, &start_buf)
    _fill_span_buf(count, &count_buf)
    cdef string msg
    cdef bint ok
    cdef cpp.const_int64_span s = _as_span(&start_buf)
    cdef cpp.const_int64_span c = _as_span(&count_buf)
    with nogil:
        ok = ptr.getValuesInto(d, s, c, &msg)
    return bool(ok), msg.decode("utf-8")


cdef tuple _set_values_from(cpp.IComponentDataItem* ptr, object source,
                            object start, object count, object stream=None):
    """Shared implementation of the typed hyperslab write."""
    if type(source) is not np.ndarray and hasattr(source, "__dlpack__"):
        return _dlpack_data_plane(ptr, source, start, count, False, stream)
    cdef cnp.ndarray arr = np.asarray(source)
    cdef cpp.BufferDescriptor d
    cdef vector[int64_t] shape_buf, strides_buf, start_buf, count_buf
    _fill_descriptor(arr, &d, &shape_buf, &strides_buf, False)
    _fill_span_buf(start, &start_buf)
    _fill_span_buf(count, &count_buf)
    cdef string msg
    cdef bint ok
    cdef cpp.const_int64_span s = _as_span(&start_buf)
    cdef cpp.const_int64_span c = _as_span(&count_buf)
    with nogil:
        ok = ptr.setValuesFrom(d, s, c, &msg)
    return bool(ok), msg.decode("utf-8")


# ---------------------------------------------------------------------------
# BufferDescriptor -> DLPack capsule (the producer side), and the mapping
# primitives hydrocouple.dlpack re-exports. One implementation, in
# dlpack_bridge.h; Python never re-derives a mapping.
# ---------------------------------------------------------------------------

def _make_capsule(uintptr_t address, int kind, shape, strides_bytes,
                  int device_type, int device_id, bint read_only,
                  bint versioned, uint64_t byte_offset=0):
    """A DLPack capsule over foreign memory, without copying it.

    ``strides_bytes`` may be ``None`` (C-contiguous). The memory is
    borrowed: the caller guarantees it outlives every tensor made from the
    capsule. Raises ``BufferError`` if the descriptor cannot be expressed.
    """
    cdef vector[int64_t] shape_buf, strides_buf
    _fill_span_buf(shape, &shape_buf)
    cdef cpp.BufferDescriptor d
    d.data = <void*>address
    d.kind = <cpp.DataKind>kind
    d.rank = <int32_t>shape_buf.size()
    d.shape = shape_buf.data() if shape_buf.size() > 0 else NULL
    if strides_bytes is None:
        d.stridesBytes = NULL
    else:
        _fill_span_buf(strides_bytes, &strides_buf)
        if strides_buf.size() != shape_buf.size():
            raise ValueError("strides_bytes and shape differ in length")
        d.stridesBytes = strides_buf.data() if strides_buf.size() > 0 else NULL
    cdef dl.DLDevice device
    device.device_type = <dl.DLDeviceType>device_type
    device.device_id = device_id
    cdef string msg
    cdef PyObject* capsule = dl.makeCapsule(d, device, read_only, versioned,
                                            msg, byte_offset)
    if capsule == NULL:
        raise BufferError(msg.decode("utf-8"))
    result = <object>capsule
    Py_DECREF(result)
    return result


def _device_from_space(int space, int device_id) -> tuple:
    """(DLDeviceType, device_id) for a MemorySpace under the accelerator."""
    cdef dl.DLDevice device = dl.deviceFromSpace(<cpp.MemorySpace>space,
                                                 device_id)
    return int(device.device_type), int(device.device_id)


def _space_from_device_type(int device_type):
    """The MemorySpace value for a DLDeviceType, or ``None`` if unknown."""
    cdef cpp.MemorySpace space = cpp.MemorySpace.Host
    if not dl.spaceFromDeviceType(device_type, space):
        return None
    return int(<int>space)


def _accelerator() -> int:
    """The DLDeviceType that MemorySpace.Device exports as."""
    return int(dl.accelerator())


def _set_accelerator(int device_type) -> bool:
    """Set the accelerator; False (unchanged) for a non-accelerator type."""
    return bool(dl.setAccelerator(device_type))


def _live_exports() -> int:
    """Capsules this module produced whose deleter has not yet run."""
    return int(dl.liveExports())


# ---------------------------------------------------------------------------
# Signals: one handle type for every C++ signal a wrapper can reach
# ---------------------------------------------------------------------------
#
# The Python ABCs give every IPropertyChanged object connect(slot),
# disconnect(slot) and block_signals(block). Which signal connect() reaches
# is the one the ABC documents for the class: property changes on plain
# objects, status changes on components and workflows, value changes on data
# items. The on_* methods reach a named signal and return a handle.

cdef enum _SlotKind:
    _PROPERTY = 0
    _STATUS = 1
    _VALUE = 2
    _WORKFLOW_STATUS = 3


cdef class _SlotHandle:
    """One Python callable connected to one C++ signal.

    Holds the wrapper it came from, so the C++ object and whatever owns it
    stay alive while the callable is connected.
    """

    cdef int _kind
    cdef void* _target
    cdef shared_ptr[bridge.PropertySlotBridge] _property
    cdef shared_ptr[bridge.StatusSlotBridge] _status
    cdef shared_ptr[bridge.DataItemValueSlotBridge] _value
    cdef shared_ptr[bridge.WorkflowStatusSlotBridge] _workflow_status
    cdef object _wrapper
    cdef bint _connected

    def disconnect(self):
        """Disconnect this slot; a no-op if already disconnected."""
        if not self._connected:
            return
        if self._kind == _PROPERTY:
            bridge.disconnect_property_slot_any(
                <cpp.IPropertyChanged*>self._target, self._property)
        elif self._kind == _STATUS:
            bridge.disconnect_status_slot(
                <cpp.IModelComponent*>self._target, self._status)
        elif self._kind == _VALUE:
            bridge.disconnect_value_changed_slot(
                <cpp.IComponentDataItem*>self._target, self._value)
        else:
            bridge.disconnect_workflow_status_slot(
                <cpp.IWorkflowComponent*>self._target, self._workflow_status)
        self._connected = False
        self._wrapper = None

    @property
    def connected(self) -> bool:
        """Whether the slot is currently connected."""
        return self._connected


cdef _SlotHandle _new_handle(CppPropertyChangedWrapper wrapper, int kind,
                             object callback):
    """Connect ``callback`` to one signal of ``wrapper``'s C++ object."""
    if not callable(callback):
        raise TypeError("a slot must be callable")
    if wrapper._signal == NULL:
        raise ValueError("wrapper holds a null pointer")
    cdef _SlotHandle handle = _SlotHandle.__new__(_SlotHandle)
    cdef cpp.IModelComponent* component
    cdef cpp.IComponentDataItem* item
    cdef cpp.IWorkflowComponent* workflow
    handle._kind = kind
    handle._wrapper = wrapper
    if kind == _PROPERTY:
        handle._target = <void*>wrapper._signal
        handle._property = bridge.make_property_slot(<PyObject*>callback)
        bridge.connect_property_slot_any(wrapper._signal, handle._property)
    elif kind == _STATUS:
        component = (<CppModelComponentWrapper>wrapper)._ptr
        handle._target = <void*>component
        handle._status = bridge.make_status_slot(<PyObject*>callback)
        bridge.connect_status_slot(component, handle._status)
    elif kind == _VALUE:
        item = (<CppComponentDataItemWrapper>wrapper)._ptr
        handle._target = <void*>item
        handle._value = bridge.make_data_item_value_slot(<PyObject*>callback)
        bridge.connect_value_changed_slot(item, handle._value)
    else:
        workflow = (<CppWorkflowComponentWrapper>wrapper)._ptr
        handle._target = <void*>workflow
        handle._workflow_status = bridge.make_workflow_status_slot(
            <PyObject*>callback)
        bridge.connect_workflow_status_slot(workflow, handle._workflow_status)
    handle._connected = True
    return handle


#: (kind, C++ address, slot) -> _SlotHandle for every connect(slot) in force,
#: so disconnect(slot) finds it from any wrapper of the same object.
cdef dict _CONNECTIONS = {}


cdef object _connect(CppPropertyChangedWrapper wrapper, int kind,
                     object slot, object adapter):
    """connect(slot): the adapter (or the slot itself) goes to C++."""
    key = (kind, <uintptr_t>wrapper._signal, slot)
    if key in _CONNECTIONS:
        return None          # connecting a slot twice connects it once
    _CONNECTIONS[key] = _new_handle(wrapper, kind,
                                    slot if adapter is None else adapter)
    return None


cdef object _disconnect(CppPropertyChangedWrapper wrapper, int kind,
                        object slot):
    handle = _CONNECTIONS.pop((kind, <uintptr_t>wrapper._signal, slot), None)
    if handle is not None:
        handle.disconnect()
    return None


# Event-argument objects for the slots connect() reaches: the ABCs say
# status slots receive an IComponentStatusChangeEventArgs, value slots an
# IComponentDataItemValueChanged, workflow slots an
# IWorkflowComponentStatusChangeEventArgs.

from hydrocouple.core import (
    ComponentStatus as _ComponentStatus,
    IComponentDataItemValueChanged as _IValueChanged,
    IComponentStatusChangeEventArgs as _IStatusChange,
    IWorkflowComponentStatusChangeEventArgs as _IWorkflowStatusChange,
    WorkflowStatus as _WorkflowStatus,
)


class _StatusChange(_IStatusChange):
    __slots__ = ("_component", "_previous", "_status", "_message",
                 "_has_progress", "_percent")

    def __init__(self, component, previous, status, message, has_progress,
                 percent):
        self._component = component
        self._previous = _ComponentStatus(previous)
        self._status = _ComponentStatus(status)
        self._message = message
        self._has_progress = bool(has_progress)
        self._percent = float(percent)

    component = property(lambda self: self._component)
    previous_status = property(lambda self: self._previous)
    status = property(lambda self: self._status)
    message = property(lambda self: self._message)
    has_progress_monitor = property(lambda self: self._has_progress)
    percent_progress = property(lambda self: self._percent)


class _WorkflowStatusChange(_IWorkflowStatusChange):
    __slots__ = ("_workflow", "_previous", "_status", "_message",
                 "_has_progress", "_percent")

    def __init__(self, workflow, previous, status, message, has_progress,
                 percent):
        self._workflow = workflow
        self._previous = _WorkflowStatus(previous)
        self._status = _WorkflowStatus(status)
        self._message = message
        self._has_progress = bool(has_progress)
        self._percent = float(percent)

    workflow_component = property(lambda self: self._workflow)
    previous_status = property(lambda self: self._previous)
    status = property(lambda self: self._status)
    message = property(lambda self: self._message)
    has_progress_monitor = property(lambda self: self._has_progress)
    percent_progress = property(lambda self: self._percent)


class _ValueChange(_IValueChanged):
    __slots__ = ("_item", "_start", "_count")

    def __init__(self, item, start, count):
        self._item = item
        self._start = list(start)
        self._count = list(count)

    component_data_item = property(lambda self: self._item)
    start = property(lambda self: list(self._start))
    count = property(lambda self: list(self._count))


def _status_adapter(component, slot):
    def adapter(previous, status, message, has_progress, percent):
        slot(_StatusChange(component, previous, status, message,
                           has_progress, percent))
    return adapter


def _workflow_status_adapter(workflow, slot):
    def adapter(previous, status, message, has_progress, percent):
        slot(_WorkflowStatusChange(workflow, previous, status, message,
                                   has_progress, percent))
    return adapter


def _value_adapter(item, slot):
    def adapter(start, count):
        slot(_ValueChange(item, start, count))
    return adapter


# ---------------------------------------------------------------------------
# Base wrappers: signals, description, identity
# ---------------------------------------------------------------------------

cdef void bind_signal(CppPropertyChangedWrapper wrapper,
                      cpp.IPropertyChanged* ptr):
    wrapper._signal = ptr


cdef void bind_description(CppDescriptionWrapper wrapper,
                           cpp.IDescription* ptr):
    wrapper._description = ptr
    wrapper._signal = <cpp.IPropertyChanged*>ptr


cdef void bind_identity(CppIdentityWrapper wrapper, cpp.IIdentity* ptr):
    wrapper._identity = ptr
    wrapper._description = <cpp.IDescription*>ptr
    wrapper._signal = <cpp.IPropertyChanged*>ptr


cdef object owned_by(object child, object owner):
    """Make ``child`` keep ``owner`` alive; returns ``child``.

    A wrapper reached through another (an input of a component, a ring of
    a polygon) points into memory the parent's owner controls.
    """
    if child is not None and isinstance(child, CppPropertyChangedWrapper):
        (<CppPropertyChangedWrapper>child)._owner = owner
    return child


cdef class CppPropertyChangedWrapper:
    """Base of every wrapper whose C++ object is an ``IPropertyChanged``.

    Two wrappers are equal when they wrap the same C++ object.
    """

    def __cinit__(self):
        self._signal = NULL

    def __eq__(self, other):
        if not isinstance(other, CppPropertyChangedWrapper):
            return NotImplemented
        return self._signal == (<CppPropertyChangedWrapper>other)._signal

    def __hash__(self):
        return hash(<uintptr_t>self._signal)

    def connect(self, slot):
        """Connect ``slot(property_name: str)`` to the property-changed
        signal. Connecting the same slot twice connects it once."""
        _connect(self, _PROPERTY, slot, None)

    def disconnect(self, slot):
        """Disconnect a slot connected with :meth:`connect`."""
        _disconnect(self, _PROPERTY, slot)

    def block_signals(self, bint block):
        """Block (``True``) or unblock every signal this object emits."""
        if self._signal == NULL:
            raise ValueError("wrapper holds a null pointer")
        bridge.block_signals_any(self._signal, block)

    def on_property_changed(self, callback):
        """Connect ``callback(property_name: str)`` to the property-changed
        signal; returns a disconnectable handle."""
        return _new_handle(self, _PROPERTY, callback)


cdef class CppDescriptionWrapper(CppPropertyChangedWrapper):
    """Wrapper of a C++ ``IDescription``: caption and description."""

    def __cinit__(self):
        self._description = NULL

    @property
    def caption(self) -> str:
        """Human-readable caption."""
        return self._description.caption().decode("utf-8")

    @caption.setter
    def caption(self, str value):
        self._description.setCaption(value.encode("utf-8"))

    @property
    def description(self) -> str:
        """Detailed description."""
        return self._description.description().decode("utf-8")

    @description.setter
    def description(self, str value):
        self._description.setDescription(value.encode("utf-8"))


cdef class CppIdentityWrapper(CppDescriptionWrapper):
    """Wrapper of a C++ ``IIdentity``: a description with a unique id."""

    def __cinit__(self):
        self._identity = NULL

    @staticmethod
    cdef CppIdentityWrapper wrap_identity(cpp.IIdentity* ptr):
        cdef CppIdentityWrapper obj = CppIdentityWrapper.__new__(
            CppIdentityWrapper)
        bind_identity(obj, ptr)
        return obj

    @property
    def id(self) -> str:
        """Unique identifier."""
        return self._identity.id().decode("utf-8")

    def __repr__(self):
        return f"<{type(self).__name__} id={self.id!r}>"


cdef object _wrap_identity_or_none(cpp.IIdentity* ptr, object owner):
    if ptr == NULL:
        return None
    return owned_by(CppIdentityWrapper.wrap_identity(ptr), owner)


cdef const cpp.IIdentity* _identity_pointer(object label) except? NULL:
    """The IIdentity* behind a wrapper, or NULL for None."""
    if label is None:
        return NULL
    if not isinstance(label, CppIdentityWrapper):
        raise TypeError(f"expected a C++ identity wrapper or None, got "
                        f"{type(label).__name__}")
    return (<CppIdentityWrapper>label)._identity


# ---------------------------------------------------------------------------
# Dimensions
# ---------------------------------------------------------------------------

cdef class CppDimensionWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``HydroCouple::IDimension`` pointer."""

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppDimensionWrapper wrap(cpp.IDimension* ptr):
        cdef CppDimensionWrapper obj = CppDimensionWrapper.__new__(
            CppDimensionWrapper)
        obj._ptr = ptr
        bind_identity(obj, <cpp.IIdentity*>ptr)
        return obj

    @property
    def length_type(self):
        """The length type of the dimension (static or dynamic)."""
        from hydrocouple.core import LengthType
        return LengthType(<int>self._ptr.lengthType())

    @property
    def role(self):
        """What this axis means (time, entity, layer, ...)."""
        from hydrocouple.core import DimensionRole
        return DimensionRole(<int>self._ptr.role())


cdef object wrap_dimension(cpp.IDimension* ptr):
    return None if ptr == NULL else CppDimensionWrapper.wrap(ptr)


# ---------------------------------------------------------------------------
# Value definitions, quantities, qualities, units
# ---------------------------------------------------------------------------

cdef class CppValueDefinitionWrapper(CppDescriptionWrapper):
    """Wrapper around a C++ ``HydroCouple::IValueDefinition`` pointer.

    :func:`wrap_value_definition` hands out the quantity or quality
    subclass when the C++ object is one.
    """

    cdef cpp.IValueDefinition* _ptr

    def __cinit__(self):
        self._ptr = NULL

    cdef void _bind(self, cpp.IValueDefinition* ptr):
        self._ptr = ptr
        bind_description(self, <cpp.IDescription*>ptr)

    @property
    def value_kind(self):
        """How values behave under regridding and aggregation."""
        from hydrocouple.core import ValueKind
        return ValueKind(<int>self._ptr.valueKind())

    @property
    def missing_value(self) -> float:
        """The sentinel used to indicate missing data (numeric kinds)."""
        return self._ptr.missingValue()

    @property
    def default_value(self) -> float:
        """The default value (numeric kinds)."""
        return self._ptr.defaultValue()


cdef class CppUnitDimensionsWrapper(CppDescriptionWrapper):
    """Wrapper around a C++ ``HydroCouple::IUnitDimensions`` pointer."""

    cdef cpp.IUnitDimensions* _ptr

    def __cinit__(self):
        self._ptr = NULL

    def power(self, dimension) -> float:
        """The power of one fundamental dimension (L, M, T, ...)."""
        return self._ptr.power(
            <cpp.IUnitDimensions_FundamentalUnitDimension><int>int(dimension))


cdef class CppUnitWrapper(CppDescriptionWrapper):
    """Wrapper around a C++ ``HydroCouple::IUnit`` pointer."""

    cdef cpp.IUnit* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @property
    def dimensions(self):
        """The unit's powers of the fundamental dimensions."""
        cdef cpp.IUnitDimensions* dims = self._ptr.dimensions()
        if dims == NULL:
            return None
        cdef CppUnitDimensionsWrapper obj = CppUnitDimensionsWrapper.__new__(
            CppUnitDimensionsWrapper)
        obj._ptr = dims
        bind_description(obj, <cpp.IDescription*>dims)
        return owned_by(obj, self)

    @property
    def conversion_factor_to_si(self) -> float:
        """Multiply a value in this unit by this to get SI."""
        return self._ptr.conversionFactorToSI()

    @property
    def offset_to_si(self) -> float:
        """Add this after multiplying to get SI."""
        return self._ptr.offsetToSI()


cdef class CppQuantityWrapper(CppValueDefinitionWrapper):
    """Wrapper around a C++ ``HydroCouple::IQuantity`` pointer."""

    cdef cpp.IQuantity* _quantity

    def __cinit__(self):
        self._quantity = NULL

    @property
    def unit(self):
        """The unit values are expressed in, or ``None``."""
        cdef cpp.IUnit* unit = self._quantity.unit()
        if unit == NULL:
            return None
        cdef CppUnitWrapper obj = CppUnitWrapper.__new__(CppUnitWrapper)
        obj._ptr = unit
        bind_description(obj, <cpp.IDescription*>unit)
        return owned_by(obj, self)

    @property
    def min_value(self) -> float:
        """Smallest valid value."""
        return self._quantity.minValue()

    @property
    def max_value(self) -> float:
        """Largest valid value."""
        return self._quantity.maxValue()


cdef class CppQualityWrapper(CppValueDefinitionWrapper):
    """Wrapper around a C++ ``HydroCouple::IQuality`` pointer."""

    cdef cpp.IQuality* _quality

    def __cinit__(self):
        self._quality = NULL

    @property
    def categories(self) -> list:
        """Category labels; index k of a value means categories[k]."""
        cdef vector[string] labels = self._quality.categories()
        return [labels[i].decode("utf-8") for i in range(labels.size())]

    @property
    def is_ordered(self) -> bool:
        """Whether the categories are ordered."""
        return self._quality.isOrdered()


cdef object wrap_value_definition(cpp.IValueDefinition* ptr):
    """The most specific wrapper for a value definition, or ``None``."""
    if ptr == NULL:
        return None
    cdef cpp.IQuantity* quantity = cpp.asQuantity(ptr)
    cdef cpp.IQuality* quality = cpp.asQuality(ptr)
    cdef CppQuantityWrapper q
    cdef CppQualityWrapper c
    cdef CppValueDefinitionWrapper v
    if quantity != NULL:
        q = CppQuantityWrapper.__new__(CppQuantityWrapper)
        q._bind(ptr)
        q._quantity = quantity
        return q
    if quality != NULL:
        c = CppQualityWrapper.__new__(CppQualityWrapper)
        c._bind(ptr)
        c._quality = quality
        return c
    v = CppValueDefinitionWrapper.__new__(CppValueDefinitionWrapper)
    v._bind(ptr)
    return v


# ---------------------------------------------------------------------------
# CppComponentDataItemWrapper — the typed data plane
# ---------------------------------------------------------------------------

cdef void bind_data_item(CppComponentDataItemWrapper wrapper,
                         cpp.IComponentDataItem* ptr):
    wrapper._ptr = ptr
    bind_identity(wrapper, <cpp.IIdentity*>ptr)


cdef class CppComponentDataItemWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``HydroCouple::IComponentDataItem`` pointer.

    Exposes the v2 typed hyperslab data plane: ``get_values_into`` /
    ``set_values_from`` marshal NumPy arrays as zero-copy
    ``BufferDescriptor`` views and release the GIL around the C++ call.
    Every exchange-item, temporal and spatial item wrapper derives from it.
    """

    # _ptr is declared in _core.pxd.

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppComponentDataItemWrapper wrap(cpp.IComponentDataItem* ptr):
        cdef CppComponentDataItemWrapper obj = (
            CppComponentDataItemWrapper.__new__(CppComponentDataItemWrapper))
        bind_data_item(obj, ptr)
        return obj

    @property
    def model_component(self):
        """The component that owns this item, or ``None``."""
        return owned_by(wrap_model_component(self._ptr.modelComponent()),
                        self)

    @property
    def dimensions(self) -> list:
        """Dimension metadata objects, parallel to :attr:`shape`."""
        cdef vector[cpp.IDimension*] dims = self._ptr.dimensions()
        return [owned_by(wrap_dimension(dims[i]), self)
                for i in range(dims.size())]

    @property
    def shape(self) -> tuple:
        """The extent of each dimension."""
        cdef vector[int64_t] s = self._ptr.shape()
        return tuple(s[i] for i in range(s.size()))

    @property
    def data_kind(self):
        """The element type of this item's values."""
        from hydrocouple.core import DataKind
        return DataKind(<int>self._ptr.dataKind())

    @property
    def value_definition(self):
        """The value definition of the stored values (a quantity or a
        quality where the item declares one), or ``None``."""
        return owned_by(wrap_value_definition(self._ptr.valueDefinition()),
                        self)

    @property
    def data_item(self):
        """This item seen as a plain ``IComponentDataItem`` -- the same C++
        object, not a copy."""
        return owned_by(CppComponentDataItemWrapper.wrap(self._ptr), self)

    def get_values_into(self, destination, start, count, *, stream=None):
        """Copy a hyperslab into ``destination`` (zero-copy descriptor).

        :param destination: a writable ndarray, or any object implementing
            the DLPack protocol (``__dlpack__``: a ``torch.Tensor``, a CuPy
            array, ...) on any device, whose element type corresponds to
            :attr:`data_kind`; may be a non-contiguous view. The item writes
            into the destination's own memory -- the binding makes no copy.
            A device destination reaches the C++ item as a
            ``MemorySpace.Device`` descriptor; an item that is host-only
            refuses it with a message.
        :param start: first index of the selection per dimension.
        :param count: selection extent per dimension.
        :param stream: DLPack consumer stream for a device destination
            (array-API semantics; ``None`` = the legacy default stream).
            Ignored for NumPy and CPU tensors. The C++ call is host
            synchronous: when it returns, the values are in place.
        :returns: ``(ok, message)``.
        """
        return _get_values_into(self._ptr, destination, start, count, stream)

    def set_values_from(self, source, start, count, *, stream=None):
        """Copy values from ``source`` into a hyperslab of this item.

        ``source`` is an ndarray, any DLPack producer (see
        :meth:`get_values_into`), or anything ``numpy.asarray`` accepts.

        :returns: ``(ok, message)``.
        """
        return _set_values_from(self._ptr, source, start, count, stream)

    # -- Signal/slot -------------------------------------------------------

    def connect(self, slot):
        """Connect ``slot(event_args)`` to the value-changed signal, where
        ``event_args`` is an ``IComponentDataItemValueChanged`` (the ABC's
        contract for a data item). Property changes are
        :meth:`on_property_changed`."""
        _connect(self, _VALUE, slot, _value_adapter(self, slot))

    def disconnect(self, slot):
        """Disconnect a slot connected with :meth:`connect`."""
        _disconnect(self, _VALUE, slot)

    def on_value_changed(self, callback):
        """Connect ``callback(start: list[int], count: list[int])`` to the
        value-changed signal; returns a disconnectable handle."""
        return _new_handle(self, _VALUE, callback)


# ---------------------------------------------------------------------------
# CppArgumentWrapper
# ---------------------------------------------------------------------------

cdef class CppArgumentWrapper(CppComponentDataItemWrapper):
    """Wrapper around a C++ ``HydroCouple::IArgument`` pointer."""

    cdef cpp.IArgument* _argument

    def __cinit__(self):
        self._argument = NULL

    @staticmethod
    cdef CppArgumentWrapper wrap_argument(cpp.IArgument* ptr):
        cdef CppArgumentWrapper obj = CppArgumentWrapper.__new__(
            CppArgumentWrapper)
        obj._argument = ptr
        bind_data_item(obj, <cpp.IComponentDataItem*>ptr)
        return obj

    @property
    def role(self):
        """What the argument is for (configuration, parameter, forcing...)."""
        from hydrocouple.core import ArgumentRole
        return ArgumentRole(<int>self._argument.role())

    @property
    def valid_component_data_item_types(self) -> list:
        """The C++ item types this argument can be initialized from, as
        readable type names (a C++ ``type_info`` has no Python type)."""
        cdef vector[const type_info*] types = (
            self._argument.validComponentDataItemTypes())
        return [cpp.typeName(types[i]).decode("utf-8")
                for i in range(types.size())]

    @property
    def is_optional(self) -> bool:
        """Whether this argument is optional."""
        return self._argument.isOptional()

    @property
    def is_read_only(self) -> bool:
        """Whether this argument is read-only."""
        return self._argument.isReadOnly()

    def __str__(self) -> str:
        return self._argument.toString().decode("utf-8")

    def save_data(self):
        """Write data to files associated with this argument, if any."""
        self._argument.saveData()

    @property
    def file_filters(self) -> list:
        """File filter strings readable by this argument."""
        cdef vector[string] ff = self._argument.fileFilters()
        return [ff[i].decode("utf-8") for i in range(ff.size())]

    def is_valid_arg_type(self, arg_type) -> bool:
        """Whether the given input representation is supported."""
        return self._argument.isValidArgType(
            <cpp.IArgument_ArgumentInputType><int>arg_type)

    @property
    def current_argument_input_type(self):
        """How this argument was initialized."""
        from hydrocouple.core import ArgumentInputType
        return ArgumentInputType(
            <int>self._argument.currentArgumentInputType())

    def initialize(self, value, arg_type=None):
        """Read the argument value from a string representation (``value``
        a ``str``, ``arg_type`` its representation) or from an equivalent
        C++ data item (``value`` a data-item wrapper).

        :returns: ``(ok, message)``.
        """
        cdef string msg
        cdef bint ok
        cdef cpp.IComponentDataItem* source
        if isinstance(value, CppComponentDataItemWrapper):
            source = (<CppComponentDataItemWrapper>value)._ptr
            if source == NULL:
                raise ValueError("data item wrapper holds a null pointer")
            ok = self._argument.initialize(source[0], msg)
            return bool(ok), msg.decode("utf-8")
        if arg_type is None:
            raise TypeError("arg_type is required when value is a string")
        ok = self._argument.initialize(
            (<str>value).encode("utf-8"),
            <cpp.IArgument_ArgumentInputType><int>arg_type, msg)
        return bool(ok), msg.decode("utf-8")

    def serialize(self, arg_type):
        """Serialize the current value to the requested representation.

        :returns: ``(ok, value, message)``.
        """
        cdef string out
        cdef string msg
        cdef bint ok = self._argument.serialize(
            <cpp.IArgument_ArgumentInputType><int>arg_type, out, msg)
        return bool(ok), out.decode("utf-8"), msg.decode("utf-8")


# ---------------------------------------------------------------------------
# CppInputWrapper / CppMultiInputWrapper
# ---------------------------------------------------------------------------

cdef cpp.IOutput* _output_pointer(object output) except? NULL:
    """The IOutput* behind an output wrapper, or NULL for None."""
    if output is None:
        return NULL
    if not isinstance(output, CppOutputWrapper):
        raise TypeError(f"expected a C++ output wrapper or None, got "
                        f"{type(output).__name__}")
    return (<CppOutputWrapper>output)._output


cdef cpp.IInput* _input_pointer(object input) except? NULL:
    """The IInput* behind an input wrapper, or NULL for None."""
    if input is None:
        return NULL
    if not isinstance(input, CppInputWrapper):
        raise TypeError(f"expected a C++ input wrapper or None, got "
                        f"{type(input).__name__}")
    return (<CppInputWrapper>input)._input


cdef class CppInputWrapper(CppComponentDataItemWrapper):
    """Wrapper around a C++ ``HydroCouple::IInput`` pointer."""

    cdef cpp.IInput* _input

    def __cinit__(self):
        self._input = NULL

    cdef void _bind_input(self, cpp.IInput* ptr):
        self._input = ptr
        bind_data_item(self, <cpp.IComponentDataItem*>ptr)

    @property
    def provider(self):
        """The output providing data to this input, or ``None``."""
        return owned_by(_wrap_output(self._input.provider()), self)

    def set_provider(self, provider) -> bool:
        """Assign an output (or ``None``) as this input's provider."""
        return self._input.setProvider(_output_pointer(provider))

    def can_consume(self, provider):
        """Whether this input can consume the given output.

        :returns: ``(ok, message)``.
        """
        cdef string msg
        cdef cpp.IOutput* candidate = _output_pointer(provider)
        if candidate == NULL:
            raise ValueError("provider must not be None")
        cdef bint ok = self._input.canConsume(candidate, msg)
        return bool(ok), msg.decode("utf-8")


cdef class CppMultiInputWrapper(CppInputWrapper):
    """Wrapper around a C++ ``HydroCouple::IMultiInput`` pointer."""

    cdef cpp.IMultiInput* _multi

    def __cinit__(self):
        self._multi = NULL

    @property
    def provider_labels(self) -> list:
        """The provider roles this input declares."""
        cdef vector[cpp.IIdentity*] labels = self._multi.providerLabels()
        return [_wrap_identity_or_none(labels[i], self)
                for i in range(labels.size())]

    def is_required_provider(self, provider_label) -> bool:
        """Whether the labeled provider role must be filled."""
        return self._multi.isRequiredProvider(_identity_pointer(provider_label))

    @property
    def providers(self) -> list:
        """Every output this input consumes."""
        cdef vector[cpp.IOutput*] outputs = self._multi.providers()
        return [owned_by(_wrap_output(outputs[i]), self)
                for i in range(outputs.size())]

    def add_provider(self, provider, provider_role_identifier=None) -> bool:
        """Add a provider, optionally into a declared role."""
        cdef cpp.IOutput* output = _output_pointer(provider)
        if output == NULL:
            raise ValueError("provider must not be None")
        return self._multi.addProvider(
            output, _identity_pointer(provider_role_identifier))

    def remove_provider(self, provider) -> bool:
        """Remove a provider."""
        return self._multi.removeProvider(_output_pointer(provider))


cdef object _wrap_input(cpp.IInput* ptr):
    """An input wrapper -- the multi-input one when the input is one."""
    if ptr == NULL:
        return None
    cdef cpp.IMultiInput* multi = cpp.asMultiInput(ptr)
    cdef CppMultiInputWrapper m
    cdef CppInputWrapper w
    if multi != NULL:
        m = CppMultiInputWrapper.__new__(CppMultiInputWrapper)
        m._bind_input(ptr)
        m._multi = multi
        return m
    w = CppInputWrapper.__new__(CppInputWrapper)
    w._bind_input(ptr)
    return w


# ---------------------------------------------------------------------------
# CppOutputWrapper / CppAdaptedOutputWrapper
# ---------------------------------------------------------------------------

cdef cpp.IAdaptedOutput* _adapted_pointer(object adapted) except NULL:
    if not isinstance(adapted, CppAdaptedOutputWrapper):
        raise TypeError(f"expected a C++ adapted-output wrapper, got "
                        f"{type(adapted).__name__}")
    cdef cpp.IAdaptedOutput* ptr = (<CppAdaptedOutputWrapper>adapted)._adapted
    if ptr == NULL:
        raise ValueError("adapted-output wrapper holds a null pointer")
    return ptr


cdef class CppOutputWrapper(CppComponentDataItemWrapper):
    """Wrapper around a C++ ``HydroCouple::IOutput`` pointer."""

    cdef cpp.IOutput* _output

    def __cinit__(self):
        self._output = NULL

    cdef void _bind_output(self, cpp.IOutput* ptr):
        self._output = ptr
        bind_data_item(self, <cpp.IComponentDataItem*>ptr)

    @property
    def consumers(self) -> list:
        """Inputs currently consuming from this output."""
        cdef vector[cpp.IInput*] c = self._output.consumers()
        return [owned_by(_wrap_input(c[i]), self) for i in range(c.size())]

    def add_consumer(self, consumer):
        """Register an input as a consumer of this output (the provider
        checks ``can_consume`` and may refuse by raising)."""
        cdef cpp.IInput* input = _input_pointer(consumer)
        if input == NULL:
            raise ValueError("consumer must not be None")
        self._output.addConsumer(input)

    def remove_consumer(self, consumer) -> bool:
        """Remove an input from this output's consumer list."""
        return self._output.removeConsumer(_input_pointer(consumer))

    @property
    def adapted_outputs(self) -> list:
        """Adapted outputs chained onto this output."""
        cdef vector[cpp.IAdaptedOutput*] adapted = self._output.adaptedOutputs()
        return [owned_by(_wrap_output(<cpp.IOutput*>adapted[i]), self)
                for i in range(adapted.size())]

    def add_adapted_output(self, adapted_output):
        """Chain an adapted output onto this output. The output does not
        take ownership: keep the adapted output's wrapper alive."""
        self._output.addAdaptedOutput(_adapted_pointer(adapted_output))

    def remove_adapted_output(self, adapted_output) -> bool:
        """Unchain an adapted output."""
        return self._output.removeAdaptedOutput(
            _adapted_pointer(adapted_output))

    def update_values(self, query_specifier=None):
        """Bring the values up to date for ``query_specifier`` (an input,
        or ``None``). Releases the GIL while the C++ side computes."""
        cdef cpp.IInput* q = _input_pointer(query_specifier)
        with nogil:
            self._output.updateValues(q)


cdef class CppAdaptedOutputWrapper(CppOutputWrapper):
    """Wrapper around a C++ ``HydroCouple::IAdaptedOutput`` pointer.

    One created through an adapted-output factory is owned by its wrapper
    and destroyed with it.
    """

    cdef cpp.IAdaptedOutput* _adapted
    cdef unique_ptr[cpp.IAdaptedOutput] _owned_adapted

    def __cinit__(self):
        self._adapted = NULL

    def __dealloc__(self):
        self._owned_adapted.reset()

    @property
    def adapted_output_factory(self):
        """The factory that made this adapted output, or ``None``."""
        return owned_by(_wrap_factory(self._adapted.adaptedOutputFactory()),
                        self)

    @property
    def arguments(self) -> list:
        """Arguments that configure the adaptation."""
        cdef vector[cpp.IArgument*] args = self._adapted.arguments()
        return [owned_by(CppArgumentWrapper.wrap_argument(args[i]), self)
                for i in range(args.size())]

    def initialize(self):
        """Initialize from the arguments."""
        self._adapted.initialize()

    @property
    def adaptee(self):
        """The output this one adapts."""
        return owned_by(_wrap_output(self._adapted.adaptee()), self)

    def refresh(self):
        """Pull from the adaptee and recompute."""
        self._adapted.refresh()

    @property
    def states(self) -> list:
        """The data items that carry state from one refresh to the next
        (empty for a stateless adapter)."""
        cdef vector[cpp.IComponentDataItem*] st = self._adapted.states()
        return [owned_by(wrap_data_item(st[i]), self)
                for i in range(st.size())]

    # -- Checkpointing (ICheckpointableAdaptedOutput) -----------------------

    cdef cpp.ICheckpointableAdaptedOutput* _checkpointable(self) except NULL:
        cdef cpp.ICheckpointableAdaptedOutput* p = (
            cpp.asCheckpointableAdapter(self._adapted))
        if p == NULL:
            raise TypeError(f"adapted output '{self.id}' does not implement "
                            "ICheckpointableAdaptedOutput")
        return p

    def save_state(self):
        """Save the adapter's state; returns ``(ok, token, message)``."""
        cdef string token, msg
        cdef bint ok = self._checkpointable().saveState(token, msg)
        return bool(ok), token.decode("utf-8", "surrogateescape"), msg.decode("utf-8")

    def restore_state(self, str token):
        """Restore a saved state; returns ``(ok, message)``."""
        cdef string msg
        cdef bint ok = self._checkpointable().restoreState(
            token.encode("utf-8", "surrogateescape"), msg)
        return bool(ok), msg.decode("utf-8")

    def release_state(self, str token):
        """Release a saved state that will not be restored; returns
        ``(ok, message)``."""
        cdef string msg
        cdef bint ok = self._checkpointable().releaseState(
            token.encode("utf-8", "surrogateescape"), msg)
        return bool(ok), msg.decode("utf-8")

    # -- Differentiation (IDifferentiableAdaptedOutput) ---------------------

    cdef cpp.IDifferentiableAdaptedOutput* _differentiable(self) except NULL:
        cdef cpp.IDifferentiableAdaptedOutput* p = (
            cpp.asDifferentiableAdapter(self._adapted))
        if p == NULL:
            raise TypeError(f"adapted output '{self.id}' does not implement "
                            "IDifferentiableAdaptedOutput")
        return p

    def differentiable_arguments(self) -> list:
        """Arguments of this adapter a derivative reaches."""
        cdef vector[cpp.IArgument*] v = (
            self._differentiable().differentiableArguments())
        return [owned_by(CppArgumentWrapper.wrap_argument(v[i]), self)
                for i in range(v.size())]

    def differentiable_states(self) -> list:
        """State items the derivative follows."""
        cdef vector[cpp.IComponentDataItem*] v = (
            self._differentiable().differentiableStates())
        return [owned_by(wrap_data_item(v[i]), self)
                for i in range(v.size())]

    def vjp(self, seeds, results, *, stream=None):
        """Vector-Jacobian product of the most recent refresh; entries as
        for :meth:`CppModelComponentWrapper.vjp`. :returns: ``(ok,
        message)``."""
        return _differential_call(NULL, seeds, results, False, stream,
                                  self._differentiable())

    def jvp(self, seeds, results, *, stream=None):
        """Jacobian-vector product of the most recent refresh.
        :returns: ``(ok, message)``."""
        return _differential_call(NULL, seeds, results, True, stream,
                                  self._differentiable())


cdef object _wrap_output(cpp.IOutput* ptr):
    """An output wrapper -- the adapted-output one when it is one."""
    if ptr == NULL:
        return None
    cdef cpp.IAdaptedOutput* adapted = cpp.asAdaptedOutput(ptr)
    cdef CppAdaptedOutputWrapper a
    cdef CppOutputWrapper w
    if adapted != NULL:
        a = CppAdaptedOutputWrapper.__new__(CppAdaptedOutputWrapper)
        a._bind_output(ptr)
        a._adapted = adapted
        return a
    w = CppOutputWrapper.__new__(CppOutputWrapper)
    w._bind_output(ptr)
    return w


cdef object wrap_data_item(cpp.IComponentDataItem* ptr):
    """The most specific core wrapper for a data item, or ``None``."""
    if ptr == NULL:
        return None
    cdef cpp.IArgument* argument = cpp.asArgument(ptr)
    if argument != NULL:
        return CppArgumentWrapper.wrap_argument(argument)
    cdef cpp.IInput* input = cpp.asInput(ptr)
    if input != NULL:
        return _wrap_input(input)
    cdef cpp.IOutput* output = cpp.asOutput(ptr)
    if output != NULL:
        return _wrap_output(output)
    return CppComponentDataItemWrapper.wrap(ptr)


# ---------------------------------------------------------------------------
# Adapted-output factories
# ---------------------------------------------------------------------------

cdef class CppAdaptedOutputFactoryWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``HydroCouple::IAdaptedOutputFactory``."""

    cdef cpp.IAdaptedOutputFactory* _ptr

    def __cinit__(self):
        self._ptr = NULL

    def get_available_adapted_output_ids(self, provider, consumer=None) -> list:
        """Identities of the adapted outputs this factory can put between
        ``provider`` and ``consumer``."""
        cdef cpp.IOutput* output = _output_pointer(provider)
        if output == NULL:
            raise ValueError("provider must not be None")
        cdef vector[cpp.IIdentity*] ids = (
            self._ptr.getAvailableAdaptedOutputIds(
                output, _input_pointer(consumer)))
        return [_wrap_identity_or_none(ids[i], self) for i in range(ids.size())]

    def create_adapted_output(self, adapted_provider_id, provider,
                              consumer=None):
        """Create the identified adapted output over ``provider``.

        The returned wrapper owns the adapted output. Chaining it onto the
        provider (``add_adapted_output``) does not transfer ownership.
        """
        cdef cpp.IIdentity* identity = <cpp.IIdentity*>_identity_pointer(
            adapted_provider_id)
        cdef cpp.IOutput* output = _output_pointer(provider)
        if identity == NULL or output == NULL:
            raise ValueError("adapted_provider_id and provider are required")
        cdef unique_ptr[cpp.IAdaptedOutput] made = (
            self._ptr.createAdaptedOutput(identity, output,
                                          _input_pointer(consumer)))
        if made.get() == NULL:
            return None
        cdef cpp.IAdaptedOutput* raw = made.get()
        cdef CppAdaptedOutputWrapper wrapper = CppAdaptedOutputWrapper.__new__(
            CppAdaptedOutputWrapper)
        wrapper._bind_output(<cpp.IOutput*>raw)
        wrapper._adapted = raw
        wrapper._owned_adapted.reset(made.release())
        return owned_by(wrapper, self)


cdef object _wrap_factory(cpp.IAdaptedOutputFactory* ptr):
    if ptr == NULL:
        return None
    cdef CppAdaptedOutputFactoryWrapper w = (
        CppAdaptedOutputFactoryWrapper.__new__(CppAdaptedOutputFactoryWrapper))
    w._ptr = ptr
    bind_identity(w, <cpp.IIdentity*>ptr)
    return w


# ---------------------------------------------------------------------------
# Component information
# ---------------------------------------------------------------------------

cdef class CppComponentInfoWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``HydroCouple::IComponentInfo`` pointer."""

    cdef cpp.IComponentInfo* _info

    def __cinit__(self):
        self._info = NULL

    cdef void _bind_info(self, cpp.IComponentInfo* ptr):
        self._info = ptr
        bind_identity(self, <cpp.IIdentity*>ptr)

    @property
    def library_file_path(self) -> str:
        """Path of the library the component was loaded from."""
        return self._info.libraryFilePath().decode("utf-8")

    @library_file_path.setter
    def library_file_path(self, str value):
        self._info.setLibraryFilePath(value.encode("utf-8"))

    @property
    def icon_file_path(self) -> str:
        """Path of the component's icon."""
        return self._info.iconFilePath().decode("utf-8")

    @property
    def developer(self) -> str:
        """Name of the component developer or organization."""
        return self._info.developer().decode("utf-8")

    @property
    def documentation(self) -> list:
        """Documentation references for this component."""
        cdef vector[string] docs = self._info.documentation()
        return [docs[i].decode("utf-8") for i in range(docs.size())]

    @property
    def license(self) -> str:
        """License under which this component is distributed."""
        return self._info.license().decode("utf-8")

    @property
    def copyright(self) -> str:
        """Copyright notice."""
        return self._info.copyright().decode("utf-8")

    @property
    def url(self) -> str:
        """URL for the component's homepage or repository."""
        return self._info.url().decode("utf-8")

    @property
    def email(self) -> str:
        """Contact email for the component developer."""
        return self._info.email().decode("utf-8")

    @property
    def version(self) -> str:
        """Version string of this component."""
        return self._info.version().decode("utf-8")

    @property
    def tags(self) -> set:
        """Free-form tags."""
        cdef cppset[string] tags = self._info.tags()
        return {tag.decode("utf-8") for tag in tags}


cdef class CppModelComponentInfoWrapper(CppComponentInfoWrapper):
    """Wrapper around a C++ ``HydroCouple::IModelComponentInfo`` pointer."""

    cdef cpp.IModelComponentInfo* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppModelComponentInfoWrapper wrap(cpp.IModelComponentInfo* ptr):
        cdef CppModelComponentInfoWrapper obj = (
            CppModelComponentInfoWrapper.__new__(
                CppModelComponentInfoWrapper))
        obj._ptr = ptr
        obj._bind_info(<cpp.IComponentInfo*>ptr)
        return obj

    def create_component_instance(self):
        """A new component instance, owned by the returned wrapper (which
        keeps this info -- and the library behind it -- alive)."""
        cdef unique_ptr[cpp.IModelComponent] made = (
            self._ptr.createComponentInstance())
        if made.get() == NULL:
            return None
        cdef CppModelComponentWrapper wrapper = CppModelComponentWrapper.wrap(
            made.get())
        wrapper._owned.reset(made.release())
        return owned_by(wrapper, self)

    @property
    def adapted_output_factories(self) -> list:
        """Adapted-output factories the component brings."""
        cdef vector[cpp.IAdaptedOutputFactory*] factories = (
            self._ptr.adaptedOutputFactories())
        return [owned_by(_wrap_factory(factories[i]), self)
                for i in range(factories.size())]


cdef class CppWorkflowComponentInfoWrapper(CppComponentInfoWrapper):
    """Wrapper around a C++ ``HydroCouple::IWorkflowComponentInfo``."""

    cdef cpp.IWorkflowComponentInfo* _ptr

    def __cinit__(self):
        self._ptr = NULL

    def create_component_instance(self):
        """A new workflow, owned by the returned wrapper."""
        cdef unique_ptr[cpp.IWorkflowComponent] made = (
            self._ptr.createComponentInstance())
        if made.get() == NULL:
            return None
        cdef CppWorkflowComponentWrapper wrapper = (
            CppWorkflowComponentWrapper.wrap(made.get()))
        wrapper._owned.reset(made.release())
        return owned_by(wrapper, self)


# ---------------------------------------------------------------------------
# Differentiation (Phase G1): buffers and items for DifferentialEntry
# ---------------------------------------------------------------------------

cdef class _BufferLease:
    """One buffer lent to C++ as a BufferDescriptor for one call.

    An ndarray is described in place; any other DLPack producer is borrowed
    and its deleter runs when the lease is dropped.
    """

    cdef dl.DLPackBorrow* borrow
    cdef vector[int64_t] shape_buf
    cdef vector[int64_t] strides_buf
    cdef cpp.BufferDescriptor d
    cdef object keep

    def __cinit__(self):
        self.borrow = NULL

    def __dealloc__(self):
        if self.borrow != NULL:
            del self.borrow
            self.borrow = NULL


cdef _BufferLease _lease(object buffer, bint writable, object stream):
    cdef _BufferLease lease = _BufferLease.__new__(_BufferLease)
    cdef string msg
    if type(buffer) is np.ndarray:
        lease.keep = buffer
        _fill_descriptor(<cnp.ndarray>buffer, &lease.d, &lease.shape_buf,
                         &lease.strides_buf, writable)
        return lease
    if writable:
        _refuse_immutable_destination(buffer)
    if not hasattr(buffer, "__dlpack__"):
        raise TypeError("a derivative buffer must be a numpy.ndarray or "
                        "implement the DLPack protocol (__dlpack__)")
    capsule = _dlpack_capsule_of(buffer, stream)
    lease.keep = buffer
    lease.borrow = new dl.DLPackBorrow()
    if not dl.borrowCapsule(<PyObject*>capsule, lease.borrow[0], msg):
        raise BufferError(msg.decode("utf-8"))
    if not dl.descriptorFromBorrow(lease.borrow[0], writable, lease.d, msg):
        raise ValueError(msg.decode("utf-8"))
    return lease


cdef cpp.IComponentDataItem* _item_pointer(object item) except NULL:
    """The IComponentDataItem* behind any C++ data-item wrapper."""
    if not isinstance(item, CppComponentDataItemWrapper):
        raise TypeError(f"{type(item).__name__} is not a C++ data item "
                        "wrapper of this component")
    cdef cpp.IComponentDataItem* p = (<CppComponentDataItemWrapper>item)._ptr
    if p == NULL:
        raise ValueError("data item wrapper holds a null pointer")
    return p


cdef tuple _differential_call(cpp.IDifferentiableModelComponent* comp,
                              object seeds, object results, bint forward,
                              object stream,
                              cpp.IDifferentiableAdaptedOutput* adapter=NULL):
    """Marshal (item, role, buffer) triples and call vjp or jvp on the
    component, or on the adapter when ``comp`` is NULL."""
    leases = []
    cdef vector[cpp.DifferentialEntry] seed_entries, result_entries
    cdef cpp.DifferentialEntry entry
    cdef _BufferLease lease
    for group, writable in ((seeds, False), (results, True)):
        for item, role, buffer in group:
            lease = _lease(buffer, writable, stream)
            leases.append(lease)
            entry.item = _item_pointer(item)
            entry.role = <cpp.DifferentialRole><int>int(role)
            entry.value = lease.d
            if writable:
                result_entries.push_back(entry)
            else:
                seed_entries.push_back(entry)
    cdef cpp.DifferentialSet s = cpp.DifferentialSet(
        seed_entries.data(), seed_entries.size())
    cdef cpp.DifferentialSet r = cpp.DifferentialSet(
        result_entries.data(), result_entries.size())
    cdef string msg
    cdef bint ok
    if comp != NULL:
        if forward:
            with nogil:
                ok = comp.jvp(s, r, &msg)
        else:
            with nogil:
                ok = comp.vjp(s, r, &msg)
    else:
        if forward:
            with nogil:
                ok = adapter.jvp(s, r, &msg)
        else:
            with nogil:
                ok = adapter.vjp(s, r, &msg)
    del leases
    return bool(ok), msg.decode("utf-8")


# ---------------------------------------------------------------------------
# CppModelComponentWrapper
# ---------------------------------------------------------------------------

cdef list _python_errors(vector[cpp.ErrorEntry]& entries):
    from hydrocouple.core import ErrorEntry as PyErrorEntry
    result = []
    for i in range(entries.size()):
        result.append(PyErrorEntry(
            severity=PyErrorEntry.Severity(<int>entries[i].severity),
            code=entries[i].code,
            source=entries[i].source.decode("utf-8"),
            message=entries[i].message.decode("utf-8"),
        ))
    return result


cdef class CppModelComponentWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``HydroCouple::IModelComponent`` pointer.

    Exposes the full lifecycle, exchange items, capabilities, and the
    error queue. ``update()`` releases the GIL while the component
    computes, so multiple C++ components can advance concurrently from
    Python threads. A component made by
    ``CppModelComponentInfoWrapper.create_component_instance()`` is owned
    by its wrapper and destroyed with it.
    """

    def __cinit__(self):
        self._ptr = NULL

    def __dealloc__(self):
        self._owned.reset()

    @staticmethod
    cdef CppModelComponentWrapper wrap(cpp.IModelComponent* ptr):
        cdef CppModelComponentWrapper obj = (
            CppModelComponentWrapper.__new__(CppModelComponentWrapper))
        obj._ptr = ptr
        bind_identity(obj, <cpp.IIdentity*>ptr)
        return obj

    @property
    def component_info(self):
        """Metadata about this component, or ``None``."""
        cdef cpp.IModelComponentInfo* info = self._ptr.componentInfo()
        if info == NULL:
            return None
        return owned_by(CppModelComponentInfoWrapper.wrap(info), self)

    @property
    def status(self):
        """Current lifecycle status of this component."""
        from hydrocouple.core import ComponentStatus
        return ComponentStatus(<int>self._ptr.status())

    @property
    def arguments(self) -> list:
        """Arguments that configure this component."""
        cdef vector[cpp.IArgument*] args = self._ptr.arguments()
        return [owned_by(CppArgumentWrapper.wrap_argument(args[i]), self)
                for i in range(args.size())]

    @property
    def inputs(self) -> list:
        """Input exchange items for this component."""
        cdef vector[cpp.IInput*] inp = self._ptr.inputs()
        return [owned_by(_wrap_input(inp[i]), self) for i in range(inp.size())]

    @property
    def outputs(self) -> list:
        """Output exchange items for this component."""
        cdef vector[cpp.IOutput*] out = self._ptr.outputs()
        return [owned_by(_wrap_output(out[i]), self)
                for i in range(out.size())]

    @property
    def results(self) -> list:
        """Result data items produced by this component."""
        cdef vector[cpp.IComponentDataItem*] res = self._ptr.results()
        return [owned_by(wrap_data_item(res[i]), self)
                for i in range(res.size())]

    @property
    def states(self) -> list:
        """The data items that make up the state carried between updates."""
        cdef vector[cpp.IComponentDataItem*] st = self._ptr.states()
        return [owned_by(wrap_data_item(st[i]), self)
                for i in range(st.size())]

    def initialize(self):
        """Initialize the component, reading arguments and setting up state."""
        self._ptr.initialize()

    def validate(self) -> list:
        """Validate the component; an empty message list means valid."""
        cdef vector[string] msgs = self._ptr.validate()
        return [msgs[i].decode("utf-8") for i in range(msgs.size())]

    def prepare(self):
        """Prepare the component for execution after validation."""
        self._ptr.prepare()

    def update(self, required_outputs=None):
        """Advance the component, bringing ``required_outputs`` (output
        wrappers) up to date at least; releases the GIL during the C++
        compute."""
        cdef vector[cpp.IOutput*] required
        if required_outputs is not None:
            for output in required_outputs:
                required.push_back(_output_pointer(output))
        with nogil:
            self._ptr.update(required)

    def finish(self):
        """Finalize the component and release resources."""
        self._ptr.finish()

    def capabilities(self) -> set:
        """The optional capabilities this component supports."""
        from hydrocouple.core import Capability
        cdef cppset[cpp.Capability] caps = self._ptr.capabilities()
        return {Capability(<int>c) for c in caps}

    def errors(self, clear_after_read=False) -> list:
        """Drain the component's diagnostic queue."""
        cdef vector[cpp.ErrorEntry] entries = self._ptr.errors(
            clear_after_read)
        return _python_errors(entries)

    @property
    def workflow(self):
        """The workflow managing this component, or ``None``."""
        cdef const cpp.IWorkflowComponent* workflow = self._ptr.workflow()
        if workflow == NULL:
            return None
        return CppWorkflowComponentWrapper.wrap(
            <cpp.IWorkflowComponent*>workflow)

    @workflow.setter
    def workflow(self, value):
        cdef const cpp.IWorkflowComponent* workflow = NULL
        if value is not None:
            if not isinstance(value, CppWorkflowComponentWrapper):
                raise TypeError("workflow must be a C++ workflow wrapper or "
                                "None")
            workflow = (<CppWorkflowComponentWrapper>value)._ptr
        self._ptr.setWorkflow(workflow)

    @property
    def reference_directory(self) -> str:
        """Directory from which relative paths resolve for this component."""
        return self._ptr.referenceDirectory().decode("utf-8")

    @reference_directory.setter
    def reference_directory(self, str value):
        self._ptr.setReferenceDirectory(value.encode("utf-8"))

    # -- Checkpointing (ICheckpointableModelComponent) ----------------------

    cdef cpp.ICheckpointableModelComponent* _checkpointable(self) except NULL:
        cdef cpp.ICheckpointableModelComponent* p = cpp.asCheckpointable(
            self._ptr)
        if p == NULL:
            raise TypeError(f"component '{self.id}' does not implement "
                            "ICheckpointableModelComponent")
        return p

    def save_state(self):
        """Save the complete state; returns ``(ok, token, message)``."""
        cdef string token, msg
        cdef bint ok = self._checkpointable().saveState(token, msg)
        return bool(ok), token.decode("utf-8", "surrogateescape"), msg.decode("utf-8")

    def restore_state(self, str token):
        """Restore a saved state; returns ``(ok, message)``."""
        cdef string msg
        cdef bint ok = self._checkpointable().restoreState(
            token.encode("utf-8", "surrogateescape"), msg)
        return bool(ok), msg.decode("utf-8")

    def release_state(self, str token):
        """Release a saved state that will not be restored; returns
        ``(ok, message)``."""
        cdef string msg
        cdef bint ok = self._checkpointable().releaseState(
            token.encode("utf-8", "surrogateescape"), msg)
        return bool(ok), msg.decode("utf-8")

    # -- Differentiation (IDifferentiableModelComponent) --------------------

    cdef cpp.IDifferentiableModelComponent* _differentiable(self) except NULL:
        cdef cpp.IDifferentiableModelComponent* p = cpp.asDifferentiable(
            self._ptr)
        if p == NULL:
            raise TypeError(f"component '{self.id}' does not implement "
                            "IDifferentiableModelComponent")
        return p

    def differentiable_inputs(self) -> list:
        """Inputs a derivative reaches."""
        cdef vector[cpp.IInput*] v = self._differentiable().differentiableInputs()
        return [owned_by(_wrap_input(v[i]), self) for i in range(v.size())]

    def differentiable_arguments(self) -> list:
        """Arguments (parameters) a derivative reaches."""
        cdef vector[cpp.IArgument*] v = (
            self._differentiable().differentiableArguments())
        return [owned_by(CppArgumentWrapper.wrap_argument(v[i]), self)
                for i in range(v.size())]

    def differentiable_outputs(self) -> list:
        """Outputs whose derivative the component reports."""
        cdef vector[cpp.IOutput*] v = (
            self._differentiable().differentiableOutputs())
        return [owned_by(_wrap_output(v[i]), self) for i in range(v.size())]

    def differentiable_states(self) -> list:
        """Items that carry state from one step to the next."""
        cdef vector[cpp.IComponentDataItem*] v = (
            self._differentiable().differentiableStates())
        return [owned_by(wrap_data_item(v[i]), self)
                for i in range(v.size())]

    def vjp(self, seeds, results, *, stream=None):
        """Vector-Jacobian product of the most recent step.

        ``seeds`` / ``results`` are sequences of ``(item, role, buffer)``
        where ``item`` is a wrapper returned by the ``differentiable_*``
        methods, ``role`` a :class:`~hydrocouple.core.DifferentialRole` and
        ``buffer`` an ndarray or any DLPack tensor (device tensors
        included). Result buffers are overwritten. GIL released.
        :returns: ``(ok, message)``.
        """
        return _differential_call(self._differentiable(), seeds, results,
                                  False, stream)

    def jvp(self, seeds, results, *, stream=None):
        """Jacobian-vector product of the most recent step; see :meth:`vjp`.
        :returns: ``(ok, message)``."""
        return _differential_call(self._differentiable(), seeds, results,
                                  True, stream)

    # -- Signal/slot -------------------------------------------------------

    def connect(self, slot):
        """Connect ``slot(event_args)`` to the status signal, where
        ``event_args`` is an ``IComponentStatusChangeEventArgs`` (the ABC's
        contract for a component). Property changes are
        :meth:`on_property_changed`."""
        _connect(self, _STATUS, slot, _status_adapter(self, slot))

    def disconnect(self, slot):
        """Disconnect a slot connected with :meth:`connect`."""
        _disconnect(self, _STATUS, slot)

    def on_status_changed(self, callback):
        """Connect ``callback(previous_status, status, message,
        has_progress_monitor, percent_progress)`` to the status signal;
        returns a disconnectable handle."""
        return _new_handle(self, _STATUS, callback)


cdef object wrap_model_component(cpp.IModelComponent* ptr):
    return None if ptr == NULL else CppModelComponentWrapper.wrap(ptr)


cdef object wrap_model_component_info(cpp.IModelComponentInfo* ptr):
    return None if ptr == NULL else CppModelComponentInfoWrapper.wrap(ptr)


# ---------------------------------------------------------------------------
# CppWorkflowComponentWrapper
# ---------------------------------------------------------------------------
cdef class CppWorkflowComponentWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``HydroCouple::IWorkflowComponent`` pointer."""

    cdef cpp.IWorkflowComponent* _ptr
    cdef unique_ptr[cpp.IWorkflowComponent] _owned

    def __cinit__(self):
        self._ptr = NULL

    def __dealloc__(self):
        self._owned.reset()

    @staticmethod
    cdef CppWorkflowComponentWrapper wrap(cpp.IWorkflowComponent* ptr):
        cdef CppWorkflowComponentWrapper obj = (
            CppWorkflowComponentWrapper.__new__(
                CppWorkflowComponentWrapper))
        obj._ptr = ptr
        bind_identity(obj, <cpp.IIdentity*>ptr)
        return obj

    @property
    def component_info(self):
        """Metadata about this workflow, or ``None``."""
        cdef cpp.IWorkflowComponentInfo* info = self._ptr.componentInfo()
        if info == NULL:
            return None
        cdef CppWorkflowComponentInfoWrapper wrapper = (
            CppWorkflowComponentInfoWrapper.__new__(
                CppWorkflowComponentInfoWrapper))
        wrapper._ptr = info
        wrapper._bind_info(<cpp.IComponentInfo*>info)
        return owned_by(wrapper, self)

    @property
    def model_component_labels(self) -> list:
        """The component roles this workflow declares."""
        cdef vector[cpp.IIdentity*] labels = self._ptr.modelComponentLabels()
        return [_wrap_identity_or_none(labels[i], self)
                for i in range(labels.size())]

    def is_required_model_component(self, label) -> bool:
        """Whether the labeled component role must be filled."""
        return self._ptr.isRequiredModelComponent(_identity_pointer(label))

    @property
    def status(self):
        """Current status of the workflow."""
        from hydrocouple.core import WorkflowStatus
        return WorkflowStatus(<int>self._ptr.status())

    def initialize(self):
        """Initialize the workflow component."""
        self._ptr.initialize()

    def validate(self) -> list:
        """Validate the composition; an empty list means valid."""
        cdef vector[string] msgs = self._ptr.validate()
        return [msgs[i].decode("utf-8") for i in range(msgs.size())]

    def prepare(self):
        """Prepare every managed component and build the execution plan."""
        self._ptr.prepare()

    def update(self):
        """Advance the workflow; releases the GIL during the C++ compute."""
        with nogil:
            self._ptr.update()

    def finish(self):
        """Finalize the workflow and release resources."""
        self._ptr.finish()

    def request_stop(self):
        """Cooperative stop: takes effect after the step in flight."""
        self._ptr.requestStop()

    def request_pause(self):
        """Cooperative pause: takes effect after the step in flight."""
        self._ptr.requestPause()

    def resume(self):
        """Resume a ``Paused`` workflow; a no-op otherwise."""
        self._ptr.resume()

    def errors(self, clear_after_read=False) -> list:
        """Drain the workflow's diagnostic queue."""
        cdef vector[cpp.ErrorEntry] entries = self._ptr.errors(
            clear_after_read)
        return _python_errors(entries)

    @property
    def model_components(self) -> list:
        """Model components managed by this workflow."""
        cdef vector[cpp.IModelComponent*] comps = (
            self._ptr.modelComponents())
        return [owned_by(CppModelComponentWrapper.wrap(comps[i]), self)
                for i in range(comps.size())]

    def add_model_component(self, CppModelComponentWrapper component,
                            model_role_identifier=None) -> bool:
        """Add a model component to the workflow, optionally into a role."""
        cdef string message
        return self._ptr.addModelComponent(
            component._ptr, _identity_pointer(model_role_identifier),
            &message)

    def remove_model_component(
            self, CppModelComponentWrapper component) -> bool:
        """Remove a model component from the workflow."""
        return self._ptr.removeModelComponent(component._ptr)

    # -- Signal/slot -------------------------------------------------------

    def connect(self, slot):
        """Connect ``slot(event_args)`` to the workflow status signal,
        where ``event_args`` is an ``IWorkflowComponentStatusChangeEventArgs``.
        Property changes are :meth:`on_property_changed`."""
        _connect(self, _WORKFLOW_STATUS, slot,
                 _workflow_status_adapter(self, slot))

    def disconnect(self, slot):
        """Disconnect a slot connected with :meth:`connect`."""
        _disconnect(self, _WORKFLOW_STATUS, slot)

    def on_status_changed(self, callback):
        """Connect ``callback(previous_status, status, message,
        has_progress_monitor, percent_progress)`` to the workflow status
        signal; returns a disconnectable handle."""
        return _new_handle(self, _WORKFLOW_STATUS, callback)


cdef object wrap_workflow(cpp.IWorkflowComponent* ptr):
    return None if ptr == NULL else CppWorkflowComponentWrapper.wrap(ptr)


# ---------------------------------------------------------------------------
# PyComponentBridge
# ---------------------------------------------------------------------------
cdef class PyComponentBridge:
    """Wrap a Python ``IModelComponent`` so it can be passed to C++ code.

    The bridge owns a C++ ``PyModelComponentBridge`` instance that forwards
    every virtual call back into the wrapped Python object, including the
    v2 ``capabilities()`` and ``errors()`` contracts.

    Usage::

        from hydrocouple.core import IModelComponent
        class MyModel(IModelComponent):
            ...

        bridge = PyComponentBridge(MyModel())
        # bridge exposes a C++ IModelComponent* usable by C++ workflows
    """

    def __cinit__(self, object py_component):
        self._bridge = cpp.make_py_component_bridge(<PyObject*>py_component)

    def __dealloc__(self):
        if self._bridge != NULL:
            del self._bridge
            self._bridge = NULL

    cdef cpp.IModelComponent* ptr(self):
        """Return the underlying C++ IModelComponent pointer."""
        return <cpp.IModelComponent*>self._bridge

    def emit_property_changed(self, str property_name):
        """Emit a property-changed signal to all connected C++ slots."""
        self._bridge.emitPropertyChanged(property_name.encode("utf-8"))

    @property
    def py_object(self):
        """The wrapped Python component object."""
        return <object>self._bridge.pyObject()


# ---------------------------------------------------------------------------
# Register wrappers as virtual subclasses of the Python ABCs
# ---------------------------------------------------------------------------
#: Every (ABC, wrapper) pair this module registers. A registration asserts
#: that the wrapper implements the ABC; tests/test_wrapper_conformance.py holds
#: each wrapper to it, since ABC.register() itself checks nothing.
ABC_REGISTRATIONS = []


def _register(abc_class, wrapper):
    abc_class.register(wrapper)
    ABC_REGISTRATIONS.append((abc_class, wrapper))


def _register_abc_subclasses():
    """Register all Cpp*Wrapper types with the corresponding ABCs so
    ``isinstance`` checks against the Python ABCs work transparently."""
    import hydrocouple.core as abcs

    _register(abcs.IDescription, CppDescriptionWrapper)
    _register(abcs.IIdentity, CppIdentityWrapper)
    _register(abcs.IDimension, CppDimensionWrapper)
    _register(abcs.IValueDefinition, CppValueDefinitionWrapper)
    _register(abcs.IQuantity, CppQuantityWrapper)
    _register(abcs.IQuality, CppQualityWrapper)
    _register(abcs.IUnit, CppUnitWrapper)
    _register(abcs.IUnitDimensions, CppUnitDimensionsWrapper)
    _register(abcs.IComponentDataItem, CppComponentDataItemWrapper)
    _register(abcs.IArgument, CppArgumentWrapper)
    _register(abcs.IInput, CppInputWrapper)
    _register(abcs.IMultiInput, CppMultiInputWrapper)
    _register(abcs.IOutput, CppOutputWrapper)
    _register(abcs.IAdaptedOutput, CppAdaptedOutputWrapper)
    _register(abcs.IAdaptedOutputFactory, CppAdaptedOutputFactoryWrapper)
    _register(abcs.IComponentInfo, CppComponentInfoWrapper)
    _register(abcs.IModelComponentInfo, CppModelComponentInfoWrapper)
    _register(abcs.IWorkflowComponentInfo, CppWorkflowComponentInfoWrapper)
    _register(abcs.IModelComponent, CppModelComponentWrapper)
    _register(abcs.IWorkflowComponent, CppWorkflowComponentWrapper)


# Perform registration at import time
_register_abc_subclasses()


# ---------------------------------------------------------------------------
# Component loader
# ---------------------------------------------------------------------------
from posix.dlfcn cimport dlopen, dlsym, dlclose, dlerror, RTLD_LAZY

ctypedef cpp.IModelComponentInfo* (*ComponentInfoFactory)()


cdef class LoadedLibrary:
    """Wraps a ``dlopen`` handle and ensures ``dlclose`` on deallocation.

    Keep this alive for the entire lifetime of any loaded component.
    """

    cdef void* _handle
    cdef str _path

    def __cinit__(self):
        self._handle = NULL

    def __dealloc__(self):
        if self._handle != NULL:
            dlclose(self._handle)
            self._handle = NULL

    @property
    def path(self) -> str:
        """File-system path of the loaded shared library."""
        return self._path


def load_component(str library_path, str symbol_name="CreateComponentInfo"):
    """Load a compiled C++ HydroCouple component from a shared library.

    :param library_path: Path to the shared library (.so / .dylib / .dll).
    :param symbol_name: Exported ``extern "C"`` factory returning an
        ``IModelComponentInfo*``.
    :returns: ``(component, component_info, library_handle)``; keep the
        handle alive for the component's lifetime.
    :raises OSError: if the library or symbol cannot be loaded.
    :raises RuntimeError: if a factory returns null.
    """
    cdef bytes c_path = library_path.encode("utf-8")
    cdef bytes c_sym = symbol_name.encode("utf-8")
    cdef const char* err_msg
    cdef void* sym

    cdef LoadedLibrary lib = LoadedLibrary.__new__(LoadedLibrary)
    lib._path = library_path
    lib._handle = dlopen(c_path, RTLD_LAZY)
    if lib._handle == NULL:
        err_msg = dlerror()
        raise OSError(
            f"Cannot load library '{library_path}': "
            f"{err_msg.decode('utf-8') if err_msg != NULL else 'unknown error'}"
        )

    sym = dlsym(lib._handle, c_sym)
    if sym == NULL:
        err_msg = dlerror()
        raise OSError(
            f"Symbol '{symbol_name}' not found in '{library_path}': "
            f"{err_msg.decode('utf-8') if err_msg != NULL else 'unknown error'}"
        )

    cdef ComponentInfoFactory factory = <ComponentInfoFactory>sym
    cdef cpp.IModelComponentInfo* info = factory()
    if info == NULL:
        raise RuntimeError(
            f"Factory '{symbol_name}' in '{library_path}' returned null"
        )

    cdef unique_ptr[cpp.IModelComponent] comp = info.createComponentInstance()
    if comp.get() == NULL:
        raise RuntimeError(
            f"createComponentInstance() returned null for '{library_path}'"
        )

    cdef cpp.IModelComponent* comp_ptr = comp.release()

    # The info lives in the library: it keeps the library open, and the
    # component (deliberately never destroyed, see above) keeps the info.
    info_wrapper = owned_by(CppModelComponentInfoWrapper.wrap(info), lib)
    return (
        owned_by(CppModelComponentWrapper.wrap(comp_ptr), info_wrapper),
        info_wrapper,
        lib,
    )
