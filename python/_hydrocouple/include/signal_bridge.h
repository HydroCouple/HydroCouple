/*
 * signal_bridge.h
 * C++ bridge classes that implement ISlot<> specializations and forward
 * signal notifications to Python callables via the Python C-API.
 *
 * Each bridge:
 *   1. Holds a reference-counted PyObject* callback.
 *   2. Acquires the GIL in operator() before calling into Python.
 *   3. Releases the GIL when done.
 *
 * Free-standing helper functions avoid exposing C++ template instantiations
 * to Cython (which cannot express them directly).
 */
#pragma once

// Python.h MUST come before any standard header.
#include <Python.h>

#include <memory>
#include <string>
#include <vector>
#include "hydrocouple.h"

namespace HydroCouple {
namespace Python {

// ============================================================================
// StatusSlotBridge
//   ISlot< const shared_ptr<IComponentStatusChangeEventArgs>& >
//   Callback signature:
//     callback(previous_status: int, status: int, message: str,
//              has_progress: bool, percent_progress: float)
// ============================================================================
class StatusSlotBridge
    : public ISlot<const std::shared_ptr<IComponentStatusChangeEventArgs> &>
{
    PyObject *m_callback;

  public:
    explicit StatusSlotBridge(PyObject *cb) : m_callback(cb)
    {
        Py_XINCREF(m_callback);
    }

    ~StatusSlotBridge() override
    {
        if (m_callback)
        {
            PyGILState_STATE gs = PyGILState_Ensure();
            Py_DECREF(m_callback);
            PyGILState_Release(gs);
        }
    }

    // Non-copyable
    StatusSlotBridge(const StatusSlotBridge &) = delete;
    StatusSlotBridge &operator=(const StatusSlotBridge &) = delete;

    void operator()(
        const ISignal<const std::shared_ptr<IComponentStatusChangeEventArgs> &> & /*sender*/,
        const std::shared_ptr<IComponentStatusChangeEventArgs> &args) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *py_args = Py_BuildValue(
            "(iisOf)",
            static_cast<int>(args->previousStatus()),
            static_cast<int>(args->status()),
            args->message().c_str(),
            args->hasProgressMonitor() ? Py_True : Py_False,
            static_cast<double>(args->percentProgress()));
        if (py_args)
        {
            PyObject *result = PyObject_CallObject(m_callback, py_args);
            Py_XDECREF(result);
            Py_DECREF(py_args);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }
};

// ============================================================================
// DataItemValueSlotBridge
//   ISlot< const shared_ptr<IComponentDataItemValueChanged>& >
//   Callback signature:
//     callback(start: list[int], count: list[int])
// ============================================================================

namespace detail {
inline PyObject *int64_vec_to_list(const std::vector<int64_t> &v)
{
    PyObject *lst = PyList_New(static_cast<Py_ssize_t>(v.size()));
    if (!lst)
        return nullptr;
    for (size_t i = 0; i < v.size(); ++i)
        PyList_SET_ITEM(lst, static_cast<Py_ssize_t>(i),
                        PyLong_FromLongLong(static_cast<long long>(v[i])));
    return lst;
}
} // namespace detail

class DataItemValueSlotBridge
    : public ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>
{
    PyObject *m_callback;

  public:
    explicit DataItemValueSlotBridge(PyObject *cb) : m_callback(cb)
    {
        Py_XINCREF(m_callback);
    }

    ~DataItemValueSlotBridge() override
    {
        if (m_callback)
        {
            PyGILState_STATE gs = PyGILState_Ensure();
            Py_DECREF(m_callback);
            PyGILState_Release(gs);
        }
    }

    DataItemValueSlotBridge(const DataItemValueSlotBridge &) = delete;
    DataItemValueSlotBridge &operator=(const DataItemValueSlotBridge &) = delete;

    void operator()(
        const ISignal<const std::shared_ptr<IComponentDataItemValueChanged> &> & /*sender*/,
        const std::shared_ptr<IComponentDataItemValueChanged> &args) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *indexes = detail::int64_vec_to_list(args->start());
        PyObject *lengths = detail::int64_vec_to_list(args->count());
        if (indexes && lengths)
        {
            PyObject *py_args = PyTuple_Pack(2, indexes, lengths);
            if (py_args)
            {
                PyObject *result = PyObject_CallObject(m_callback, py_args);
                Py_XDECREF(result);
                Py_DECREF(py_args);
            }
        }
        Py_XDECREF(indexes);
        Py_XDECREF(lengths);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }
};

// ============================================================================
// PropertySlotBridge
//   ISlot< std::string >
//   Callback signature:  callback(property_name: str)
// ============================================================================
class PropertySlotBridge
    : public ISlot<std::string>
{
    PyObject *m_callback;

  public:
    explicit PropertySlotBridge(PyObject *cb) : m_callback(cb)
    {
        Py_XINCREF(m_callback);
    }

    ~PropertySlotBridge() override
    {
        if (m_callback)
        {
            PyGILState_STATE gs = PyGILState_Ensure();
            Py_DECREF(m_callback);
            PyGILState_Release(gs);
        }
    }

    PropertySlotBridge(const PropertySlotBridge &) = delete;
    PropertySlotBridge &operator=(const PropertySlotBridge &) = delete;

    void operator()(
        const ISignal<std::string> & /*sender*/,
        std::string property_name) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *py_args = Py_BuildValue("(s)", property_name.c_str());
        if (py_args)
        {
            PyObject *result = PyObject_CallObject(m_callback, py_args);
            Py_XDECREF(result);
            Py_DECREF(py_args);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }
};

// ============================================================================
// WorkflowStatusSlotBridge
//   ISlot< const shared_ptr<IWorkflowComponentStatusChangeEventArgs>& >
//   Callback signature:
//     callback(previous_status: int, status: int, message: str,
//              has_progress: bool, percent_progress: float)
// ============================================================================
class WorkflowStatusSlotBridge
    : public ISlot<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>
{
    PyObject *m_callback;

  public:
    explicit WorkflowStatusSlotBridge(PyObject *cb) : m_callback(cb)
    {
        Py_XINCREF(m_callback);
    }

    ~WorkflowStatusSlotBridge() override
    {
        if (m_callback)
        {
            PyGILState_STATE gs = PyGILState_Ensure();
            Py_DECREF(m_callback);
            PyGILState_Release(gs);
        }
    }

    WorkflowStatusSlotBridge(const WorkflowStatusSlotBridge &) = delete;
    WorkflowStatusSlotBridge &operator=(const WorkflowStatusSlotBridge &) = delete;

    void operator()(
        const ISignal<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &> & /*sender*/,
        const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &args) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *py_args = Py_BuildValue(
            "(iisOf)",
            static_cast<int>(args->previousStatus()),
            static_cast<int>(args->status()),
            args->message().c_str(),
            args->hasProgressMonitor() ? Py_True : Py_False,
            static_cast<double>(args->percentProgress()));
        if (py_args)
        {
            PyObject *result = PyObject_CallObject(m_callback, py_args);
            Py_XDECREF(result);
            Py_DECREF(py_args);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }
};

// ============================================================================
// Factory helpers — create shared_ptr of each bridge type
// ============================================================================
inline std::shared_ptr<StatusSlotBridge>
make_status_slot(PyObject *cb)
{
    return std::make_shared<StatusSlotBridge>(cb);
}

inline std::shared_ptr<DataItemValueSlotBridge>
make_data_item_value_slot(PyObject *cb)
{
    return std::make_shared<DataItemValueSlotBridge>(cb);
}

inline std::shared_ptr<PropertySlotBridge>
make_property_slot(PyObject *cb)
{
    return std::make_shared<PropertySlotBridge>(cb);
}

inline std::shared_ptr<WorkflowStatusSlotBridge>
make_workflow_status_slot(PyObject *cb)
{
    return std::make_shared<WorkflowStatusSlotBridge>(cb);
}

// ============================================================================
// Connect / disconnect helpers
// These free functions hide the template-heavy ISignal::connect / disconnect
// from Cython.  Explicit static_cast to the correct ISignal<> base avoids
// ambiguity when a class inherits multiple ISignal specializations.
// ============================================================================

// Type aliases for the ISignal specializations
using StatusSignal = ISignal<const std::shared_ptr<IComponentStatusChangeEventArgs> &>;
using ValueChangedSignal = ISignal<const std::shared_ptr<IComponentDataItemValueChanged> &>;
using PropertySignal = ISignal<std::string>;
using WorkflowStatusSignal = ISignal<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>;

// -- Component status (IModelComponent) ------------------------------------
inline void connect_status_slot(
    IModelComponent *comp,
    const std::shared_ptr<StatusSlotBridge> &slot)
{
    static_cast<StatusSignal *>(comp)->connect(slot);
}

inline void disconnect_status_slot(
    IModelComponent *comp,
    const std::shared_ptr<StatusSlotBridge> &slot)
{
    static_cast<StatusSignal *>(comp)->disconnect(slot);
}

// -- Data item value changed (IComponentDataItem) --------------------------
inline void connect_value_changed_slot(
    IComponentDataItem *item,
    const std::shared_ptr<DataItemValueSlotBridge> &slot)
{
    static_cast<ValueChangedSignal *>(item)->connect(slot);
}

inline void disconnect_value_changed_slot(
    IComponentDataItem *item,
    const std::shared_ptr<DataItemValueSlotBridge> &slot)
{
    static_cast<ValueChangedSignal *>(item)->disconnect(slot);
}

// -- Property changed (via ISignal<string>) --------------------------------
// Overloads for each wrapper type that exposes property change signals.
inline void connect_property_slot_comp(
    IModelComponent *comp,
    const std::shared_ptr<PropertySlotBridge> &slot)
{
    static_cast<PropertySignal *>(comp)->connect(slot);
}

inline void disconnect_property_slot_comp(
    IModelComponent *comp,
    const std::shared_ptr<PropertySlotBridge> &slot)
{
    static_cast<PropertySignal *>(comp)->disconnect(slot);
}

inline void connect_property_slot_item(
    IComponentDataItem *item,
    const std::shared_ptr<PropertySlotBridge> &slot)
{
    static_cast<PropertySignal *>(item)->connect(slot);
}

inline void disconnect_property_slot_item(
    IComponentDataItem *item,
    const std::shared_ptr<PropertySlotBridge> &slot)
{
    static_cast<PropertySignal *>(item)->disconnect(slot);
}

inline void connect_property_slot_workflow(
    IWorkflowComponent *comp,
    const std::shared_ptr<PropertySlotBridge> &slot)
{
    static_cast<PropertySignal *>(comp)->connect(slot);
}

inline void disconnect_property_slot_workflow(
    IWorkflowComponent *comp,
    const std::shared_ptr<PropertySlotBridge> &slot)
{
    static_cast<PropertySignal *>(comp)->disconnect(slot);
}

// -- Workflow status (IWorkflowComponent) ----------------------------------
inline void connect_workflow_status_slot(
    IWorkflowComponent *comp,
    const std::shared_ptr<WorkflowStatusSlotBridge> &slot)
{
    static_cast<WorkflowStatusSignal *>(comp)->connect(slot);
}

inline void disconnect_workflow_status_slot(
    IWorkflowComponent *comp,
    const std::shared_ptr<WorkflowStatusSlotBridge> &slot)
{
    static_cast<WorkflowStatusSignal *>(comp)->disconnect(slot);
}

} // namespace Python
} // namespace HydroCouple
