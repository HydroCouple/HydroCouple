/*
 * test_component.cpp
 *
 * A minimal but complete native C++20 HydroCouple v2.0.0 component used to
 * prove the C++ -> Python direction of the bindings end-to-end: the pytest
 * suite compiles this file to a shared library, loads it through
 * hydrocouple.loader.load(), drives the lifecycle, and reads the result
 * field through the typed hyperslab data plane into NumPy.
 *
 * Behavior: a 10-cell Float64 field where cell i holds
 * 1000 * completed_steps + i; Done after 3 updates; queues one Warning
 * during prepare() so errors() translation can be verified.
 */

#include "hydrocouple.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

using namespace HydroCouple;

namespace {

constexpr int64_t kCells = 10;
constexpr int kTotalSteps = 3;

// ---------------------------------------------------------------------------
// Small reusable base supplying the signal plumbing every entity needs.
// ---------------------------------------------------------------------------
template <typename EventArgs>
class SignalBase : public virtual ISignal<const std::shared_ptr<EventArgs> &>,
                   public virtual IPropertyChanged
{
public:
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<EventArgs> &>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<EventArgs> &>> &) override {}
    void connect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void blockSignals(bool) override {}

protected:
    void emit(const std::shared_ptr<EventArgs> &) override {}
    void emit(std::string) override {}
};

// ---------------------------------------------------------------------------
// Result data item: [kCells] Float64 field with real hyperslab semantics.
// ---------------------------------------------------------------------------
class SineField final : public virtual IComponentDataItem,
                        public SignalBase<IComponentDataItemValueChanged>
{
    IModelComponent *m_owner;
    std::string m_id = "cpp-sine-field";
    std::string m_caption = "C++ test field";
    std::string m_description = "cell value = 1000 * step + index";
    std::vector<double> m_values;

public:
    explicit SineField(IModelComponent *owner)
        : m_owner(owner), m_values(static_cast<size_t>(kCells), 0.0) {}

    void fill(int step)
    {
        for (int64_t i = 0; i < kCells; ++i)
            m_values[static_cast<size_t>(i)] = 1000.0 * step + static_cast<double>(i);
    }

    // IDescription / IIdentity
    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }

    // IComponentDataItem
    [[nodiscard]] IModelComponent *modelComponent() const override { return m_owner; }
    [[nodiscard]] std::vector<IDimension *> dimensions() const override { return {}; }
    [[nodiscard]] std::vector<int64_t> shape() const override { return {kCells}; }
    [[nodiscard]] DataKind dataKind() const override { return DataKind::Float64; }
    [[nodiscard]] IValueDefinition *valueDefinition() const override { return nullptr; }

    [[nodiscard]] bool getValuesInto(const BufferDescriptor &destination,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message) const override
    {
        if (destination.kind != DataKind::Float64)
        {
            if (message) *message = "kind mismatch";
            return false;
        }
        if (start.size() != 1 || count.size() != 1)
        {
            if (message) *message = "selection rank must be 1";
            return false;
        }
        if (start[0] < 0 || start[0] + count[0] > kCells)
        {
            if (message) *message = "selection out of bounds";
            return false;
        }
        std::memcpy(destination.data,
                    m_values.data() + start[0],
                    static_cast<size_t>(count[0]) * sizeof(double));
        return true;
    }

    [[nodiscard]] bool setValuesFrom(const BufferDescriptor &source,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message) override
    {
        if (source.kind != DataKind::Float64)
        {
            if (message) *message = "kind mismatch";
            return false;
        }
        if (start.size() != 1 || count.size() != 1 ||
            start[0] < 0 || start[0] + count[0] > kCells)
        {
            if (message) *message = "selection out of bounds";
            return false;
        }
        std::memcpy(m_values.data() + start[0],
                    source.data,
                    static_cast<size_t>(count[0]) * sizeof(double));
        return true;
    }
};

class SineComponentInfo; // forward

// ---------------------------------------------------------------------------
// Component
// ---------------------------------------------------------------------------
class SineComponent final : public virtual IModelComponent,
                            public SignalBase<IComponentStatusChangeEventArgs>
{
    std::string m_id = "cpp-sine";
    std::string m_caption = "C++ sine test component";
    std::string m_description = "native fixture for python interop tests";
    std::string m_refDir = ".";
    ComponentStatus m_status = ComponentStatus::Created;
    int m_step = 0;
    std::unique_ptr<SineField> m_field;
    std::vector<ErrorEntry> m_errors;
    IModelComponentInfo *m_info;

public:
    explicit SineComponent(IModelComponentInfo *info) : m_info(info)
    {
        m_field = std::make_unique<SineField>(this);
    }

    // IDescription / IIdentity
    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }

    // IModelComponent
    [[nodiscard]] IModelComponentInfo *componentInfo() const override { return m_info; }
    [[nodiscard]] ComponentStatus status() const override { return m_status; }
    [[nodiscard]] std::vector<IArgument *> arguments() const override { return {}; }
    [[nodiscard]] std::vector<IInput *> inputs() const override { return {}; }
    [[nodiscard]] std::vector<IOutput *> outputs() const override { return {}; }

    [[nodiscard]] std::vector<IComponentDataItem *> results() const override
    {
        return {m_field.get()};
    }

    void initialize() override
    {
        m_status = ComponentStatus::Initializing;
        m_status = ComponentStatus::Initialized;
    }

    [[nodiscard]] std::vector<std::string> validate() override
    {
        m_status = ComponentStatus::Validating;
        m_status = ComponentStatus::Valid;
        return {};
    }

    void prepare() override
    {
        m_status = ComponentStatus::Preparing;
        m_errors.push_back(ErrorEntry{
            ErrorEntry::Severity::Warning, 7, m_id,
            "prepare issued a demonstration warning"});
        m_field->fill(0);
        m_status = ComponentStatus::Updated;
    }

    void update(const std::vector<IOutput *> & = {}) override
    {
        m_status = ComponentStatus::Updating;
        ++m_step;
        m_field->fill(m_step);
        m_status = (m_step >= kTotalSteps) ? ComponentStatus::Done
                                           : ComponentStatus::Updated;
    }

    void finish() override
    {
        m_status = ComponentStatus::Finishing;
        m_status = ComponentStatus::Finished;
    }

    [[nodiscard]] const IWorkflowComponent *workflow() const override { return nullptr; }
    void setWorkflow(const IWorkflowComponent *) override {}

    [[nodiscard]] std::set<Capability> capabilities() const override
    {
        return {};
    }

    [[nodiscard]] std::vector<ErrorEntry> errors(bool clearAfterRead) override
    {
        std::vector<ErrorEntry> out = m_errors;
        if (clearAfterRead)
            m_errors.clear();
        return out;
    }

    [[nodiscard]] std::string referenceDirectory() const override { return m_refDir; }
    void setReferenceDirectory(const std::string &value) override { m_refDir = value; }
};

// ---------------------------------------------------------------------------
// Component info / factory
// ---------------------------------------------------------------------------
class SineComponentInfo final : public virtual IModelComponentInfo,
                                public SignalBase<IComponentStatusChangeEventArgs>
{
    std::string m_id = "cpp-sine-info";
    std::string m_caption = "C++ sine test component";
    std::string m_description = "native fixture for python interop tests";
    std::string m_libraryPath;

public:
    // IDescription / IIdentity
    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }

    // IComponentInfo
    [[nodiscard]] std::string libraryFilePath() const override { return m_libraryPath; }
    void setLibraryFilePath(const std::string &filePath) override { m_libraryPath = filePath; }
    [[nodiscard]] std::string iconFilePath() const override { return {}; }
    [[nodiscard]] std::string developer() const override { return "HydroCouple interop test"; }
    [[nodiscard]] std::vector<std::string> documentation() const override { return {}; }
    [[nodiscard]] std::string license() const override { return "MIT"; }
    [[nodiscard]] std::string copyright() const override { return "2026 Caleb Buahin"; }
    [[nodiscard]] std::string url() const override { return "https://hydrocouple.org"; }
    [[nodiscard]] std::string email() const override { return "caleb.buahin@gmail.com"; }
    [[nodiscard]] std::string version() const override { return "2.0.0-alpha.1"; }
    [[nodiscard]] std::set<std::string> tags() const override { return {"test"}; }

    // IModelComponentInfo
    [[nodiscard]] std::unique_ptr<IModelComponent> createComponentInstance() override
    {
        return std::make_unique<SineComponent>(this);
    }

    [[nodiscard]] std::vector<IAdaptedOutputFactory *> adaptedOutputFactories() const override
    {
        return {};
    }
};

} // namespace

extern "C" IModelComponentInfo *CreateComponentInfo()
{
    static SineComponentInfo info;
    return &info;
}
