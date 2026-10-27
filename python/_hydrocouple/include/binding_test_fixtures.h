/*
 * binding_test_fixtures.h
 *
 * Native C++ objects that exercise every path the core, temporal and spatial
 * bindings take: a time-stepping component that really emits its signals, an
 * info that makes instances (and counts the live ones), a workflow, a
 * multi-input, an adapted output and its factory, quantities with units, an
 * argument, geometries whose operations make new geometries (counted too), a
 * raster that reads and writes through BufferDescriptors, and a spatial
 * reference system with a vertical datum.
 *
 * Python reaches them the way it reaches a loaded component -- through the
 * ordinary wrappers -- so the tests prove the bindings, not a back door.
 */
#pragma once

#include "layered_test_fixtures.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace HydroCouple {
namespace Python {
namespace Testing {

// -- Counters a test reads to prove ownership -------------------------------

inline int &liveComponents() { static int n = 0; return n; }
inline int &liveGeometries() { static int n = 0; return n; }
inline int &liveAdaptedOutputs() { static int n = 0; return n; }

// -- A real signal: slots connected, disconnected, blocked, called ------------

template <typename... Args>
class SlotList
{
public:
    void connect(const std::shared_ptr<ISlot<Args...>> &slot)
    {
        if (std::find(m_slots.begin(), m_slots.end(), slot) == m_slots.end())
            m_slots.push_back(slot);
    }

    void disconnect(const std::shared_ptr<ISlot<Args...>> &slot)
    {
        m_slots.erase(std::remove(m_slots.begin(), m_slots.end(), slot),
                      m_slots.end());
    }

    void call(const ISignal<Args...> &sender, Args... args)
    {
        if (blocked)
            return;
        auto slots = m_slots; // a slot may disconnect itself
        for (auto &slot : slots)
            (*slot)(sender, args...);
    }

    [[nodiscard]] size_t size() const { return m_slots.size(); }

    bool blocked = false;

private:
    std::vector<std::shared_ptr<ISlot<Args...>>> m_slots;
};

/// IDescription with quiet signals (value types: units, value definitions).
template <typename Interface>
class QuietDescription : public virtual Interface
{
public:
    explicit QuietDescription(std::string caption) : m_caption(std::move(caption)) {}

    void connect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void blockSignals(bool) override {}
    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }

protected:
    void emit(std::string) override {}

private:
    std::string m_caption;
    std::string m_description;
};

/// IPropertyChanged with quiet signals (dates).
template <typename Interface>
class QuietSignal : public virtual Interface
{
public:
    void connect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void blockSignals(bool) override {}

protected:
    void emit(std::string) override {}
};

class FixtureIdentity final : public QuietIdentity<IIdentity>
{
public:
    explicit FixtureIdentity(std::string id) : QuietIdentity<IIdentity>(std::move(id)) {}
};

// -- Value definitions ---------------------------------------------------------

/// Cubic feet per second: L^3 T^-1, 0.0283168 to SI.
class FixtureUnitDimensions final : public QuietDescription<IUnitDimensions>
{
public:
    FixtureUnitDimensions() : QuietDescription<IUnitDimensions>("L3 T-1") {}

    [[nodiscard]] double power(FundamentalUnitDimension dimension) const override
    {
        switch (dimension)
        {
            case FundamentalUnitDimension::Length: return 3.0;
            case FundamentalUnitDimension::Time:   return -1.0;
            default:                               return 0.0;
        }
    }
};

class FixtureUnit final : public QuietDescription<IUnit>
{
public:
    FixtureUnit() : QuietDescription<IUnit>("ft3/s") {}
    [[nodiscard]] IUnitDimensions *dimensions() const override
    {
        return const_cast<FixtureUnitDimensions *>(&m_dimensions);
    }
    [[nodiscard]] double conversionFactorToSI() const override { return 0.0283168; }
    [[nodiscard]] double offsetToSI() const override { return 0.0; }

private:
    FixtureUnitDimensions m_dimensions;
};

class FixtureQuantity final : public QuietDescription<IQuantity>
{
public:
    FixtureQuantity() : QuietDescription<IQuantity>("Discharge") {}
    [[nodiscard]] ValueKind valueKind() const override { return ValueKind::Flux; }
    [[nodiscard]] double missingValue() const override { return -9999.0; }
    [[nodiscard]] double defaultValue() const override { return 0.0; }
    [[nodiscard]] IUnit *unit() const override { return const_cast<FixtureUnit *>(&m_unit); }
    [[nodiscard]] double minValue() const override { return 0.0; }
    [[nodiscard]] double maxValue() const override { return 1.0e6; }

private:
    FixtureUnit m_unit;
};

class FixtureQuality final : public QuietDescription<IQuality>
{
public:
    FixtureQuality() : QuietDescription<IQuality>("Land use") {}
    [[nodiscard]] ValueKind valueKind() const override { return ValueKind::Intensive; }
    [[nodiscard]] double missingValue() const override { return -1.0; }
    [[nodiscard]] double defaultValue() const override { return 0.0; }
    [[nodiscard]] std::vector<std::string> categories() const override
    {
        return {"water", "forest", "urban"};
    }
    [[nodiscard]] bool isOrdered() const override { return false; }
};

// -- Event arguments -----------------------------------------------------------

class FixtureStatusChange final : public IComponentStatusChangeEventArgs
{
public:
    FixtureStatusChange(IModelComponent *component, IModelComponent::ComponentStatus previous,
                        IModelComponent::ComponentStatus status, std::string message)
        : m_component(component), m_previous(previous), m_status(status),
          m_message(std::move(message)) {}
    [[nodiscard]] IModelComponent *component() const override { return m_component; }
    [[nodiscard]] IModelComponent::ComponentStatus previousStatus() const override { return m_previous; }
    [[nodiscard]] IModelComponent::ComponentStatus status() const override { return m_status; }
    [[nodiscard]] std::string message() const override { return m_message; }
    [[nodiscard]] bool hasProgressMonitor() const override { return true; }
    [[nodiscard]] float percentProgress() const override { return 50.0f; }

private:
    IModelComponent *m_component;
    IModelComponent::ComponentStatus m_previous, m_status;
    std::string m_message;
};

class FixtureWorkflowStatusChange final : public IWorkflowComponentStatusChangeEventArgs
{
public:
    FixtureWorkflowStatusChange(IWorkflowComponent *workflow,
                                IWorkflowComponent::WorkflowStatus previous,
                                IWorkflowComponent::WorkflowStatus status, std::string message)
        : m_workflow(workflow), m_previous(previous), m_status(status),
          m_message(std::move(message)) {}
    [[nodiscard]] IWorkflowComponent *workflowComponent() const override { return m_workflow; }
    [[nodiscard]] IWorkflowComponent::WorkflowStatus previousStatus() const override { return m_previous; }
    [[nodiscard]] IWorkflowComponent::WorkflowStatus status() const override { return m_status; }
    [[nodiscard]] std::string message() const override { return m_message; }
    [[nodiscard]] bool hasProgressMonitor() const override { return false; }
    [[nodiscard]] float percentProgress() const override { return 0.0f; }

private:
    IWorkflowComponent *m_workflow;
    IWorkflowComponent::WorkflowStatus m_previous, m_status;
    std::string m_message;
};

class FixtureValueChange final : public IComponentDataItemValueChanged
{
public:
    FixtureValueChange(IComponentDataItem *item, std::vector<int64_t> start,
                       std::vector<int64_t> count)
        : m_item(item), m_start(std::move(start)), m_count(std::move(count)) {}
    [[nodiscard]] IComponentDataItem *componentDataItem() const override { return m_item; }
    [[nodiscard]] std::vector<int64_t> start() const override { return m_start; }
    [[nodiscard]] std::vector<int64_t> count() const override { return m_count; }

private:
    IComponentDataItem *m_item;
    std::vector<int64_t> m_start, m_count;
};

// -- A data item whose signals are real ------------------------------------

/// The plumbing every fixture exchange item shares: identity, a [4] Float64
/// field over one entity dimension, a quantity, and both signals.
template <typename Interface>
class SignalingItem : public virtual Interface
{
public:
    SignalingItem(std::string id, IModelComponent *owner, IValueDefinition *value)
        : m_id(std::move(id)), m_caption(m_id), m_owner(owner), m_value(value) {}

    // signals
    void connect(const std::shared_ptr<ISlot<std::string>> &slot) override { m_property.connect(slot); }
    void disconnect(const std::shared_ptr<ISlot<std::string>> &slot) override { m_property.disconnect(slot); }
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &slot) override { m_values.connect(slot); }
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &slot) override { m_values.disconnect(slot); }
    void blockSignals(bool block) override { m_property.blocked = block; m_values.blocked = block; }

    // identity
    [[nodiscard]] const std::string &id() const override { return m_id; }
    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override
    {
        m_caption = value;
        emit(std::string("Caption"));
    }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }

    // data item
    [[nodiscard]] IModelComponent *modelComponent() const override { return m_owner; }
    [[nodiscard]] std::vector<IDimension *> dimensions() const override
    {
        return {const_cast<FixtureDimension *>(&m_entities)};
    }
    [[nodiscard]] std::vector<int64_t> shape() const override { return {4}; }
    [[nodiscard]] DataKind dataKind() const override { return DataKind::Float64; }
    [[nodiscard]] IValueDefinition *valueDefinition() const override { return m_value; }
    [[nodiscard]] bool getValuesInto(const BufferDescriptor &destination,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message = nullptr) const override
    {
        if (destination.kind != DataKind::Float64 || start.size() != 1 ||
            count.size() != 1 || start[0] < 0 || start[0] + count[0] > 4)
        {
            if (message)
                *message = "bad selection";
            return false;
        }
        auto *out = static_cast<double *>(destination.data);
        for (int64_t i = 0; i < count[0]; ++i)
            out[i] = m_values_store[static_cast<size_t>(start[0] + i)];
        return true;
    }
    [[nodiscard]] bool setValuesFrom(const BufferDescriptor &source,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message = nullptr) override
    {
        if (source.kind != DataKind::Float64 || start.size() != 1 ||
            count.size() != 1 || start[0] < 0 || start[0] + count[0] > 4)
        {
            if (message)
                *message = "bad selection";
            return false;
        }
        const auto *in = static_cast<const double *>(source.data);
        for (int64_t i = 0; i < count[0]; ++i)
            m_values_store[static_cast<size_t>(start[0] + i)] = in[i];
        emit(std::make_shared<FixtureValueChange>(
            this, std::vector<int64_t>{start[0]}, std::vector<int64_t>{count[0]}));
        return true;
    }

    [[nodiscard]] size_t valueSlotCount() const { return m_values.size(); }

protected:
    void emit(std::string property) override { m_property.call(*this, property); }
    void emit(const std::shared_ptr<IComponentDataItemValueChanged> &args) override
    {
        m_values.call(*this, args);
    }

private:
    std::string m_id, m_caption, m_description;
    IModelComponent *m_owner;
    IValueDefinition *m_value;
    FixtureDimension m_entities{"cells", IDimension::DimensionRole::Entity};
    double m_values_store[4] = {1.0, 2.0, 3.0, 4.0};
    SlotList<std::string> m_property;
    SlotList<const std::shared_ptr<IComponentDataItemValueChanged> &> m_values;
};

class FixtureArgument final : public SignalingItem<IArgument>
{
public:
    FixtureArgument(IModelComponent *owner, IValueDefinition *value)
        : SignalingItem<IArgument>("roughness", owner, value) {}

    [[nodiscard]] ArgumentRole role() const override { return ArgumentRole::Parameter; }
    [[nodiscard]] bool isOptional() const override { return true; }
    [[nodiscard]] bool isReadOnly() const override { return false; }
    [[nodiscard]] std::string toString() const override { return m_text; }
    void saveData() override {}
    [[nodiscard]] std::vector<std::string> fileFilters() const override { return {"*.json"}; }
    [[nodiscard]] std::vector<const std::type_info *> validComponentDataItemTypes() const override
    {
        return {&typeid(FixtureArgument)};
    }
    [[nodiscard]] bool isValidArgType(ArgumentInputType type) const override
    {
        return type == ArgumentInputType::JSON;
    }
    [[nodiscard]] ArgumentInputType currentArgumentInputType() const override { return m_type; }
    [[nodiscard]] bool initialize(const std::string &value, ArgumentInputType type,
                                  std::string &message) override
    {
        if (type != ArgumentInputType::JSON)
        {
            message = "JSON only";
            return false;
        }
        m_text = value;
        m_type = type;
        message.clear();
        return true;
    }
    [[nodiscard]] bool initialize(const IComponentDataItem &item, std::string &message) override
    {
        m_text = "from " + item.id();
        message.clear();
        return true;
    }
    [[nodiscard]] bool serialize(ArgumentInputType type, std::string &value,
                                 std::string &message) const override
    {
        value = m_text;
        message.clear();
        return type == ArgumentInputType::JSON;
    }

private:
    std::string m_text = "{}";
    ArgumentInputType m_type = ArgumentInputType::JSON;
};

class FixtureAdaptedOutput;

class FixtureOutput : public SignalingItem<IOutput>
{
public:
    FixtureOutput(std::string id, IModelComponent *owner, IValueDefinition *value)
        : SignalingItem<IOutput>(std::move(id), owner, value) {}

    [[nodiscard]] std::vector<IInput *> consumers() const override { return m_consumers; }
    void addConsumer(IInput *consumer) override
    {
        std::string message;
        if (!consumer->canConsume(this, message))
            throw std::invalid_argument(message);
        if (!consumer->setProvider(this))
            throw std::invalid_argument("the input refused this output as its provider");
        m_consumers.push_back(consumer);
    }
    bool removeConsumer(IInput *consumer) override
    {
        auto it = std::find(m_consumers.begin(), m_consumers.end(), consumer);
        if (it == m_consumers.end())
            return false;
        m_consumers.erase(it);
        return true;
    }
    [[nodiscard]] std::vector<IAdaptedOutput *> adaptedOutputs() const override { return m_adapted; }
    void addAdaptedOutput(IAdaptedOutput *adapted) override { m_adapted.push_back(adapted); }
    bool removeAdaptedOutput(IAdaptedOutput *adapted) override
    {
        auto it = std::find(m_adapted.begin(), m_adapted.end(), adapted);
        if (it == m_adapted.end())
            return false;
        m_adapted.erase(it);
        return true;
    }
    void updateValues(const IInput *query) override
    {
        ++updates;
        lastQuery = query;
    }

    int updates = 0;
    const IInput *lastQuery = nullptr;

private:
    std::vector<IInput *> m_consumers;
    std::vector<IAdaptedOutput *> m_adapted;
};

class FixtureAdaptedOutputFactory;

/// A stateful adapter, y <- (y + x) / 2 at every refresh, that is both
/// differentiable and checkpointable: enough to drive every member of the
/// adapter wrapper through the bindings.
class FixtureAdaptedOutput final : public SignalingItem<IAdaptedOutput>,
                                   public virtual IDifferentiableAdaptedOutput,
                                   public virtual ICheckpointableAdaptedOutput
{
public:
    FixtureAdaptedOutput(IAdaptedOutputFactory *factory, IOutput *adaptee, IValueDefinition *value)
        : SignalingItem<IAdaptedOutput>("scaled", nullptr, value), m_factory(factory),
          m_adaptee(adaptee)
    {
        ++liveAdaptedOutputs();
    }
    ~FixtureAdaptedOutput() override { --liveAdaptedOutputs(); }

    [[nodiscard]] std::vector<IInput *> consumers() const override { return {}; }
    void addConsumer(IInput *) override {}
    bool removeConsumer(IInput *) override { return false; }
    [[nodiscard]] std::vector<IAdaptedOutput *> adaptedOutputs() const override { return {}; }
    void addAdaptedOutput(IAdaptedOutput *) override {}
    bool removeAdaptedOutput(IAdaptedOutput *) override { return false; }
    void updateValues(const IInput *) override {}
    [[nodiscard]] IAdaptedOutputFactory *adaptedOutputFactory() const override { return m_factory; }
    [[nodiscard]] std::vector<IArgument *> arguments() const override { return {}; }
    void initialize() override { initialized = true; }
    [[nodiscard]] IOutput *adaptee() const override { return m_adaptee; }
    void refresh() override
    {
        ++refreshes;
        m_source = read(m_adaptee);
        std::vector<double> y = read(this);
        for (size_t i = 0; i < y.size(); ++i)
            y[i] = 0.5 * (y[i] + m_source[i]);
        write(y);
        m_haveRefresh = true;
    }
    [[nodiscard]] std::vector<IComponentDataItem *> states() const override
    {
        return {const_cast<FixtureAdaptedOutput *>(this)};
    }

    // IDifferentiableAdaptedOutput: y' = (y + x) / 2, so both partials are 1/2.
    [[nodiscard]] std::vector<IArgument *> differentiableArguments() const override { return {}; }
    [[nodiscard]] std::vector<IComponentDataItem *> differentiableStates() const override
    {
        return states();
    }
    bool vjp(DifferentialSet seeds, DifferentialSet results, std::string *message) override
    {
        if (!m_haveRefresh)
            return refuse(message, "no refresh to differentiate");
        if (!wellFormed(seeds, message) || !wellFormed(results, message))
            return false;
        double t[4] = {0, 0, 0, 0};
        for (const DifferentialEntry &e : seeds)
        {
            if (e.item != self() || (e.role != DifferentialRole::Output &&
                                     e.role != DifferentialRole::StateAfter))
                return refuse(message, "unknown seed");
            for (int i = 0; i < 4; ++i)
                t[i] += static_cast<const double *>(e.value.data)[i];
        }
        for (const DifferentialEntry &e : results)
        {
            const bool input = e.item == m_adaptee && e.role == DifferentialRole::Input;
            const bool state = e.item == self() && e.role == DifferentialRole::StateBefore;
            if (!input && !state)
                return refuse(message, "unknown result");
            for (int i = 0; i < 4; ++i)
                static_cast<double *>(e.value.data)[i] = 0.5 * t[i];
        }
        return true;
    }
    bool jvp(DifferentialSet seeds, DifferentialSet results, std::string *message) override
    {
        if (!m_haveRefresh)
            return refuse(message, "no refresh to differentiate");
        if (!wellFormed(seeds, message) || !wellFormed(results, message))
            return false;
        double d[4] = {0, 0, 0, 0};
        for (const DifferentialEntry &e : seeds)
        {
            const bool input = e.item == m_adaptee && e.role == DifferentialRole::Input;
            const bool state = e.item == self() && e.role == DifferentialRole::StateBefore;
            if (!input && !state)
                return refuse(message, "unknown seed");
            for (int i = 0; i < 4; ++i)
                d[i] += 0.5 * static_cast<const double *>(e.value.data)[i];
        }
        for (const DifferentialEntry &e : results)
        {
            if (e.item != self() || (e.role != DifferentialRole::Output &&
                                     e.role != DifferentialRole::StateAfter))
                return refuse(message, "unknown result");
            for (int i = 0; i < 4; ++i)
                static_cast<double *>(e.value.data)[i] = d[i];
        }
        return true;
    }

    // ICheckpointableAdaptedOutput: the token is the state.
    bool saveState(std::string &token, std::string &) override
    {
        std::ostringstream out;
        out.precision(17);
        for (double v : read(this))
            out << v << ' ';
        token = out.str();
        ++saves;
        return true;
    }
    bool restoreState(const std::string &token, std::string &message) override
    {
        std::istringstream in(token);
        std::vector<double> y(4);
        for (double &v : y)
            in >> v;
        if (in.fail())
        {
            message = "bad token";
            return false;
        }
        write(y);
        m_haveRefresh = false;
        return true;
    }
    bool releaseState(const std::string &token, std::string &message) override
    {
        if (token.empty())
        {
            message = "empty token";
            return false;
        }
        ++releases;
        return true;
    }

    bool initialized = false;
    int refreshes = 0;
    int saves = 0;
    int releases = 0;

private:
    [[nodiscard]] const IComponentDataItem *self() const
    {
        return static_cast<const IComponentDataItem *>(static_cast<const IAdaptedOutput *>(this));
    }
    /// Every buffer a contiguous host Float64 [4] -- all this fixture reads.
    static bool wellFormed(DifferentialSet entries, std::string *message)
    {
        for (const DifferentialEntry &e : entries)
            if (e.value.kind != DataKind::Float64 || e.value.rank != 1 || !e.value.shape ||
                e.value.shape[0] != 4 ||
                (e.value.stridesBytes && e.value.stridesBytes[0] != sizeof(double)) ||
                e.value.space != MemorySpace::Host || !e.value.data)
                return refuse(message, "a buffer is not a contiguous host Float64 [4]");
        return true;
    }
    static std::vector<double> read(const IOutput *item)
    {
        std::vector<double> values(4, 0.0);
        const int64_t shape[1] = {4}, start[1] = {0};
        BufferDescriptor d;
        d.data = values.data();
        d.kind = DataKind::Float64;
        d.rank = 1;
        d.shape = shape;
        (void)item->getValuesInto(d, start, shape);
        return values;
    }
    void write(std::vector<double> values)
    {
        const int64_t shape[1] = {4}, start[1] = {0};
        BufferDescriptor d;
        d.data = values.data();
        d.kind = DataKind::Float64;
        d.rank = 1;
        d.shape = shape;
        (void)setValuesFrom(d, start, shape);
    }
    static bool refuse(std::string *message, const char *text)
    {
        if (message)
            *message = text;
        return false;
    }

    IAdaptedOutputFactory *m_factory;
    IOutput *m_adaptee;
    std::vector<double> m_source;
    bool m_haveRefresh = false;
};

class FixtureAdaptedOutputFactory final : public QuietIdentity<IAdaptedOutputFactory>
{
public:
    explicit FixtureAdaptedOutputFactory(IValueDefinition *value)
        : QuietIdentity<IAdaptedOutputFactory>("scaling-factory"), m_value(value) {}

    [[nodiscard]] std::vector<IIdentity *> getAvailableAdaptedOutputIds(
        const IOutput *, const IInput *) override
    {
        return {&m_scale};
    }

    [[nodiscard]] std::unique_ptr<IAdaptedOutput> createAdaptedOutput(
        IIdentity *id, IOutput *provider, IInput *) override
    {
        if (!id || id->id() != "scale")
            return nullptr;
        return std::make_unique<FixtureAdaptedOutput>(this, provider, m_value);
    }

private:
    FixtureIdentity m_scale{"scale"};
    IValueDefinition *m_value;
};

class FixtureMultiInput final : public SignalingItem<IMultiInput>
{
public:
    FixtureMultiInput(IModelComponent *owner, IValueDefinition *value)
        : SignalingItem<IMultiInput>("inflows", owner, value) {}

    [[nodiscard]] IOutput *provider() const override
    {
        return m_providers.empty() ? nullptr : m_providers.front();
    }
    bool setProvider(IOutput *provider) override
    {
        m_providers.clear();
        if (provider)
            m_providers.push_back(provider);
        return true;
    }
    [[nodiscard]] bool canConsume(IOutput *provider, std::string &message) const override
    {
        return canConsume(provider, message, nullptr);
    }
    [[nodiscard]] bool canConsume(IOutput *provider, std::string &message,
                                  const IIdentity *) const override
    {
        if (provider && provider->id() == "refused")
        {
            message = "refused by fixture";
            return false;
        }
        message.clear();
        return true;
    }
    [[nodiscard]] std::vector<IIdentity *> providerLabels() const override
    {
        return {const_cast<FixtureIdentity *>(&m_upstream)};
    }
    [[nodiscard]] bool isRequiredProvider(const IIdentity *label) const override
    {
        return label == &m_upstream;
    }
    [[nodiscard]] std::vector<IOutput *> providers() const override { return m_providers; }
    bool addProvider(IOutput *provider, const IIdentity *) override
    {
        m_providers.push_back(provider);
        return true;
    }
    bool removeProvider(IOutput *provider) override
    {
        auto it = std::find(m_providers.begin(), m_providers.end(), provider);
        if (it == m_providers.end())
            return false;
        m_providers.erase(it);
        return true;
    }

private:
    FixtureIdentity m_upstream{"upstream"};
    std::vector<IOutput *> m_providers;
};

// -- Dates -------------------------------------------------------------------

class FixtureDateTime final : public QuietSignal<Temporal::IDateTime>
{
public:
    explicit FixtureDateTime(double julian) : m_julian(julian) {}
    void set(double julian) { m_julian = julian; }
    [[nodiscard]] double julianDay() const override { return m_julian; }
    [[nodiscard]] double modifiedJulianDay() const override { return m_julian - 2400000.5; }
    [[nodiscard]] double serialDate() const override { return m_julian - 2415018.5; }

private:
    double m_julian;
};

class FixtureTimeSpan final : public QuietSignal<Temporal::ITimeSpan>
{
public:
    FixtureTimeSpan(double start, double days) : m_start(start), m_days(days) {}
    [[nodiscard]] double julianDay() const override { return m_start; }
    [[nodiscard]] double modifiedJulianDay() const override { return m_start - 2400000.5; }
    [[nodiscard]] double serialDate() const override { return m_start - 2415018.5; }
    [[nodiscard]] double duration() const override { return m_days; }
    [[nodiscard]] double endJulianDay() const override { return m_start + m_days; }

private:
    double m_start, m_days;
};

// -- A time-stepping component -------------------------------------------------

class FixtureComponentInfo;

class FixtureComponent final : public QuietIdentity<Temporal::ITimeModelComponent>
{
public:
    using CS = IModelComponent::ComponentStatus;

    FixtureComponent(std::string id, IModelComponentInfo *info)
        : QuietIdentity<Temporal::ITimeModelComponent>(std::move(id)), m_info(info),
          m_argument(this, &m_quantity), m_inflows(this, &m_quantity),
          m_discharge("discharge", this, &m_quantity), m_landUse("land_use", this, &m_quality)
    {
        ++liveComponents();
    }
    ~FixtureComponent() override { --liveComponents(); }

    // signals: property (quiet base) is overridden to be real, plus status
    void connect(const std::shared_ptr<ISlot<std::string>> &slot) override { m_property.connect(slot); }
    void disconnect(const std::shared_ptr<ISlot<std::string>> &slot) override { m_property.disconnect(slot); }
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentStatusChangeEventArgs> &>> &slot) override { m_status_slots.connect(slot); }
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentStatusChangeEventArgs> &>> &slot) override { m_status_slots.disconnect(slot); }
    void blockSignals(bool block) override
    {
        m_property.blocked = block;
        m_status_slots.blocked = block;
    }

    void setCaption(const std::string &value) override
    {
        QuietIdentity<Temporal::ITimeModelComponent>::setCaption(value);
        emit(std::string("Caption"));
    }

    // IModelComponent
    [[nodiscard]] IModelComponentInfo *componentInfo() const override { return m_info; }
    [[nodiscard]] ComponentStatus status() const override { return m_status; }
    [[nodiscard]] std::vector<IArgument *> arguments() const override
    {
        return {const_cast<FixtureArgument *>(&m_argument)};
    }
    [[nodiscard]] std::vector<IInput *> inputs() const override
    {
        return {const_cast<FixtureMultiInput *>(&m_inflows)};
    }
    [[nodiscard]] std::vector<IOutput *> outputs() const override
    {
        return {const_cast<FixtureOutput *>(&m_discharge)};
    }
    [[nodiscard]] std::vector<IComponentDataItem *> results() const override
    {
        return {const_cast<FixtureOutput *>(&m_landUse)};
    }
    [[nodiscard]] std::vector<IComponentDataItem *> states() const override { return {}; }
    void initialize() override { change(CS::Initializing); change(CS::Initialized); }
    [[nodiscard]] std::vector<std::string> validate() override
    {
        change(CS::Validating);
        change(CS::Valid);
        return {};
    }
    void prepare() override { change(CS::Preparing); change(CS::Updated); }
    void update(const std::vector<IOutput *> &required = {}) override
    {
        lastRequired = required.size();
        change(CS::Updating);
        m_julian += 1.0;
        change(CS::Updated, "stepped");
    }
    void finish() override { change(CS::Finishing); change(CS::Finished); }
    [[nodiscard]] const IWorkflowComponent *workflow() const override { return m_workflow; }
    void setWorkflow(const IWorkflowComponent *workflow) override { m_workflow = workflow; }
    [[nodiscard]] std::set<Capability> capabilities() const override { return {}; }
    [[nodiscard]] std::vector<ErrorEntry> errors(bool clearAfterRead = false) override
    {
        std::vector<ErrorEntry> copy = m_errors;
        if (clearAfterRead)
            m_errors.clear();
        return copy;
    }
    [[nodiscard]] std::string referenceDirectory() const override { return m_directory; }
    void setReferenceDirectory(const std::string &directory) override { m_directory = directory; }

    // ITimeModelComponent
    [[nodiscard]] Temporal::IDateTime *currentDateTime() const override
    {
        m_now.set(m_julian);
        return &m_now;
    }
    [[nodiscard]] Temporal::ITimeSpan *simulationPeriod() const override
    {
        return const_cast<FixtureTimeSpan *>(&m_period);
    }
    [[nodiscard]] double nextDateTimeJulianDay() const override { return m_julian + 1.0; }

    size_t lastRequired = 0;
    FixtureOutput &discharge() { return m_discharge; }
    FixtureMultiInput &inflows() { return m_inflows; }

private:
    void change(CS to, const std::string &message = {})
    {
        const CS from = m_status;
        m_status = to;
        if (to == CS::Failed)
            m_errors.push_back({ErrorEntry::Severity::Fatal, 7, id(), message});
        emit(std::make_shared<FixtureStatusChange>(this, from, to, message));
    }

protected:
    void emit(std::string property) override { m_property.call(*this, property); }
    void emit(const std::shared_ptr<IComponentStatusChangeEventArgs> &args) override
    {
        m_status_slots.call(*this, args);
    }

private:
    IModelComponentInfo *m_info;
    ComponentStatus m_status = CS::Created;
    const IWorkflowComponent *m_workflow = nullptr;
    std::string m_directory = "/tmp";
    std::vector<ErrorEntry> m_errors{{ErrorEntry::Severity::Warning, 3, "fixture", "warmed up"}};
    double m_julian = 2461000.5;
    mutable FixtureDateTime m_now{2461000.5};
    FixtureTimeSpan m_period{2461000.5, 10.0};
    FixtureQuantity m_quantity;
    FixtureQuality m_quality;
    FixtureArgument m_argument;
    FixtureMultiInput m_inflows;
    FixtureOutput m_discharge;
    FixtureOutput m_landUse;
    SlotList<std::string> m_property;
    SlotList<const std::shared_ptr<IComponentStatusChangeEventArgs> &> m_status_slots;
};

class FixtureComponentInfo final : public QuietIdentity<IModelComponentInfo>
{
public:
    FixtureComponentInfo() : QuietIdentity<IModelComponentInfo>("fixture-info"), m_factory(&m_quantity) {}

    [[nodiscard]] std::string libraryFilePath() const override { return m_path; }
    void setLibraryFilePath(const std::string &path) override { m_path = path; }
    [[nodiscard]] std::string iconFilePath() const override { return "icon.png"; }
    [[nodiscard]] std::string developer() const override { return "HydroCouple"; }
    [[nodiscard]] std::vector<std::string> documentation() const override { return {"README.md"}; }
    [[nodiscard]] std::string license() const override { return "MIT"; }
    [[nodiscard]] std::string copyright() const override { return "2026"; }
    [[nodiscard]] std::string url() const override { return "https://example.org"; }
    [[nodiscard]] std::string email() const override { return "dev@example.org"; }
    [[nodiscard]] std::string version() const override { return "1.2.3"; }
    [[nodiscard]] std::set<std::string> tags() const override { return {"hydrology", "test"}; }
    [[nodiscard]] std::unique_ptr<IModelComponent> createComponentInstance() override
    {
        return std::make_unique<FixtureComponent>("made-" + std::to_string(++m_made), this);
    }
    [[nodiscard]] std::vector<IAdaptedOutputFactory *> adaptedOutputFactories() const override
    {
        return {const_cast<FixtureAdaptedOutputFactory *>(&m_factory)};
    }

private:
    std::string m_path = "libfixture.so";
    int m_made = 0;
    FixtureQuantity m_quantity;
    FixtureAdaptedOutputFactory m_factory;
};

// -- A workflow ------------------------------------------------------------------

class FixtureWorkflow final : public QuietIdentity<IWorkflowComponent>
{
public:
    using WS = IWorkflowComponent::WorkflowStatus;

    FixtureWorkflow() : QuietIdentity<IWorkflowComponent>("fixture-workflow") {}

    using QuietIdentity<IWorkflowComponent>::connect;
    using QuietIdentity<IWorkflowComponent>::disconnect;
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>> &slot) override { m_slots.connect(slot); }
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>> &slot) override { m_slots.disconnect(slot); }
    void blockSignals(bool block) override { m_slots.blocked = block; }

    [[nodiscard]] IWorkflowComponentInfo *componentInfo() const override { return nullptr; }
    [[nodiscard]] std::vector<IIdentity *> modelComponentLabels() const override
    {
        return {const_cast<FixtureIdentity *>(&m_driver)};
    }
    [[nodiscard]] bool isRequiredModelComponent(const IIdentity *label) const override
    {
        return label == &m_driver;
    }
    void initialize() override { change(WS::Initializing); change(WS::Initialized); }
    [[nodiscard]] std::vector<std::string> validate() override
    {
        change(WS::Validating);
        if (m_components.empty())
        {
            change(WS::Failed, "no components");
            return {"no components"};
        }
        change(WS::Validated);
        return {};
    }
    void prepare() override { change(WS::Preparing); change(WS::Prepared); }
    void update() override { change(WS::Updating); change(WS::Updated); }
    void finish() override { change(WS::Finishing); change(WS::Finished); }
    void requestStop() override { stopRequested = true; }
    void requestPause() override { pauseRequested = true; }
    void resume() override { resumed = true; }
    [[nodiscard]] std::vector<ErrorEntry> errors(bool) override { return m_errors; }
    [[nodiscard]] WorkflowStatus status() const override { return m_status; }
    [[nodiscard]] std::vector<IModelComponent *> modelComponents() const override { return m_components; }
    bool addModelComponent(IModelComponent *component, const IIdentity *role = nullptr,
                           std::string *message = nullptr) override
    {
        if (role && role != &m_driver)
        {
            if (message)
                *message = "unknown role";
            return false;
        }
        m_components.push_back(component);
        component->setWorkflow(this);
        lastRole = role;
        return true;
    }
    bool removeModelComponent(IModelComponent *component) override
    {
        auto it = std::find(m_components.begin(), m_components.end(), component);
        if (it == m_components.end())
            return false;
        m_components.erase(it);
        return true;
    }

    bool stopRequested = false, pauseRequested = false, resumed = false;
    const IIdentity *lastRole = nullptr;

private:
    void change(WS to, const std::string &message = {})
    {
        const WS from = m_status;
        m_status = to;
        if (to == WS::Failed)
            m_errors.push_back({ErrorEntry::Severity::Error, 1, id(), message});
        emit(std::make_shared<FixtureWorkflowStatusChange>(this, from, to, message));
    }

protected:
    using QuietIdentity<IWorkflowComponent>::emit;
    void emit(const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &args) override
    {
        m_slots.call(*this, args);
    }

private:
    FixtureIdentity m_driver{"driver"};
    WorkflowStatus m_status = WS::Created;
    std::vector<IModelComponent *> m_components;
    std::vector<ErrorEntry> m_errors;
    SlotList<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &> m_slots;
};

// -- Spatial reference --------------------------------------------------------

class FixtureSRS final : public Spatial::ISpatialReferenceSystem
{
public:
    [[nodiscard]] int authSRID() const override { return 26917; }
    [[nodiscard]] const std::string &authName() const override { return m_epsg; }
    [[nodiscard]] const std::string &srText() const override { return m_wkt; }
    [[nodiscard]] IUnit::DistanceUnits distanceUnits() const override { return IUnit::DistanceUnits::Meters; }
    [[nodiscard]] const std::string &verticalAuthName() const override { return m_epsg; }
    [[nodiscard]] int verticalAuthSRID() const override { return 5703; }
    [[nodiscard]] const std::string &verticalSrText() const override { return m_vertical; }
    [[nodiscard]] IUnit::DistanceUnits verticalDistanceUnits() const override { return IUnit::DistanceUnits::Feet; }

private:
    std::string m_epsg = "EPSG";
    std::string m_wkt = "PROJCS[\"NAD83 / UTM zone 17N\"]";
    std::string m_vertical = "VERT_CS[\"NAVD88\"]";
};

inline FixtureSRS &fixtureSRS()
{
    static FixtureSRS srs;
    return srs;
}

// -- Geometries whose operations make geometries -----------------------------

class FixturePoint;
class FixturePolygon;

/// IGeometry's members for every fixture geometry. Operations that make a
/// geometry make a FixturePoint or FixturePolygon, so a test can count them.
class FixtureGeometry : public virtual Spatial::IGeometry
{
public:
    explicit FixtureGeometry(GeometryType type, std::string id)
        : m_type(type), m_id(std::move(id)) { ++liveGeometries(); }
    ~FixtureGeometry() override { --liveGeometries(); }

    [[nodiscard]] const std::string &id() const override { return m_id; }
    [[nodiscard]] int64_t index() const override { return m_index; }
    [[nodiscard]] int dimension() const override;
    [[nodiscard]] int coordinateDimension() const override { return 2; }
    [[nodiscard]] GeometryType geometryType() const override { return m_type; }
    [[nodiscard]] Spatial::ISpatialReferenceSystem *spatialReferenceSystem() const override
    {
        return &fixtureSRS();
    }
    [[nodiscard]] Spatial::IEnvelope *envelope() const override { return nullptr; }
    [[nodiscard]] std::string getWKT() const override { return "FIXTURE(" + m_id + ")"; }
    [[nodiscard]] std::vector<unsigned char> getWKB() const override { return {1, 2, 3}; }
    [[nodiscard]] bool isEmpty() const override { return false; }
    [[nodiscard]] bool isSimple() const override { return true; }
    [[nodiscard]] bool is3D() const override { return false; }
    [[nodiscard]] bool isMeasured() const override { return false; }
    [[nodiscard]] std::unique_ptr<IGeometry> boundary() const override;
    [[nodiscard]] bool equals(const IGeometry &g) const override { return &g == this; }
    [[nodiscard]] bool disjoint(const IGeometry &g) const override { return !intersects(g); }
    [[nodiscard]] bool intersects(const IGeometry &) const override { return true; }
    [[nodiscard]] bool touches(const IGeometry &) const override { return false; }
    [[nodiscard]] bool crosses(const IGeometry &) const override { return false; }
    [[nodiscard]] bool within(const IGeometry &g) const override { return &g != this; }
    [[nodiscard]] bool contains(const IGeometry &g) const override { return &g == this; }
    [[nodiscard]] bool overlaps(const IGeometry &) const override { return false; }
    [[nodiscard]] bool relate(const IGeometry &, const std::string &pattern) const override
    {
        return pattern == "T*F**FFF*";
    }
    [[nodiscard]] std::unique_ptr<IGeometry> locateAlong(double) const override { return nullptr; }
    [[nodiscard]] std::unique_ptr<IGeometry> locateBetween(double, double) const override { return nullptr; }
    [[nodiscard]] double distance(const IGeometry &) const override { return 1.5; }
    [[nodiscard]] std::unique_ptr<IGeometry> buffer(double distance) const override;
    [[nodiscard]] std::unique_ptr<IGeometry> convexHull() const override;
    [[nodiscard]] std::unique_ptr<IGeometry> intersection(const IGeometry &) const override;
    [[nodiscard]] std::unique_ptr<IGeometry> unionG(const IGeometry &) const override;
    [[nodiscard]] std::unique_ptr<IGeometry> difference(const IGeometry &) const override { return nullptr; }
    [[nodiscard]] std::unique_ptr<IGeometry> symmetricDifference(const IGeometry &) const override;

    int64_t m_index = 0;

private:
    GeometryType m_type;
    std::string m_id;
};

class FixturePoint final : public FixtureGeometry, public virtual Spatial::IVertex
{
public:
    FixturePoint(double x, double y, std::string id = "point")
        : FixtureGeometry(GeometryType::Point, std::move(id)), m_x(x), m_y(y) {}
    [[nodiscard]] double x() const override { return m_x; }
    [[nodiscard]] double y() const override { return m_y; }
    [[nodiscard]] double z() const override { return 0.0; }
    [[nodiscard]] double m() const override { return 0.0; }
    [[nodiscard]] Spatial::IEdge *edge() const override { return nullptr; }

private:
    double m_x, m_y;
};

class FixtureLineString final : public FixtureGeometry, public virtual Spatial::ILineString
{
public:
    explicit FixtureLineString(std::vector<std::pair<double, double>> xy)
        : FixtureGeometry(GeometryType::LineString, "ring")
    {
        for (size_t i = 0; i < xy.size(); ++i)
            m_points.push_back(std::make_unique<FixturePoint>(xy[i].first, xy[i].second,
                                                              "p" + std::to_string(i)));
    }
    [[nodiscard]] double length() const override
    {
        double total = 0.0;
        for (size_t i = 1; i < m_points.size(); ++i)
            total += std::hypot(m_points[i]->x() - m_points[i - 1]->x(),
                                m_points[i]->y() - m_points[i - 1]->y());
        return total;
    }
    [[nodiscard]] Spatial::IPoint *startPoint() const override { return m_points.front().get(); }
    [[nodiscard]] Spatial::IPoint *endPoint() const override { return m_points.back().get(); }
    [[nodiscard]] bool isClosed() const override
    {
        return m_points.front()->x() == m_points.back()->x() &&
               m_points.front()->y() == m_points.back()->y();
    }
    [[nodiscard]] bool isRing() const override { return isClosed(); }
    [[nodiscard]] int64_t pointCount() const override { return static_cast<int64_t>(m_points.size()); }
    [[nodiscard]] Spatial::IPoint *point(int64_t index) const override
    {
        if (index < 0 || index >= pointCount())
            throw std::out_of_range("point index");
        return m_points[static_cast<size_t>(index)].get();
    }

private:
    std::vector<std::unique_ptr<FixturePoint>> m_points;
};

/// An axis-aligned square of side `side` with its corner at the origin.
class FixturePolygon final : public FixtureGeometry, public virtual Spatial::IPolygon
{
public:
    explicit FixturePolygon(double side, std::string id = "square")
        : FixtureGeometry(GeometryType::Polygon, std::move(id)), m_side(side),
          m_ring({{0, 0}, {side, 0}, {side, side}, {0, side}, {0, 0}}) {}

    [[nodiscard]] double area() const override { return m_side * m_side; }
    [[nodiscard]] std::unique_ptr<Spatial::IPoint> centroid() const override
    {
        return std::make_unique<FixturePoint>(m_side / 2, m_side / 2, "centroid");
    }
    [[nodiscard]] std::unique_ptr<Spatial::IPoint> pointOnSurface() const override { return centroid(); }
    [[nodiscard]] std::unique_ptr<Spatial::IMultiCurve> boundaryMultiCurve() const override { return nullptr; }
    [[nodiscard]] Spatial::ILineString *exteriorRing() const override
    {
        return const_cast<FixtureLineString *>(&m_ring);
    }
    [[nodiscard]] int64_t interiorRingCount() const override { return 0; }
    [[nodiscard]] Spatial::ILineString *interiorRing(int64_t) const override
    {
        throw std::out_of_range("no interior rings");
    }
    [[nodiscard]] Spatial::IEdge *edge() const override { return nullptr; }
    [[nodiscard]] Spatial::IPolyhedralSurface *polyhedralSurface() const override { return nullptr; }

private:
    double m_side;
    FixtureLineString m_ring;
};

inline int FixtureGeometry::dimension() const
{
    return m_type == GeometryType::Point ? 0 : m_type == GeometryType::LineString ? 1 : 2;
}
inline std::unique_ptr<Spatial::IGeometry> FixtureGeometry::boundary() const
{
    return std::make_unique<FixturePoint>(0, 0, "boundary");
}
inline std::unique_ptr<Spatial::IGeometry> FixtureGeometry::buffer(double distance) const
{
    return std::make_unique<FixturePolygon>(2.0 * distance, "buffer");
}
inline std::unique_ptr<Spatial::IGeometry> FixtureGeometry::convexHull() const
{
    return std::make_unique<FixturePolygon>(1.0, "hull");
}
inline std::unique_ptr<Spatial::IGeometry> FixtureGeometry::intersection(const IGeometry &) const
{
    return std::make_unique<FixturePoint>(0.5, 0.5, "intersection");
}
inline std::unique_ptr<Spatial::IGeometry> FixtureGeometry::unionG(const IGeometry &) const
{
    return std::make_unique<FixturePolygon>(3.0, "union");
}
inline std::unique_ptr<Spatial::IGeometry> FixtureGeometry::symmetricDifference(const IGeometry &) const
{
    return std::make_unique<FixturePoint>(9, 9, "symmetric");
}

/// Geometries as a data item: {square of side 2, point (1, 1)}.
class FixtureGeometryItem final : public FixtureItemBase,
                                  public virtual Spatial::IGeometryComponentDataItem
{
public:
    FixtureGeometryItem()
        : FixtureItemBase("shapes", {2}, {&m_dimension}), m_square(2.0), m_point(1.0, 1.0)
    {
        m_point.m_index = 1;
    }
    [[nodiscard]] Spatial::IGeometry::GeometryType geometryType() const override
    {
        return Spatial::IGeometry::GeometryType::Geometry;
    }
    [[nodiscard]] int64_t geometryCount() const override { return 2; }
    [[nodiscard]] Spatial::IGeometry *geometry(int64_t index) const override
    {
        if (index == 0)
            return const_cast<FixturePolygon *>(&m_square);
        if (index == 1)
            return const_cast<FixturePoint *>(&m_point);
        throw std::out_of_range("geometry index");
    }
    [[nodiscard]] IDimension *geometryDimension() const override
    {
        return const_cast<FixtureDimension *>(&m_dimension);
    }
    [[nodiscard]] Spatial::IEnvelope *envelope() const override { return nullptr; }

private:
    FixtureDimension m_dimension{"geometries", IDimension::DimensionRole::Entity};
    FixturePolygon m_square;
    FixturePoint m_point;
};

// -- Raster -------------------------------------------------------------------------

class FixtureRaster;

/// 3 columns x 2 rows of Float64, values 10*row + column.
class FixtureBand final : public QuietIdentity<Spatial::IRasterBand>
{
public:
    using RasterDataType = Spatial::IRaster::RasterDataType;

    FixtureBand(FixtureRaster *raster, std::string id, RasterDataType type)
        : QuietIdentity<Spatial::IRasterBand>(std::move(id)), m_raster(raster), m_type(type)
    {
        for (int r = 0; r < 2; ++r)
            for (int c = 0; c < 3; ++c)
                m_values[r * 3 + c] = 10.0 * r + c;
    }
    [[nodiscard]] int64_t xSize() const override { return 3; }
    [[nodiscard]] int64_t ySize() const override { return 2; }
    [[nodiscard]] Spatial::IRaster *raster() const override;
    [[nodiscard]] RasterDataType dataType() const override { return m_type; }
    [[nodiscard]] bool read(int64_t x0, int64_t y0, int64_t nx, int64_t ny,
                            const BufferDescriptor &d, std::string *message = nullptr) const override
    {
        if (!fits(x0, y0, nx, ny, d, message))
            return false;
        auto *base = static_cast<char *>(d.data);
        for (int64_t r = 0; r < ny; ++r)
            for (int64_t c = 0; c < nx; ++c)
                *reinterpret_cast<double *>(base + offset(d, r, c)) =
                    m_values[(y0 + r) * 3 + (x0 + c)];
        return true;
    }
    [[nodiscard]] bool write(int64_t x0, int64_t y0, int64_t nx, int64_t ny,
                             const BufferDescriptor &d, std::string *message = nullptr) override
    {
        if (!fits(x0, y0, nx, ny, d, message))
            return false;
        const auto *base = static_cast<const char *>(d.data);
        for (int64_t r = 0; r < ny; ++r)
            for (int64_t c = 0; c < nx; ++c)
                m_values[(y0 + r) * 3 + (x0 + c)] =
                    *reinterpret_cast<const double *>(base + offset(d, r, c));
        return true;
    }
    [[nodiscard]] double noData() const override { return -32768.0; }

    [[nodiscard]] double value(int64_t row, int64_t column) const { return m_values[row * 3 + column]; }

private:
    static bool fits(int64_t x0, int64_t y0, int64_t nx, int64_t ny, const BufferDescriptor &d,
                     std::string *message)
    {
        const bool ok = d.kind == DataKind::Float64 && d.rank == 2 && d.shape &&
                        d.shape[0] == ny && d.shape[1] == nx && x0 >= 0 && y0 >= 0 &&
                        x0 + nx <= 3 && y0 + ny <= 2 && d.space == MemorySpace::Host;
        if (!ok && message)
            *message = "window or buffer does not fit";
        return ok;
    }
    static int64_t offset(const BufferDescriptor &d, int64_t r, int64_t c)
    {
        const int64_t s0 = d.stridesBytes ? d.stridesBytes[0] : d.shape[1] * 8;
        const int64_t s1 = d.stridesBytes ? d.stridesBytes[1] : 8;
        return r * s0 + c * s1;
    }

    FixtureRaster *m_raster;
    RasterDataType m_type;
    double m_values[6];
};

class FixtureRaster final : public QuietIdentity<Spatial::IRaster>
{
public:
    FixtureRaster() : QuietIdentity<Spatial::IRaster>("dem")
    {
        addRasterBand(RasterDataType::Float64);
    }
    [[nodiscard]] int64_t xSize() const override { return 3; }
    [[nodiscard]] int64_t ySize() const override { return 2; }
    [[nodiscard]] int64_t rasterBandCount() const override { return static_cast<int64_t>(m_bands.size()); }
    void addRasterBand(RasterDataType type) override
    {
        m_bands.push_back(std::make_unique<FixtureBand>(
            this, "band" + std::to_string(m_bands.size()), type));
    }
    [[nodiscard]] Spatial::ISpatialReferenceSystem *spatialReferenceSystem() const override
    {
        return &fixtureSRS();
    }
    void geoTransformation(double *matrix) const override
    {
        const double gt[6] = {100.0, 10.0, 0.0, 200.0, 0.0, -10.0};
        std::memcpy(matrix, gt, sizeof(gt));
    }
    [[nodiscard]] Spatial::IRasterBand *getRasterBand(int64_t index) const override
    {
        if (index < 0 || index >= rasterBandCount())
            throw std::out_of_range("band index");
        return m_bands[static_cast<size_t>(index)].get();
    }

private:
    std::vector<std::unique_ptr<FixtureBand>> m_bands;
};

inline Spatial::IRaster *FixtureBand::raster() const { return m_raster; }

class FixtureRasterItem final : public FixtureItemBase,
                                public virtual Spatial::IRasterComponentDataItem
{
public:
    FixtureRasterItem()
        : FixtureItemBase("elevation", {1, 2, 3}, {&m_band, &m_y, &m_x}) {}
    [[nodiscard]] Spatial::IRaster *raster() const override { return const_cast<FixtureRaster *>(&m_raster); }
    [[nodiscard]] IDimension *xDimension() const override { return const_cast<FixtureDimension *>(&m_x); }
    [[nodiscard]] IDimension *yDimension() const override { return const_cast<FixtureDimension *>(&m_y); }
    [[nodiscard]] IDimension *bandDimension() const override { return const_cast<FixtureDimension *>(&m_band); }

    FixtureRaster &rasterFixture() { return m_raster; }

private:
    FixtureDimension m_band{"band", IDimension::DimensionRole::Band};
    FixtureDimension m_y{"y", IDimension::DimensionRole::Row};
    FixtureDimension m_x{"x", IDimension::DimensionRole::Column};
    FixtureRaster m_raster;
};

// -- Everything, owned in one place ---------------------------------------------

class BindingFixtures
{
public:
    BindingFixtures()
        : component("fixture", &info) {}

    IModelComponentInfo *infoPointer() { return &info; }
    IModelComponent *componentPointer() { return &component; }
    IWorkflowComponent *workflowPointer() { return &workflow; }
    IComponentDataItem *geometryItem() { return &shapes; }
    IComponentDataItem *rasterItem() { return &elevation; }
    IComponentDataItem *refusedOutput() { return &refused; }

    int dischargeUpdates() { return component.discharge().updates; }
    bool dischargeQueriedBy(IComponentDataItem *input)
    {
        return component.discharge().lastQuery == dynamic_cast<IInput *>(input);
    }
    size_t lastRequired() const { return component.lastRequired; }
    bool stopRequested() const { return workflow.stopRequested; }
    bool pauseRequested() const { return workflow.pauseRequested; }
    bool resumed() const { return workflow.resumed; }
    bool workflowRoleWasDriver() const
    {
        return workflow.lastRole && workflow.lastRole->id() == "driver";
    }
    double bandValue(int64_t row, int64_t column)
    {
        return dynamic_cast<FixtureBand *>(elevation.rasterFixture().getRasterBand(0))->value(row, column);
    }

    FixtureComponentInfo info;
    FixtureComponent component;
    FixtureWorkflow workflow;
    FixtureGeometryItem shapes;
    FixtureRasterItem elevation;
    FixtureQuantity quantity;
    FixtureOutput refused{"refused", nullptr, &quantity};
};

} // namespace Testing
} // namespace Python
} // namespace HydroCouple
