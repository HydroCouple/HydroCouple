# distutils: language = c++
# cython: language_level = 3
"""
Cython wrapper classes for the core HydroCouple v2.0.0 C++ interfaces.

Each ``Cpp*Wrapper`` class holds a borrowed C++ pointer and exposes a
Pythonic API. After construction the wrappers are registered as virtual
subclasses of the corresponding Python ABCs so that ``isinstance`` checks
work transparently.

Data plane: ``get_values_into`` / ``set_values_from`` convert NumPy arrays
to C++ ``BufferDescriptor`` views **zero-copy** — the descriptor carries
the array's data pointer, DataKind (from dtype), shape, and byte strides —
and release the GIL around the C++ virtual call.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.set cimport set as cppset
from libcpp.memory cimport shared_ptr
from libc.stdint cimport int64_t
from cpython.ref cimport PyObject

cimport _hydrocouple._core as cpp
cimport _hydrocouple._signal as bridge

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


cdef tuple _get_values_into(cpp.IComponentDataItem* ptr, object destination,
                            object start, object count):
    """Shared implementation of the typed hyperslab read."""
    cdef cnp.ndarray arr = np.asarray(destination)
    if arr is not destination:
        raise TypeError("destination must be a numpy.ndarray")
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
                            object start, object count):
    """Shared implementation of the typed hyperslab write."""
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
# Slot handle classes
# ---------------------------------------------------------------------------
cdef class _StatusSlotHandle:
    """Handle for a component status-changed callback connection."""

    cdef shared_ptr[bridge.StatusSlotBridge] _slot
    cdef cpp.IModelComponent* _component
    cdef bint _connected

    def disconnect(self):
        """Disconnect this slot; a no-op if already disconnected."""
        if self._connected:
            bridge.disconnect_status_slot(self._component, self._slot)
            self._connected = False

    @property
    def connected(self) -> bool:
        """Whether the slot is currently connected."""
        return self._connected


cdef class _ValueChangedSlotHandle:
    """Handle for a data-item value-changed callback connection."""

    cdef shared_ptr[bridge.DataItemValueSlotBridge] _slot
    cdef cpp.IComponentDataItem* _item
    cdef bint _connected

    def disconnect(self):
        """Disconnect this slot; a no-op if already disconnected."""
        if self._connected:
            bridge.disconnect_value_changed_slot(self._item, self._slot)
            self._connected = False

    @property
    def connected(self) -> bool:
        """Whether the slot is currently connected."""
        return self._connected


cdef class _PropertySlotHandleComp:
    """Handle for a property-changed callback on an IModelComponent."""

    cdef shared_ptr[bridge.PropertySlotBridge] _slot
    cdef cpp.IModelComponent* _component
    cdef bint _connected

    def disconnect(self):
        """Disconnect this slot; a no-op if already disconnected."""
        if self._connected:
            bridge.disconnect_property_slot_comp(self._component, self._slot)
            self._connected = False

    @property
    def connected(self) -> bool:
        """Whether the slot is currently connected."""
        return self._connected


cdef class _PropertySlotHandleItem:
    """Handle for a property-changed callback on an IComponentDataItem."""

    cdef shared_ptr[bridge.PropertySlotBridge] _slot
    cdef cpp.IComponentDataItem* _item
    cdef bint _connected

    def disconnect(self):
        """Disconnect this slot; a no-op if already disconnected."""
        if self._connected:
            bridge.disconnect_property_slot_item(self._item, self._slot)
            self._connected = False

    @property
    def connected(self) -> bool:
        """Whether the slot is currently connected."""
        return self._connected


cdef class _PropertySlotHandleWorkflow:
    """Handle for a property-changed callback on an IWorkflowComponent."""

    cdef shared_ptr[bridge.PropertySlotBridge] _slot
    cdef cpp.IWorkflowComponent* _component
    cdef bint _connected

    def disconnect(self):
        """Disconnect this slot; a no-op if already disconnected."""
        if self._connected:
            bridge.disconnect_property_slot_workflow(
                self._component, self._slot)
            self._connected = False

    @property
    def connected(self) -> bool:
        """Whether the slot is currently connected."""
        return self._connected


cdef class _WorkflowStatusSlotHandle:
    """Handle for a workflow status-changed callback connection."""

    cdef shared_ptr[bridge.WorkflowStatusSlotBridge] _slot
    cdef cpp.IWorkflowComponent* _component
    cdef bint _connected

    def disconnect(self):
        """Disconnect this slot; a no-op if already disconnected."""
        if self._connected:
            bridge.disconnect_workflow_status_slot(
                self._component, self._slot)
            self._connected = False

    @property
    def connected(self) -> bool:
        """Whether the slot is currently connected."""
        return self._connected


# ---------------------------------------------------------------------------
# CppDimensionWrapper
# ---------------------------------------------------------------------------
cdef class CppDimensionWrapper:
    """Wrapper around a C++ ``HydroCouple::IDimension`` pointer."""

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppDimensionWrapper wrap(cpp.IDimension* ptr):
        cdef CppDimensionWrapper obj = CppDimensionWrapper.__new__(
            CppDimensionWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for the dimension."""
        return self._ptr.id().decode("utf-8")

    @property
    def caption(self) -> str:
        """Human-readable caption for the dimension."""
        return self._ptr.caption().decode("utf-8")

    @caption.setter
    def caption(self, str value):
        self._ptr.setCaption(value.encode("utf-8"))

    @property
    def description(self) -> str:
        """Detailed description of the dimension."""
        return self._ptr.description().decode("utf-8")

    @description.setter
    def description(self, str value):
        self._ptr.setDescription(value.encode("utf-8"))

    @property
    def length_type(self):
        """The length type of the dimension (static or dynamic)."""
        from hydrocouple.core import LengthType
        return LengthType(<int>self._ptr.lengthType())


# ---------------------------------------------------------------------------
# CppValueDefinitionWrapper
# ---------------------------------------------------------------------------
cdef class CppValueDefinitionWrapper:
    """Wrapper around a C++ ``HydroCouple::IValueDefinition`` pointer."""

    cdef cpp.IValueDefinition* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppValueDefinitionWrapper wrap(cpp.IValueDefinition* ptr):
        cdef CppValueDefinitionWrapper obj = (
            CppValueDefinitionWrapper.__new__(CppValueDefinitionWrapper))
        obj._ptr = ptr
        return obj

    @property
    def caption(self) -> str:
        """Human-readable caption for this value definition."""
        return self._ptr.caption().decode("utf-8")

    @property
    def description(self) -> str:
        """Detailed description of this value definition."""
        return self._ptr.description().decode("utf-8")

    @property
    def missing_value(self) -> float:
        """The sentinel value used to indicate missing data (numeric
        DataKinds only)."""
        return self._ptr.missingValue()

    @property
    def default_value(self) -> float:
        """The default value (numeric DataKinds only)."""
        return self._ptr.defaultValue()


# ---------------------------------------------------------------------------
# CppComponentDataItemWrapper — the typed data plane
# ---------------------------------------------------------------------------
cdef class CppComponentDataItemWrapper:
    """Wrapper around a C++ ``HydroCouple::IComponentDataItem`` pointer.

    Exposes the v2 typed hyperslab data plane: ``get_values_into`` /
    ``set_values_from`` marshal NumPy arrays as zero-copy
    ``BufferDescriptor`` views and release the GIL around the C++ call.
    """

    cdef cpp.IComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppComponentDataItemWrapper wrap(cpp.IComponentDataItem* ptr):
        cdef CppComponentDataItemWrapper obj = (
            CppComponentDataItemWrapper.__new__(CppComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return self._ptr.id().decode("utf-8")

    @property
    def caption(self) -> str:
        """Human-readable caption for this data item."""
        return self._ptr.caption().decode("utf-8")

    @property
    def description(self) -> str:
        """Detailed description of this data item."""
        return self._ptr.description().decode("utf-8")

    @property
    def dimensions(self) -> list:
        """Dimension metadata objects, parallel to :attr:`shape`."""
        cdef vector[cpp.IDimension*] dims = self._ptr.dimensions()
        return [CppDimensionWrapper.wrap(dims[i])
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
        """The value definition of the stored values."""
        return CppValueDefinitionWrapper.wrap(self._ptr.valueDefinition())

    def get_values_into(self, destination, start, count):
        """Copy a hyperslab into ``destination`` (zero-copy descriptor).

        :param destination: writable ndarray whose dtype corresponds to
            :attr:`data_kind`; may be a non-contiguous view.
        :param start: first index of the selection per dimension.
        :param count: selection extent per dimension.
        :returns: ``(ok, message)``.
        """
        return _get_values_into(self._ptr, destination, start, count)

    def set_values_from(self, source, start, count):
        """Copy values from ``source`` into a hyperslab of this item.

        :returns: ``(ok, message)``.
        """
        return _set_values_from(self._ptr, source, start, count)

    # -- Signal/slot -------------------------------------------------------

    def on_value_changed(self, callback):
        """Connect ``callback(start: list[int], count: list[int])`` to the
        value-changed signal; returns a disconnectable handle."""
        cdef shared_ptr[bridge.DataItemValueSlotBridge] slot = (
            bridge.make_data_item_value_slot(<PyObject*>callback))
        bridge.connect_value_changed_slot(self._ptr, slot)
        cdef _ValueChangedSlotHandle h = _ValueChangedSlotHandle.__new__(
            _ValueChangedSlotHandle)
        h._slot = slot
        h._item = self._ptr
        h._connected = True
        return h

    def on_property_changed(self, callback):
        """Connect ``callback(property_name: str)`` to the property-changed
        signal; returns a disconnectable handle."""
        cdef shared_ptr[bridge.PropertySlotBridge] slot = (
            bridge.make_property_slot(<PyObject*>callback))
        bridge.connect_property_slot_item(self._ptr, slot)
        cdef _PropertySlotHandleItem h = _PropertySlotHandleItem.__new__(
            _PropertySlotHandleItem)
        h._slot = slot
        h._item = self._ptr
        h._connected = True
        return h


# ---------------------------------------------------------------------------
# CppArgumentWrapper
# ---------------------------------------------------------------------------
cdef class CppArgumentWrapper:
    """Wrapper around a C++ ``HydroCouple::IArgument`` pointer."""

    cdef cpp.IArgument* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppArgumentWrapper wrap(cpp.IArgument* ptr):
        cdef CppArgumentWrapper obj = CppArgumentWrapper.__new__(
            CppArgumentWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this argument."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def caption(self) -> str:
        """Human-readable caption for this argument."""
        return (<cpp.IComponentDataItem*>self._ptr).caption().decode("utf-8")

    @property
    def is_optional(self) -> bool:
        """Whether this argument is optional."""
        return self._ptr.isOptional()

    @property
    def is_read_only(self) -> bool:
        """Whether this argument is read-only."""
        return self._ptr.isReadOnly()

    def __str__(self) -> str:
        return self._ptr.toString().decode("utf-8")

    def save_data(self):
        """Write data to files associated with this argument, if any."""
        self._ptr.saveData()

    @property
    def file_filters(self) -> list:
        """File filter strings readable by this argument."""
        cdef vector[string] ff = self._ptr.fileFilters()
        return [ff[i].decode("utf-8") for i in range(ff.size())]

    def is_valid_arg_type(self, arg_type) -> bool:
        """Whether the given input representation is supported."""
        return self._ptr.isValidArgType(
            <cpp.IArgument_ArgumentInputType><int>arg_type)

    @property
    def current_argument_input_type(self):
        """How this argument was initialized."""
        from hydrocouple.core import ArgumentInputType
        return ArgumentInputType(<int>self._ptr.currentArgumentInputType())

    def initialize(self, str value, arg_type):
        """Read the argument value from a string representation.

        :returns: ``(ok, message)``.
        """
        cdef string msg
        cdef bint ok = self._ptr.initialize(
            value.encode("utf-8"),
            <cpp.IArgument_ArgumentInputType><int>arg_type, msg)
        return bool(ok), msg.decode("utf-8")

    def serialize(self, arg_type):
        """Serialize the current value to the requested representation.

        :returns: ``(ok, value, message)``.
        """
        cdef string out
        cdef string msg
        cdef bint ok = self._ptr.serialize(
            <cpp.IArgument_ArgumentInputType><int>arg_type, out, msg)
        return bool(ok), out.decode("utf-8"), msg.decode("utf-8")

    # -- Data plane (inherited from IComponentDataItem) --------------------

    @property
    def shape(self) -> tuple:
        """The extent of each dimension."""
        cdef vector[int64_t] s = (
            <cpp.IComponentDataItem*>self._ptr).shape()
        return tuple(s[i] for i in range(s.size()))

    @property
    def data_kind(self):
        """The element type of this argument's values."""
        from hydrocouple.core import DataKind
        return DataKind(<int>(<cpp.IComponentDataItem*>self._ptr).dataKind())

    def get_values_into(self, destination, start, count):
        """Typed hyperslab read; returns ``(ok, message)``."""
        return _get_values_into(
            <cpp.IComponentDataItem*>self._ptr, destination, start, count)

    def set_values_from(self, source, start, count):
        """Typed hyperslab write; returns ``(ok, message)``."""
        return _set_values_from(
            <cpp.IComponentDataItem*>self._ptr, source, start, count)


# ---------------------------------------------------------------------------
# CppInputWrapper
# ---------------------------------------------------------------------------
cdef class CppInputWrapper:
    """Wrapper around a C++ ``HydroCouple::IInput`` pointer."""

    cdef cpp.IInput* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppInputWrapper wrap(cpp.IInput* ptr):
        cdef CppInputWrapper obj = CppInputWrapper.__new__(CppInputWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this input exchange item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def caption(self) -> str:
        """Human-readable caption for this input exchange item."""
        return (<cpp.IComponentDataItem*>self._ptr).caption().decode("utf-8")

    @property
    def provider(self):
        """The output providing data to this input, or ``None``."""
        cdef cpp.IOutput* p = self._ptr.provider()
        if p == NULL:
            return None
        return CppOutputWrapper.wrap(p)

    def set_provider(self, CppOutputWrapper provider) -> bool:
        """Assign an output as the provider for this input."""
        return self._ptr.setProvider(provider._ptr)

    def can_consume(self, CppOutputWrapper provider):
        """Whether this input can consume the given output.

        :returns: ``(ok, message)``.
        """
        cdef string msg
        cdef bint ok = self._ptr.canConsume(provider._ptr, msg)
        return bool(ok), msg.decode("utf-8")

    # -- Data plane ---------------------------------------------------------

    @property
    def shape(self) -> tuple:
        """The extent of each dimension."""
        cdef vector[int64_t] s = (
            <cpp.IComponentDataItem*>self._ptr).shape()
        return tuple(s[i] for i in range(s.size()))

    @property
    def data_kind(self):
        """The element type of this input's values."""
        from hydrocouple.core import DataKind
        return DataKind(<int>(<cpp.IComponentDataItem*>self._ptr).dataKind())

    def get_values_into(self, destination, start, count):
        """Typed hyperslab read; returns ``(ok, message)``."""
        return _get_values_into(
            <cpp.IComponentDataItem*>self._ptr, destination, start, count)

    def set_values_from(self, source, start, count):
        """Typed hyperslab write; returns ``(ok, message)``."""
        return _set_values_from(
            <cpp.IComponentDataItem*>self._ptr, source, start, count)


# ---------------------------------------------------------------------------
# CppOutputWrapper
# ---------------------------------------------------------------------------
cdef class CppOutputWrapper:
    """Wrapper around a C++ ``HydroCouple::IOutput`` pointer."""

    cdef cpp.IOutput* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppOutputWrapper wrap(cpp.IOutput* ptr):
        cdef CppOutputWrapper obj = CppOutputWrapper.__new__(CppOutputWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this output exchange item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def caption(self) -> str:
        """Human-readable caption for this output exchange item."""
        return (<cpp.IComponentDataItem*>self._ptr).caption().decode("utf-8")

    @property
    def consumers(self) -> list:
        """Inputs currently consuming from this output."""
        cdef vector[cpp.IInput*] c = self._ptr.consumers()
        return [CppInputWrapper.wrap(c[i]) for i in range(c.size())]

    def add_consumer(self, CppInputWrapper consumer):
        """Register an input as a consumer of this output."""
        self._ptr.addConsumer(consumer._ptr)

    def remove_consumer(self, CppInputWrapper consumer) -> bool:
        """Remove an input from this output's consumer list."""
        return self._ptr.removeConsumer(consumer._ptr)

    def update_values(self, CppInputWrapper query_specifier):
        """Request the output to update its values for the query specifier.

        Releases the GIL while the C++ side computes.
        """
        cdef cpp.IInput* q = query_specifier._ptr
        with nogil:
            self._ptr.updateValues(q)

    # -- Data plane ---------------------------------------------------------

    @property
    def shape(self) -> tuple:
        """The extent of each dimension."""
        cdef vector[int64_t] s = (
            <cpp.IComponentDataItem*>self._ptr).shape()
        return tuple(s[i] for i in range(s.size()))

    @property
    def data_kind(self):
        """The element type of this output's values."""
        from hydrocouple.core import DataKind
        return DataKind(<int>(<cpp.IComponentDataItem*>self._ptr).dataKind())

    def get_values_into(self, destination, start, count):
        """Typed hyperslab read; returns ``(ok, message)``."""
        return _get_values_into(
            <cpp.IComponentDataItem*>self._ptr, destination, start, count)

    def set_values_from(self, source, start, count):
        """Typed hyperslab write; returns ``(ok, message)``."""
        return _set_values_from(
            <cpp.IComponentDataItem*>self._ptr, source, start, count)


# ---------------------------------------------------------------------------
# CppModelComponentInfoWrapper
# ---------------------------------------------------------------------------
cdef class CppModelComponentInfoWrapper:
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
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this component info."""
        return (<cpp.IIdentity*>self._ptr).id().decode("utf-8")

    @property
    def caption(self) -> str:
        """Human-readable caption for this component info."""
        return (<cpp.IDescription*>self._ptr).caption().decode("utf-8")

    @property
    def description(self) -> str:
        """Detailed description of this component."""
        return (<cpp.IDescription*>self._ptr).description().decode("utf-8")

    @property
    def developer(self) -> str:
        """Name of the component developer or organization."""
        return (<cpp.IComponentInfo*>self._ptr).developer().decode("utf-8")

    @property
    def version(self) -> str:
        """Version string of this component."""
        return (<cpp.IComponentInfo*>self._ptr).version().decode("utf-8")

    @property
    def license(self) -> str:
        """License under which this component is distributed."""
        return (<cpp.IComponentInfo*>self._ptr).license().decode("utf-8")

    @property
    def url(self) -> str:
        """URL for the component's homepage or repository."""
        return (<cpp.IComponentInfo*>self._ptr).url().decode("utf-8")

    @property
    def email(self) -> str:
        """Contact email for the component developer."""
        return (<cpp.IComponentInfo*>self._ptr).email().decode("utf-8")

    @property
    def documentation(self) -> list:
        """Documentation references for this component."""
        cdef vector[string] docs = (
            <cpp.IComponentInfo*>self._ptr).documentation()
        return [docs[i].decode("utf-8") for i in range(docs.size())]


# ---------------------------------------------------------------------------
# CppModelComponentWrapper
# ---------------------------------------------------------------------------
cdef class CppModelComponentWrapper:
    """Wrapper around a C++ ``HydroCouple::IModelComponent`` pointer.

    Exposes the full lifecycle, exchange items, capabilities, and the
    error queue. ``update()`` releases the GIL while the component
    computes, so multiple C++ components can advance concurrently from
    Python threads.
    """

    cdef cpp.IModelComponent* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppModelComponentWrapper wrap(cpp.IModelComponent* ptr):
        cdef CppModelComponentWrapper obj = (
            CppModelComponentWrapper.__new__(CppModelComponentWrapper))
        obj._ptr = ptr
        return obj

    # -- IIdentity / IDescription ------------------------------------------

    @property
    def id(self) -> str:
        """Unique identifier for this model component."""
        return self._ptr.id().decode("utf-8")

    @property
    def caption(self) -> str:
        """Human-readable caption for this model component."""
        return (<cpp.IDescription*>self._ptr).caption().decode("utf-8")

    @caption.setter
    def caption(self, str value):
        (<cpp.IDescription*>self._ptr).setCaption(value.encode("utf-8"))

    @property
    def description(self) -> str:
        """Detailed description of this model component."""
        return (<cpp.IDescription*>self._ptr).description().decode("utf-8")

    @description.setter
    def description(self, str value):
        (<cpp.IDescription*>self._ptr).setDescription(value.encode("utf-8"))

    # -- IModelComponent ---------------------------------------------------

    @property
    def component_info(self):
        """Metadata information object for this component."""
        return CppModelComponentInfoWrapper.wrap(self._ptr.componentInfo())

    @property
    def status(self):
        """Current lifecycle status of this component."""
        from hydrocouple.core import ComponentStatus
        return ComponentStatus(<int>self._ptr.status())

    @property
    def arguments(self) -> list:
        """Arguments that configure this component."""
        cdef vector[cpp.IArgument*] args = self._ptr.arguments()
        return [CppArgumentWrapper.wrap(args[i])
                for i in range(args.size())]

    @property
    def inputs(self) -> list:
        """Input exchange items for this component."""
        cdef vector[cpp.IInput*] inp = self._ptr.inputs()
        return [CppInputWrapper.wrap(inp[i]) for i in range(inp.size())]

    @property
    def outputs(self) -> list:
        """Output exchange items for this component."""
        cdef vector[cpp.IOutput*] out = self._ptr.outputs()
        return [CppOutputWrapper.wrap(out[i]) for i in range(out.size())]

    @property
    def results(self) -> list:
        """Result data items produced by this component."""
        cdef vector[cpp.IComponentDataItem*] res = self._ptr.results()
        return [CppComponentDataItemWrapper.wrap(res[i])
                for i in range(res.size())]

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
        """Advance the component; releases the GIL during the C++ compute."""
        cdef vector[cpp.IOutput*] empty
        with nogil:
            self._ptr.update(empty)

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
        from hydrocouple.core import ErrorEntry as PyErrorEntry
        cdef vector[cpp.ErrorEntry] entries = self._ptr.errors(
            clear_after_read)
        result = []
        for i in range(entries.size()):
            result.append(PyErrorEntry(
                severity=PyErrorEntry.Severity(<int>entries[i].severity),
                code=entries[i].code,
                source=entries[i].source.decode("utf-8"),
                message=entries[i].message.decode("utf-8"),
            ))
        return result

    # -- Signal/slot -------------------------------------------------------

    def on_status_changed(self, callback):
        """Connect ``callback(previous_status, status, message,
        has_progress_monitor, percent_progress)`` to the status signal;
        returns a disconnectable handle."""
        cdef shared_ptr[bridge.StatusSlotBridge] slot = (
            bridge.make_status_slot(<PyObject*>callback))
        bridge.connect_status_slot(self._ptr, slot)
        cdef _StatusSlotHandle h = _StatusSlotHandle.__new__(
            _StatusSlotHandle)
        h._slot = slot
        h._component = self._ptr
        h._connected = True
        return h

    def on_property_changed(self, callback):
        """Connect ``callback(property_name: str)`` to the property signal;
        returns a disconnectable handle."""
        cdef shared_ptr[bridge.PropertySlotBridge] slot = (
            bridge.make_property_slot(<PyObject*>callback))
        bridge.connect_property_slot_comp(self._ptr, slot)
        cdef _PropertySlotHandleComp h = _PropertySlotHandleComp.__new__(
            _PropertySlotHandleComp)
        h._slot = slot
        h._component = self._ptr
        h._connected = True
        return h

    # -- Misc --------------------------------------------------------------

    @property
    def reference_directory(self) -> str:
        """Directory from which relative paths resolve for this component."""
        return self._ptr.referenceDirectory().decode("utf-8")

    @reference_directory.setter
    def reference_directory(self, str value):
        self._ptr.setReferenceDirectory(value.encode("utf-8"))


# ---------------------------------------------------------------------------
# CppWorkflowComponentWrapper
# ---------------------------------------------------------------------------
cdef class CppWorkflowComponentWrapper:
    """Wrapper around a C++ ``HydroCouple::IWorkflowComponent`` pointer."""

    cdef cpp.IWorkflowComponent* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppWorkflowComponentWrapper wrap(cpp.IWorkflowComponent* ptr):
        cdef CppWorkflowComponentWrapper obj = (
            CppWorkflowComponentWrapper.__new__(
                CppWorkflowComponentWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this workflow component."""
        return (<cpp.IIdentity*>self._ptr).id().decode("utf-8")

    @property
    def status(self):
        """Current status of the workflow."""
        from hydrocouple.core import WorkflowStatus
        return WorkflowStatus(<int>self._ptr.status())

    def initialize(self):
        """Initialize the workflow component."""
        self._ptr.initialize()

    def update(self):
        """Advance the workflow; releases the GIL during the C++ compute."""
        with nogil:
            self._ptr.update()

    def finish(self):
        """Finalize the workflow and release resources."""
        self._ptr.finish()

    @property
    def model_components(self) -> list:
        """Model components managed by this workflow."""
        cdef vector[cpp.IModelComponent*] comps = (
            self._ptr.modelComponents())
        return [CppModelComponentWrapper.wrap(comps[i])
                for i in range(comps.size())]

    def add_model_component(self, CppModelComponentWrapper component,
                            model_role_identifier=None) -> bool:
        """Add a model component to the workflow."""
        return self._ptr.addModelComponent(component._ptr, NULL)

    def remove_model_component(
            self, CppModelComponentWrapper component) -> bool:
        """Remove a model component from the workflow."""
        return self._ptr.removeModelComponent(component._ptr)

    # -- Signal/slot -------------------------------------------------------

    def on_status_changed(self, callback):
        """Connect ``callback(previous_status, status, message,
        has_progress_monitor, percent_progress)`` to the workflow status
        signal; returns a disconnectable handle."""
        cdef shared_ptr[bridge.WorkflowStatusSlotBridge] slot = (
            bridge.make_workflow_status_slot(<PyObject*>callback))
        bridge.connect_workflow_status_slot(self._ptr, slot)
        cdef _WorkflowStatusSlotHandle h = (
            _WorkflowStatusSlotHandle.__new__(_WorkflowStatusSlotHandle))
        h._slot = slot
        h._component = self._ptr
        h._connected = True
        return h

    def on_property_changed(self, callback):
        """Connect ``callback(property_name: str)`` to the property signal;
        returns a disconnectable handle."""
        cdef shared_ptr[bridge.PropertySlotBridge] slot = (
            bridge.make_property_slot(<PyObject*>callback))
        bridge.connect_property_slot_workflow(self._ptr, slot)
        cdef _PropertySlotHandleWorkflow h = (
            _PropertySlotHandleWorkflow.__new__(_PropertySlotHandleWorkflow))
        h._slot = slot
        h._component = self._ptr
        h._connected = True
        return h


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
def _register_abc_subclasses():
    """Register all Cpp*Wrapper types with the corresponding ABCs so
    ``isinstance`` checks against the Python ABCs work transparently."""
    from hydrocouple.core import (
        IArgument as PyIArgument,
        IComponentDataItem as PyIComponentDataItem,
        IDimension as PyIDimension,
        IInput as PyIInput,
        IModelComponent as PyIModelComponent,
        IModelComponentInfo as PyIModelComponentInfo,
        IOutput as PyIOutput,
        IValueDefinition as PyIValueDefinition,
        IWorkflowComponent as PyIWorkflowComponent,
    )

    PyIDimension.register(CppDimensionWrapper)
    PyIValueDefinition.register(CppValueDefinitionWrapper)
    PyIComponentDataItem.register(CppComponentDataItemWrapper)
    PyIArgument.register(CppArgumentWrapper)
    PyIInput.register(CppInputWrapper)
    PyIOutput.register(CppOutputWrapper)
    PyIModelComponentInfo.register(CppModelComponentInfoWrapper)
    PyIModelComponent.register(CppModelComponentWrapper)
    PyIWorkflowComponent.register(CppWorkflowComponentWrapper)


# Perform registration at import time
_register_abc_subclasses()


# ---------------------------------------------------------------------------
# Component loader
# ---------------------------------------------------------------------------
from posix.dlfcn cimport dlopen, dlsym, dlclose, dlerror, RTLD_LAZY
from libcpp.memory cimport unique_ptr

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

    return (
        CppModelComponentWrapper.wrap(comp_ptr),
        CppModelComponentInfoWrapper.wrap(info),
        lib,
    )
