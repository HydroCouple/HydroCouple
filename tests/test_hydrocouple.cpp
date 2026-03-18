/*!
 * \file test_hydrocouple.cpp
 * \brief Tests for HydroCouple core interface definitions, types, and type properties.
 */

#include <gtest/gtest.h>

#include "hydrocouple.h"
#include "hydrocoupletemporal.h"
#include "hydrocouplespatial.h"
#include "hydrocouplespatiotemporal.h"

#include <string>
#include <type_traits>
#include <variant>

using namespace HydroCouple;

// ============================================================================
// ByteOrder enum class tests
// ============================================================================

TEST(ByteOrderTest, Values)
{
    EXPECT_EQ(static_cast<uint8_t>(ByteOrder::BigEndian), 0);
    EXPECT_EQ(static_cast<uint8_t>(ByteOrder::LittleEndian), 1);
}

TEST(ByteOrderTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<ByteOrder>);
    // enum class should NOT be implicitly convertible to int
    EXPECT_FALSE((std::is_convertible_v<ByteOrder, int>));
}

// ============================================================================
// hydrocouple_variant tests
// ============================================================================

TEST(HydroCoupleVariantTest, HoldsAllNumericTypes)
{
    hydrocouple_variant v_bool = true;
    EXPECT_TRUE(std::get<bool>(v_bool));

    hydrocouple_variant v_char = 'A';
    EXPECT_EQ(std::get<char>(v_char), 'A');

    hydrocouple_variant v_short = static_cast<short>(42);
    EXPECT_EQ(std::get<short>(v_short), 42);

    hydrocouple_variant v_int = 100;
    EXPECT_EQ(std::get<int>(v_int), 100);

    hydrocouple_variant v_long = 123456789L;
    EXPECT_EQ(std::get<long>(v_long), 123456789L);

    hydrocouple_variant v_float = 3.14f;
    EXPECT_FLOAT_EQ(std::get<float>(v_float), 3.14f);

    hydrocouple_variant v_double = 2.71828;
    EXPECT_DOUBLE_EQ(std::get<double>(v_double), 2.71828);

    hydrocouple_variant v_ldouble = static_cast<long double>(1.41421356L);
    EXPECT_DOUBLE_EQ(static_cast<double>(std::get<long double>(v_ldouble)),
                     static_cast<double>(1.41421356L));
}

TEST(HydroCoupleVariantTest, HoldsUnsignedTypes)
{
    hydrocouple_variant v_uchar = static_cast<unsigned char>(255);
    EXPECT_EQ(std::get<unsigned char>(v_uchar), 255);

    hydrocouple_variant v_ushort = static_cast<unsigned short>(65535);
    EXPECT_EQ(std::get<unsigned short>(v_ushort), 65535);

    hydrocouple_variant v_uint = static_cast<unsigned int>(4294967295U);
    EXPECT_EQ(std::get<unsigned int>(v_uint), 4294967295U);

    hydrocouple_variant v_ulong = static_cast<unsigned long>(123456789UL);
    EXPECT_EQ(std::get<unsigned long>(v_ulong), 123456789UL);
}

TEST(HydroCoupleVariantTest, HoldsString)
{
    hydrocouple_variant v_string = std::string("Hello HydroCouple");
    EXPECT_EQ(std::get<std::string>(v_string), "Hello HydroCouple");
}

TEST(HydroCoupleVariantTest, HoldsVoidPointer)
{
    int data = 42;
    hydrocouple_variant v_ptr = static_cast<void *>(&data);
    EXPECT_EQ(std::get<void *>(v_ptr), &data);
}

TEST(HydroCoupleVariantTest, VariantSize)
{
    // Variant should hold exactly 14 alternative types
    EXPECT_EQ(std::variant_size_v<hydrocouple_variant>, 14u);
}

TEST(HydroCoupleVariantTest, ValueSemantics)
{
    hydrocouple_variant v1 = 42;
    hydrocouple_variant v2 = v1;
    EXPECT_EQ(std::get<int>(v2), 42);

    v2 = 99;
    EXPECT_EQ(std::get<int>(v1), 42);
    EXPECT_EQ(std::get<int>(v2), 99);
}

// ============================================================================
// IModelComponent::ComponentStatus enum tests
// ============================================================================

TEST(ComponentStatusTest, EnumValues)
{
    EXPECT_EQ(IModelComponent::Created, 0);
    EXPECT_EQ(IModelComponent::Initializing, 1);
    EXPECT_EQ(IModelComponent::Initialized, 2);
    EXPECT_EQ(IModelComponent::Validating, 3);
    EXPECT_EQ(IModelComponent::Valid, 4);
    EXPECT_EQ(IModelComponent::WaitingForData, 5);
    EXPECT_EQ(IModelComponent::Invalid, 6);
    EXPECT_EQ(IModelComponent::Preparing, 7);
    EXPECT_EQ(IModelComponent::Updating, 8);
    EXPECT_EQ(IModelComponent::Updated, 9);
    EXPECT_EQ(IModelComponent::Done, 10);
    EXPECT_EQ(IModelComponent::Finishing, 11);
    EXPECT_EQ(IModelComponent::Finished, 12);
    EXPECT_EQ(IModelComponent::Failed, 13);
}

// ============================================================================
// IArgument::ArgumentInputType enum tests
// ============================================================================

TEST(ArgumentInputTypeTest, EnumValues)
{
    EXPECT_EQ(IArgument::String, 0);
    EXPECT_EQ(IArgument::File, 1);
    EXPECT_EQ(IArgument::JSON, 2);
    EXPECT_EQ(IArgument::XML, 3);
    EXPECT_EQ(IArgument::URL, 4);
    EXPECT_EQ(IArgument::MEMORY_OBJECT, 5);
}

// ============================================================================
// IWorkflowComponent::WorkflowStatus enum tests
// ============================================================================

TEST(WorkflowStatusTest, EnumValues)
{
    EXPECT_EQ(IWorkflowComponent::Created, 0);
    EXPECT_EQ(IWorkflowComponent::Initializing, 1);
    EXPECT_EQ(IWorkflowComponent::Initialized, 2);
    EXPECT_EQ(IWorkflowComponent::Updating, 3);
    EXPECT_EQ(IWorkflowComponent::Updated, 4);
    EXPECT_EQ(IWorkflowComponent::Done, 5);
    EXPECT_EQ(IWorkflowComponent::Finishing, 6);
    EXPECT_EQ(IWorkflowComponent::Finished, 7);
    EXPECT_EQ(IWorkflowComponent::Failed, 8);
}

// ============================================================================
// Spatial enum class tests
// ============================================================================

using namespace HydroCouple::Spatial;

TEST(MeshDataObjectTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<MeshDataObjectType>);
    EXPECT_FALSE((std::is_convertible_v<MeshDataObjectType, int>));
}

TEST(MeshDataObjectTypeTest, Values)
{
    EXPECT_EQ(static_cast<int>(MeshDataObjectType::Cell), 0);
    EXPECT_EQ(static_cast<int>(MeshDataObjectType::Vertex), 1);
    EXPECT_EQ(static_cast<int>(MeshDataObjectType::Edge), 2);
    EXPECT_EQ(static_cast<int>(MeshDataObjectType::Face), 3);
}

TEST(NetworkDataObjectTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<NetworkDataObjectType>);
    EXPECT_FALSE((std::is_convertible_v<NetworkDataObjectType, int>));
}

TEST(MeshDataTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<MeshDataType>);
    EXPECT_FALSE((std::is_convertible_v<MeshDataType, int>));
}

TEST(NetworkDataTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<NetworkDataType>);
    EXPECT_FALSE((std::is_convertible_v<NetworkDataType, int>));
}

// ============================================================================
// IGeometry::GeometryType enum tests
// ============================================================================

TEST(GeometryTypeTest, CoreValues)
{
    EXPECT_EQ(IGeometry::Point, 1);
    EXPECT_EQ(IGeometry::LineString, 2);
    EXPECT_EQ(IGeometry::Polygon, 3);
    EXPECT_EQ(IGeometry::Triangle, 17);
    EXPECT_EQ(IGeometry::MultiPoint, 4);
    EXPECT_EQ(IGeometry::MultiLineString, 5);
    EXPECT_EQ(IGeometry::MultiPolygon, 6);
}

// ============================================================================
// Interface virtual destructor tests (compilation and linkage validation)
// ============================================================================

TEST(InterfaceTest, SlotTemplateInstantiates)
{
    // Verify ISlot<string> can be used as a type
    EXPECT_TRUE((std::is_abstract_v<ISlot<std::string>>));
    EXPECT_TRUE((std::has_virtual_destructor_v<ISlot<std::string>>));
}

TEST(InterfaceTest, SignalTemplateInstantiates)
{
    EXPECT_TRUE((std::is_abstract_v<ISignal<std::string>>));
    EXPECT_TRUE((std::has_virtual_destructor_v<ISignal<std::string>>));
}

TEST(InterfaceTest, CoreInterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<IDescription>);
    EXPECT_TRUE(std::is_abstract_v<IIdentity>);
    EXPECT_TRUE(std::is_abstract_v<IComponentInfo>);
    EXPECT_TRUE(std::is_abstract_v<IModelComponentInfo>);
    EXPECT_TRUE(std::is_abstract_v<IModelComponent>);
    EXPECT_TRUE(std::is_abstract_v<IArgument>);
    EXPECT_TRUE(std::is_abstract_v<IInput>);
    EXPECT_TRUE(std::is_abstract_v<IOutput>);
    EXPECT_TRUE(std::is_abstract_v<IAdaptedOutput>);
    EXPECT_TRUE(std::is_abstract_v<IMultiInput>);
}

TEST(InterfaceTest, CoreInterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<IDescription>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IIdentity>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IComponentInfo>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IModelComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IArgument>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IInput>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IOutput>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IAdaptedOutput>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IExchangeItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IWorkflowComponent>);
}

// ============================================================================
// Temporal interface tests
// ============================================================================

using namespace HydroCouple::Temporal;

TEST(TemporalInterfaceTest, InterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<IDateTime>);
    EXPECT_TRUE(std::is_abstract_v<ITimeSpan>);
    EXPECT_TRUE(std::is_abstract_v<ITimeModelComponent>);
    EXPECT_TRUE(std::is_abstract_v<ITimeSeriesComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ITimeIdBasedComponentDataItem>);
}

TEST(TemporalInterfaceTest, InterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<IDateTime>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeSpan>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeModelComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeSeriesComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeIdBasedComponentDataItem>);
}

// ============================================================================
// Spatial interface tests
// ============================================================================

TEST(SpatialInterfaceTest, InterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<IGeometry>);
    EXPECT_TRUE(std::is_abstract_v<IPoint>);
    EXPECT_TRUE(std::is_abstract_v<ILineString>);
    EXPECT_TRUE(std::is_abstract_v<IPolygon>);
    EXPECT_TRUE(std::is_abstract_v<IEnvelope>);
    EXPECT_TRUE(std::is_abstract_v<ISpatialReferenceSystem>);
    EXPECT_TRUE(std::is_abstract_v<INetwork>);
    EXPECT_TRUE(std::is_abstract_v<IPolyhedralSurface>);
    EXPECT_TRUE(std::is_abstract_v<IRaster>);
}

TEST(SpatialInterfaceTest, InterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<IGeometry>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IPoint>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ILineString>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IPolygon>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IEnvelope>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ISpatialReferenceSystem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IVertex>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IEdge>);
    EXPECT_TRUE(std::has_virtual_destructor_v<INetwork>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IPolyhedralSurface>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IRaster>);
}

// ============================================================================
// Spatiotemporal interface tests
// ============================================================================

using namespace HydroCouple::SpatioTemporal;

TEST(SpatioTemporalInterfaceTest, InterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<ITimeGeometryComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ITimeNetworkComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ITimeSeriesPolyhedralSurfaceComponentDataItem>);
}

TEST(SpatioTemporalInterfaceTest, InterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeGeometryComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeNetworkComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeSeriesPolyhedralSurfaceComponentDataItem>);
}
