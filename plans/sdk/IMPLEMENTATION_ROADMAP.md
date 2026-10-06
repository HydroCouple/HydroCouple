# HydroCouple Implementation Roadmap: Four Focus Areas

## Overview

This roadmap addresses the four highest-impact improvements to HydroCouple:
1. **Type Introspection** (Quick Win) - Reduce variant brittleness
2. **Mixin Composition** (Refactor Path) - Simplify inheritance hierarchy
3. **Workflow Orchestration** (New Feature) - Add composition layer
4. **Error Handling** (Polish) - Persistent error queues

**Timeline**: ~12-16 weeks across 4 tracks (can run in parallel with dependencies)

## Important: Qt GUI Integration

**Context**: These improvements will be wrapped with **QProperty** wrappers for GUI frontend use.

**Implications for Design**:
- Type introspection must be observable via Qt signals (property change notifications)
- Error queues must emit Qt signals when errors are added
- Data items must be bindable to QML properties
- Storage backends should support Qt's mutable/read-only property patterns
- Workflow orchestrator must emit Qt signals for cycle completion and status changes

**Qt Integration Layer** (separate from core, added after each focus area stabilizes):
- `qml/properties/TypeProperty.qml` - Binds `DataType` to UI
- `qml/properties/ErrorQueueBinding.h` - Bridges `ErrorQueue` to Qt signals
- `qml/properties/DataItemProperty.h` - Makes `IComponentDataItem` bindable to QML
- `qml/workflows/WorkflowVisualizer.qml` - Displays dependency graph and status

See **Qt Integration Appendix** (Section A) for detailed QProperty bindings.

---

# Focus Area 1: Reduce Variants with Type Introspection (Quick Win)

## Current Problem

```cpp
// Current: No way to safely query variant type
hydrocouple_variant data = 42.0;  // double
try {
    int val = std::get<int>(data);  // Throws std::bad_variant_access
} catch (...) {
    // How to recover? Which type was it?
}

// Adapters use trial-and-error
void UniversalAdapter::adapt(const hydrocouple_variant& src, 
                              hydrocouple_variant& dst) {
    try { dst = std::get<int>(src); return; } catch(...) {}
    try { dst = std::get<double>(src); return; } catch(...) {}
    try { dst = std::get<std::string>(src); return; } catch(...) {}
    // Falls through without converting
}
```

## Design: Safe Type Introspection

### 1.1 Add Type Information to IComponentDataItem

**File**: `include/hydrocouple/abstractcomponentdataitem.h`

```cpp
// NEW: Type information carrier
class HYDROCOUPLESDK_EXPORT DataType
{
public:
    enum class BaseType {
        Bool,
        Char,
        Short,
        Int,
        Long,
        UChar,
        UShort,
        UInt,
        ULong,
        Float,
        Double,
        LongDouble,
        String,
        VoidPtr,
        Unknown
    };

    explicit DataType(BaseType base);
    
    [[nodiscard]] BaseType baseType() const;
    [[nodiscard]] bool isNumeric() const;
    [[nodiscard]] bool isFloating() const;
    [[nodiscard]] bool isInteger() const;
    [[nodiscard]] bool isString() const;
    [[nodiscard]] const std::type_info& typeInfo() const;
    [[nodiscard]] std::string name() const;
    
    // Type matching
    [[nodiscard]] bool canConvertTo(const DataType& other) const;
    
    static DataType fromVariant(const hydrocouple_variant& v);
    static DataType fromTypeInfo(const std::type_info& info);
};

// IN IComponentDataItem interface
class IComponentDataItem : public virtual IIdentity
{
    // ... existing methods ...
    
    // NEW: Type introspection
    virtual DataType valueType() const = 0;
};
```

### 1.2 Implement Type Detection

**File**: `src/hydrocouple/abstractcomponentdataitem.cpp`

```cpp
DataType::DataType(BaseType base) : m_baseType(base) {}

DataType DataType::fromVariant(const hydrocouple_variant& v)
{
    if (std::holds_alternative<bool>(v)) return DataType(BaseType::Bool);
    if (std::holds_alternative<int>(v)) return DataType(BaseType::Int);
    if (std::holds_alternative<double>(v)) return DataType(BaseType::Double);
    if (std::holds_alternative<std::string>(v)) return DataType(BaseType::String);
    // ... etc for all types
    return DataType(BaseType::Unknown);
}

bool DataType::canConvertTo(const DataType& other) const
{
    // Numeric types can convert to each other
    if (isNumeric() && other.isNumeric()) return true;
    // String can convert from anything
    if (other.isString()) return true;
    // Void ptr is special case
    if (m_baseType == BaseType::VoidPtr || other.m_baseType == BaseType::VoidPtr) 
        return true;
    return false;
}

const std::type_info& DataType::typeInfo() const
{
    switch (m_baseType) {
        case BaseType::Bool: return typeid(bool);
        case BaseType::Int: return typeid(int);
        case BaseType::Double: return typeid(double);
        case BaseType::String: return typeid(std::string);
        // ... etc
        default: return typeid(void);
    }
}
```

### 1.3 Safe Accessor Functions

**File**: `include/hydrocouple/abstractcomponentdataitem.h` (in public helper section)

```cpp
namespace HydroCouple {

// Safe variant accessors - never throw
template<typename T>
std::optional<T> getSafeValue(const hydrocouple_variant& v)
{
    if (std::holds_alternative<T>(v)) {
        return std::get<T>(v);
    }
    return std::nullopt;
}

// Type-aware conversion
class HYDROCOUPLESDK_EXPORT VariantConverter
{
public:
    // Convert between compatible types
    static std::optional<hydrocouple_variant> convert(
        const hydrocouple_variant& src, 
        DataType targetType,
        bool strict = false  // If true, only identical types
    );
    
    // Convert to common type (negotiated between two variants)
    static std::optional<DataType> commonType(
        const hydrocouple_variant& a,
        const hydrocouple_variant& b
    );
    
    // Debug: print variant contents
    static std::string toString(const hydrocouple_variant& v);
};

}  // namespace HydroCouple
```

### 1.4 Adapter Usage Example

**File**: `src/hydrocouple/timeseriesadaptedoutput.cpp` (example usage)

```cpp
void TimeSeriesAdaptedOutput::refresh()
{
    // OLD APPROACH: Exception-based type handling
    // try { 
    //     dst = std::get<double>(src); 
    // } catch (...) { 
    //     // Fallback
    // }
    
    // NEW APPROACH: Type-safe introspection
    const IComponentDataItem* adaptee = m_adaptee->modelComponent()->results()[0];
    DataType adapteeType = adaptee->valueType();
    
    if (adapteeType.canConvertTo(m_valueType)) {
        // Safe to proceed with conversion
        hydrocouple_variant src, dst;
        adaptee->getValue(src, {0});
        
        auto converted = VariantConverter::convert(src, m_valueType);
        if (converted) {
            this->setValue(converted.value(), {0});
        } else {
            emitMessage("Failed to convert " + adapteeType.name() + 
                       " to " + m_valueType.name());
        }
    } else {
        emitMessage("Incompatible types: " + adapteeType.name() + 
                   " cannot convert to " + m_valueType.name());
    }
}
```

## Implementation Plan: Focus Area 1

### Phase 1A: Type Introspection Infrastructure (Week 1-2)

**Tasks:**
- [ ] Create `include/hydrocouple/datatype.h` with `DataType` enum and class
- [ ] Implement `DataType::fromVariant()` and type matching logic
- [ ] Add `valueType()` abstract method to `IComponentDataItem`
- [ ] Create unit tests: `tests/test_datatype.cpp`
  - Test type detection from variants
  - Test canConvertTo() logic
  - Test numeric/string conversion paths

**Deliverable**: Type introspection core; no existing code changes

**Estimate**: 10 developer-hours

### Phase 1B: Implement Converters (Week 2)

**Tasks:**
- [ ] Implement `VariantConverter::convert()` for all type combinations
- [ ] Implement `VariantConverter::commonType()`
- [ ] Implement `VariantConverter::toString()` for debugging
- [ ] Create comprehensive conversion tests: `tests/test_variant_converter.cpp`

**Deliverable**: Production-ready type conversion utilities

**Estimate**: 8 developer-hours

### Phase 1C: Migrate High-Risk Code (Week 3)

**Tasks:**
- [ ] Audit adapter implementations for try/catch variant handling
- [ ] Migrate adapters to use `getSafeValue()` and `VariantConverter`
- [ ] Migrate IdBasedComponentDataItem to use type-safe access
- [ ] Add integration tests: `tests/test_adapter_type_safety.cpp`

**High-risk adapters to migrate:**
- `TimeSeriesAdaptedOutput`
- Any adapters doing numeric conversions (unit conversion, interpolation)
- ID-based argument loaders

**Estimate**: 12 developer-hours

### Phase 1D: Documentation & Deprecation (Week 3)

**Tasks:**
- [ ] Update DEVELOPER.md with type introspection pattern
- [ ] Add migration guide for adapters (old try/catch → new type-safe)
- [ ] Mark direct `std::get<>()` calls with `[[deprecated]]` in type traits
- [ ] Create examples: `examples/safe_adapter_pattern.cpp`

**Deliverable**: Complete type introspection story

**Estimate**: 5 developer-hours

## Metrics & Success Criteria

- ✅ Zero unhandled variant exceptions in adapters (verified by code audit)
- ✅ All existing tests pass with new type system
- ✅ New test coverage: 95%+ of `DataType` and `VariantConverter` methods
- ✅ Adapter code is 30%+ more readable (subjective; measured by LOC reduction)

---

# Focus Area 2: Simplify Mixins via Composition Instead of Inheritance

## Current Problem

**Explosion of concrete types** (inherited multiple classes):

```
AbstractArgument + IdBasedComponentDataItem<T> → Argument1D<T>
AbstractArgument + IdBasedComponentDataItem<T> + IIdBasedComponentDataItem → IdBasedArgumentInt
AbstractOutput + IdBasedComponentDataItem<T> → OutputId1D<T>
AbstractOutput + TimeSeriesComponentDataItem<T> → TimeSeriesOutput<T>
...
(15+ combinations)
```

**Code smell:**
- `template<typename T> class Argument1D : public AbstractArgument, public IdBasedComponentDataItem<T>`
- Virtual diamond inheritance issues
- Adding new storage type requires new concrete class
- Maintenance burden: storage logic duplicated across classes

## Design: Composable Storage Backends

### 2.1 Storage Backend Abstraction

**File**: `include/hydrocouple/storagebackend.h` (NEW)

```cpp
namespace HydroCouple {

// Base interface for data storage backends
class HYDROCOUPLESDK_EXPORT IStorageBackend
{
public:
    virtual ~IStorageBackend() = default;
    
    // Dimension queries
    virtual int length(int dimensionIndex = 0) const = 0;
    virtual std::vector<int> dimensions() const = 0;
    
    // Typed get/set (uses variant for type erasure)
    virtual void getValue(hydrocouple_variant& data, 
                          const std::initializer_list<int>& indexes) const = 0;
    virtual void getValues(hydrocouple_variant* data,
                           const std::initializer_list<int>& indexes,
                           const std::initializer_list<int>& lengths = {}) const = 0;
    virtual void setValue(const hydrocouple_variant& data,
                          const std::initializer_list<int>& indexes) = 0;
    virtual void setValues(const hydrocouple_variant* data,
                           const std::initializer_list<int>& indexes,
                           const std::initializer_list<int>& lengths = {}) = 0;
};

// ID-based storage backend: m_identifiers + m_values
template<typename T>
class HYDROCOUPLESDK_EXPORT IdBasedStorageBackend : public IStorageBackend
{
public:
    explicit IdBasedStorageBackend(const std::vector<std::string>& ids = {}, 
                                    const T& defaultValue = T{});
    
    // IStorageBackend
    int length(int dimensionIndex = 0) const override;
    void getValue(hydrocouple_variant& data, const std::initializer_list<int>& indexes) const override;
    // ... etc
    
    // Direct typed access
    const T& valueAt(const std::string& id) const;
    T& valueAt(const std::string& id);
    [[nodiscard]] std::vector<std::string> identifiers() const;
    void setIdentifiers(const std::vector<std::string>& ids);
    
private:
    std::vector<std::string> m_identifiers;
    std::vector<T> m_values;
    std::unordered_map<std::string, int> m_idIndex;
    T m_defaultValue;
};

// 1D array backend: flat vector<T>
template<typename T>
class HYDROCOUPLESDK_EXPORT Array1DStorageBackend : public IStorageBackend
{
public:
    explicit Array1DStorageBackend(int size = 0, const T& defaultValue = T{});
    
    int length(int dimensionIndex = 0) const override;
    void getValue(hydrocouple_variant& data, const std::initializer_list<int>& indexes) const override;
    // ... etc
    
    [[nodiscard]] const std::vector<T>& data() const;
    std::vector<T>& data();
    void resize(int newSize, const T& defaultValue = T{});
    
private:
    std::vector<T> m_values;
};

// 2D grid backend: vector<vector<T>>
template<typename T>
class HYDROCOUPLESDK_EXPORT Array2DStorageBackend : public IStorageBackend
{
public:
    explicit Array2DStorageBackend(int rows = 0, int cols = 0, 
                                    const T& defaultValue = T{});
    
    int length(int dimensionIndex = 0) const override;
    void getValue(hydrocouple_variant& data, const std::initializer_list<int>& indexes) const override;
    // ... etc
    
    T& at(int row, int col);
    const T& at(int row, int col) const;
    void resize(int newRows, int newCols, const T& defaultValue = T{});
    
private:
    std::vector<std::vector<T>> m_values;
};

// Raster/grid backend: contiguous memory with strides
template<typename T>
class HYDROCOUPLESDK_EXPORT RasterStorageBackend : public IStorageBackend
{
public:
    RasterStorageBackend(int width, int height, const T& noDataValue = T{});
    
    int length(int dimensionIndex = 0) const override;
    // ... etc
    
    T& at(int x, int y);
    const T& at(int x, int y) const;
    
private:
    int m_width, m_height;
    std::vector<T> m_values;  // Row-major: row * width + col
    T m_noDataValue;
};

}  // namespace HydroCouple
```

### 2.2 Refactored AbstractComponentDataItem

**File**: `include/hydrocouple/abstractcomponentdataitem.h` (MODIFIED)

```cpp
class HYDROCOUPLESDK_EXPORT AbstractComponentDataItem
    : public Identity,
      public virtual HydroCouple::IComponentDataItem
{
public:
    // Constructor: provide custom backend or use default
    AbstractComponentDataItem(std::string_view id,
                              const std::vector<Dimension*>& dimensions,
                              ValueDefinition* valueDefinition,
                              HydroCouple::IModelComponent* modelComponent,
                              std::unique_ptr<IStorageBackend> backend = nullptr);
    
    // ... existing methods ...
    
    // Storage backend access
    [[nodiscard]] IStorageBackend* storageBackend();
    [[nodiscard]] const IStorageBackend* storageBackend() const;
    void setStorageBackend(std::unique_ptr<IStorageBackend> backend);

protected:
    // Convenience: create typed backends
    template<typename T>
    void useIdBasedStorage(const std::vector<std::string>& ids, 
                           const T& defaultValue = T{})
    {
        m_backend = std::make_unique<IdBasedStorageBackend<T>>(ids, defaultValue);
    }
    
    template<typename T>
    void use1DStorage(int size, const T& defaultValue = T{})
    {
        m_backend = std::make_unique<Array1DStorageBackend<T>>(size, defaultValue);
    }

private:
    std::unique_ptr<IStorageBackend> m_backend;
};
```

### 2.3 Simplified Concrete Types (Argument Example)

**File**: `include/hydrocouple/argument.h` (REPLACEMENT for argument1d.h, argument2d.h)

```cpp
namespace HydroCouple {

// Generic Argument that composes a storage backend
template<typename T>
class HYDROCOUPLESDK_EXPORT Argument : public AbstractArgument
{
public:
    // Constructor: specify storage type
    Argument(std::string_view id,
             const std::vector<Dimension*>& dims,
             ValueDefinition* valueDef,
             IModelComponent* component,
             std::unique_ptr<IStorageBackend> storage);
    
    // Convenience constructors for common cases
    static std::unique_ptr<Argument<T>> createIdBased(
        std::string_view id,
        const std::vector<std::string>& identifiers,
        Dimension* idDimension,
        ValueDefinition* valueDef,
        IModelComponent* component);
    
    static std::unique_ptr<Argument<T>> create1D(
        std::string_view id,
        int size,
        Dimension* sizeDimension,
        ValueDefinition* valueDef,
        IModelComponent* component);
    
    static std::unique_ptr<Argument<T>> create2D(
        std::string_view id,
        int rows, int cols,
        Dimension* rowDim, Dimension* colDim,
        ValueDefinition* valueDef,
        IModelComponent* component);
    
    // IArgument/IComponentDataItem
    std::string toString() const override;
    bool initialize(const std::string& value,
                   ArgumentInputType argType,
                   std::string& message) override;
    
    // Typed access (convenience)
    T& valueAt(int index);
    const T& valueAt(int index) const;
};

}  // namespace HydroCouple
```

### 2.4 Usage Example (Before/After)

**OLD APPROACH** (template soup):
```cpp
// Before: Must choose the "right" concrete class
#include "argument1d.h"
Argument1D<double> temperature("temp", 
                               {&timeDim}, 
                               &tempValue, 
                               component);
temperature.setIdentifiers({"A", "B", "C"});  // Oops, can't do this in 1D
```

**NEW APPROACH** (composition):
```cpp
// After: Create any combination
auto temperature = Argument<double>::createIdBased(
    "temp",
    {"A", "B", "C"},           // IDs
    &sensorDim,                // ID dimension
    &tempValue,
    component);

// Or 1D:
auto elevation = Argument<double>::create1D(
    "elevation",
    256,
    &indexDim,
    &elevValue,
    component);

// Or 2D grid:
auto precipitation = Argument<double>::create2D(
    "precip",
    100, 100,                  // rows, cols
    &rowDim, &colDim,
    &precipValue,
    component);
```

## Implementation Plan: Focus Area 2

### Phase 2A: Storage Backend Architecture (Week 3-4)

**Tasks:**
- [ ] Create `include/hydrocouple/storagebackend.h` with interface and templates
- [ ] Implement `IdBasedStorageBackend<T>`
- [ ] Implement `Array1DStorageBackend<T>`
- [ ] Implement `Array2DStorageBackend<T>`
- [ ] Create unit tests: `tests/test_storage_backends.cpp`
  - Test get/set on each backend type
  - Test dimension queries
  - Test edge cases (out of bounds, empty)

**Deliverable**: Storage abstraction layer, no integration yet

**Estimate**: 16 developer-hours

### Phase 2B: Integrate with AbstractComponentDataItem (Week 4-5)

**Tasks:**
- [ ] Modify `AbstractComponentDataItem` to use `std::unique_ptr<IStorageBackend>`
- [ ] Implement `AbstractComponentDataItem::getValue/setValue` dispatching to backend
- [ ] Add convenience template methods (`useIdBasedStorage<T>()`, `use1DStorage<T>()`)
- [ ] Create integration tests: `tests/test_composite_data_item.cpp`
- [ ] Verify all existing tests still pass

**Deliverable**: Refactored core data item class

**Estimate**: 12 developer-hours

### Phase 2C: Create Unified Concrete Classes (Week 5-6)

**Tasks:**
- [ ] Create new `Argument<T>` template with factory methods
- [ ] Create new `Input<T>` template with factory methods
- [ ] Create new `Output<T>` template with factory methods
- [ ] Create migration guide: OLD class → NEW factory method
- [ ] Create compatibility layer: Old classes as type aliases (deprecation path)

**Factory method examples:**
```cpp
// From old: Argument1D<double> arg(...)
// To new: auto arg = Argument<double>::create1D(...)

// From old: TimeSeriesExchangeItem1D<double> ts(...)
// To new: auto ts = Output<double>::create1DTimeSeries(...)
```

**Estimate**: 20 developer-hours

### Phase 2D: Gradual Migration (Week 6-8)

**Tasks:**
- [ ] Identify and migrate internal SDK uses (arguments, exchange items, results)
- [ ] Update examples/ to use new unified classes
- [ ] Migrate test suite to new API
- [ ] Create deprecation warnings for old concrete classes
- [ ] Update documentation: class hierarchy diagrams, usage patterns

**High-priority files to migrate:**
- `src/hydrocouple/argument1d.cpp` → replace with single template
- `src/hydrocouple/exchangeitems1d.cpp` → replace with single template
- `src/hydrocouple/timeseriesexchangeitems.cpp` → factory method on Output

**Estimate**: 24 developer-hours

### Phase 2E: Binary Compatibility (Week 8)

**Tasks:**
- [ ] Define ABI-stable interfaces (abstract classes only)
- [ ] Isolate template implementations to headers
- [ ] Add version compatibility layer in CMakeLists
- [ ] Test: Old binaries link with new SDK? (No; template changes break ABI)
- [ ] Document: Breaking change, requires recompile

**Deliverable**: Clear migration path for library users

**Estimate**: 8 developer-hours

## Metrics & Success Criteria

- ✅ Reduce concrete exchange item classes from 15 to 5 (1D, 2D, Raster, ID-based, TimeSeries)
- ✅ Zero code duplication in storage implementations (verified by code audit)
- ✅ 100% of existing tests pass with new architecture
- ✅ New test coverage: 90%+ of storage backends
- ✅ Reduce SDK binary size by 10-15% (fewer template instantiations)
- ✅ Compilation time: neutral or faster (simpler hierarchy)

---

# Focus Area 3: Add Workflow Layer for Realistic Multi-Component Scenarios

## Current Problem

**No orchestration primitives:**
```cpp
// Current: User manually orchestrates (fragile, error-prone)
comp1->prepare();
comp2->prepare();

comp1->update();
comp2->update();  // What if comp1 changes output while comp2 reads? Race condition.

// No way to:
// - Detect component dependencies
// - Schedule updates in topological order
// - Detect circular dependencies
// - Handle data races or deadlocks
```

## Design: Workflow Orchestration Layer

### 3.1 Component Dependency Graph

**File**: `include/hydrocouple/workflow.h` (NEW)

```cpp
namespace HydroCouple {

// Represents component dependencies
class HYDROCOUPLESDK_EXPORT WorkflowGraph
{
public:
    explicit WorkflowGraph(const std::vector<IModelComponent*>& components);
    
    // Build dependency graph from input/output connections
    void build();
    
    // Queries
    [[nodiscard]] std::vector<IModelComponent*> topologicalOrder() const;
    [[nodiscard]] bool hasCycles() const;
    [[nodiscard]] std::vector<IModelComponent*> dependencies(IModelComponent* comp) const;
    [[nodiscard]] std::vector<IModelComponent*> dependents(IModelComponent* comp) const;
    
    // Validation
    [[nodiscard]] std::vector<std::string> validate() const;  // All issues
    
    // Debug
    [[nodiscard]] std::string toDOT() const;  // Graphviz format
    
private:
    struct Node {
        IModelComponent* component;
        std::set<IModelComponent*> dependencies;
        std::set<IModelComponent*> dependents;
    };
    
    std::unordered_map<IModelComponent*, Node> m_graph;
    bool m_built = false;
};

// Orchestrator: executes components in order
class HYDROCOUPLESDK_EXPORT WorkflowOrchestrator
{
public:
    explicit WorkflowOrchestrator(const WorkflowGraph& graph);
    
    // Initialize all components in order
    void initialize(std::vector<std::string>& errors);
    
    // Validate dependencies
    void validate(std::vector<std::string>& errors);
    
    // Prepare all components
    void prepare(std::vector<std::string>& errors);
    
    // Execute one update cycle (all components in topological order)
    void updateCycle(std::vector<std::string>& errors,
                     bool stopOnError = true);
    
    // Execute N update cycles
    void updateNCycles(int n, std::vector<std::string>& errors);
    
    // Finish all components
    void finish(std::vector<std::string>& errors);
    
    // Monitoring
    [[nodiscard]] IModelComponent::ComponentStatus status(IModelComponent* comp) const;
    [[nodiscard]] int cyclesCompleted() const;
    
private:
    const WorkflowGraph& m_graph;
    int m_cyclesCompleted = 0;
};

}  // namespace HydroCouple
```

### 3.2 Component Dependency Detection

**File**: `src/hydrocouple/workflow.cpp` (NEW)

```cpp
void WorkflowGraph::build()
{
    // Build reverse map: input → component using it
    std::unordered_map<IOutput*, std::set<IModelComponent*>> outputConsumers;
    
    for (auto comp : m_graph) {
        for (auto input : comp.inputs()) {
            if (auto provider = input->provider()) {
                if (auto providerComp = provider->modelComponent()) {
                    outputConsumers[provider].insert(providerComp);
                }
            }
        }
    }
    
    // For each output, mark dependents
    for (auto [output, consumers] : outputConsumers) {
        auto producer = output->modelComponent();
        for (auto consumer : consumers) {
            m_graph[consumer].dependencies.insert(producer);
            m_graph[producer].dependents.insert(consumer);
        }
    }
    
    m_built = true;
}

std::vector<IModelComponent*> WorkflowGraph::topologicalOrder() const
{
    if (!m_built) 
        throw std::logic_error("WorkflowGraph not built");
    
    std::vector<IModelComponent*> result;
    std::unordered_map<IModelComponent*, int> inDegree;
    std::queue<IModelComponent*> queue;
    
    // Calculate in-degrees
    for (const auto& [comp, node] : m_graph) {
        inDegree[comp] = node.dependencies.size();
        if (inDegree[comp] == 0) {
            queue.push(comp);
        }
    }
    
    // Kahn's algorithm
    while (!queue.empty()) {
        auto comp = queue.front();
        queue.pop();
        result.push_back(comp);
        
        for (auto dependent : m_graph.at(comp).dependents) {
            if (--inDegree[dependent] == 0) {
                queue.push(dependent);
            }
        }
    }
    
    if (result.size() != m_graph.size()) {
        throw std::logic_error("Circular dependency detected in workflow");
    }
    
    return result;
}

bool WorkflowGraph::hasCycles() const
{
    try {
        topologicalOrder();
        return false;
    } catch (const std::logic_error&) {
        return true;
    }
}

std::vector<std::string> WorkflowGraph::validate() const
{
    std::vector<std::string> errors;
    
    if (!m_built) {
        errors.push_back("WorkflowGraph not built");
        return errors;
    }
    
    // Check for cycles
    if (hasCycles()) {
        errors.push_back("Circular dependency detected in component workflow");
    }
    
    // Check for missing inputs
    for (const auto& [comp, node] : m_graph) {
        for (const auto& input : comp->inputs()) {
            if (!input->provider()) {
                errors.push_back("Component " + comp->id() + 
                               " has unconnected required input: " + input->id());
            }
        }
    }
    
    return errors;
}

std::string WorkflowGraph::toDOT() const
{
    std::stringstream ss;
    ss << "digraph Workflow {\n";
    
    for (const auto& [comp, node] : m_graph) {
        ss << "  \"" << comp->id() << "\";\n";
        for (const auto& dep : node.dependencies) {
            ss << "  \"" << dep->id() << "\" -> \"" << comp->id() << "\";\n";
        }
    }
    
    ss << "}\n";
    return ss.str();
}
```

### 3.3 Orchestrator Execution Logic

**File**: `src/hydrocouple/workflow.cpp` (orchestrator methods)

```cpp
void WorkflowOrchestrator::initialize(std::vector<std::string>& errors)
{
    auto order = m_graph.topologicalOrder();
    
    for (auto comp : order) {
        try {
            comp->initialize();
            if (comp->status() == IModelComponent::ComponentStatus::Failed) {
                errors.push_back("Component " + comp->id() + " failed during initialize()");
                if (stopOnError) return;
            }
        } catch (const std::exception& e) {
            errors.push_back("Exception in " + comp->id() + "::initialize(): " + e.what());
            if (stopOnError) return;
        }
    }
}

void WorkflowOrchestrator::updateCycle(std::vector<std::string>& errors, bool stopOnError)
{
    auto order = m_graph.topologicalOrder();
    
    for (auto comp : order) {
        // Only update if valid
        if (comp->status() != IModelComponent::ComponentStatus::Valid &&
            comp->status() != IModelComponent::ComponentStatus::Updated) {
            errors.push_back("Component " + comp->id() + " is not valid; skipping update");
            if (stopOnError) return;
        }
        
        try {
            // Collect required outputs from dependents
            std::vector<IOutput*> requiredOutputs;
            for (auto dependent : m_graph.dependents(comp)) {
                for (auto input : dependent->inputs()) {
                    if (input->provider() && 
                        input->provider()->modelComponent() == comp) {
                        requiredOutputs.push_back(input->provider());
                    }
                }
            }
            
            comp->update(requiredOutputs);
            
            if (comp->status() == IModelComponent::ComponentStatus::Failed ||
                comp->status() == IModelComponent::ComponentStatus::Done) {
                // Stop if component explicitly failed or finished
                if (comp->status() == IModelComponent::ComponentStatus::Failed) {
                    errors.push_back("Component " + comp->id() + " failed during update()");
                }
            }
        } catch (const std::exception& e) {
            errors.push_back("Exception in " + comp->id() + "::update(): " + e.what());
            if (stopOnError) return;
        }
    }
    
    m_cyclesCompleted++;
}
```

### 3.4 Usage Example

**File**: `examples/workflow_orchestration.cpp` (NEW)

```cpp
#include "hydrocouple/workflow.h"
#include <iostream>

using namespace HydroCouple;

int main()
{
    // Create components
    auto hydrology = createHydrologyComponent();      // Provides discharge
    auto chemistry = createChemistryComponent();      // Consumes discharge, provides concentration
    auto riverModel = createRiverModelComponent();    // Consumes both
    
    // Connect components
    chemistry->inputs()[0]->setProvider(hydrology->outputs()[0]);
    riverModel->inputs()[0]->setProvider(hydrology->outputs()[0]);
    riverModel->inputs()[1]->setProvider(chemistry->outputs()[0]);
    
    // Build workflow
    std::vector<IModelComponent*> components = {hydrology, chemistry, riverModel};
    WorkflowGraph graph(components);
    
    try {
        graph.build();
        
        // Check for issues
        auto validation = graph.validate();
        if (!validation.empty()) {
            for (const auto& msg : validation) {
                std::cerr << "Validation issue: " << msg << "\n";
            }
            return 1;
        }
        
        // Print dependency order
        std::cout << "Execution order: ";
        for (auto comp : graph.topologicalOrder()) {
            std::cout << comp->id() << " ";
        }
        std::cout << "\n";
        
        // Optional: visualize
        std::cout << "Workflow graph (Graphviz):\n" << graph.toDOT();
        
        // Orchestrate execution
        WorkflowOrchestrator orchestrator(graph);
        std::vector<std::string> errors;
        
        orchestrator.initialize(errors);
        if (!errors.empty()) {
            for (const auto& e : errors) std::cerr << "Error: " << e << "\n";
            return 1;
        }
        
        orchestrator.validate(errors);
        orchestrator.prepare(errors);
        
        // Run 10 time steps
        orchestrator.updateNCycles(10, errors);
        
        orchestrator.finish(errors);
        
        if (!errors.empty()) {
            for (const auto& e : errors) std::cerr << "Error: " << e << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
```

## Implementation Plan: Focus Area 3

### Phase 3A: Dependency Detection (Week 4-5)

**Tasks:**
- [ ] Create `include/hydrocouple/workflow.h` with `WorkflowGraph`
- [ ] Implement `WorkflowGraph::build()` and dependency detection
- [ ] Implement `WorkflowGraph::topologicalOrder()` (Kahn's algorithm)
- [ ] Implement cycle detection
- [ ] Create unit tests: `tests/test_workflow_graph.cpp`
  - Test simple 2-component graph
  - Test complex multi-layer graph
  - Test cycle detection
  - Test topological ordering

**Deliverable**: Dependency analysis layer

**Estimate**: 12 developer-hours

### Phase 3B: Orchestration Engine (Week 5-6)

**Tasks:**
- [ ] Implement `WorkflowOrchestrator::initialize()`
- [ ] Implement `WorkflowOrchestrator::validate()`
- [ ] Implement `WorkflowOrchestrator::prepare()`
- [ ] Implement `WorkflowOrchestrator::updateCycle()` (topological update order)
- [ ] Implement `WorkflowOrchestrator::finish()`
- [ ] Create integration tests: `tests/test_workflow_orchestrator.cpp`
  - Test full initialization → update → finish cycle
  - Test error handling and recovery
  - Test cycle counting

**Deliverable**: Production orchestration engine

**Estimate**: 14 developer-hours

### Phase 3C: Advanced Features (Week 6-7)

**Tasks:**
- [ ] Implement visualization: `WorkflowGraph::toDOT()` for Graphviz
- [ ] Implement validation: `WorkflowGraph::validate()` (missing inputs, cycles)
- [ ] Add event hooks: `onComponentStatusChanged()`, `onCycleComplete()`
- [ ] Optional: Thread pool executor (deferred to Phase 4 if needed)
- [ ] Create examples: `examples/workflow_orchestration.cpp`

**Deliverable**: Production-ready features + visualization

**Estimate**: 10 developer-hours

### Phase 3D: Testing & Documentation (Week 7-8)

**Tasks:**
- [ ] Create realistic multi-component scenario tests
- [ ] Add user guide: `docs/WORKFLOW_ORCHESTRATION.md`
- [ ] Add API reference: Doxygen comments on all public methods
- [ ] Create migration guide for users currently orchestrating manually
- [ ] Add performance benchmarks: 10-component, 100-component workflows

**Deliverable**: Complete workflow system with docs

**Estimate**: 10 developer-hours

## Metrics & Success Criteria

- ✅ Automatic topological sort of any component graph (verified by unit tests)
- ✅ Cycle detection with clear error messages
- ✅ All existing multi-component test scenarios pass with orchestrator
- ✅ New test coverage: 90%+ of WorkflowGraph and WorkflowOrchestrator
- ✅ Zero deadlocks in orchestrated scenarios (single-threaded model assumed)
- ✅ Performance: 1000-component graph builds in <10ms

---

# Focus Area 4: Improve Error Handling with Persistent Error Queues

## Current Problem

**Error states are transient and unpersistent:**
```cpp
// Current: No way to retrieve error context
comp->initialize();  // Fails
comp->status();  // Returns Failed
// But what was the error? Inspect logs? No API for this.

// Event args lack persistent context
void ComponentStatusChangeEventArgs {
    IModelComponent* component() const;  // But no error message
    // Message was passed to setStatus() but lost
};
```

## Design: Persistent Error Queue

### 4.1 Error Information Carrier

**File**: `include/hydrocouple/errorcontext.h` (NEW)

```cpp
namespace HydroCouple {

// Severity levels for errors
enum class ErrorSeverity {
    Informational,  // Non-blocking issue (e.g., "initializing with defaults")
    Warning,        // Potential issue (e.g., "missing optional input")
    Error,          // Blocking issue (e.g., "required input not connected")
    Fatal           // Unrecoverable (e.g., "out of memory")
};

// Single error entry
class HYDROCOUPLESDK_EXPORT ErrorInfo
{
public:
    ErrorInfo(ErrorSeverity severity,
              const std::string& message,
              const std::string& context = "");
    
    [[nodiscard]] ErrorSeverity severity() const;
    [[nodiscard]] std::string severityName() const;
    [[nodiscard]] std::string message() const;
    [[nodiscard]] std::string context() const;  // Method/phase that failed
    [[nodiscard]] std::chrono::system_clock::time_point timestamp() const;
    
    // For stack traces
    void setStackTrace(const std::vector<std::string>& frames);
    [[nodiscard]] std::vector<std::string> stackTrace() const;

private:
    ErrorSeverity m_severity;
    std::string m_message;
    std::string m_context;
    std::chrono::system_clock::time_point m_timestamp;
    std::vector<std::string> m_stackTrace;
};

// Error queue for a component
class HYDROCOUPLESDK_EXPORT ErrorQueue
{
public:
    ErrorQueue();
    
    // Add error
    void addError(ErrorSeverity severity, const std::string& message,
                  const std::string& context = "");
    
    void addWarning(const std::string& message) {
        addError(ErrorSeverity::Warning, message);
    }
    
    void addError(const std::string& message) {
        addError(ErrorSeverity::Error, message);
    }
    
    void addFatal(const std::string& message) {
        addError(ErrorSeverity::Fatal, message);
    }
    
    // Query
    [[nodiscard]] const std::vector<ErrorInfo>& all() const;
    [[nodiscard]] std::vector<ErrorInfo> errors(ErrorSeverity minSeverity = ErrorSeverity::Error) const;
    [[nodiscard]] int errorCount() const;
    [[nodiscard]] int warningCount() const;
    [[nodiscard]] bool hasErrors() const;
    [[nodiscard]] bool hasFatal() const;
    
    // Cleanup
    void clear();
    void clearBySeverity(ErrorSeverity severity);
    
    // Debug output
    [[nodiscard]] std::string summary() const;  // "5 errors, 2 warnings"
    [[nodiscard]] std::string report() const;   // Full formatted report
    
    // Integration with exceptions
    void addException(const std::exception& e, const std::string& context = "");

private:
    std::vector<ErrorInfo> m_errors;
    std::shared_mutex m_mutex;
};

}  // namespace HydroCouple
```

### 4.2 Extend IModelComponent with Error Queue

**File**: `include/hydrocouple/abstractmodelcomponent.h` (MODIFIED)

```cpp
class IModelComponent : public virtual IIdentity,
                        public virtual ISignal<const shared_ptr<IComponentStatusChangeEventArgs>&>
{
    // ... existing methods ...
    
    // NEW: Error handling
    virtual ErrorQueue& errorQueue() = 0;
    virtual const ErrorQueue& errorQueue() const = 0;
    
    // Convenience: last error message
    virtual std::string lastErrorMessage() const = 0;
    
    // NEW: Status change args include errors
    virtual const std::vector<ErrorInfo>& lastErrors() const = 0;
};
```

### 4.3 Concrete Implementation

**File**: `include/hydrocouple/abstractmodelcomponent.h` (member)

```cpp
class AbstractModelComponent : public virtual IModelComponent
{
private:
    ErrorQueue m_errorQueue;
    std::vector<ErrorInfo> m_lastStatusChangeErrors;  // Snapshot at each status change
};
```

### 4.4 Enhanced setStatus Methods

**File**: `src/hydrocouple/abstractmodelcomponent.cpp` (MODIFIED)

```cpp
void AbstractModelComponent::setStatus(ComponentStatus status,
                                        const std::string& message)
{
    setStatus(status, message, -1.0f);  // -1 = no progress
}

void AbstractModelComponent::setStatus(ComponentStatus status,
                                        const std::string& message,
                                        float progress)
{
    if (!message.empty()) {
        // Determine severity based on status
        auto severity = (status == ComponentStatus::Failed || 
                        status == ComponentStatus::Invalid)
            ? ErrorSeverity::Error
            : ErrorSeverity::Informational;
        
        m_errorQueue.addError(severity, message, statusToString(status));
    }
    
    m_status = status;
    m_lastStatusChangeErrors = m_errorQueue.errors(ErrorSeverity::Warning);
    
    // Emit signal with updated error context
    auto args = std::make_shared<ComponentStatusChangeEventArgs>(this, message);
    emit(args);
}

std::string AbstractModelComponent::lastErrorMessage() const
{
    const auto& errors = m_errorQueue.errors(ErrorSeverity::Error);
    if (errors.empty()) return "";
    return errors.back().message();
}

const std::vector<ErrorInfo>& AbstractModelComponent::lastErrors() const
{
    return m_lastStatusChangeErrors;
}

std::string AbstractModelComponent::errorReport() const
{
    return m_errorQueue.report();
}
```

### 4.5 Usage Examples

**File**: `examples/error_handling.cpp` (NEW)

```cpp
#include "hydrocouple/abstractmodelcomponent.h"
#include <iostream>

using namespace HydroCouple;

int main()
{
    auto component = createMyComponent();
    
    try {
        component->initialize();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    
    // Check status
    if (component->status() == IModelComponent::ComponentStatus::Failed) {
        // NEW: Inspect detailed error queue
        std::cerr << "Component failed. Errors:\n";
        std::cerr << component->errorQueue().report();  // Formatted report
        
        std::cerr << "\nLast error: " << component->lastErrorMessage() << "\n";
        std::cerr << "Total errors: " << component->errorQueue().errorCount() << "\n";
    }
    
    return 0;
}
```

**Advanced: Error Recovery**
```cpp
// Handle specific error type
bool shouldRetry = false;
for (const auto& error : component->errorQueue().errors()) {
    if (error.context() == "initializeArguments" &&
        error.message().find("file not found") != std::string::npos) {
        // This is a recoverable error: missing optional file
        shouldRetry = true;
    }
}

if (shouldRetry) {
    component->errorQueue().clear();
    component->initialize();  // Try again
}
```

## Implementation Plan: Focus Area 4

### Phase 4A: Error Infrastructure (Week 2-3)

**Tasks:**
- [ ] Create `include/hydrocouple/errorcontext.h` with `ErrorInfo` and `ErrorQueue`
- [ ] Implement `ErrorInfo` constructors and getters
- [ ] Implement `ErrorQueue::add*()`, `errors()`, `summary()`, `report()` methods
- [ ] Add formatting: `ErrorInfo::severityName()`, colorized output for terminals
- [ ] Create unit tests: `tests/test_error_queue.cpp`
  - Test adding errors
  - Test filtering by severity
  - Test report formatting

**Deliverable**: Core error infrastructure

**Estimate**: 10 developer-hours

### Phase 4B: Integration with AbstractModelComponent (Week 3)

**Tasks:**
- [ ] Add `ErrorQueue m_errorQueue` member to `AbstractModelComponent`
- [ ] Modify `setStatus()` to capture errors in queue
- [ ] Implement `errorQueue()`, `lastErrorMessage()`, `lastErrors()` methods
- [ ] Update `ComponentStatusChangeEventArgs` to include error context
- [ ] Ensure thread-safe access to error queue (shared_mutex in ErrorQueue)
- [ ] Create integration tests: `tests/test_component_error_handling.cpp`

**Deliverable**: Error handling integrated into component lifecycle

**Estimate**: 8 developer-hours

### Phase 4C: Migration of Existing Error Handling (Week 3-4)

**Tasks:**
- [ ] Audit all `setStatus(..., message)` calls in SDK
- [ ] Migrate to new `ErrorQueue::add*()` API where appropriate
- [ ] Replace try/catch blocks that lose error context with ErrorQueue captures
- [ ] Add error capture to: `initialize()`, `prepare()`, `update()`, `finish()`
- [ ] Verify all existing tests still pass

**High-priority files:**
- `src/hydrocouple/abstractmodelcomponent.cpp`
- `src/hydrocouple/abstractargument.cpp`
- `src/hydrocouple/abstractinput.cpp`
- Adapter implementations

**Estimate**: 12 developer-hours

### Phase 4D: Documentation & Examples (Week 4)

**Tasks:**
- [ ] Create `docs/ERROR_HANDLING.md` guide
- [ ] Add examples: `examples/error_handling.cpp`
- [ ] Add troubleshooting section: common errors and recovery
- [ ] Update API docs with error queue usage patterns
- [ ] Create migration guide: old error handling → new queue API

**Deliverable**: Complete error handling story with docs

**Estimate**: 6 developer-hours

## Metrics & Success Criteria

- ✅ All component errors are captured in persistent queue
- ✅ No error messages are lost during status transitions
- ✅ Error reports are human-readable (formatted, timestamped, with context)
- ✅ All existing tests pass; error-related tests increased by 50%
- ✅ Error recovery examples work: clear queue, retry initialization

---

# Integration & Sequencing

## Dependency Graph

```
Focus 1 (Type Introspection)
    ↓
    └─→ Focus 4 (Error Handling)  [can start immediately]
    └─→ Focus 2 (Mixin Composition)  [wait for Focus 1 before migrating adapters]
    
Focus 2 (Mixin Composition)
    ↓
    └─→ Focus 3 (Workflow Orchestration)  [needs simplified data items]
```

## Recommended Parallel Tracks

**Track A (Weeks 1-4): Types & Errors (Low Risk)**
- Week 1-2: Focus 1 (Type Introspection) - Phase 1A, 1B
- Week 2-3: Focus 4 (Error Handling) - Phase 4A, 4B (parallel with 1C)
- Week 3-4: Focus 1 (Type Introspection) - Phase 1C, 1D + Focus 4 - Phase 4C

**Track B (Weeks 3-8): Refactor Mixins (Medium Risk)**
- Week 3-4: Focus 2 (Mixin Composition) - Phase 2A
- Week 4-5: Focus 2 - Phase 2B (parallel with 1C, 4C)
- Week 5-6: Focus 2 - Phase 2C
- Week 6-8: Focus 2 - Phase 2D, 2E (migration)

**Track C (Weeks 4-8): Workflow (Medium Risk)**
- Week 4-5: Focus 3 (Workflow) - Phase 3A (parallel with Focus 2A)
- Week 5-6: Focus 3 - Phase 3B (parallel with Focus 2B)
- Week 6-7: Focus 3 - Phase 3C
- Week 7-8: Focus 3 - Phase 3D (parallel with Focus 2E)

## Critical Path

**Critical Path** (longest without parallelization):
- Phase 1A (Type Introspection) → 10h
- Phase 1C (Migration) → 12h → Phase 2B (Integration) → 12h
- Total: ~34 hours (~8 days at 4h/day)

**With Parallelization**:
- Weeks 1-8: All four focus areas in parallel
- Longest single track: Focus 2 (Mixin) at ~80 hours over 5 weeks
- Realistic: **8-10 weeks** with team of 2-3 developers

---

# Risk Assessment

## Focus 1: Type Introspection (LOW RISK)

| Risk | Mitigation |
|------|-----------|
| Type detection misses edge cases | Comprehensive unit tests with all 14 variant types |
| Converters introduce bugs | Integration tests with real adapters; fuzz testing |
| Performance impact of type checking | Benchmark: type detection should be <1μs per call |

**Rollback Plan**: Easy; old direct `std::get<T>()` code remains functional

## Focus 2: Mixin Composition (MEDIUM RISK)

| Risk | Mitigation |
|------|-----------|
| Break existing library users | Create type aliases for old concrete classes; deprecation period |
| ABI incompatibility | Template code in headers; no binary change to abstract classes |
| Incomplete migration | Feature branch before merging; comprehensive test coverage |
| Performance regression | Benchmark: storage backend dispatch ~1% overhead vs. direct access |

**Rollback Plan**: Hard; requires reverting multiple commits; plan 2-week stabilization

## Focus 3: Workflow Orchestration (MEDIUM RISK)

| Risk | Mitigation |
|------|-----------|
| Circular dependencies not detected | Unit test cycles; verify all test suites pass |
| Deadlock in orchestration | Single-threaded model assumed (no locks); add thread-safety in Phase 4 |
| User code relies on manual orchestration | New feature doesn't break old pattern; both coexist |

**Rollback Plan**: Easy; independent module; can be disabled

## Focus 4: Error Handling (LOW RISK)

| Risk | Mitigation |
|------|-----------|
| Thread safety issues with error queue | Use `std::shared_mutex`; lock during capture and read |
| Error queue memory bloat | Implement rotation: keep last 100 errors; warn on cap |
| Existing error-handling code breaks | ErrorQueue is additive; doesn't change existing setStatus() calls |

**Rollback Plan**: Easy; optional feature; can be removed

---

# Success Metrics (Overall)

## Code Quality

- ✅ No unhandled variant exceptions in adapters (code audit)
- ✅ All 4 focus areas: >90% test coverage
- ✅ Compilation time: ±5% change (baseline: ~2 minutes for full SDK)
- ✅ Binary size: -10% to +5% (hope for reduction from simplified hierarchy)

## Architecture

- ✅ Reduce concrete exchange item classes: 15 → 5
- ✅ Eliminate manual workflow orchestration need: 100% of test cases use orchestrator
- ✅ Error recovery implemented in ≥3 test scenarios
- ✅ Type mismatches caught at adapter initialization, not runtime

## Performance

- ✅ Type introspection: <1μs per operation
- ✅ Workflow graph building: <10ms for 1000-component graph
- ✅ No overhead to single-component performance (backward compatible)

## Documentation

- ✅ 3 new user guides: Type Introspection, Workflow Orchestration, Error Handling
- ✅ 5+ worked examples for each focus area
- ✅ Migration guide for existing users
- ✅ API reference: 100% of new public methods documented

---

# Timeline Summary

| Phase | Duration | Effort (dev-hours) | Notes |
|-------|----------|-------------------|-------|
| **Focus 1: Type Introspection** | 3 weeks | 35h | Can run in parallel with Focus 4 |
| Phase 1A | 1-2 weeks | 10h | Infrastructure only |
| Phase 1B | Week 2 | 8h | Converters |
| Phase 1C | Week 3 | 12h | Adapter migration |
| Phase 1D | Week 3 | 5h | Documentation |
| **Focus 4: Error Handling** | 2 weeks | 36h | Can run in parallel with Focus 1 |
| Phase 4A | Week 2-3 | 10h | Infrastructure |
| Phase 4B | Week 3 | 8h | Integration |
| Phase 4C | Week 3-4 | 12h | Migration |
| Phase 4D | Week 4 | 6h | Documentation |
| **Focus 2: Mixin Composition** | 5 weeks | 80h | Depends on Focus 1 stabilization |
| Phase 2A | Week 3-4 | 16h | Storage backends |
| Phase 2B | Week 4-5 | 12h | Integration |
| Phase 2C | Week 5-6 | 20h | Unified classes |
| Phase 2D | Week 6-8 | 24h | Migration |
| Phase 2E | Week 8 | 8h | ABI compatibility |
| **Focus 3: Workflow** | 4 weeks | 46h | Depends on Focus 2 for data items |
| Phase 3A | Week 4-5 | 12h | Dependency detection |
| Phase 3B | Week 5-6 | 14h | Orchestration |
| Phase 3C | Week 6-7 | 10h | Advanced features |
| Phase 3D | Week 7-8 | 10h | Documentation |
| **Total** | **8-10 weeks** | **~197 hours** | **~1 FTE + 0.5 FTE** |

---

# Sign-Off Template

**Implementation Roadmap Approved By:**

- [ ] Architecture Lead: _____________________ Date: _______
- [ ] Project Manager: _____________________ Date: _______
- [ ] QA Lead: _____________________ Date: _______

**Start Date**: ___________
**Target Completion**: ___________
**Constraints/Notes**: 

---

# Appendix A: Qt GUI Integration (QProperty Wrappers)

## Overview

HydroCouple interfaces will be wrapped for Qt GUI use via **QProperty** (Qt 6+) and **Q_PROPERTY** (Qt 5.x compatibility).

**Architecture**:
```
HydroCouple Core (C++ interfaces)
    ↓
Qt Binding Layer (IComponentDataItem → QProperty)
    ↓
QML/Qt Widgets Frontend (QProperty → UI)
```

## A.1: Type Introspection → QProperty

### C++ Binding

**File**: `qt/hydrocouple_qt.h` (NEW)

```cpp
#include <QObject>
#include <QProperty>
#include "hydrocouple/datatype.h"
#include "hydrocouple/abstractcomponentdataitem.h"

namespace HydroCouple::Qt {

// Expose DataType to Qt
class TypeProperty : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString baseTypeName READ baseTypeName NOTIFY typeChanged)
    Q_PROPERTY(bool isNumeric READ isNumeric NOTIFY typeChanged)
    Q_PROPERTY(bool isFloating READ isFloating NOTIFY typeChanged)
    
public:
    explicit TypeProperty(const DataType& dataType, QObject* parent = nullptr);
    
    [[nodiscard]] QString baseTypeName() const;
    [[nodiscard]] bool isNumeric() const;
    [[nodiscard]] bool isFloating() const;
    
    void setDataType(const DataType& type);
    
Q_SIGNALS:
    void typeChanged();
    
private:
    DataType m_dataType;
};

// Component data item as Qt object with bindable properties
class ComponentDataItemProperty : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString caption READ caption WRITE setCaption NOTIFY captionChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(TypeProperty* valueType READ valueType NOTIFY valueTypeChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int dimensionCount READ dimensionCount NOTIFY dimensionCountChanged)
    
public:
    explicit ComponentDataItemProperty(IComponentDataItem* item, QObject* parent = nullptr);
    
    [[nodiscard]] QString id() const;
    [[nodiscard]] QString caption() const;
    void setCaption(const QString& caption);
    
    [[nodiscard]] QString description() const;
    void setDescription(const QString& description);
    
    [[nodiscard]] TypeProperty* valueType() const;
    
    [[nodiscard]] QVariant value() const;  // IComponentDataItem->getValue() as QVariant
    void setValue(const QVariant& value);  // setValue() with Qt type checking
    
    [[nodiscard]] int dimensionCount() const;
    
    // Typed access (for adapters)
    template<typename T>
    std::optional<T> safeGetValue() const {
        auto v = hydrocouple_variant();
        m_item->getValue(v, {});
        return getSafeValue<T>(v);
    }
    
Q_SIGNALS:
    void captionChanged(const QString& newCaption);
    void descriptionChanged(const QString& newDescription);
    void valueTypeChanged();
    void valueChanged(const QVariant& newValue);
    void dimensionCountChanged(int newCount);
    
private:
    IComponentDataItem* m_item;
    std::unique_ptr<TypeProperty> m_valueType;
    
    // Listen to HydroCouple signals and translate to Qt signals
    void connectSignals();
};

}  // namespace HydroCouple::Qt
```

### QML Usage

```qml
// components/DataItemView.qml
import QtQuick
import HydroCoupleQt 1.0

Rectangle {
    required property ComponentDataItemProperty dataItem
    
    Column {
        Text {
            text: "Type: " + dataItem.valueType.baseTypeName
            color: dataItem.valueType.isNumeric ? "blue" : "black"
        }
        
        TextField {
            text: dataItem.value
            onEditingFinished: dataItem.value = text
        }
        
        Text {
            text: dataItem.valueType.isNumeric 
                ? "Numeric input allowed"
                : "String input allowed"
        }
    }
}
```

## A.2: Error Queue → Qt Signal Model

### C++ Binding

**File**: `qt/errorqueueadapter.h` (NEW)

```cpp
#include <QObject>
#include <QAbstractListModel>
#include "hydrocouple/errorcontext.h"

namespace HydroCouple::Qt {

// Single error for Qt list model
struct ErrorItem {
    QString severity;      // "Error", "Warning", "Fatal"
    QString message;
    QString context;       // Method name
    QDateTime timestamp;
    QColor severityColor;  // Red for Error, Orange for Warning, etc.
};

// Error queue exposed as Qt signals + list model
class ErrorQueueAdapter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int errorCount READ errorCount NOTIFY errorCountChanged)
    Q_PROPERTY(int warningCount READ warningCount NOTIFY warningCountChanged)
    Q_PROPERTY(bool hasErrors READ hasErrors NOTIFY errorStatusChanged)
    Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)
    Q_PROPERTY(ErrorListModel* model READ model CONSTANT)
    
public:
    explicit ErrorQueueAdapter(const ErrorQueue& queue, QObject* parent = nullptr);
    
    [[nodiscard]] int errorCount() const;
    [[nodiscard]] int warningCount() const;
    [[nodiscard]] bool hasErrors() const;
    [[nodiscard]] QString summary() const;
    
    [[nodiscard]] ErrorListModel* model() const { return m_model; }
    
    // Qt slot: capture new errors
    void refresh();
    
    // Qt slot: clear errors
    Q_INVOKABLE void clear();
    void clearBySeverity(ErrorSeverity severity);
    
Q_SIGNALS:
    void errorAdded(const ErrorItem& error);
    void errorCountChanged(int newCount);
    void warningCountChanged(int newCount);
    void errorStatusChanged(bool hasErrors);
    void summaryChanged(const QString& newSummary);
    
private:
    const ErrorQueue& m_queue;
    std::unique_ptr<ErrorListModel> m_model;
    int m_lastErrorCount = 0;
    
    // Listener slot for HydroCouple error signals
    void onErrorQueueChanged();
};

// List model for QML repeaters
class ErrorListModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    explicit ErrorListModel(const ErrorQueue& queue, QObject* parent = nullptr);
    
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    enum Roles {
        SeverityRole = Qt::UserRole,
        MessageRole,
        ContextRole,
        TimestampRole,
        ColorRole
    };
    
    void refresh();
    
private:
    const ErrorQueue& m_queue;
    std::vector<ErrorItem> m_items;
};

}  // namespace HydroCouple::Qt
```

### QML Usage

```qml
// ErrorPanel.qml
import QtQuick
import QtQuick.Controls
import HydroCoupleQt 1.0

Rectangle {
    required property ErrorQueueAdapter errorQueue
    
    Column {
        spacing: 10
        
        // Summary row
        Row {
            Text {
                text: errorQueue.summary
                font.bold: true
                color: errorQueue.hasErrors ? "red" : "green"
            }
            
            Button {
                text: "Clear"
                onClicked: errorQueue.clear()
                visible: errorQueue.hasErrors
            }
        }
        
        // Errors list
        ListView {
            width: parent.width
            height: 300
            model: errorQueue.model
            clip: true
            
            delegate: Rectangle {
                width: parent.width
                height: 50
                color: model.color
                opacity: 0.2
                
                Column {
                    anchors.fill: parent
                    anchors.margins: 5
                    
                    Text {
                        text: model.severity + ": " + model.message
                        font.bold: true
                    }
                    
                    Text {
                        text: model.context + " @ " + model.timestamp
                        font.pointSize: 8
                        color: "gray"
                    }
                }
            }
        }
    }
}
```

## A.3: Workflow Orchestrator → Qt Status Model

### C++ Binding

**File**: `qt/workflowstatemodel.h` (NEW)

```cpp
#include <QObject>
#include <QAbstractListModel>
#include "hydrocouple/workflow.h"

namespace HydroCouple::Qt {

// Component status for Qt list model
struct ComponentStatus {
    QString id;
    QString status;          // "Created", "Initialized", "Valid", "Updating", etc.
    QColor statusColor;      // Green for Valid, Blue for Updating, Red for Failed
    int cycleCount;          // Update cycles completed
    QString lastError;
};

// Workflow orchestrator status exposed to Qt
class WorkflowStateModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString workflowStatus READ workflowStatus NOTIFY workflowStatusChanged)
    Q_PROPERTY(int cyclesCompleted READ cyclesCompleted NOTIFY cyclesCompletedChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(ComponentStatusListModel* components READ components CONSTANT)
    
public:
    explicit WorkflowStateModel(const WorkflowOrchestrator& orchestrator, 
                                QObject* parent = nullptr);
    
    [[nodiscard]] QString workflowStatus() const;
    [[nodiscard]] int cyclesCompleted() const;
    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] ComponentStatusListModel* components() const;
    
    // Control orchestration from Qt
    Q_INVOKABLE void initialize();
    Q_INVOKABLE void step();      // One update cycle
    Q_INVOKABLE void run(int numCycles);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void finish();
    
Q_SIGNALS:
    void workflowStatusChanged(const QString& newStatus);
    void cyclesCompletedChanged(int newCount);
    void isRunningChanged(bool running);
    void componentStatusChanged(const ComponentStatus& status);
    void cycleCompleted();
    
private:
    const WorkflowOrchestrator& m_orchestrator;
    std::unique_ptr<ComponentStatusListModel> m_componentModel;
    QTimer* m_updateTimer = nullptr;  // For continuous run()
    
    void onComponentStatusChanged(IModelComponent* comp);
    void refreshComponentStatus(IModelComponent* comp);
};

// List model for component statuses
class ComponentStatusListModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    explicit ComponentStatusListModel(const WorkflowOrchestrator& orch, 
                                       QObject* parent = nullptr);
    
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    enum Roles {
        IdRole = Qt::UserRole,
        StatusRole,
        ColorRole,
        CycleCountRole,
        ErrorRole
    };
    
    void updateComponent(const ComponentStatus& status);
    void refreshAll();
    
private:
    const WorkflowOrchestrator& m_orchestrator;
    std::vector<ComponentStatus> m_components;
};

}  // namespace HydroCouple::Qt
```

### QML Usage: Workflow Control Panel

```qml
// WorkflowControlPanel.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import HydroCoupleQt 1.0

Rectangle {
    required property WorkflowStateModel workflow
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // Control buttons
        RowLayout {
            Button {
                text: "Initialize"
                onClicked: workflow.initialize()
                enabled: !workflow.isRunning
            }
            
            Button {
                text: "Step"
                onClicked: workflow.step()
                enabled: !workflow.isRunning
            }
            
            Button {
                text: workflow.isRunning ? "Pause" : "Run 10 Cycles"
                onClicked: workflow.isRunning ? workflow.pause() : workflow.run(10)
            }
            
            Button {
                text: "Finish"
                onClicked: workflow.finish()
            }
        }
        
        // Status row
        Text {
            text: "Status: " + workflow.workflowStatus + 
                  " | Cycles: " + workflow.cyclesCompleted
            font.bold: true
        }
        
        // Component grid
        TableView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            model: workflow.components
            
            delegate: Rectangle {
                width: 200
                height: 40
                color: model.color
                opacity: 0.3
                border.color: "black"
                
                Text {
                    anchors.fill: parent
                    anchors.margins: 5
                    text: model.id + ": " + model.status
                    elide: Text.ElideRight
                }
            }
        }
    }
}
```

## A.4: Mixin Composition → QML Binding

### Storage Backend Observable

**File**: `qt/storageobservable.h` (NEW)

```cpp
#include <QObject>
#include "hydrocouple/storagebackend.h"

namespace HydroCouple::Qt {

// Expose storage backend changes to Qt
class StorageObservable : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int length READ length NOTIFY lengthChanged)
    Q_PROPERTY(QVariant valueAt READ valueAt NOTIFY valuesChanged)
    
public:
    explicit StorageObservable(IStorageBackend* backend, QObject* parent = nullptr);
    
    [[nodiscard]] int length() const;
    [[nodiscard]] QVariant valueAt(int index) const;
    
Q_SIGNALS:
    void lengthChanged(int newLength);
    void valuesChanged(int startIndex, int count);  // Range of changed values
    
private:
    IStorageBackend* m_backend;
};

}  // namespace HydroCouple::Qt
```

### QML Data Display

```qml
// StorageView.qml
import QtQuick
import QtQuick.Controls
import HydroCoupleQt 1.0

Rectangle {
    required property StorageObservable storage
    
    ListView {
        model: storage.length
        delegate: Row {
            Text { text: "Index " + index }
            TextField {
                readOnly: true
                text: storage.valueAt(index)
            }
        }
    }
}
```

## A.5: Implementation Timeline for Qt Bindings

### Phase 5: Qt Integration Layer (Weeks 9-12, after Focus Areas 1-4 stable)

**Week 9**: Type Introspection Qt Binding
- [ ] Implement `TypeProperty` (QProperty wrapper for DataType)
- [ ] Implement `ComponentDataItemProperty` (observable data items)
- [ ] Create QML component library for type display
- [ ] Estimate: 16 dev-hours

**Week 10**: Error Queue Qt Binding
- [ ] Implement `ErrorQueueAdapter` and `ErrorListModel`
- [ ] Implement error panel QML component
- [ ] Create error visualization examples
- [ ] Estimate: 12 dev-hours

**Week 11**: Workflow Orchestrator Qt Binding
- [ ] Implement `WorkflowStateModel` and component status model
- [ ] Create workflow control panel QML
- [ ] Create dependency graph visualization (using GraphViz or QML layout)
- [ ] Estimate: 14 dev-hours

**Week 12**: Storage & Polish
- [ ] Implement `StorageObservable` for mixin composition binding
- [ ] Create integrated example: full app with all bindings
- [ ] Performance testing: QProperty update latency
- [ ] Estimate: 10 dev-hours

**Total Qt Integration**: ~52 dev-hours (1.5 FTE-weeks)

## A.6: Qt Compatibility Strategy

**Qt 6.X (Preferred)**:
- Use `QProperty<T>` for native binding support
- QML engine handles property change notifications automatically
- No manual signal/slot bridging needed

**Qt 5.X (Fallback)**:
- Use `Q_PROPERTY` + manual signal emission
- Wrapper classes same interface, different implementation
- Compatibility layer: `#if QT_VERSION < QT_VERSION_CHECK(6,0,0)`

**Example**:
```cpp
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QProperty<QString> m_caption;
#else
    QString m_caption;
    void setCaptionImpl(const QString& value) {
        if (m_caption != value) {
            m_caption = value;
            Q_EMIT captionChanged(value);
        }
    }
#endif
```

## A.7: QML Type Registration

**File**: `qt/hydrocouple_plugin.cpp` (for QML module)

```cpp
#include <QtQml>
#include "typeProperty.h"
#include "componentdataitemProperty.h"
#include "errorqueueadapter.h"
#include "workflowstatemodel.h"

static void registerTypes(const char *uri)
{
    qmlRegisterType<HydroCouple::Qt::TypeProperty>(uri, 1, 0, "TypeProperty");
    qmlRegisterType<HydroCouple::Qt::ComponentDataItemProperty>(uri, 1, 0, "ComponentDataItem");
    qmlRegisterType<HydroCouple::Qt::ErrorQueueAdapter>(uri, 1, 0, "ErrorQueue");
    qmlRegisterType<HydroCouple::Qt::WorkflowStateModel>(uri, 1, 0, "WorkflowState");
    qmlRegisterUncreatableType<HydroCouple::Qt::ErrorListModel>(
        uri, 1, 0, "ErrorListModel",
        "ErrorListModel is read-only");
}

Q_CREGISTER_PLUGIN_METADATA(uri, registerTypes)
```

**QML module definition**: `qt/qmldir`
```
module HydroCoupleQt
plugin hydrocoupleplugin
classname HydroCouplePlugin
```

---

## Summary: Qt Integration Points

| Focus Area | Qt Wrapper | QML Component | Signal Bridge |
|---|---|---|---|
| Type Introspection | `TypeProperty` | `TypeDisplay.qml` | `valueTypeChanged()` |
| Error Handling | `ErrorQueueAdapter` | `ErrorPanel.qml` | `errorAdded()`, `summaryChanged()` |
| Workflow | `WorkflowStateModel` | `ControlPanel.qml` | `cycleCompleted()`, `componentStatusChanged()` |
| Mixin Composition | `StorageObservable` | `StorageView.qml` | `valuesChanged()` |

**Key Principle**: Core HydroCouple remains Qt-free; Qt bindings are a separate optional layer for GUI applications.

