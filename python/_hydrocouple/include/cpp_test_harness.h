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

} // namespace Testing
} // namespace Python
} // namespace HydroCouple
