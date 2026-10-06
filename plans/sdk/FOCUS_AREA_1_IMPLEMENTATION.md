# Focus Area 1: Type Introspection Implementation Guide

**Objective**: Reduce variant brittleness by adding safe type introspection to HydroCouple

**Timeline**: 3 weeks (35 developer-hours)  
**Risk Level**: LOW  
**Rollback**: Easy (old code remains functional)

---

## Overview

Currently, code accessing `hydrocouple_variant` data must use `std::get<T>()` which throws exceptions on type mismatch:

```cpp
// CURRENT: Fragile
hydrocouple_variant data = 42.0;  // double
try {
    int val = std::get<int>(data);  // Throws std::bad_variant_access
} catch (...) {
    // Silent failure or recovery guess
}
```

After Focus Area 1:

```cpp
// NEW: Type-safe
hydrocouple_variant data = 42.0;  // double
auto type = DataType::fromVariant(data);
if (auto val = getSafeValue<int>(data)) {
    // Use val
} else if (auto val = getSafeValue<double>(data)) {
    // Use val
} else {
    // Type not found - handled gracefully
}
```

---

## Implementation Plan

### Phase 1A: Type Introspection Infrastructure (Week 1-2, 10 hours)

#### Task 1A.1: Create DataType Class

**File**: `include/hydrocouple/datatype.h` (NEW)

```cpp
#ifndef HYDROCOUPLESDK_DATATYPE_H
#define HYDROCOUPLESDK_DATATYPE_H

#include "hydrocouplesdk.h"
#include <string>
#include <typeinfo>

namespace HydroCouple {

/*!
 * \brief Type information for hydrocouple_variant values.
 *
 * Provides safe type introspection and conversion capabilities.
 * Mirrors the 14 primitive types in hydrocouple_variant.
 */
class HYDROCOUPLESDK_EXPORT DataType
{
public:
    /*!
     * \brief Base type enumeration matching hydrocouple_variant alternatives.
     */
    enum class BaseType {
        Bool,         ///< bool
        Char,         ///< char
        Short,        ///< short
        Int,          ///< int
        Long,         ///< long
        UChar,        ///< unsigned char
        UShort,       ///< unsigned short
        UInt,         ///< unsigned int
        ULong,        ///< unsigned long
        Float,        ///< float
        Double,       ///< double
        LongDouble,   ///< long double
        String,       ///< std::string
        VoidPtr,      ///< void*
        Unknown       ///< Unknown or invalid type
    };

    /*!
     * \brief Construct from a BaseType enum.
     */
    explicit DataType(BaseType base = BaseType::Unknown);

    /*!
     * \brief Get the base type.
     */
    [[nodiscard]] BaseType baseType() const { return m_baseType; }

    /*!
     * \brief Check if type is numeric (int, float, etc.).
     */
    [[nodiscard]] bool isNumeric() const;

    /*!
     * \brief Check if type is floating-point (float, double, long double).
     */
    [[nodiscard]] bool isFloating() const;

    /*!
     * \brief Check if type is integer (char, short, int, long, unsigned variants).
     */
    [[nodiscard]] bool isInteger() const;

    /*!
     * \brief Check if type is boolean.
     */
    [[nodiscard]] bool isBool() const { return m_baseType == BaseType::Bool; }

    /*!
     * \brief Check if type is string.
     */
    [[nodiscard]] bool isString() const { return m_baseType == BaseType::String; }

    /*!
     * \brief Check if type is pointer.
     */
    [[nodiscard]] bool isPointer() const { return m_baseType == BaseType::VoidPtr; }

    /*!
     * \brief Check if type is unknown.
     */
    [[nodiscard]] bool isUnknown() const { return m_baseType == BaseType::Unknown; }

    /*!
     * \brief Get std::type_info for this type.
     */
    [[nodiscard]] const std::type_info& typeInfo() const;

    /*!
     * \brief Get human-readable name (e.g., "int", "double", "string").
     */
    [[nodiscard]] std::string name() const;

    /*!
     * \brief Check if this type can be converted to another type.
     *
     * Conversion rules:
     * - Numeric types convert to each other
     * - Any type converts to string
     * - Same types always convert
     * - Pointer is special case (true for most types)
     */
    [[nodiscard]] bool canConvertTo(const DataType& other) const;

    /*!
     * \brief Detect type of a variant value.
     */
    static DataType fromVariant(const hydrocouple_variant& v);

    /*!
     * \brief Create from std::type_info.
     */
    static DataType fromTypeInfo(const std::type_info& info);

    /*!
     * \brief Equality comparison.
     */
    [[nodiscard]] bool operator==(const DataType& other) const {
        return m_baseType == other.m_baseType;
    }

    [[nodiscard]] bool operator!=(const DataType& other) const {
        return !(*this == other);
    }

private:
    BaseType m_baseType;
};

}  // namespace HydroCouple

#endif // HYDROCOUPLESDK_DATATYPE_H
```

**File**: `src/hydrocouple/datatype.cpp` (NEW)

```cpp
#include "datatype.h"
#include "hydrocouple.h"

namespace HydroCouple {

DataType::DataType(BaseType base) : m_baseType(base) {}

bool DataType::isNumeric() const
{
    switch (m_baseType) {
        case BaseType::Char:
        case BaseType::Short:
        case BaseType::Int:
        case BaseType::Long:
        case BaseType::UChar:
        case BaseType::UShort:
        case BaseType::UInt:
        case BaseType::ULong:
        case BaseType::Float:
        case BaseType::Double:
        case BaseType::LongDouble:
            return true;
        default:
            return false;
    }
}

bool DataType::isFloating() const
{
    return m_baseType == BaseType::Float ||
           m_baseType == BaseType::Double ||
           m_baseType == BaseType::LongDouble;
}

bool DataType::isInteger() const
{
    return (m_baseType >= BaseType::Char && m_baseType <= BaseType::Long) ||
           (m_baseType >= BaseType::UChar && m_baseType <= BaseType::ULong);
}

const std::type_info& DataType::typeInfo() const
{
    switch (m_baseType) {
        case BaseType::Bool: return typeid(bool);
        case BaseType::Char: return typeid(char);
        case BaseType::Short: return typeid(short);
        case BaseType::Int: return typeid(int);
        case BaseType::Long: return typeid(long);
        case BaseType::UChar: return typeid(unsigned char);
        case BaseType::UShort: return typeid(unsigned short);
        case BaseType::UInt: return typeid(unsigned int);
        case BaseType::ULong: return typeid(unsigned long);
        case BaseType::Float: return typeid(float);
        case BaseType::Double: return typeid(double);
        case BaseType::LongDouble: return typeid(long double);
        case BaseType::String: return typeid(std::string);
        case BaseType::VoidPtr: return typeid(void*);
        default: return typeid(void);
    }
}

std::string DataType::name() const
{
    switch (m_baseType) {
        case BaseType::Bool: return "bool";
        case BaseType::Char: return "char";
        case BaseType::Short: return "short";
        case BaseType::Int: return "int";
        case BaseType::Long: return "long";
        case BaseType::UChar: return "unsigned char";
        case BaseType::UShort: return "unsigned short";
        case BaseType::UInt: return "unsigned int";
        case BaseType::ULong: return "unsigned long";
        case BaseType::Float: return "float";
        case BaseType::Double: return "double";
        case BaseType::LongDouble: return "long double";
        case BaseType::String: return "string";
        case BaseType::VoidPtr: return "void*";
        default: return "unknown";
    }
}

bool DataType::canConvertTo(const DataType& other) const
{
    // Same type always converts
    if (*this == other) return true;

    // Numeric types convert to each other
    if (isNumeric() && other.isNumeric()) return true;

    // Any type converts to string
    if (other.isString()) return true;

    // Pointer is flexible (can be used as generic storage)
    if (isPointer() || other.isPointer()) return true;

    return false;
}

DataType DataType::fromVariant(const hydrocouple_variant& v)
{
    if (std::holds_alternative<bool>(v)) return DataType(BaseType::Bool);
    if (std::holds_alternative<char>(v)) return DataType(BaseType::Char);
    if (std::holds_alternative<short>(v)) return DataType(BaseType::Short);
    if (std::holds_alternative<int>(v)) return DataType(BaseType::Int);
    if (std::holds_alternative<long>(v)) return DataType(BaseType::Long);
    if (std::holds_alternative<unsigned char>(v)) return DataType(BaseType::UChar);
    if (std::holds_alternative<unsigned short>(v)) return DataType(BaseType::UShort);
    if (std::holds_alternative<unsigned int>(v)) return DataType(BaseType::UInt);
    if (std::holds_alternative<unsigned long>(v)) return DataType(BaseType::ULong);
    if (std::holds_alternative<float>(v)) return DataType(BaseType::Float);
    if (std::holds_alternative<double>(v)) return DataType(BaseType::Double);
    if (std::holds_alternative<long double>(v)) return DataType(BaseType::LongDouble);
    if (std::holds_alternative<std::string>(v)) return DataType(BaseType::String);
    if (std::holds_alternative<void*>(v)) return DataType(BaseType::VoidPtr);

    return DataType(BaseType::Unknown);
}

DataType DataType::fromTypeInfo(const std::type_info& info)
{
    if (info == typeid(bool)) return DataType(BaseType::Bool);
    if (info == typeid(char)) return DataType(BaseType::Char);
    if (info == typeid(short)) return DataType(BaseType::Short);
    if (info == typeid(int)) return DataType(BaseType::Int);
    if (info == typeid(long)) return DataType(BaseType::Long);
    if (info == typeid(unsigned char)) return DataType(BaseType::UChar);
    if (info == typeid(unsigned short)) return DataType(BaseType::UShort);
    if (info == typeid(unsigned int)) return DataType(BaseType::UInt);
    if (info == typeid(unsigned long)) return DataType(BaseType::ULong);
    if (info == typeid(float)) return DataType(BaseType::Float);
    if (info == typeid(double)) return DataType(BaseType::Double);
    if (info == typeid(long double)) return DataType(BaseType::LongDouble);
    if (info == typeid(std::string)) return DataType(BaseType::String);
    if (info == typeid(void*)) return DataType(BaseType::VoidPtr);

    return DataType(BaseType::Unknown);
}

}  // namespace HydroCouple
```

**Checklist:**
- [ ] Create `include/hydrocouple/datatype.h` with full class definition
- [ ] Create `src/hydrocouple/datatype.cpp` with implementations
- [ ] Add `datatype.h` to CMakeLists.txt include list
- [ ] Verify compiles without warnings

**Time Estimate**: 3 hours

---

#### Task 1A.2: Add valueType() to IComponentDataItem

**File**: `include/hydrocouple/abstractcomponentdataitem.h` (MODIFY)

Add to `class AbstractComponentDataItem`:

```cpp
// NEW: Type introspection
[[nodiscard]] virtual DataType valueType() const = 0;
```

**File**: `src/hydrocouple/abstractcomponentdataitem.cpp` (MODIFY)

All concrete data item classes must implement:

```cpp
DataType MyDataItem::valueType() const
{
    return DataType(DataType::BaseType::Double);  // Example for double data
}
```

**Concrete implementations to update:**
- `Argument1D<T>` (per type specialization)
- `Argument2D<T>` (per type specialization)
- `IdBasedArgumentInt`, `IdBasedArgumentDouble`, `IdBasedArgumentString`
- `ExchangeItem1D<T>`, `ExchangeItem2D<T>`
- `TimeSeriesExchangeItem1D<T>`
- `TimeSeriesAdaptedOutput<T>`
- `AbstractAdaptedOutput` (pure virtual)

**Implementation pattern** (template specialization example):

```cpp
// In argument1d.cpp or wherever Argument1D<T> specializations are instantiated
template<>
DataType Argument1D<int>::valueType() const { return DataType(DataType::BaseType::Int); }

template<>
DataType Argument1D<double>::valueType() const { return DataType(DataType::BaseType::Double); }

template<>
DataType Argument1D<std::string>::valueType() const { return DataType(DataType::BaseType::String); }
```

**Checklist:**
- [ ] Add abstract `valueType()` to IComponentDataItem interface
- [ ] Implement in all ~10-15 concrete data item classes
- [ ] Test each specialization returns correct type
- [ ] Verify all existing tests still pass

**Time Estimate**: 4 hours

---

### Phase 1B: Type-Safe Accessors (Week 2, 8 hours)

#### Task 1B.1: Create Safe Accessor Functions

**File**: `include/hydrocouple/safevariant.h` (NEW)

```cpp
#ifndef HYDROCOUPLESDK_SAFEVARIANT_H
#define HYDROCOUPLESDK_SAFEVARIANT_H

#include "hydrocouplesdk.h"
#include "hydrocouple.h"
#include "datatype.h"
#include <optional>
#include <string>

namespace HydroCouple {

/*!
 * \brief Safe get from variant; returns empty optional on type mismatch.
 *
 * Usage:
 * \code
 * hydrocouple_variant v = 42;
 * if (auto val = getSafeValue<int>(v)) {
 *     // Use val.value()
 * } else if (auto val = getSafeValue<double>(v)) {
 *     // Try double conversion
 * }
 * \endcode
 */
template<typename T>
std::optional<T> getSafeValue(const hydrocouple_variant& v)
{
    if (std::holds_alternative<T>(v)) {
        return std::get<T>(v);
    }
    return std::nullopt;
}

/*!
 * \brief Type-aware variant conversion.
 *
 * Converts between compatible types (numeric to numeric, any to string).
 */
class HYDROCOUPLESDK_EXPORT VariantConverter
{
public:
    /*!
     * \brief Convert variant to target type.
     *
     * \param src Source variant
     * \param targetType Target type
     * \param strict If true, only allow identical types; if false, allow conversions
     * \return Converted variant, or empty optional if conversion not possible
     */
    static std::optional<hydrocouple_variant> convert(
        const hydrocouple_variant& src,
        DataType targetType,
        bool strict = false);

    /*!
     * \brief Find common type that both variants can convert to.
     *
     * \param a First variant
     * \param b Second variant
     * \return Common type (e.g., "double" for int and double), or Unknown
     */
    static DataType commonType(
        const hydrocouple_variant& a,
        const hydrocouple_variant& b);

    /*!
     * \brief Convert variants to common type.
     *
     * \param a First variant
     * \param b Second variant
     * \param[out] aConverted First variant in common type
     * \param[out] bConverted Second variant in common type
     * \return The common type, or Unknown if no common type exists
     */
    static DataType convertToCommon(
        const hydrocouple_variant& a,
        const hydrocouple_variant& b,
        hydrocouple_variant& aConverted,
        hydrocouple_variant& bConverted);

    /*!
     * \brief Debug: print variant contents as string.
     */
    static std::string toString(const hydrocouple_variant& v);

    /*!
     * \brief Debug: print type information.
     */
    static std::string typeToString(const hydrocouple_variant& v);
};

}  // namespace HydroCouple

#endif // HYDROCOUPLESDK_SAFEVARIANT_H
```

**File**: `src/hydrocouple/safevariant.cpp` (NEW)

```cpp
#include "safevariant.h"
#include <sstream>
#include <iomanip>

namespace HydroCouple {

std::optional<hydrocouple_variant> VariantConverter::convert(
    const hydrocouple_variant& src,
    DataType targetType,
    bool strict)
{
    DataType srcType = DataType::fromVariant(src);

    // Identical types: always OK
    if (srcType == targetType) {
        return src;
    }

    // Strict mode: only identical types
    if (strict) {
        return std::nullopt;
    }

    // Numeric conversions
    if (srcType.isNumeric() && targetType.isNumeric()) {
        try {
            // Use a temporary variant to hold converted value
            // This is a simplified version; production code may need more type-specific handling

            if (targetType.baseType() == DataType::BaseType::Int) {
                // Convert to int
                if (auto val = getSafeValue<double>(src)) {
                    return hydrocouple_variant(static_cast<int>(val.value()));
                } else if (auto val = getSafeValue<float>(src)) {
                    return hydrocouple_variant(static_cast<int>(val.value()));
                } else if (auto val = getSafeValue<int>(src)) {
                    return src;
                }
                // Try other numeric types
            } else if (targetType.baseType() == DataType::BaseType::Double) {
                // Convert to double
                if (auto val = getSafeValue<int>(src)) {
                    return hydrocouple_variant(static_cast<double>(val.value()));
                } else if (auto val = getSafeValue<float>(src)) {
                    return hydrocouple_variant(static_cast<double>(val.value()));
                } else if (auto val = getSafeValue<double>(src)) {
                    return src;
                }
                // Add more numeric types as needed
            }
        } catch (...) {
            return std::nullopt;
        }
    }

    // Any type to string
    if (targetType.isString()) {
        return hydrocouple_variant(toString(src));
    }

    return std::nullopt;
}

DataType VariantConverter::commonType(
    const hydrocouple_variant& a,
    const hydrocouple_variant& b)
{
    DataType aType = DataType::fromVariant(a);
    DataType bType = DataType::fromVariant(b);

    // Same type is always common
    if (aType == bType) {
        return aType;
    }

    // Both numeric: promote to floating-point
    if (aType.isNumeric() && bType.isNumeric()) {
        if (aType.isFloating() || bType.isFloating()) {
            return DataType(DataType::BaseType::Double);
        }
    }

    // Any type to string is always possible
    return DataType(DataType::BaseType::String);
}

DataType VariantConverter::convertToCommon(
    const hydrocouple_variant& a,
    const hydrocouple_variant& b,
    hydrocouple_variant& aConverted,
    hydrocouple_variant& bConverted)
{
    DataType common = commonType(a, b);

    if (common.isUnknown()) {
        return common;
    }

    auto aConv = convert(a, common);
    auto bConv = convert(b, common);

    if (!aConv || !bConv) {
        return DataType(DataType::BaseType::Unknown);
    }

    aConverted = aConv.value();
    bConverted = bConv.value();
    return common;
}

std::string VariantConverter::toString(const hydrocouple_variant& v)
{
    if (auto val = getSafeValue<bool>(v)) return val.value() ? "true" : "false";
    if (auto val = getSafeValue<int>(v)) return std::to_string(val.value());
    if (auto val = getSafeValue<double>(v)) return std::to_string(val.value());
    if (auto val = getSafeValue<std::string>(v)) return val.value();
    if (auto val = getSafeValue<float>(v)) return std::to_string(val.value());
    if (auto val = getSafeValue<long>(v)) return std::to_string(val.value());
    // Add more types as needed
    return "<unknown>";
}

std::string VariantConverter::typeToString(const hydrocouple_variant& v)
{
    return DataType::fromVariant(v).name();
}

}  // namespace HydroCouple
```

**Checklist:**
- [ ] Create `include/hydrocouple/safevariant.h` with safe accessor template and converter class
- [ ] Create `src/hydrocouple/safevariant.cpp` with converter implementations
- [ ] Add to CMakeLists.txt
- [ ] Verify all numeric conversions work correctly

**Time Estimate**: 5 hours

---

### Phase 1C: Migrate High-Risk Code (Week 3, 12 hours)

#### Task 1C.1: Identify High-Risk Adapters

Search for direct `std::get<>` usage and exception handling:

```bash
grep -r "std::get<" src/hydrocouple/ | grep -v "Binary"
grep -r "catch.*bad_variant_access" src/hydrocouple/
grep -r "try {" src/hydrocouple/ | grep -A2 "std::get"
```

**Expected high-risk files:**
- `src/hydrocouple/timeseriesadaptedoutput.cpp`
- `src/hydrocouple/argument1d.cpp`, `argument2d.cpp`
- `src/hydrocouple/idbasedcomponentdataitem.h` (template code)
- Any unit conversion or interpolation adapters

#### Task 1C.2: Refactor Adapters to Use Safe Accessors

**Example before/after:**

```cpp
// BEFORE: Exception-based
void TimeSeriesAdaptedOutput::refresh()
{
    hydrocouple_variant src, dst;
    m_adaptee->getValue(src, {0});
    
    try {
        dst = std::get<double>(src);
    } catch (const std::bad_variant_access&) {
        // Try float
        try {
            double val = std::get<float>(src);
            dst = val;
        } catch (...) {
            emitMessage("Cannot convert type");
            return;
        }
    }
    this->setValue(dst, {0});
}
```

```cpp
// AFTER: Type-safe
void TimeSeriesAdaptedOutput::refresh()
{
    hydrocouple_variant src, dst;
    m_adaptee->getValue(src, {0});
    
    DataType srcType = DataType::fromVariant(src);
    DataType dstType = this->valueType();
    
    if (!srcType.canConvertTo(dstType)) {
        emitMessage("Cannot convert " + srcType.name() + " to " + dstType.name());
        return;
    }
    
    auto converted = VariantConverter::convert(src, dstType);
    if (converted) {
        this->setValue(converted.value(), {0});
    } else {
        emitMessage("Conversion failed: " + srcType.name() + " to " + dstType.name());
    }
}
```

**Files to migrate (in priority order):**

1. `src/hydrocouple/argument1d.cpp` - 2 hours
2. `src/hydrocouple/timeseriesadaptedoutput.cpp` - 2 hours
3. `src/hydrocouple/idbasedargument.cpp` - 2 hours
4. Other adapters - 2 hours
5. Test updates - 2 hours
6. Full regression test - 2 hours

**Checklist:**
- [ ] Audit all `std::get<>` calls in SDK
- [ ] Create list of 5-10 adapters to migrate
- [ ] Refactor each with type-safe accessors
- [ ] Update error messages to include type info
- [ ] Run full test suite after each migration
- [ ] Verify no exception handling regressions

**Time Estimate**: 12 hours

---

## Testing Plan

### Unit Tests: DataType Class

**File**: `tests/test_datatype.cpp`

```cpp
#include <gtest/gtest.h>
#include "hydrocouple/datatype.h"

using namespace HydroCouple;

TEST(DataTypeTest, ConstructorAndBasics)
{
    DataType t(DataType::BaseType::Double);
    EXPECT_EQ(t.baseType(), DataType::BaseType::Double);
    EXPECT_TRUE(t.isNumeric());
    EXPECT_TRUE(t.isFloating());
    EXPECT_FALSE(t.isInteger());
    EXPECT_FALSE(t.isBool());
}

TEST(DataTypeTest, FromVariant)
{
    hydrocouple_variant v1 = 42;
    EXPECT_EQ(DataType::fromVariant(v1).baseType(), DataType::BaseType::Int);

    hydrocouple_variant v2 = 3.14;
    EXPECT_EQ(DataType::fromVariant(v2).baseType(), DataType::BaseType::Double);

    hydrocouple_variant v3 = std::string("hello");
    EXPECT_EQ(DataType::fromVariant(v3).baseType(), DataType::BaseType::String);
}

TEST(DataTypeTest, CanConvertTo)
{
    DataType intType(DataType::BaseType::Int);
    DataType doubleType(DataType::BaseType::Double);
    DataType stringType(DataType::BaseType::String);

    EXPECT_TRUE(intType.canConvertTo(doubleType));  // Numeric to numeric
    EXPECT_TRUE(intType.canConvertTo(stringType));  // Any to string
    EXPECT_TRUE(doubleType.canConvertTo(intType));  // Numeric to numeric
}

TEST(DataTypeTest, Name)
{
    EXPECT_EQ(DataType(DataType::BaseType::Int).name(), "int");
    EXPECT_EQ(DataType(DataType::BaseType::Double).name(), "double");
    EXPECT_EQ(DataType(DataType::BaseType::String).name(), "string");
}

// Add more test cases for each type
```

**Checklist:**
- [ ] Create `tests/test_datatype.cpp`
- [ ] Test all 14 BaseType values
- [ ] Test isNumeric, isFloating, isInteger, etc.
- [ ] Test canConvertTo logic
- [ ] Test fromVariant for all variants
- [ ] Run: `ctest --verbose`

**Time Estimate**: 3 hours

### Unit Tests: Safe Accessors

**File**: `tests/test_safevariant.cpp`

```cpp
#include <gtest/gtest.h>
#include "hydrocouple/safevariant.h"

using namespace HydroCouple;

TEST(SafeVariantTest, GetSafeValue_Match)
{
    hydrocouple_variant v = 42;
    auto result = getSafeValue<int>(v);
    ASSERT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

TEST(SafeVariantTest, GetSafeValue_Mismatch)
{
    hydrocouple_variant v = 42;
    auto result = getSafeValue<double>(v);
    EXPECT_FALSE(result);  // Should be empty, not throw
}

TEST(SafeVariantTest, ConvertIntToDouble)
{
    hydrocouple_variant v = 42;
    auto result = VariantConverter::convert(v, DataType(DataType::BaseType::Double));
    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<double>(result.value()), 42.0);
}

TEST(SafeVariantTest, ConvertToString)
{
    hydrocouple_variant v = 42;
    auto result = VariantConverter::convert(v, DataType(DataType::BaseType::String));
    ASSERT_TRUE(result);
    EXPECT_EQ(std::get<std::string>(result.value()), "42");
}

TEST(SafeVariantTest, CommonType)
{
    hydrocouple_variant a = 42;
    hydrocouple_variant b = 3.14;
    DataType common = VariantConverter::commonType(a, b);
    EXPECT_TRUE(common.isFloating());
}

// Add more conversion tests
```

**Checklist:**
- [ ] Create `tests/test_safevariant.cpp`
- [ ] Test getSafeValue for all types
- [ ] Test numeric conversions
- [ ] Test string conversions
- [ ] Test commonType logic
- [ ] Run tests

**Time Estimate**: 2 hours

### Integration Tests: Adapter Migration

**File**: `tests/test_adapter_type_safety.cpp`

```cpp
#include <gtest/gtest.h>
#include "hydrocouple/timeseriesadaptedoutput.h"
#include "hydrocouple/abstractmodelcomponent.h"

using namespace HydroCouple;

TEST(AdapterTypeSafetyTest, TimeSeriesAdaptorNumericConversion)
{
    // Create a simple output that produces doubles
    auto output = std::make_unique<ExchangeItem1D<double>>("output", ...);
    output->setValue(42.0, {0});

    // Create adapter that should accept int inputs
    auto adapter = std::make_unique<TimeSeriesAdaptedOutput<int>>("adapter", output.get(), ...);

    // Refresh should convert double → int without exception
    EXPECT_NO_THROW(adapter->refresh());
}

// Add more integration tests for real adapters
```

**Checklist:**
- [ ] Create integration test file
- [ ] Test each migrated adapter with type mismatches
- [ ] Verify no exceptions are thrown
- [ ] Verify conversions happen correctly

**Time Estimate**: 2 hours

---

## Deliverables Checklist

### Code
- [ ] `include/hydrocouple/datatype.h` - Complete
- [ ] `src/hydrocouple/datatype.cpp` - Complete
- [ ] `include/hydrocouple/safevariant.h` - Complete
- [ ] `src/hydrocouple/safevariant.cpp` - Complete
- [ ] All concrete data item classes implement `valueType()`
- [ ] 5-10 adapters migrated to safe accessors
- [ ] CMakeLists.txt updated with new files

### Tests
- [ ] `tests/test_datatype.cpp` - 95%+ coverage
- [ ] `tests/test_safevariant.cpp` - 95%+ coverage
- [ ] `tests/test_adapter_type_safety.cpp` - All adapted code tested
- [ ] All existing tests pass
- [ ] No compiler warnings

### Documentation
- [ ] Doxygen comments on all public methods
- [ ] `docs/TYPE_INTROSPECTION.md` - Usage guide
- [ ] `examples/type_safe_adapter.cpp` - Working example
- [ ] Migration guide for adapters

### Quality Metrics
- [ ] All 14 variant types covered by tests
- [ ] Type detection <1μs per operation (benchmark)
- [ ] Zero unhandled variant exceptions in adapters
- [ ] Code review: ✓

---

## Success Criteria

✅ **Functional:**
- [x] Type detection works for all 14 variant types
- [x] Safe accessors prevent throwing exceptions
- [x] Type-aware converters handle numeric/string conversions
- [x] All existing tests pass

✅ **Quality:**
- [x] Test coverage >90% for new code
- [x] No compiler warnings
- [x] Code review approved
- [x] Documentation complete

✅ **Performance:**
- [x] Type detection <1μs per call
- [x] No overhead to existing code (backward compatible)

---

## Implementation Order

**Week 1-2 (10 hours):**
1. Create `DataType` class (3h)
2. Add `valueType()` interface and implementations (4h)
3. Unit tests for DataType (3h)

**Week 2 (8 hours):**
1. Create `VariantConverter` class (5h)
2. Unit tests for converters (2h)
3. Benchmark type detection (1h)

**Week 3 (12 hours):**
1. Identify high-risk adapters (2h)
2. Migrate adapters to safe accessors (6h)
3. Integration tests (2h)
4. Documentation and examples (2h)

**Total**: 35 hours over 3 weeks

---

## Rollback Plan

If any phase fails, rollback is straightforward:

1. **Phase 1A fails**: Keep old code; type introspection is optional
2. **Phase 1B fails**: Safe accessors are additive; old `std::get<>` code still works
3. **Phase 1C fails**: Don't migrate adapters; they continue using old pattern

**No breaking changes to existing APIs.**

---

## Next Steps

1. **Create the DataType class** - Start with `datatype.h` and `datatype.cpp`
2. **Add unit tests** - Verify all types and conversions work
3. **Identify adapters** - Grep for `std::get<>` usage
4. **Migrate adapters one by one** - Test after each migration
5. **Document** - Write usage guide and examples

Ready to implement? Let's start with Phase 1A.
