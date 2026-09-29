/*
 * cpp_test_harness.h
 *
 * Pure C++ code that consumes a HydroCouple::IModelComponent* exactly the
 * way a C++ workflow would: through the virtual interface, with no
 * knowledge of whether the component is native C++ or a bridged Python
 * object. Used by the _hydrocouple._testing extension to prove the
 * Python -> C++ direction of the bindings end-to-end.
 */
#pragma once

#include "hydrocouple.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace HydroCouple {
namespace Python {
namespace Testing {

inline const char *statusName(IModelComponent::ComponentStatus status)
{
    using CS = IModelComponent::ComponentStatus;
    switch (status)
    {
        case CS::Created:        return "Created";
        case CS::Initializing:   return "Initializing";
        case CS::Initialized:    return "Initialized";
        case CS::Validating:     return "Validating";
        case CS::Valid:          return "Valid";
        case CS::WaitingForData: return "WaitingForData";
        case CS::Invalid:        return "Invalid";
        case CS::Preparing:      return "Preparing";
        case CS::Updating:       return "Updating";
        case CS::Updated:        return "Updated";
        case CS::Checkpointing:  return "Checkpointing";
        case CS::Done:           return "Done";
        case CS::Finishing:      return "Finishing";
        case CS::Finished:       return "Finished";
        case CS::Failed:         return "Failed";
    }
    return "?";
}

/// Drive the full component lifecycle through the C++ virtual interface
/// and return a trace of what C++ observed.
inline std::string driveLifecycle(IModelComponent *component)
{
    std::ostringstream trace;
    trace << "id=" << component->id();
    trace << ";caption=" << component->caption();
    trace << ";start=" << statusName(component->status());

    component->initialize();
    trace << ";after_init=" << statusName(component->status());

    std::vector<std::string> messages = component->validate();
    trace << ";validate_msgs=" << messages.size();

    component->prepare();
    trace << ";after_prepare=" << statusName(component->status());

    int updates = 0;
    while (component->status() != IModelComponent::ComponentStatus::Done &&
           component->status() != IModelComponent::ComponentStatus::Failed &&
           updates < 64)
    {
        component->update();
        ++updates;
    }
    trace << ";updates=" << updates;
    trace << ";after_updates=" << statusName(component->status());

    component->finish();
    trace << ";final=" << statusName(component->status());
    return trace.str();
}

/// The capability values C++ observes, sorted ascending.
inline std::vector<int32_t> queryCapabilities(IModelComponent *component)
{
    std::vector<int32_t> values;
    for (Capability capability : component->capabilities())
        values.push_back(static_cast<int32_t>(capability));
    return values;
}

/// Drain the component's error queue as (severity, code, source, message)
/// rows flattened into strings "severity|code|source|message".
inline std::vector<std::string> drainErrors(IModelComponent *component)
{
    std::vector<std::string> rows;
    for (const ErrorEntry &entry : component->errors(true))
    {
        std::ostringstream row;
        row << static_cast<int>(entry.severity) << '|' << entry.code << '|'
            << entry.source << '|' << entry.message;
        rows.push_back(row.str());
    }
    return rows;
}

/// Read a Float64 hyperslab from the component's result item through the
/// C++ data plane: the BufferDescriptor is constructed in C++ over a
/// C++-owned buffer, exactly as an engine would do it.
inline bool readResultSlab(IModelComponent *component,
                           int resultIndex,
                           const int64_t *start,
                           const int64_t *count,
                           int32_t rank,
                           double *out,
                           std::string &message)
{
    std::vector<IComponentDataItem *> results = component->results();
    if (resultIndex < 0 ||
        static_cast<size_t>(resultIndex) >= results.size())
    {
        message = "result index out of range";
        return false;
    }
    IComponentDataItem *item = results[static_cast<size_t>(resultIndex)];
    if (item->dataKind() != DataKind::Float64)
    {
        message = "harness only reads Float64 items";
        return false;
    }

    BufferDescriptor destination;
    destination.data = out;
    destination.kind = DataKind::Float64;
    destination.rank = rank;
    destination.shape = count;      // contiguous destination shaped as count
    destination.stridesBytes = nullptr;
    destination.space = MemorySpace::Host;

    return item->getValuesInto(
        destination,
        std::span<const int64_t>(start, static_cast<size_t>(rank)),
        std::span<const int64_t>(count, static_cast<size_t>(rank)),
        &message);
}

/// Write a Float64 hyperslab into the component's result item through the
/// C++ data plane.
inline bool writeResultSlab(IModelComponent *component,
                            int resultIndex,
                            const int64_t *start,
                            const int64_t *count,
                            int32_t rank,
                            const double *values,
                            std::string &message)
{
    std::vector<IComponentDataItem *> results = component->results();
    if (resultIndex < 0 ||
        static_cast<size_t>(resultIndex) >= results.size())
    {
        message = "result index out of range";
        return false;
    }
    IComponentDataItem *item = results[static_cast<size_t>(resultIndex)];

    BufferDescriptor source;
    source.data = const_cast<double *>(values);
    source.kind = DataKind::Float64;
    source.rank = rank;
    source.shape = count;
    source.stridesBytes = nullptr;
    source.space = MemorySpace::Host;

    return item->setValuesFrom(
        source,
        std::span<const int64_t>(start, static_cast<size_t>(rank)),
        std::span<const int64_t>(count, static_cast<size_t>(rank)),
        &message);
}

// ---------------------------------------------------------------------------
// Phase G0: a native data item that records the descriptor it was handed.
// ---------------------------------------------------------------------------

/// What a ProbeDataItem saw on its last data-plane call.
struct ProbeRecord
{
    uintptr_t address = 0;
    int kind = 0;
    int32_t rank = -1;
    std::vector<int64_t> shape;
    std::vector<int64_t> stridesBytes;   // empty: the descriptor's were null
    bool stridesNull = true;
    int space = -1;
    int32_t deviceId = -1;
    int calls = 0;
};

/// A native C++ Float64 item of shape (3, 4) holding 10*i + j, host only.
/// Every data-plane call records the BufferDescriptor it received -- the
/// address above all, which is how a test proves the binding lent the
/// caller's own memory rather than a staging copy. A non-Host descriptor is
/// recorded and then refused with a message, as the interface specifies for
/// a host-only item, and its memory is never touched.
class ProbeDataItem final : public virtual IComponentDataItem
{
public:
    static constexpr int64_t Rows = 3;
    static constexpr int64_t Cols = 4;

    ProbeDataItem()
    {
        for (int64_t i = 0; i < Rows; ++i)
            for (int64_t j = 0; j < Cols; ++j)
                m_values[static_cast<size_t>(i * Cols + j)] =
                    static_cast<double>(10 * i + j);
    }

    ProbeRecord last;

    double value(int64_t i, int64_t j) const
    {
        return m_values[static_cast<size_t>(i * Cols + j)];
    }

    // -- signals (unused) ---------------------------------------------------
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    void connect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void blockSignals(bool) override {}

protected:
    void emit(const std::shared_ptr<IComponentDataItemValueChanged> &) override {}
    void emit(std::string) override {}

public:
    // -- identity -------------------------------------------------------------
    [[nodiscard]] const std::string &caption() const override { return m_caption; }
    void setCaption(const std::string &value) override { m_caption = value; }
    [[nodiscard]] const std::string &description() const override { return m_description; }
    void setDescription(const std::string &value) override { m_description = value; }
    [[nodiscard]] const std::string &id() const override { return m_id; }

    // -- data item ------------------------------------------------------------
    [[nodiscard]] IModelComponent *modelComponent() const override { return nullptr; }
    [[nodiscard]] std::vector<IDimension *> dimensions() const override { return {}; }
    [[nodiscard]] std::vector<int64_t> shape() const override { return {Rows, Cols}; }
    [[nodiscard]] DataKind dataKind() const override { return DataKind::Float64; }
    [[nodiscard]] IValueDefinition *valueDefinition() const override { return nullptr; }

    [[nodiscard]] bool getValuesInto(const BufferDescriptor &destination,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message = nullptr) const override
    {
        auto *self = const_cast<ProbeDataItem *>(this);
        self->record(destination);
        return self->transfer(destination, start, count, /*read=*/true, message);
    }

    [[nodiscard]] bool setValuesFrom(const BufferDescriptor &source,
                                     std::span<const int64_t> start,
                                     std::span<const int64_t> count,
                                     std::string *message = nullptr) override
    {
        record(source);
        return transfer(source, start, count, /*read=*/false, message);
    }

private:
    std::string m_caption = "Probe";
    std::string m_description = "Records the BufferDescriptor it receives";
    std::string m_id = "probe";
    double m_values[Rows * Cols] = {};

    void record(const BufferDescriptor &d)
    {
        last.address = reinterpret_cast<uintptr_t>(d.data);
        last.kind = static_cast<int>(d.kind);
        last.rank = d.rank;
        last.shape.assign(d.shape, d.shape + (d.shape ? d.rank : 0));
        last.stridesNull = d.stridesBytes == nullptr;
        last.stridesBytes.assign(d.stridesBytes,
                                 d.stridesBytes + (d.stridesBytes ? d.rank : 0));
        last.space = static_cast<int>(d.space);
        last.deviceId = d.deviceId;
        ++last.calls;
    }

    static bool fail(std::string *message, const char *text)
    {
        if (message)
            *message = text;
        return false;
    }

    bool transfer(const BufferDescriptor &d, std::span<const int64_t> start,
                  std::span<const int64_t> count, bool read,
                  std::string *message)
    {
        if (d.space != MemorySpace::Host)
            return fail(message, "probe is host-only");
        if (d.kind != DataKind::Float64)
            return fail(message, "kind mismatch");
        if (start.size() != 2 || count.size() != 2)
            return fail(message, "rank mismatch");
        if (start[0] < 0 || start[1] < 0 || count[0] < 0 || count[1] < 0 ||
            start[0] + count[0] > Rows || start[1] + count[1] > Cols)
            return fail(message, "selection out of bounds");
        int64_t elements = 1;
        for (int32_t k = 0; k < d.rank; ++k)
            elements *= d.shape[k];
        if (elements != count[0] * count[1])
            return fail(message, "element count mismatch");

        // The buffer is walked as a (count0, count1) box when it is rank 2,
        // else as a flat run; its own strides are honoured either way.
        auto *base = static_cast<char *>(d.data);
        for (int64_t i = 0; i < count[0]; ++i)
        {
            for (int64_t j = 0; j < count[1]; ++j)
            {
                int64_t offset = 0;
                if (d.rank == 2)
                {
                    const int64_t s0 = d.stridesBytes ? d.stridesBytes[0]
                                                      : d.shape[1] * 8;
                    const int64_t s1 = d.stridesBytes ? d.stridesBytes[1] : 8;
                    offset = i * s0 + j * s1;
                }
                else
                {
                    const int64_t flat = i * count[1] + j;
                    const int64_t s = (d.rank == 1 && d.stridesBytes)
                                          ? d.stridesBytes[0] : 8;
                    offset = flat * s;
                }
                double *cell = reinterpret_cast<double *>(base + offset);
                double &mine = m_values[static_cast<size_t>(
                    (start[0] + i) * Cols + (start[1] + j))];
                if (read)
                    *cell = mine;
                else
                    mine = *cell;
            }
        }
        if (message)
            message->clear();
        return true;
    }
};

/// Call getValuesInto (read) or setValuesFrom on a component's result item
/// with a descriptor built from raw fields -- any space, any device id, any
/// strides -- exactly as an engine holding device memory would.
inline bool callResultWithDescriptor(IModelComponent *component,
                                     int resultIndex, bool read,
                                     uintptr_t address, int kind,
                                     const std::vector<int64_t> &shape,
                                     const std::vector<int64_t> &stridesBytes,
                                     bool stridesNull, int space,
                                     int32_t deviceId,
                                     const std::vector<int64_t> &start,
                                     const std::vector<int64_t> &count,
                                     std::string &message)
{
    std::vector<IComponentDataItem *> results = component->results();
    if (resultIndex < 0 || static_cast<size_t>(resultIndex) >= results.size())
    {
        message = "result index out of range";
        return false;
    }
    BufferDescriptor d;
    d.data = reinterpret_cast<void *>(address);
    d.kind = static_cast<DataKind>(kind);
    d.rank = static_cast<int32_t>(shape.size());
    d.shape = shape.empty() ? nullptr : shape.data();
    d.stridesBytes = stridesNull ? nullptr : stridesBytes.data();
    d.space = static_cast<MemorySpace>(space);
    d.deviceId = deviceId;
    IComponentDataItem *item = results[static_cast<size_t>(resultIndex)];
    std::span<const int64_t> s(start.data(), start.size());
    std::span<const int64_t> c(count.data(), count.size());
    return read ? item->getValuesInto(d, s, c, &message)
                : item->setValuesFrom(d, s, c, &message);
}

} // namespace Testing
} // namespace Python
} // namespace HydroCouple
