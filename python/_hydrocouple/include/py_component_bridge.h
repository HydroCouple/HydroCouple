/*
 * py_component_bridge.h
 *
 * C++ bridge class that implements HydroCouple::IModelComponent and
 * delegates every virtual call to a Python object through the Python C-API.
 *
 * This allows a pure-Python component (subclass of the hydrocouple.core ABCs)
 * to be visible to C++ workflows as a true IModelComponent*.
 *
 * The Python object must implement the hydrocouple.core.IModelComponent ABC.
 *
 * Signal support: the bridge maintains its own ISlot storage for both
 * ISignal<string> (property-changed) and
 * ISignal<shared_ptr<IComponentStatusChangeEventArgs>&> (status-changed).
 * Python code can trigger emissions via the Cython-level emit helpers.
 */
#pragma once

#include <Python.h>
#include "hydrocouple.h"

#include <algorithm>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace HydroCouple {
namespace Python {

// ============================================================================
// PyModelComponentBridge
// ============================================================================
class PyModelComponentBridge
    : public virtual IModelComponent
{
    PyObject *m_pyobj; // strong reference

    // Signal infrastructure
    std::vector<std::shared_ptr<ISlot<std::string>>> m_propSlots;
    std::vector<std::shared_ptr<ISlot<const std::shared_ptr<IComponentStatusChangeEventArgs> &>>>
        m_statusSlots;
    bool m_propBlocked = false;
    bool m_statusBlocked = false;
    std::mutex m_slotMutex;

    // Cached strings returned by reference
    mutable std::string m_cachedId;
    mutable std::string m_cachedCaption;
    mutable std::string m_cachedDescription;
    mutable std::string m_cachedRefDir;

    // ----- helper: get a Python attr as std::string ---
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

    // ----- helper: get a Python attr as int ---
    int pyint(const char *attr, int dflt = 0) const
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        int result = dflt;
        PyObject *val = PyObject_GetAttrString(m_pyobj, attr);
        if (val && PyLong_Check(val))
            result = static_cast<int>(PyLong_AsLong(val));
        Py_XDECREF(val);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return result;
    }

    // ----- helper: get a Python attr as bool ---
    bool pybool(const char *attr, bool dflt = false) const
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        bool result = dflt;
        PyObject *val = PyObject_GetAttrString(m_pyobj, attr);
        if (val)
            result = PyObject_IsTrue(val) == 1;
        Py_XDECREF(val);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return result;
    }

    // ----- helper: call a no-arg method, ignoring return ---
    void pycall(const char *method) const
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *ret = PyObject_CallMethod(m_pyobj, method, nullptr);
        Py_XDECREF(ret);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }

    // ----- helper: call a method with a string arg ---
    void pycall_str(const char *method, const std::string &arg) const
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *ret = PyObject_CallMethod(
            m_pyobj, method, "s", arg.c_str());
        Py_XDECREF(ret);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }

    // ----- helper: call validate(), return vector<string> ---
    std::vector<std::string> pycall_validate() const
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        std::vector<std::string> msgs;
        PyObject *ret = PyObject_CallMethod(m_pyobj, "validate", nullptr);
        if (ret && PyList_Check(ret))
        {
            Py_ssize_t n = PyList_Size(ret);
            msgs.reserve(static_cast<size_t>(n));
            for (Py_ssize_t i = 0; i < n; ++i)
            {
                PyObject *item = PyList_GetItem(ret, i); // borrowed
                if (item && PyUnicode_Check(item))
                {
                    const char *s = PyUnicode_AsUTF8(item);
                    if (s)
                        msgs.emplace_back(s);
                }
            }
        }
        Py_XDECREF(ret);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return msgs;
    }

public:
    explicit PyModelComponentBridge(PyObject *obj) : m_pyobj(obj)
    {
        Py_INCREF(m_pyobj);
    }

    ~PyModelComponentBridge() override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        Py_DECREF(m_pyobj);
        PyGILState_Release(gs);
    }

    // Non-copyable
    PyModelComponentBridge(const PyModelComponentBridge &) = delete;
    PyModelComponentBridge &operator=(const PyModelComponentBridge &) = delete;

    /// Get the wrapped Python object (borrowed reference).
    PyObject *pyObject() const { return m_pyobj; }

    // ==================================================================
    // ISignal<std::string> — property changed
    // ==================================================================
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

    // ==================================================================
    // ISignal<const shared_ptr<IComponentStatusChangeEventArgs>&>
    // ==================================================================
    void connect(
        const std::shared_ptr<ISlot<const std::shared_ptr<IComponentStatusChangeEventArgs> &>> &slot) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_statusSlots.push_back(slot);
    }

    void disconnect(
        const std::shared_ptr<ISlot<const std::shared_ptr<IComponentStatusChangeEventArgs> &>> &slot) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_statusSlots.erase(
            std::remove(m_statusSlots.begin(), m_statusSlots.end(), slot),
            m_statusSlots.end());
    }

    // blockSignals — applies to both signal bases
    void blockSignals(bool block) override
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        m_propBlocked = block;
        m_statusBlocked = block;
    }

    /// Emit a property-changed signal (callable from Python via Cython).
    void emitPropertyChanged(const std::string &propertyName)
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        if (m_propBlocked)
            return;
        for (auto &slot : m_propSlots)
            (*slot)(*this, propertyName);
    }

    /// Emit a status-changed signal (callable from Python via Cython).
    void emitStatusChanged(
        const std::shared_ptr<IComponentStatusChangeEventArgs> &args)
    {
        std::lock_guard<std::mutex> lk(m_slotMutex);
        if (m_statusBlocked)
            return;
        for (auto &slot : m_statusSlots)
            (*slot)(*this, args);
    }

protected:
    void emit(std::string propertyName) override
    {
        emitPropertyChanged(propertyName);
    }

    void emit(const std::shared_ptr<IComponentStatusChangeEventArgs> &args) override
    {
        emitStatusChanged(args);
    }

public:
    // ==================================================================
    // IDescription
    // ==================================================================
    [[nodiscard]] const std::string &caption() const override
    {
        m_cachedCaption = pystr("caption");
        return m_cachedCaption;
    }

    void setCaption(const std::string &cap) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *val = PyUnicode_FromStringAndSize(cap.data(),
                                                    static_cast<Py_ssize_t>(cap.size()));
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

    void setDescription(const std::string &desc) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        PyObject *val = PyUnicode_FromStringAndSize(desc.data(),
                                                    static_cast<Py_ssize_t>(desc.size()));
        if (val)
        {
            PyObject_SetAttrString(m_pyobj, "description", val);
            Py_DECREF(val);
        }
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
    }

    // ==================================================================
    // IIdentity
    // ==================================================================
    [[nodiscard]] const std::string &id() const override
    {
        m_cachedId = pystr("id");
        return m_cachedId;
    }

    // ==================================================================
    // IModelComponent
    // ==================================================================
    [[nodiscard]] IModelComponentInfo *componentInfo() const override
    {
        // Python components don't expose a C++ IModelComponentInfo.
        return nullptr;
    }

    [[nodiscard]] ComponentStatus status() const override
    {
        return static_cast<ComponentStatus>(pyint("status"));
    }

    [[nodiscard]] std::vector<IArgument *> arguments() const override
    {
        // Phase 1: Python-side arguments not bridged to C++ IArgument*.
        return {};
    }

    [[nodiscard]] std::vector<IInput *> inputs() const override
    {
        // Phase 1: Python-side inputs not bridged to C++ IInput*.
        return {};
    }

    [[nodiscard]] std::vector<IOutput *> outputs() const override
    {
        // Phase 1: Python-side outputs not bridged to C++ IOutput*.
        return {};
    }

    [[nodiscard]] std::vector<IComponentDataItem *> results() const override
    {
        return {};
    }

    void initialize() override { pycall("initialize"); }

    [[nodiscard]] std::vector<std::string> validate() override
    {
        return pycall_validate();
    }

    void prepare() override { pycall("prepare"); }

    void update(const std::vector<IOutput *> & /*requiredOutputs*/ = {}) override
    {
        pycall("update");
    }

    void finish() override { pycall("finish"); }

    [[nodiscard]] const IWorkflowComponent *workflow() const override
    {
        return nullptr;
    }

    void setWorkflow(const IWorkflowComponent * /*workflow*/) override
    {
        // Store on the Python side if needed; not bridged in Phase 1.
    }

    // ==================================================================
    // Capabilities — translate the Python set of Capability IntEnums
    // ==================================================================
    [[nodiscard]] std::set<Capability> capabilities() const override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        std::set<Capability> caps;
        PyObject *ret = PyObject_CallMethod(m_pyobj, "capabilities", nullptr);
        if (ret)
        {
            PyObject *iter = PyObject_GetIter(ret);
            if (iter)
            {
                PyObject *item;
                while ((item = PyIter_Next(iter)) != nullptr)
                {
                    long v = PyLong_AsLong(item);
                    if (v >= 0 && !PyErr_Occurred())
                        caps.insert(static_cast<Capability>(v));
                    Py_DECREF(item);
                }
                Py_DECREF(iter);
            }
        }
        Py_XDECREF(ret);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return caps;
    }

    // ==================================================================
    // Error queue — translate the Python list of ErrorEntry dataclasses
    // ==================================================================
    [[nodiscard]] std::vector<ErrorEntry> errors(bool clearAfterRead = false) override
    {
        PyGILState_STATE gs = PyGILState_Ensure();
        std::vector<ErrorEntry> entries;
        PyObject *ret = PyObject_CallMethod(
            m_pyobj, "errors", "O", clearAfterRead ? Py_True : Py_False);
        if (ret)
        {
            PyObject *iter = PyObject_GetIter(ret);
            if (iter)
            {
                PyObject *item;
                while ((item = PyIter_Next(iter)) != nullptr)
                {
                    ErrorEntry e;
                    PyObject *sev = PyObject_GetAttrString(item, "severity");
                    PyObject *code = PyObject_GetAttrString(item, "code");
                    PyObject *src = PyObject_GetAttrString(item, "source");
                    PyObject *msg = PyObject_GetAttrString(item, "message");
                    if (sev && PyLong_Check(sev))
                        e.severity = static_cast<ErrorEntry::Severity>(PyLong_AsLong(sev));
                    if (code && PyLong_Check(code))
                        e.code = static_cast<int32_t>(PyLong_AsLong(code));
                    if (src && PyUnicode_Check(src))
                    {
                        const char *s = PyUnicode_AsUTF8(src);
                        if (s) e.source = s;
                    }
                    if (msg && PyUnicode_Check(msg))
                    {
                        const char *s = PyUnicode_AsUTF8(msg);
                        if (s) e.message = s;
                    }
                    Py_XDECREF(sev);
                    Py_XDECREF(code);
                    Py_XDECREF(src);
                    Py_XDECREF(msg);
                    entries.push_back(std::move(e));
                    Py_DECREF(item);
                }
                Py_DECREF(iter);
            }
        }
        Py_XDECREF(ret);
        if (PyErr_Occurred())
            PyErr_Print();
        PyGILState_Release(gs);
        return entries;
    }

    // ==================================================================
    // Reference directory
    // ==================================================================
    [[nodiscard]] std::string referenceDirectory() const override
    {
        return pystr("reference_directory");
    }

    void setReferenceDirectory(const std::string &dir) override
    {
        pycall_str("set_reference_directory", dir);
    }
};

// ============================================================================
// Factory helper — callable from Cython
// ============================================================================
inline PyModelComponentBridge *make_py_component_bridge(PyObject *pyobj)
{
    return new PyModelComponentBridge(pyobj);
}

} // namespace Python
} // namespace HydroCouple
