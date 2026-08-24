/*!
 * \file test_core_types.cpp
 * \brief Tests for the v2 data-plane core types: DataKind, BufferDescriptor,
 * MemorySpace, Capability, ErrorEntry, the component lifecycle
 * transition table, and a functional hyperslab data-item implementation.
 */

#include <gtest/gtest.h>

#include "hydrocouple.h"
#include "hydrocouplehelpers.h"

#include <array>
#include <cstring>
#include <numeric>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

using namespace HydroCouple;
using namespace HydroCouple::Helpers;

// ============================================================================
// DataKind tests
// ============================================================================

TEST(DataKindTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<DataKind>);
    EXPECT_FALSE((std::is_convertible_v<DataKind, int>));
}

TEST(DataKindTest, ElementSizes)
{
    EXPECT_EQ(dataKindSize(DataKind::Int8), 1);
    EXPECT_EQ(dataKindSize(DataKind::UInt8), 1);
    EXPECT_EQ(dataKindSize(DataKind::Boolean), 1);
    EXPECT_EQ(dataKindSize(DataKind::Int16), 2);
    EXPECT_EQ(dataKindSize(DataKind::UInt16), 2);
    EXPECT_EQ(dataKindSize(DataKind::Int32), 4);
    EXPECT_EQ(dataKindSize(DataKind::UInt32), 4);
    EXPECT_EQ(dataKindSize(DataKind::Float32), 4);
    EXPECT_EQ(dataKindSize(DataKind::Int64), 8);
    EXPECT_EQ(dataKindSize(DataKind::UInt64), 8);
    EXPECT_EQ(dataKindSize(DataKind::Float64), 8);
    EXPECT_EQ(dataKindSize(DataKind::String), 0);
    EXPECT_EQ(dataKindSize(DataKind::Opaque), 0);
    EXPECT_EQ(dataKindSize(DataKind::Unknown), 0);
}

TEST(DataKindTest, SizesAreConstexpr)
{
    static_assert(dataKindSize(DataKind::Float64) == 8);
    static_assert(dataKindSize(DataKind::Int32) == 4);
    SUCCEED();
}

TEST(DataKindTest, TraitsMapping)
{
    static_assert(DataKindOf<int8_t>::kind == DataKind::Int8);
    static_assert(DataKindOf<uint8_t>::kind == DataKind::UInt8);
    static_assert(DataKindOf<int16_t>::kind == DataKind::Int16);
    static_assert(DataKindOf<uint16_t>::kind == DataKind::UInt16);
    static_assert(DataKindOf<int32_t>::kind == DataKind::Int32);
    static_assert(DataKindOf<uint32_t>::kind == DataKind::UInt32);
    static_assert(DataKindOf<int64_t>::kind == DataKind::Int64);
    static_assert(DataKindOf<uint64_t>::kind == DataKind::UInt64);
    static_assert(DataKindOf<float>::kind == DataKind::Float32);
    static_assert(DataKindOf<double>::kind == DataKind::Float64);
    static_assert(DataKindOf<bool>::kind == DataKind::Boolean);
    static_assert(DataKindOf<std::string>::kind == DataKind::String);
    // Unmapped types are Opaque
    struct Custom {};
    static_assert(DataKindOf<Custom>::kind == DataKind::Opaque);
    SUCCEED();
}

TEST(DataKindTest, TraitsAgreeWithSizes)
{
    EXPECT_EQ(dataKindSize(DataKindOf<double>::kind), static_cast<int64_t>(sizeof(double)));
    EXPECT_EQ(dataKindSize(DataKindOf<float>::kind), static_cast<int64_t>(sizeof(float)));
    EXPECT_EQ(dataKindSize(DataKindOf<int32_t>::kind), static_cast<int64_t>(sizeof(int32_t)));
    EXPECT_EQ(dataKindSize(DataKindOf<int64_t>::kind), static_cast<int64_t>(sizeof(int64_t)));
}

// ============================================================================
// MemorySpace tests
// ============================================================================

TEST(MemorySpaceTest, Values)
{
    EXPECT_EQ(static_cast<uint8_t>(MemorySpace::Host), 0);
    EXPECT_EQ(static_cast<uint8_t>(MemorySpace::HostPinned), 1);
    EXPECT_EQ(static_cast<uint8_t>(MemorySpace::Device), 2);
    EXPECT_EQ(static_cast<uint8_t>(MemorySpace::Unified), 3);
}

// ============================================================================
// BufferDescriptor tests
// ============================================================================

TEST(BufferDescriptorTest, DefaultIsEmptyScalar)
{
    BufferDescriptor d;
    EXPECT_EQ(d.data, nullptr);
    EXPECT_EQ(d.kind, DataKind::Unknown);
    EXPECT_EQ(d.rank, 0);
    EXPECT_EQ(elementCount(d), 1); // rank 0 == scalar
    EXPECT_TRUE(isContiguous(d));
    EXPECT_EQ(d.space, MemorySpace::Host);
}

TEST(BufferDescriptorTest, ElementCountIsShapeProduct)
{
    const int64_t shape[] = {3, 4, 5};
    double dummy = 0.0;
    auto d = makeContiguous(&dummy, DataKind::Float64, shape);
    EXPECT_EQ(d.rank, 3);
    EXPECT_EQ(elementCount(d), 60);
    EXPECT_EQ(contiguousByteSize(d), 60 * 8);
}

TEST(BufferDescriptorTest, NullStridesAreContiguous)
{
    const int64_t shape[] = {10, 20};
    double dummy = 0.0;
    auto d = makeContiguous(&dummy, DataKind::Float64, shape);
    EXPECT_TRUE(isContiguous(d));
}

TEST(BufferDescriptorTest, ExplicitRowMajorStridesAreContiguous)
{
    const int64_t shape[] = {10, 20};
    const int64_t strides[] = {20 * 8, 8}; // row-major packed doubles
    BufferDescriptor d;
    d.kind = DataKind::Float64;
    d.rank = 2;
    d.shape = shape;
    d.stridesBytes = strides;
    EXPECT_TRUE(isContiguous(d));
}

TEST(BufferDescriptorTest, PaddedRowsAreNotContiguous)
{
    const int64_t shape[] = {10, 20};
    const int64_t strides[] = {32 * 8, 8}; // pitched rows: 32 elements allocated per 20 used
    BufferDescriptor d;
    d.kind = DataKind::Float64;
    d.rank = 2;
    d.shape = shape;
    d.stridesBytes = strides;
    EXPECT_FALSE(isContiguous(d));
}

TEST(BufferDescriptorTest, ColumnSliceIsNotContiguous)
{
    const int64_t shape[] = {10};
    const int64_t strides[] = {20 * 8}; // every 20th element of a [x][20] array
    BufferDescriptor d;
    d.kind = DataKind::Float64;
    d.rank = 1;
    d.shape = shape;
    d.stridesBytes = strides;
    EXPECT_FALSE(isContiguous(d));
}

TEST(BufferDescriptorTest, UnitExtentDimensionsIgnoredForContiguity)
{
    // A [1][20] slab of a larger array: dimension 0 has extent 1, so its stride
    // must not matter.
    const int64_t shape[] = {1, 20};
    const int64_t strides[] = {500 * 8, 8};
    BufferDescriptor d;
    d.kind = DataKind::Float64;
    d.rank = 2;
    d.shape = shape;
    d.stridesBytes = strides;
    EXPECT_TRUE(isContiguous(d));
}

TEST(BufferDescriptorTest, ByteOffsetContiguous)
{
    const int64_t shape[] = {4, 5};
    double dummy = 0.0;
    auto d = makeContiguous(&dummy, DataKind::Float64, shape);
    const int64_t idx[] = {2, 3};
    // Row-major: (2 * 5 + 3) * 8
    EXPECT_EQ(byteOffset(d, idx), (2 * 5 + 3) * 8);
}

TEST(BufferDescriptorTest, ByteOffsetStrided)
{
    const int64_t shape[] = {4, 5};
    const int64_t strides[] = {100, 8};
    BufferDescriptor d;
    d.kind = DataKind::Float64;
    d.rank = 2;
    d.shape = shape;
    d.stridesBytes = strides;
    const int64_t idx[] = {2, 3};
    EXPECT_EQ(byteOffset(d, idx), 2 * 100 + 3 * 8);
}

TEST(BufferDescriptorTest, BroadcastViaZeroStride)
{
    const int64_t shape[] = {1000};
    const int64_t strides[] = {0}; // every element aliases the same scalar
    BufferDescriptor d;
    d.kind = DataKind::Float64;
    d.rank = 1;
    d.shape = shape;
    d.stridesBytes = strides;
    const int64_t idx[] = {999};
    EXPECT_EQ(byteOffset(d, idx), 0);
    EXPECT_FALSE(isContiguous(d));
}

// ============================================================================
// Capability / ErrorEntry tests
// ============================================================================

TEST(CapabilityTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<Capability>);
    EXPECT_FALSE((std::is_convertible_v<Capability, int>));
}

TEST(CapabilityTest, Values)
{
    EXPECT_EQ(static_cast<uint32_t>(Capability::DeviceBuffers), 0u);
    EXPECT_EQ(static_cast<uint32_t>(Capability::PartitionedData), 1u);
    EXPECT_EQ(static_cast<uint32_t>(Capability::DistributedExecution), 2u);
    EXPECT_EQ(static_cast<uint32_t>(Capability::Checkpointing), 3u);
    EXPECT_EQ(static_cast<uint32_t>(Capability::Cloneable), 4u);
    EXPECT_EQ(static_cast<uint32_t>(Capability::UserInterface), 5u);
    EXPECT_EQ(static_cast<uint32_t>(Capability::Licensing), 6u);
}

TEST(ErrorEntryTest, Defaults)
{
    ErrorEntry e;
    EXPECT_EQ(e.severity, ErrorEntry::Severity::Information);
    EXPECT_EQ(e.code, 0);
    EXPECT_TRUE(e.source.empty());
    EXPECT_TRUE(e.message.empty());
}

// ============================================================================
// Component lifecycle transition table tests
// ============================================================================

using CS = IModelComponent::ComponentStatus;

TEST(StatusTransitionTest, HappyPathIsLegal)
{
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Created, CS::Initializing));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Initializing, CS::Initialized));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Initialized, CS::Validating));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Validating, CS::Valid));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Valid, CS::Preparing));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Preparing, CS::Updated));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Updated, CS::Updating));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Updating, CS::Updated));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Updating, CS::Done));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Done, CS::Finishing));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Finishing, CS::Finished));
}

TEST(StatusTransitionTest, CheckpointingCycle)
{
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Updated, CS::Checkpointing));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Checkpointing, CS::Updated));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Checkpointing, CS::Failed));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Checkpointing, CS::Updating));
}

TEST(StatusTransitionTest, FailureAndRecovery)
{
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Initializing, CS::Failed));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Updating, CS::Failed));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Failed, CS::Initializing));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Failed, CS::Finishing));
}

TEST(StatusTransitionTest, WaitingForDataCycle)
{
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Updating, CS::WaitingForData));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::WaitingForData, CS::Updating));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::WaitingForData, CS::Done));
}

TEST(StatusTransitionTest, IllegalTransitionsRejected)
{
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Created, CS::Updating));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Created, CS::Finished));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Finished, CS::Created));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Finished, CS::Initializing));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Done, CS::Updating));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Valid, CS::Updating));
    EXPECT_FALSE(isValidComponentStatusTransition(CS::Invalid, CS::Preparing));
}

TEST(StatusTransitionTest, ReinitializationAllowed)
{
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Initialized, CS::Initializing));
    EXPECT_TRUE(isValidComponentStatusTransition(CS::Finishing, CS::Created));
}

TEST(StatusTransitionTest, TableIsConstexpr)
{
    static_assert(isValidComponentStatusTransition(CS::Created, CS::Initializing));
    static_assert(!isValidComponentStatusTransition(CS::Finished, CS::Created));
    SUCCEED();
}

// ============================================================================
// Functional hyperslab data item: a real 2D double store
// ============================================================================

// A working IComponentDataItem over a row-major [rows][cols] double array,
// implementing correct hyperslab semantics. This is the reference contract test
// for implementers of the typed data plane.
class GridDataItem final : public IComponentDataItem
{
public:
    GridDataItem(int64_t rows, int64_t cols)
        : m_rows(rows), m_cols(cols), m_data(static_cast<size_t>(rows * cols), 0.0) {}

    double &at(int64_t r, int64_t c) { return m_data[static_cast<size_t>(r * m_cols + c)]; }

    std::vector<int64_t> shape() const override { return {m_rows, m_cols}; }
    DataKind dataKind() const override { return DataKind::Float64; }

    bool getValuesInto(const BufferDescriptor &destination,
                       std::span<const int64_t> start,
                       std::span<const int64_t> count,
                       std::string *message = nullptr) const override
    {
        if (!validate(destination, start, count, message))
            return false;
        auto *base = static_cast<std::byte *>(destination.data);
        for (int64_t r = 0; r < count[0]; ++r)
        {
            for (int64_t c = 0; c < count[1]; ++c)
            {
                const int64_t idx[] = {r, c};
                auto *dst = base + byteOffset(destination, idx);
                const double v = m_data[static_cast<size_t>((start[0] + r) * m_cols + start[1] + c)];
                std::memcpy(dst, &v, sizeof(double));
            }
        }
        return true;
    }

    bool setValuesFrom(const BufferDescriptor &source,
                       std::span<const int64_t> start,
                       std::span<const int64_t> count,
                       std::string *message = nullptr) override
    {
        if (!validate(source, start, count, message))
            return false;
        const auto *base = static_cast<const std::byte *>(source.data);
        for (int64_t r = 0; r < count[0]; ++r)
        {
            for (int64_t c = 0; c < count[1]; ++c)
            {
                const int64_t idx[] = {r, c};
                double v;
                std::memcpy(&v, base + byteOffset(source, idx), sizeof(double));
                m_data[static_cast<size_t>((start[0] + r) * m_cols + start[1] + c)] = v;
            }
        }
        return true;
    }

    // Metadata stubs
    IModelComponent *modelComponent() const override { return nullptr; }
    std::vector<IDimension *> dimensions() const override { return {}; }
    IValueDefinition *valueDefinition() const override { return nullptr; }
    const std::string &id() const override { static std::string s = "grid"; return s; }
    const std::string &caption() const override { static std::string s; return s; }
    void setCaption(const std::string &) override {}
    const std::string &description() const override { static std::string s; return s; }
    void setDescription(const std::string &) override {}
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    void blockSignals(bool) override {}
    void emit(const std::shared_ptr<IComponentDataItemValueChanged> &) override {}
    void connect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void emit(std::string) override {}

private:
    bool validate(const BufferDescriptor &buffer,
                  std::span<const int64_t> start,
                  std::span<const int64_t> count,
                  std::string *message) const
    {
        if (buffer.kind != DataKind::Float64)
        {
            if (message) *message = "kind mismatch";
            return false;
        }
        if (start.size() != 2 || count.size() != 2)
        {
            if (message) *message = "selection rank must be 2";
            return false;
        }
        if (start[0] < 0 || start[1] < 0 ||
            start[0] + count[0] > m_rows || start[1] + count[1] > m_cols)
        {
            if (message) *message = "selection out of bounds";
            return false;
        }
        if (elementCount(buffer) != count[0] * count[1])
        {
            if (message) *message = "buffer element count mismatch";
            return false;
        }
        return true;
    }

    int64_t m_rows;
    int64_t m_cols;
    std::vector<double> m_data;
};

class GridDataItemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        for (int64_t r = 0; r < 4; ++r)
            for (int64_t c = 0; c < 5; ++c)
                grid.at(r, c) = static_cast<double>(r * 10 + c);
    }
    GridDataItem grid{4, 5};
};

TEST_F(GridDataItemTest, FullSlabRoundTrip)
{
    std::vector<double> out(20);
    const int64_t start[] = {0, 0};
    const int64_t count[] = {4, 5};
    ASSERT_TRUE(getValues(grid, std::span<double>(out), start, count));
    EXPECT_DOUBLE_EQ(out[0], 0.0);
    EXPECT_DOUBLE_EQ(out[7], 12.0);  // row 1, col 2
    EXPECT_DOUBLE_EQ(out[19], 34.0); // row 3, col 4
}

TEST_F(GridDataItemTest, InteriorHyperslab)
{
    std::vector<double> out(6);
    const int64_t start[] = {1, 2};
    const int64_t count[] = {2, 3};
    ASSERT_TRUE(getValues(grid, std::span<double>(out), start, count));
    // Expected: rows 1-2, cols 2-4
    EXPECT_DOUBLE_EQ(out[0], 12.0);
    EXPECT_DOUBLE_EQ(out[1], 13.0);
    EXPECT_DOUBLE_EQ(out[2], 14.0);
    EXPECT_DOUBLE_EQ(out[3], 22.0);
    EXPECT_DOUBLE_EQ(out[4], 23.0);
    EXPECT_DOUBLE_EQ(out[5], 24.0);
}

TEST_F(GridDataItemTest, StridedDestination)
{
    // Receive a [2][3] selection into a pitched [2][8] buffer.
    std::vector<double> pitched(16, -1.0);
    const int64_t shape[] = {2, 3};
    const int64_t strides[] = {8 * 8, 8};
    BufferDescriptor dest;
    dest.data = pitched.data();
    dest.kind = DataKind::Float64;
    dest.rank = 2;
    dest.shape = shape;
    dest.stridesBytes = strides;

    const int64_t start[] = {1, 2};
    const int64_t count[] = {2, 3};
    ASSERT_TRUE(grid.getValuesInto(dest, start, count));
    EXPECT_DOUBLE_EQ(pitched[0], 12.0);
    EXPECT_DOUBLE_EQ(pitched[1], 13.0);
    EXPECT_DOUBLE_EQ(pitched[2], 14.0);
    EXPECT_DOUBLE_EQ(pitched[3], -1.0); // padding untouched
    EXPECT_DOUBLE_EQ(pitched[8], 22.0); // second pitched row
    EXPECT_DOUBLE_EQ(pitched[10], 24.0);
}

TEST_F(GridDataItemTest, ScalarHelpers)
{
    double v = 0.0;
    const int64_t idx[] = {2, 3};
    ASSERT_TRUE(getValue(grid, v, idx));
    EXPECT_DOUBLE_EQ(v, 23.0);

    ASSERT_TRUE(setValue(grid, 99.5, idx));
    ASSERT_TRUE(getValue(grid, v, idx));
    EXPECT_DOUBLE_EQ(v, 99.5);
}

TEST_F(GridDataItemTest, SetHyperslabThenReadBack)
{
    std::vector<double> in = {100.0, 101.0, 110.0, 111.0};
    const int64_t start[] = {2, 1};
    const int64_t count[] = {2, 2};
    ASSERT_TRUE(setValues(grid, std::span<const double>(in), start, count));
    EXPECT_DOUBLE_EQ(grid.at(2, 1), 100.0);
    EXPECT_DOUBLE_EQ(grid.at(2, 2), 101.0);
    EXPECT_DOUBLE_EQ(grid.at(3, 1), 110.0);
    EXPECT_DOUBLE_EQ(grid.at(3, 2), 111.0);
    // Neighbors untouched
    EXPECT_DOUBLE_EQ(grid.at(2, 0), 20.0);
    EXPECT_DOUBLE_EQ(grid.at(3, 3), 33.0);
}

TEST_F(GridDataItemTest, KindMismatchRejected)
{
    std::vector<float> wrong(6);
    const int64_t start[] = {0, 0};
    const int64_t count[] = {2, 3};
    std::string message;
    EXPECT_FALSE(grid.getValuesInto(
        makeContiguous(wrong.data(), DataKind::Float32, count), start, count, &message));
    EXPECT_EQ(message, "kind mismatch");
}

TEST_F(GridDataItemTest, OutOfBoundsRejected)
{
    std::vector<double> out(6);
    const int64_t start[] = {3, 3};
    const int64_t count[] = {2, 3};
    std::string message;
    EXPECT_FALSE(grid.getValuesInto(
        makeContiguous(out.data(), DataKind::Float64, count), start, count, &message));
    EXPECT_EQ(message, "selection out of bounds");
}

TEST_F(GridDataItemTest, WrongRankRejected)
{
    std::vector<double> out(4);
    const int64_t start[] = {0};
    const int64_t count[] = {4};
    std::string message;
    EXPECT_FALSE(grid.getValuesInto(
        makeContiguous(out.data(), DataKind::Float64, count), start, count, &message));
    EXPECT_EQ(message, "selection rank must be 2");
}
