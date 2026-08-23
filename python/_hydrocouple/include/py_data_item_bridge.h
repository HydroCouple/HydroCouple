/*
 * py_data_item_bridge.h
 *
 * C++ bridge class that implements HydroCouple::IComponentDataItem and
 * delegates every virtual call to a Python object (an implementation of
 * the hydrocouple.core.IComponentDataItem ABC) through the Python C-API.
 *
 * The typed data plane crosses the boundary zero-copy: getValuesInto /
 * setValuesFrom wrap the C++ BufferDescriptor's memory as a NumPy array
 * view (via hydrocouple._bridgesupport.ndarray_over) and invoke the
 * Python get_values_into / set_values_from, so C++ workflows can exchange
 * hyperslabs with Python components without element copies.
 */
#pragma once

#include <Python.h>
#include "hydrocouple.h"

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace HydroCouple {
namespace Python {

class PyComponentDataItemBridge
    : public virtual IComponentDataItem
{
    PyObject *m_pyobj;               // strong reference
    IModelComponent *m_owner;        // borrowed; may be nullptr

    // Signal infrastructure (value-changed + property-changed)
    std::vector<std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>>>
        m_valueSlots;
    std::vector<std::shared_ptr<ISlot<std::string>>> m_propSlots;
    bool m_blocked = false;
    std::mutex m_slotMutex;

    mutable std::string m_cachedId;
    mutable std::string m_cachedCaption;
    mutable std::string m_cachedDescription;

    std::string pystr(const char *attr) const
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        std::string result;
        PyObject *val = PyObject_GetAttrString(m_pyobj, attr);
        if (val && PyUnicode_Check(val))
        {
            const char *utf8 = PyUnicode_AsUTF8(val);
            if (utf8)
                result = utf8;
        }
        Py_XDECREF(val);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return result;
    }

    static const char *kindName(DataKind kind)
    {
        // Only used for error messages.
        switch (kind)
        {
            case DataKind::Float64: return "Float64";
            case DataKind::Float32: return "Float32";
            case DataKind::Int64:   return "Int64";
            case DataKind::Int32:   return "Int32";
            default:                return "DataKind";
        }
    }

    // Build hydrocouple._bridgesupport.ndarray_over(...) view of a
    // descriptor and call the Python data-plane method. Returns the
    // (ok, message) pair translated to C++.
    bool callDataPlane(const char *method,
                       const BufferDescriptor &buffer,
                       std::span<const int64_t> start,
                       std::span<const int64_t> count,
                       bool writable,
                       std::string *message)
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        bool ok = false;
        std::string msg = "bridge error";

        PyObject *support = PyImport_ImportModule("hydrocouple._bridgesupport");
        if (support)
        {
            PyObject *shape = PyList_New(buffer.rank);
            PyObject *strides = nullptr;
            for (int32_t k = 0; k < buffer.rank; ++k)
                PyList_SET_ITEM(shape, k, PyLong_FromLongLong(
                    static_cast<long long>(buffer.shape[k])));
            if (buffer.stridesBytes != nullptr)
            {
                strides = PyList_New(buffer.rank);
                for (int32_t k = 0; k < buffer.rank; ++k)
                    PyList_SET_ITEM(strides, k, PyLong_FromLongLong(
                        static_cast<long long>(buffer.stridesBytes[k])));
            }
            else
            {
                strides = Py_None;
                Py_INCREF(Py_None);
            }

            PyObject *arr = PyObject_CallMethod(
                support, "ndarray_over", "KiOOO",
                static_cast<unsigned long long>(
                    reinterpret_cast<uintptr_t>(buffer.data)),
                static_cast<int>(buffer.kind),
                shape, strides, writable ? Py_True : Py_False);
            Py_DECREF(shape);
            Py_DECREF(strides);

            if (arr)
            {
                PyObject *pyStart = PyList_New(static_cast<Py_ssize_t>(start.size()));
                PyObject *pyCount = PyList_New(static_cast<Py_ssize_t>(count.size()));
                for (size_t k = 0; k < start.size(); ++k)
                    PyList_SET_ITEM(pyStart, static_cast<Py_ssize_t>(k),
                                    PyLong_FromLongLong(static_cast<long long>(start[k])));
                for (size_t k = 0; k < count.size(); ++k)
                    PyList_SET_ITEM(pyCount, static_cast<Py_ssize_t>(k),
                                    PyLong_FromLongLong(static_cast<long long>(count[k])));

                PyObject *ret = PyObject_CallMethod(
                    m_pyobj, method, "OOO", arr, pyStart, pyCount);
                if (ret && PyTuple_Check(ret) && PyTuple_Size(ret) == 2)
                {
                    ok = PyObject_IsTrue(PyTuple_GetItem(ret, 0)) == 1;
                    PyObject *m = PyTuple_GetItem(ret, 1);
                    if (m && PyUnicode_Check(m))
                    {
                        const char *s = PyUnicode_AsUTF8(m);
                        msg = s ? s : "";
                    }
                    else
                    {
                        msg = "";
                    }
                }
                Py_XDECREF(ret);
                Py_DECREF(pyStart);
                Py_DECREF(pyCount);
                Py_DECREF(arr);
            }
            Py_DECREF(support);
        }

        if (PyErr_Occurred())
        {
            PyErr_Print();
            ok = false;
            if (msg.empty())
                msg = "python exception in data-plane call";
        }
        PyGILState_Release(gs);
        if (message)
            *message = msg;
        return ok;
    }

public:
    explicit PyComponentDataItemBridge(PyObject *obj,
                                       IModelComponent *owner = nullptr)
        : m_pyobj(obj), m_owner(owner)
    {
        Py_INCREF(m_pyobj);
    }

    ~PyComponentDataItemBridge() override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        Py_DECREF(m_pyobj);
        PyGILState_Release(gs);
    }

    PyComponentDataItemBridge(const PyComponentDataItemBridge &) = delete;
    PyComponentDataItemBridge &operator=(const PyComponentDataItemBridge &) = delete;

    PyObject *pyObject() const { return m_pyobj; }

    // ==================================================================
    // Signals
    // ==================================================================
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &slot) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_valueSlots.push_back(slot);
    }

    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &slot) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_valueSlots.erase(
            std::remove(m_valueSlots.begin(), m_valueSlots.end(), slot),
            m_valueSlots.end());
    }

    void connect(const std::shared_ptr<ISlot<std::string>> &slot) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_propSlots.push_back(slot);
    }

    void disconnect(const std::shared_ptr<ISlot<std::string>> &slot) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_propSlots.erase(
            std::remove(m_propSlots.begin(), m_propSlots.end(), slot),
            m_propSlots.end());
    }

    void blockSignals(bool block) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_blocked = block;
    }

protected:
    void emit(const std::shared_ptr<IComponentDataItemValueChanged> &args) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        if (m_blocked)
            return;
        for (auto &slot : m_valueSlots)
            (*slot)(*this, args);
    }

    void emit(std::string propertyName) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        if (m_blocked)
            return;
        for (auto &slot : m_propSlots)
            (*slot)(*this, propertyName);
    }

public:
    // ==================================================================
    // IDescription / IIdentity
    // ==================================================================
    [[nodiscard]] const std::string &caption() const override
    {
        m_cachedCaption = pystr("caption");
        return m_cachedCaption;
    }

    void setCaption(const std::string &value) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *val = PyUnicode_FromStringAndSize(
            value.data(), static_cast<Py_ssize_t>(value.size()));
        if (val)
        {
            PyObject_SetAttrString(m_pyobj, "caption", val);
            Py_DECREF(val);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }

    [[nodiscard]] const std::string &description() const override
    {
        m_cachedDescription = pystr("description");
        return m_cachedDescription;
    }

    void setDescription(const std::string &value) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *val = PyUnicode_FromStringAndSize(
            value.data(), static_cast<Py_ssize_t>(value.size()));
        if (val)
        {
            PyObject_SetAttrString(m_pyobj, "description", val);
            Py_DECREF(val);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }

    [[nodiscard]] const std::string &id() const override
    {
        m_cachedId = pystr("id");
        return m_cachedId;
    }

    // ==================================================================
    // IComponentDataItem
    // ==================================================================
    [[nodiscard]] IModelComponent *modelComponent() const override
    {
        return m_owner;
    }

    [[nodiscard]] std::vector<IDimension *> dimensions() const override
    {
        // Python-side IDimension objects are not bridged; shape() carries
        // the extents.
        return {};
    }

    [[nodiscard]] std::vector<int64_t> shape() const override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        std::vector<int64_t> result;
        PyObject *val = PyObject_GetAttrString(m_pyobj, "shape");
        if (val)
        {
            PyObject *seq = PySequence_Fast(val, "shape must be a sequence");
            if (seq)
            {
                Py_ssize_t n = PySequence_Fast_GET_SIZE(seq);
                result.reserve(static_cast<size_t>(n));
                for (Py_ssize_t i = 0; i < n; ++i)
                    result.push_back(static_cast<int64_t>(
                        PyLong_AsLongLong(PySequence_Fast_GET_ITEM(seq, i))));
                Py_DECREF(seq);
            }
            Py_DECREF(val);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return result;
    }

    [[nodiscard]] DataKind dataKind() const override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        DataKind kind = DataKind::Unknown;
        PyObject *val = PyObject_GetAttrString(m_pyobj, "data_kind");
        if (val)
        {
            long v = PyLong_AsLong(val);
            if (!PyErr_Occurred())
                kind = static_cast<DataKind>(v);
            Py_DECREF(val);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return kind;
    }

    [[nodiscard]] IValueDefinition *valueDefinition() const override
    {
        // Python-side value definitions are not bridged.
        return nullptr;
    }

    [[nodiscard]] bool getValuesInto(const BufferDescriptor &destination,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message = nullptr) const override
    {
        return const_cast<PyComponentDataItemBridge *>(this)->callDataPlane(
            "get_values_into", destination, start, count,
            /*writable=*/true, message);
    }

    [[nodiscard]] bool setValuesFrom(const BufferDescriptor &source,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message = nullptr) override
    {
        return callDataPlane("set_values_from", source, start, count,
                             /*writable=*/false, message);
    }
};

} // namespace Python
} // namespace HydroCouple
