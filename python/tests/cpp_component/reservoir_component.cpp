/*
 * reservoir_component.cpp
 *
 * A native C++20 HydroCouple component with a HAND-WRITTEN ADJOINT: the
 * fixture that proves gradients cross the boundary between a C++ model and
 * a PyTorch / JAX model (Phases G1 and G3 of
 * plans/hydrocouple/DIFFERENTIABLE_INTERFACE_ALTERNATIVES_2026-09-28.md).
 *
 * Physics: kCells independent nonlinear reservoirs, one step per update():
 *
 *     S' = S + dt * (I - k*S - c*S^2)        storage   (state)
 *     Q  = k * S'                            outflow   (output)
 *
 * with inflow I an IInput, recession k an IArgument (one per cell), c and
 * dt constants. The S^2 term is there on purpose: it makes the derivative
 * depend on the primal state, so a test can tell whether a vjp was taken
 * at the right linearization point.
 *
 * Adjoint (reverse mode), given Qb and Sb' (cotangents of Q and S'):
 *
 *     Sb'_total = Sb' + k*Qb
 *     Ib        = dt * Sb'_total
 *     kb        = Qb*S' - dt*S*Sb'_total
 *     Sb        = (1 - dt*k - 2*dt*c*S) * Sb'_total
 *
 * Tangent (forward mode), given dI, dk, dS:
 *
 *     dS' = (1 - dt*k - 2*dt*c*S)*dS + dt*dI - dt*S*dk
 *     dQ  = dk*S' + k*dS'
 *
 * Falsifier hooks (compile-time, used by verification/g0g1/falsify.sh):
 *     HC_FALSIFY_KB_SIGN       flip the sign of the S' term in kb
 *     HC_FALSIFY_DROP_STATE    forget Sb' (no gradient through time)
 *     HC_FALSIFY_STALE_POINT   linearize at the storage after the step
 *     HC_FALSIFY_ACCUMULATE    add into result buffers instead of writing
 *     HC_FALSIFY_JVP_DK        drop the dk*S' term from dQ
 */

#include "hydrocouple.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace HydroCouple;

namespace {

constexpr int64_t kCells = 4;
constexpr double kDt = 1.0;
constexpr double kC = 0.02;

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
// Strided 1-D access to a caller's buffer.
// ---------------------------------------------------------------------------
bool checkVector(const BufferDescriptor &d, std::string *message)
{
    auto fail = [&](const char *text) {
        if (message) *message = text;
        return false;
    };
    if (d.space != MemorySpace::Host)
        return fail("reservoir is host-only");
    if (d.kind != DataKind::Float64)
        return fail("kind mismatch");
    int64_t n = 1;
    for (int32_t k = 0; k < d.rank; ++k)
        n *= d.shape[k];
    if (n != kCells || d.rank > 1)
        return fail("buffer must hold exactly kCells values");
    return true;
}

double &at(const BufferDescriptor &d, int64_t i)
{
    const int64_t step = (d.rank == 1 && d.stridesBytes) ? d.stridesBytes[0]
                                                         : static_cast<int64_t>(sizeof(double));
    return *reinterpret_cast<double *>(static_cast<char *>(d.data) + i * step);
}

// ---------------------------------------------------------------------------
// A [kCells] Float64 item; the data plane reads and writes whole-or-part.
// ---------------------------------------------------------------------------
class VectorItem : public virtual IComponentDataItem,
                   public SignalBase<IComponentDataItemValueChanged>
{
protected:
    IModelComponent *m_owner;
    std::string m_id;
    std::string m_caption;
    std::string m_description;

public:
    std::vector<double> values;

    VectorItem(IModelComponent *owner, std::string id, double initial)
        : m_owner(owner), m_id(std::move(id)), m_caption(m_id),
          values(static_cast<size_t>(kCells), initial) {}

    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }

    [[nodiscard]] IModelComponent *modelComponent() const override { return m_owner; }
    [[nodiscard]] std::vector<IDimension *> dimensions() const override { return {}; }
    [[nodiscard]] std::vector<int64_t> shape() const override { return {kCells}; }
    [[nodiscard]] DataKind dataKind() const override { return DataKind::Float64; }
    [[nodiscard]] IValueDefinition *valueDefinition() const override { return nullptr; }

    bool slab(const BufferDescriptor &d, std::span<const int64_t> start,
              std::span<const int64_t> count, std::string *message) const
    {
        if (d.space != MemorySpace::Host)
        {
            if (message) *message = "reservoir is host-only";
            return false;
        }
        if (d.kind != DataKind::Float64)
        {
            if (message) *message = "kind mismatch";
            return false;
        }
        if (start.size() != 1 || count.size() != 1 || start[0] < 0 ||
            count[0] < 0 || start[0] + count[0] > kCells)
        {
            if (message) *message = "selection out of bounds";
            return false;
        }
        return true;
    }

    [[nodiscard]] bool getValuesInto(const BufferDescriptor &d,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message) const override
    {
        if (!slab(d, start, count, message))
            return false;
        for (int64_t i = 0; i < count[0]; ++i)
            at(d, i) = values[static_cast<size_t>(start[0] + i)];
        return true;
    }

    [[nodiscard]] bool setValuesFrom(const BufferDescriptor &d,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message) override
    {
        if (!slab(d, start, count, message))
            return false;
        for (int64_t i = 0; i < count[0]; ++i)
            values[static_cast<size_t>(start[0] + i)] = at(d, i);
        return true;
    }
};

class InflowInput final : public virtual IInput, public VectorItem
{
    IOutput *m_provider = nullptr;

public:
    explicit InflowInput(IModelComponent *owner) : VectorItem(owner, "inflow", 0.0) {}
    [[nodiscard]] IOutput *provider() const override { return m_provider; }
    [[nodiscard]] bool setProvider(IOutput *provider) override
    {
        m_provider = provider;
        return true;
    }
    [[nodiscard]] bool canConsume(IOutput *, std::string &) const override { return true; }
};

class OutflowOutput final : public virtual IOutput, public VectorItem
{
    std::vector<IInput *> m_consumers;

public:
    explicit OutflowOutput(IModelComponent *owner) : VectorItem(owner, "outflow", 0.0) {}
    [[nodiscard]] std::vector<IInput *> consumers() const override { return m_consumers; }
    void addConsumer(IInput *consumer) override { m_consumers.push_back(consumer); }
    [[nodiscard]] bool removeConsumer(IInput *) override { return false; }
    [[nodiscard]] std::vector<IAdaptedOutput *> adaptedOutputs() const override { return {}; }
    void addAdaptedOutput(IAdaptedOutput *) override {}
    [[nodiscard]] bool removeAdaptedOutput(IAdaptedOutput *) override { return false; }
    void updateValues(const IInput *) override {}
};

class RecessionArgument final : public virtual IArgument, public VectorItem
{
public:
    explicit RecessionArgument(IModelComponent *owner) : VectorItem(owner, "k", 0.3) {}
    [[nodiscard]] bool isOptional() const override { return true; }
    [[nodiscard]] bool isReadOnly() const override { return false; }
    [[nodiscard]] std::string toString() const override
    {
        std::ostringstream out;
        for (size_t i = 0; i < values.size(); ++i)
            out << (i ? "," : "") << values[i];
        return out.str();
    }
    void saveData() override {}
    [[nodiscard]] std::vector<std::string> fileFilters() const override { return {}; }
    [[nodiscard]] std::vector<const std::type_info *> validComponentDataItemTypes() const override { return {}; }
    [[nodiscard]] bool isValidArgType(ArgumentInputType argType) const override
    {
        return argType == ArgumentInputType::String;
    }
    [[nodiscard]] ArgumentInputType currentArgumentInputType() const override
    {
        return ArgumentInputType::String;
    }
    [[nodiscard]] bool initialize(const std::string &value, ArgumentInputType,
                                  std::string &message) override
    {
        std::istringstream in(value);
        std::string token;
        size_t i = 0;
        while (std::getline(in, token, ',') && i < values.size())
            values[i++] = std::stod(token);
        if (i != values.size())
        {
            message = "expected kCells comma-separated values";
            return false;
        }
        return true;
    }
    [[nodiscard]] bool initialize(const IComponentDataItem &, std::string &message) override
    {
        message = "not supported";
        return false;
    }
    [[nodiscard]] bool serialize(ArgumentInputType, std::string &value, std::string &) const override
    {
        value = toString();
        return true;
    }
};

class StorageState final : public VectorItem
{
public:
    explicit StorageState(IModelComponent *owner) : VectorItem(owner, "storage", 10.0) {}
};

//! Diagnostics: values[0] is the number of checkpoints saved and not yet
//! released -- how a test sees a leak (or a double release) through the
//! ordinary data plane.
class LiveCheckpoints final : public VectorItem
{
public:
    explicit LiveCheckpoints(IModelComponent *owner)
        : VectorItem(owner, "live_checkpoints", 0.0) {}
};

class ReservoirInfo;

// ---------------------------------------------------------------------------
// Component
// ---------------------------------------------------------------------------
class ReservoirComponent final : public virtual IDifferentiableModelComponent,
                                 public virtual ICheckpointableModelComponent,
                                 public SignalBase<IComponentStatusChangeEventArgs>
{
    std::string m_id = "cpp-reservoir";
    std::string m_caption = "C++ nonlinear reservoir";
    std::string m_description = "differentiable native fixture";
    std::string m_refDir = ".";
    ComponentStatus m_status = ComponentStatus::Created;
    IModelComponentInfo *m_info;
    int m_step = 0;

    std::unique_ptr<InflowInput> m_inflow;
    std::unique_ptr<OutflowOutput> m_outflow;
    std::unique_ptr<RecessionArgument> m_k;
    std::unique_ptr<StorageState> m_storage;
    std::unique_ptr<LiveCheckpoints> m_live;

    // Checkpoint bookkeeping: every token carries a serial number; a
    // released (or never issued) serial cannot be restored or released.
    long m_nextSerial = 1;
    std::set<long> m_liveSerials;
    void publishLive() { m_live->values[0] = static_cast<double>(m_liveSerials.size()); }

    // The linearization point: what the most recent update() consumed and
    // produced.
    bool m_haveStep = false;
    std::vector<double> m_sBefore, m_sAfter, m_inflowUsed, m_kUsed;

public:
    explicit ReservoirComponent(IModelComponentInfo *info) : m_info(info)
    {
        m_inflow = std::make_unique<InflowInput>(this);
        m_outflow = std::make_unique<OutflowOutput>(this);
        m_k = std::make_unique<RecessionArgument>(this);
        m_storage = std::make_unique<StorageState>(this);
        m_live = std::make_unique<LiveCheckpoints>(this);
    }

    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }

    [[nodiscard]] IModelComponentInfo *componentInfo() const override { return m_info; }
    [[nodiscard]] ComponentStatus status() const override { return m_status; }
    [[nodiscard]] std::vector<IArgument *> arguments() const override { return {m_k.get()}; }
    [[nodiscard]] std::vector<IInput *> inputs() const override { return {m_inflow.get()}; }
    [[nodiscard]] std::vector<IOutput *> outputs() const override { return {m_outflow.get()}; }
    [[nodiscard]] std::vector<IComponentDataItem *> results() const override
    {
        return {m_storage.get(), m_live.get()};
    }

    void initialize() override { m_status = ComponentStatus::Initialized; }
    [[nodiscard]] std::vector<std::string> validate() override
    {
        m_status = ComponentStatus::Valid;
        return {};
    }
    void prepare() override { m_status = ComponentStatus::Updated; }

    void update(const std::vector<IOutput *> & = {}) override
    {
        m_status = ComponentStatus::Updating;
        std::vector<double> &s = m_storage->values;
        const std::vector<double> &in = m_inflow->values;
        const std::vector<double> &k = m_k->values;
        m_sBefore = s;
        m_inflowUsed = in;
        m_kUsed = k;
        for (size_t i = 0; i < s.size(); ++i)
        {
            const double si = s[i];
            s[i] = si + kDt * (in[i] - k[i] * si - kC * si * si);
            m_outflow->values[i] = k[i] * s[i];
        }
        m_sAfter = s;
        m_haveStep = true;
        ++m_step;
        m_status = ComponentStatus::Updated;
    }

    void finish() override { m_status = ComponentStatus::Finished; }
    [[nodiscard]] const IWorkflowComponent *workflow() const override { return nullptr; }
    void setWorkflow(const IWorkflowComponent *) override {}
    [[nodiscard]] std::set<Capability> capabilities() const override
    {
        return {Capability::Checkpointing, Capability::Differentiable};
    }
    [[nodiscard]] std::vector<ErrorEntry> errors(bool) override { return {}; }
    [[nodiscard]] std::string referenceDirectory() const override { return m_refDir; }
    void setReferenceDirectory(const std::string &value) override { m_refDir = value; }

    // -- ICheckpointableModelComponent -----------------------------------------
    // The token is the state itself, serialized: serial, step, storage,
    // outflow. The serial exists so the test can see releases.
    [[nodiscard]] bool saveState(std::string &token, std::string &) override
    {
        const long serial = m_nextSerial++;
        m_liveSerials.insert(serial);
        publishLive();
        std::ostringstream out;
        out.precision(17);
        out << serial << ' ' << m_step;
        for (double v : m_storage->values)
            out << ' ' << v;
        for (double v : m_outflow->values)
            out << ' ' << v;
        token = out.str();
        return true;
    }

    [[nodiscard]] bool restoreState(const std::string &token, std::string &message) override
    {
        std::istringstream in(token);
        long serial = 0;
        if (!(in >> serial) || !m_liveSerials.count(serial))
        {
            message = "token was released or never issued";
            return false;
        }
        if (!(in >> m_step))
        {
            message = "bad token";
            return false;
        }
        for (double &v : m_storage->values)
            in >> v;
        for (double &v : m_outflow->values)
            in >> v;
        if (in.fail())
        {
            message = "bad token";
            return false;
        }
        // A restored component has not taken the step it will next be asked
        // to differentiate.
        m_haveStep = false;
        return true;
    }

    [[nodiscard]] bool releaseState(const std::string &token, std::string &message) override
    {
        std::istringstream in(token);
        long serial = 0;
        if (!(in >> serial) || m_liveSerials.erase(serial) != 1)
        {
            message = "token was already released or never issued";
            return false;
        }
        publishLive();
        return true;
    }

    // -- IDifferentiableModelComponent -----------------------------------------
    [[nodiscard]] std::vector<IInput *> differentiableInputs() const override { return {m_inflow.get()}; }
    [[nodiscard]] std::vector<IArgument *> differentiableArguments() const override { return {m_k.get()}; }
    [[nodiscard]] std::vector<IOutput *> differentiableOutputs() const override { return {m_outflow.get()}; }
    [[nodiscard]] std::vector<IComponentDataItem *> differentiableStates() const override
    {
        return {m_storage.get()};
    }

    [[nodiscard]] bool vjp(DifferentialSet seeds, DifferentialSet results,
                           std::string *message) override
    {
        if (!m_haveStep)
            return fail(message, "no step to differentiate: call update() first");

        std::vector<double> qb(kCells, 0.0), sab(kCells, 0.0);
        for (const DifferentialEntry &e : seeds)
        {
            if (!checkVector(e.value, message))
                return false;
            std::vector<double> *into = nullptr;
            if (isOutflow(e.item) && e.role == DifferentialRole::Output)
                into = &qb;
            else if (isStorage(e.item) && e.role == DifferentialRole::StateAfter)
                into = &sab;
            else
                return fail(message, "seed names an item or role this component does not differentiate");
            for (int64_t i = 0; i < kCells; ++i)
                (*into)[static_cast<size_t>(i)] = at(e.value, i);
        }

#ifdef HC_FALSIFY_DROP_STATE
        std::fill(sab.begin(), sab.end(), 0.0);
#endif
        const std::vector<double> &s =
#ifdef HC_FALSIFY_STALE_POINT
            m_sAfter;
#else
            m_sBefore;
#endif
        std::vector<double> total(kCells), ib(kCells), kb(kCells), sb(kCells);
        for (size_t i = 0; i < static_cast<size_t>(kCells); ++i)
        {
            const double k = m_kUsed[i];
            total[i] = sab[i] + k * qb[i];
            ib[i] = kDt * total[i];
#ifdef HC_FALSIFY_KB_SIGN
            kb[i] = -qb[i] * m_sAfter[i] - kDt * s[i] * total[i];
#else
            kb[i] = qb[i] * m_sAfter[i] - kDt * s[i] * total[i];
#endif
            sb[i] = (1.0 - kDt * k - 2.0 * kDt * kC * s[i]) * total[i];
        }

        for (const DifferentialEntry &e : results)
        {
            if (!checkVector(e.value, message))
                return false;
            const std::vector<double> *from = nullptr;
            if (isInflow(e.item) && e.role == DifferentialRole::Input)
                from = &ib;
            else if (isK(e.item) && e.role == DifferentialRole::Argument)
                from = &kb;
            else if (isStorage(e.item) && e.role == DifferentialRole::StateBefore)
                from = &sb;
            else
                return fail(message, "result names an item or role this component does not differentiate");
            for (int64_t i = 0; i < kCells; ++i)
            {
#ifdef HC_FALSIFY_ACCUMULATE
                at(e.value, i) += (*from)[static_cast<size_t>(i)];
#else
                at(e.value, i) = (*from)[static_cast<size_t>(i)];
#endif
            }
        }
        return true;
    }

    [[nodiscard]] bool jvp(DifferentialSet seeds, DifferentialSet results,
                           std::string *message) override
    {
        if (!m_haveStep)
            return fail(message, "no step to differentiate: call update() first");

        std::vector<double> di(kCells, 0.0), dk(kCells, 0.0), ds(kCells, 0.0);
        for (const DifferentialEntry &e : seeds)
        {
            if (!checkVector(e.value, message))
                return false;
            std::vector<double> *into = nullptr;
            if (isInflow(e.item) && e.role == DifferentialRole::Input)
                into = &di;
            else if (isK(e.item) && e.role == DifferentialRole::Argument)
                into = &dk;
            else if (isStorage(e.item) && e.role == DifferentialRole::StateBefore)
                into = &ds;
            else
                return fail(message, "seed names an item or role this component does not differentiate");
            for (int64_t i = 0; i < kCells; ++i)
                (*into)[static_cast<size_t>(i)] = at(e.value, i);
        }

        std::vector<double> dsa(kCells), dq(kCells);
        for (size_t i = 0; i < static_cast<size_t>(kCells); ++i)
        {
            const double k = m_kUsed[i];
            const double s = m_sBefore[i];
            dsa[i] = (1.0 - kDt * k - 2.0 * kDt * kC * s) * ds[i] + kDt * di[i] - kDt * s * dk[i];
#ifdef HC_FALSIFY_JVP_DK
            dq[i] = k * dsa[i];
#else
            dq[i] = dk[i] * m_sAfter[i] + k * dsa[i];
#endif
        }

        for (const DifferentialEntry &e : results)
        {
            if (!checkVector(e.value, message))
                return false;
            const std::vector<double> *from = nullptr;
            if (isOutflow(e.item) && e.role == DifferentialRole::Output)
                from = &dq;
            else if (isStorage(e.item) && e.role == DifferentialRole::StateAfter)
                from = &dsa;
            else
                return fail(message, "result names an item or role this component does not differentiate");
            for (int64_t i = 0; i < kCells; ++i)
                at(e.value, i) = (*from)[static_cast<size_t>(i)];
        }
        return true;
    }

private:
    static bool fail(std::string *message, const char *text)
    {
        if (message) *message = text;
        return false;
    }
    bool isInflow(const IComponentDataItem *p) const
    {
        return p == static_cast<const IComponentDataItem *>(m_inflow.get());
    }
    bool isOutflow(const IComponentDataItem *p) const
    {
        return p == static_cast<const IComponentDataItem *>(m_outflow.get());
    }
    bool isK(const IComponentDataItem *p) const
    {
        return p == static_cast<const IComponentDataItem *>(m_k.get());
    }
    bool isStorage(const IComponentDataItem *p) const
    {
        return p == static_cast<const IComponentDataItem *>(m_storage.get());
    }
};

class ReservoirInfo final : public virtual IModelComponentInfo,
                            public SignalBase<IComponentStatusChangeEventArgs>
{
    std::string m_id = "cpp-reservoir-info";
    std::string m_caption = "C++ nonlinear reservoir";
    std::string m_description = "differentiable native fixture";
    std::string m_libraryPath;

public:
    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }
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
    [[nodiscard]] std::unique_ptr<IModelComponent> createComponentInstance() override
    {
        return std::make_unique<ReservoirComponent>(this);
    }
    [[nodiscard]] std::vector<IAdaptedOutputFactory *> adaptedOutputFactories() const override
    {
        return {};
    }
};

} // namespace

extern "C" IModelComponentInfo *CreateComponentInfo()
{
    static ReservoirInfo info;
    return &info;
}
