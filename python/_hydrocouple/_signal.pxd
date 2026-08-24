# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ signal/slot bridge classes defined in
``signal_bridge.h``.
"""

from libcpp.memory cimport shared_ptr
from cpython.ref cimport PyObject

cimport _hydrocouple._core as cpp


cdef extern from "signal_bridge.h" namespace "HydroCouple::Python":

    # -- Bridge slot classes (opaque to Cython) -----------------------------
    cdef cppclass StatusSlotBridge:
        pass

    cdef cppclass DataItemValueSlotBridge:
        pass

    cdef cppclass PropertySlotBridge:
        pass

    cdef cppclass WorkflowStatusSlotBridge:
        pass

    # -- Factory functions --------------------------------------------------
    shared_ptr[StatusSlotBridge] make_status_slot(PyObject* cb)
    shared_ptr[DataItemValueSlotBridge] make_data_item_value_slot(PyObject* cb)
    shared_ptr[PropertySlotBridge] make_property_slot(PyObject* cb)
    shared_ptr[WorkflowStatusSlotBridge] make_workflow_status_slot(PyObject* cb)

    # -- Connect / disconnect: component status -----------------------------
    void connect_status_slot(
        cpp.IModelComponent* comp,
        const shared_ptr[StatusSlotBridge]& slot)
    void disconnect_status_slot(
        cpp.IModelComponent* comp,
        const shared_ptr[StatusSlotBridge]& slot)

    # -- Connect / disconnect: data-item value changed ----------------------
    void connect_value_changed_slot(
        cpp.IComponentDataItem* item,
        const shared_ptr[DataItemValueSlotBridge]& slot)
    void disconnect_value_changed_slot(
        cpp.IComponentDataItem* item,
        const shared_ptr[DataItemValueSlotBridge]& slot)

    # -- Connect / disconnect: property changed on IModelComponent ----------
    void connect_property_slot_comp(
        cpp.IModelComponent* comp,
        const shared_ptr[PropertySlotBridge]& slot)
    void disconnect_property_slot_comp(
        cpp.IModelComponent* comp,
        const shared_ptr[PropertySlotBridge]& slot)

    # -- Connect / disconnect: property changed on IComponentDataItem -------
    void connect_property_slot_item(
        cpp.IComponentDataItem* item,
        const shared_ptr[PropertySlotBridge]& slot)
    void disconnect_property_slot_item(
        cpp.IComponentDataItem* item,
        const shared_ptr[PropertySlotBridge]& slot)

    # -- Connect / disconnect: property changed on IWorkflowComponent -------
    void connect_property_slot_workflow(
        cpp.IWorkflowComponent* comp,
        const shared_ptr[PropertySlotBridge]& slot)
    void disconnect_property_slot_workflow(
        cpp.IWorkflowComponent* comp,
        const shared_ptr[PropertySlotBridge]& slot)

    # -- Connect / disconnect: workflow status ------------------------------
    void connect_workflow_status_slot(
        cpp.IWorkflowComponent* comp,
        const shared_ptr[WorkflowStatusSlotBridge]& slot)
    void disconnect_workflow_status_slot(
        cpp.IWorkflowComponent* comp,
        const shared_ptr[WorkflowStatusSlotBridge]& slot)
