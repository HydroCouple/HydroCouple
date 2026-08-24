/*!
 * \file test_hydrocouple.cpp
 * \brief Tests for HydroCouple core interface definitions, types, and type properties.
 */

#include <gtest/gtest.h>

#include "hydrocouple.h"
#include "hydrocouplehelpers.h"
#include "hydrocoupledistributed.h"
#include "hydrocoupletemporal.h"
#include "hydrocouplespatial.h"
#include "hydrocouplespatiotemporal.h"

#include <array>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

using namespace HydroCouple;
using namespace HydroCouple::Helpers;

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
// IModelComponent::ComponentStatus enum tests
// ============================================================================

TEST(ComponentStatusTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Created), 0);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Initializing), 1);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Initialized), 2);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Validating), 3);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Valid), 4);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::WaitingForData), 5);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Invalid), 6);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Preparing), 7);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Updating), 8);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Updated), 9);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Checkpointing), 10);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Done), 11);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Finishing), 12);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Finished), 13);
    EXPECT_EQ(static_cast<int>(IModelComponent::ComponentStatus::Failed), 14);
}

TEST(ComponentStatusTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IModelComponent::ComponentStatus>);
    EXPECT_FALSE((std::is_convertible_v<IModelComponent::ComponentStatus, int>));
}

// ============================================================================
// IArgument::ArgumentInputType enum tests
// ============================================================================

TEST(ArgumentInputTypeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IArgument::ArgumentInputType::String), 0);
    EXPECT_EQ(static_cast<int>(IArgument::ArgumentInputType::File), 1);
    EXPECT_EQ(static_cast<int>(IArgument::ArgumentInputType::JSON), 2);
    EXPECT_EQ(static_cast<int>(IArgument::ArgumentInputType::YAML), 3);
    EXPECT_EQ(static_cast<int>(IArgument::ArgumentInputType::XML), 4);
    EXPECT_EQ(static_cast<int>(IArgument::ArgumentInputType::URL), 5);
    EXPECT_EQ(static_cast<int>(IArgument::ArgumentInputType::MEMORY_OBJECT), 6);
}

TEST(ArgumentInputTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IArgument::ArgumentInputType>);
    EXPECT_FALSE((std::is_convertible_v<IArgument::ArgumentInputType, int>));
}

// ============================================================================
// IWorkflowComponent::WorkflowStatus enum tests
// ============================================================================

TEST(WorkflowStatusTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Created), 0);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Initializing), 1);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Initialized), 2);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Validating), 3);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Validated), 4);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Preparing), 5);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Prepared), 6);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Updating), 7);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Updated), 8);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Paused), 9);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Done), 10);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Finishing), 11);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Finished), 12);
    EXPECT_EQ(static_cast<int>(IWorkflowComponent::WorkflowStatus::Failed), 13);
}

TEST(WorkflowStatusTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IWorkflowComponent::WorkflowStatus>);
    EXPECT_FALSE((std::is_convertible_v<IWorkflowComponent::WorkflowStatus, int>));
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
    EXPECT_TRUE(std::is_enum_v<SpatialDataType>);
    EXPECT_FALSE((std::is_convertible_v<SpatialDataType, int>));
}

TEST(SpatialDataTypeTest, Values)
{
    EXPECT_EQ(static_cast<int>(SpatialDataType::Scalar), 0);
    EXPECT_EQ(static_cast<int>(SpatialDataType::MultiScalar), 1);
    EXPECT_EQ(static_cast<int>(SpatialDataType::Vector), 2);
    EXPECT_EQ(static_cast<int>(SpatialDataType::Tensor), 3);
}

// ============================================================================
// IGeometry::GeometryType enum tests
// ============================================================================

TEST(GeometryTypeTest, CoreValues)
{
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::Point), 1);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::LineString), 2);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::Polygon), 3);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::Triangle), 17);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPoint), 4);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiLineString), 5);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPolygon), 6);
}

TEST(GeometryTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IGeometry::GeometryType>);
    EXPECT_FALSE((std::is_convertible_v<IGeometry::GeometryType, int>));
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

// ============================================================================
// IDimension::LengthType enum tests
// ============================================================================

TEST(DimensionLengthTypeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IDimension::LengthType::Static), 0);
    EXPECT_EQ(static_cast<int>(IDimension::LengthType::Dynamic), 1);
}

TEST(DimensionLengthTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IDimension::LengthType>);
    EXPECT_FALSE((std::is_convertible_v<IDimension::LengthType, int>));
}

// ============================================================================
// IUnitDimensions::FundamentalUnitDimension enum tests
// ============================================================================

TEST(FundamentalUnitDimensionTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::Length), 0);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::Mass), 1);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::Time), 2);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::ElectricCurrent), 3);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::Temperature), 4);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::AmountOfSubstance), 5);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::LuminousIntensity), 6);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::Currency), 7);
    EXPECT_EQ(static_cast<int>(IUnitDimensions::FundamentalUnitDimension::Unitless), 8);
}

TEST(FundamentalUnitDimensionTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IUnitDimensions::FundamentalUnitDimension>);
    EXPECT_FALSE((std::is_convertible_v<IUnitDimensions::FundamentalUnitDimension, int>));
}

// ============================================================================
// IUnit::DistanceUnitType enum tests
// ============================================================================

TEST(DistanceUnitTypeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnitType::Standard), 0);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnitType::Geographic), 1);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnitType::Unknown), 2);
}

TEST(DistanceUnitTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IUnit::DistanceUnitType>);
    EXPECT_FALSE((std::is_convertible_v<IUnit::DistanceUnitType, int>));
}

// ============================================================================
// IUnit::DistanceUnits enum tests
// ============================================================================

TEST(DistanceUnitsTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Meters), 0);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Kilometers), 1);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Feet), 2);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::NauticalMiles), 3);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Yards), 4);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Miles), 5);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Degrees), 6);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Centimeters), 7);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Millimeters), 8);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Inches), 9);
    EXPECT_EQ(static_cast<int>(IUnit::DistanceUnits::Unknown), 10);
}

TEST(DistanceUnitsTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IUnit::DistanceUnits>);
    EXPECT_FALSE((std::is_convertible_v<IUnit::DistanceUnits, int>));
}

// ============================================================================
// IUnit::AreaUnits enum tests
// ============================================================================

TEST(AreaUnitsTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareMeters), 0);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareKilometers), 1);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareFeet), 2);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareYards), 3);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareMiles), 4);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::Hectares), 5);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::Acres), 6);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareNauticalMiles), 7);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareDegrees), 8);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareCentimeters), 9);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareMillimeters), 10);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::SquareInches), 11);
    EXPECT_EQ(static_cast<int>(IUnit::AreaUnits::Unknown), 12);
}

TEST(AreaUnitsTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IUnit::AreaUnits>);
    EXPECT_FALSE((std::is_convertible_v<IUnit::AreaUnits, int>));
}

// ============================================================================
// IGeometry::GeometryType Z/M/ZM variant enum tests
// ============================================================================

TEST(GeometryTypeTest, ZVariantValues)
{
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::GeometryZ), 1000);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PointZ), 1001);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::LineStringZ), 1002);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PolygonZ), 1003);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::TriangleZ), 1017);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPointZ), 1004);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiLineStringZ), 1005);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPolygonZ), 1006);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::GeometryCollectionZ), 1007);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CircularStringZ), 1008);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CompoundCurveZ), 1009);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CurvePolygonZ), 1010);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiCurveZ), 1011);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiSurfaceZ), 1012);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CurveZ), 1013);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::SurfaceZ), 1014);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PolyhedralSurfaceZ), 1015);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::TINZ), 1016);
}

TEST(GeometryTypeTest, MVariantValues)
{
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::GeometryM), 2000);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PointM), 2001);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::LineStringM), 2002);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PolygonM), 2003);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::TriangleM), 2017);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPointM), 2004);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiLineStringM), 2005);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPolygonM), 2006);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::GeometryCollectionM), 2007);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CircularStringM), 2008);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CompoundCurveM), 2009);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CurvePolygonM), 2010);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiCurveM), 2011);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiSurfaceM), 2012);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CurveM), 2013);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::SurfaceM), 2014);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PolyhedralSurfaceM), 2015);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::TINM), 2016);
}

TEST(GeometryTypeTest, ZMVariantValues)
{
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::GeometryZM), 3000);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PointZM), 3001);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::LineStringZM), 3002);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PolygonZM), 3003);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::TriangleZM), 3017);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPointZM), 3004);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiLineStringZM), 3005);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiPolygonZM), 3006);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::GeometryCollectionZM), 3007);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CircularStringZM), 3008);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CompoundCurveZM), 3009);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CurvePolygonZM), 3010);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiCurveZM), 3011);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiSurfaceZM), 3012);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CurveZM), 3013);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::SurfaceZM), 3014);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PolyhedralSurfaceZM), 3015);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::TINZM), 3016);
}

TEST(GeometryTypeTest, BaseVariantCompleteness)
{
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::Geometry), 0);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::GeometryCollection), 7);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CircularString), 8);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CompoundCurve), 9);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::CurvePolygon), 10);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiCurve), 11);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::MultiSurface), 12);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::Curve), 13);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::Surface), 14);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::PolyhedralSurface), 15);
    EXPECT_EQ(static_cast<int>(IGeometry::GeometryType::TIN), 16);
}

// ============================================================================
// Spatial::RegularGridType enum tests
// ============================================================================

TEST(RegularGridTypeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(Spatial::RegularGridType::Cartesian), 0);
    EXPECT_EQ(static_cast<int>(Spatial::RegularGridType::Rectilinear), 1);
    EXPECT_EQ(static_cast<int>(Spatial::RegularGridType::Curvilinear), 2);
}

TEST(RegularGridTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<Spatial::RegularGridType>);
    EXPECT_FALSE((std::is_convertible_v<Spatial::RegularGridType, int>));
}

// ============================================================================
// Spatial::NetworkDataObjectType enum tests
// ============================================================================

TEST(NetworkDataObjectTypeTest, Values)
{
    EXPECT_EQ(static_cast<int>(NetworkDataObjectType::Node), 0);
    EXPECT_EQ(static_cast<int>(NetworkDataObjectType::Edge), 1);
}

// ============================================================================
// IRaster::RasterDataType enum tests
// ============================================================================

TEST(RasterDataTypeTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::Unknown), 0);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::Byte), 1);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::UInt16), 2);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::Int16), 3);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::UInt32), 4);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::Int32), 5);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::Float32), 6);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::Float64), 7);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::CInt16), 8);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::CInt32), 9);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::CFloat32), 10);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::CFloat64), 11);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::ARGB32), 12);
    EXPECT_EQ(static_cast<int>(IRaster::RasterDataType::ARGB32_Premultiplied), 13);
}

TEST(RasterDataTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<IRaster::RasterDataType>);
    EXPECT_FALSE((std::is_convertible_v<IRaster::RasterDataType, int>));
}

// ============================================================================
// Missing core interface abstract / virtual destructor tests
// ============================================================================

TEST(InterfaceTest, AdditionalCoreInterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<IPropertyChanged>);
    EXPECT_TRUE(std::is_abstract_v<IValueDefinition>);
    EXPECT_TRUE(std::is_abstract_v<IQuality>);
    EXPECT_TRUE(std::is_abstract_v<IQuantity>);
    EXPECT_TRUE(std::is_abstract_v<IUnit>);
    EXPECT_TRUE(std::is_abstract_v<IUnitDimensions>);
    EXPECT_TRUE(std::is_abstract_v<IDimension>);
    EXPECT_TRUE(std::is_abstract_v<IComponentStatusChangeEventArgs>);
    EXPECT_TRUE(std::is_abstract_v<IExchangeItemChangeEventArgs>);
    EXPECT_TRUE(std::is_abstract_v<IComponentDataItemValueChanged>);
    EXPECT_TRUE(std::is_abstract_v<ICloneableModelComponent>);
    EXPECT_TRUE(std::is_abstract_v<IAdaptedOutputFactory>);
    EXPECT_TRUE(std::is_abstract_v<IAdaptedOutputFactoryComponent>);
    EXPECT_TRUE(std::is_abstract_v<IAdaptedOutputFactoryComponentInfo>);
    EXPECT_TRUE(std::is_abstract_v<IIdBasedComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ILicensedComponent>);
    EXPECT_TRUE(std::is_abstract_v<IUIProvider>);
    EXPECT_TRUE(std::is_abstract_v<ICheckpointableModelComponent>);
    EXPECT_TRUE(std::is_abstract_v<IWorkflowComponent>);
    EXPECT_TRUE(std::is_abstract_v<IWorkflowComponentInfo>);
    EXPECT_TRUE(std::is_abstract_v<IWorkflowComponentStatusChangeEventArgs>);
    EXPECT_TRUE(std::is_abstract_v<IModelComponentInfo>);
}

TEST(InterfaceTest, AdditionalCoreInterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<IPropertyChanged>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IValueDefinition>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IQuality>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IQuantity>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IUnit>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IUnitDimensions>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IDimension>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IComponentStatusChangeEventArgs>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IExchangeItemChangeEventArgs>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IComponentDataItemValueChanged>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ICloneableModelComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IAdaptedOutputFactory>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IAdaptedOutputFactoryComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IAdaptedOutputFactoryComponentInfo>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IIdBasedComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ILicensedComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IUIProvider>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ICheckpointableModelComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IWorkflowComponentInfo>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IWorkflowComponentStatusChangeEventArgs>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IModelComponentInfo>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IMultiInput>);
}

// ============================================================================
// Missing spatial interface abstract / virtual destructor tests
// ============================================================================

TEST(SpatialInterfaceTest, AdditionalInterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<IGeometryCollection>);
    EXPECT_TRUE(std::is_abstract_v<IMultiPoint>);
    EXPECT_TRUE(std::is_abstract_v<ICurve>);
    EXPECT_TRUE(std::is_abstract_v<IMultiCurve>);
    EXPECT_TRUE(std::is_abstract_v<IMultiLineString>);
    EXPECT_TRUE(std::is_abstract_v<ILine>);
    EXPECT_TRUE(std::is_abstract_v<ILinearRing>);
    EXPECT_TRUE(std::is_abstract_v<ISurface>);
    EXPECT_TRUE(std::is_abstract_v<IMultiSurface>);
    EXPECT_TRUE(std::is_abstract_v<IMultiPolygon>);
    EXPECT_TRUE(std::is_abstract_v<ITriangle>);
    EXPECT_TRUE(std::is_abstract_v<ITIN>);
    EXPECT_TRUE(std::is_abstract_v<IRasterBand>);
    EXPECT_TRUE(std::is_abstract_v<IRegularGrid2D>);
    EXPECT_TRUE(std::is_abstract_v<IRegularGrid3D>);
    EXPECT_TRUE(std::is_abstract_v<IGeometryComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<INetworkComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<IPolyhedralSurfaceComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ITINComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<IRasterComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<IRegularGrid2DComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<IRegularGrid3DComponentDataItem>);
}

TEST(SpatialInterfaceTest, AdditionalInterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<IGeometryCollection>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IMultiPoint>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ICurve>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IMultiCurve>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IMultiLineString>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ILine>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ILinearRing>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ISurface>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IMultiSurface>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IMultiPolygon>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITriangle>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITIN>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IRasterBand>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IRegularGrid2D>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IRegularGrid3D>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IGeometryComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<INetworkComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IPolyhedralSurfaceComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITINComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IRasterComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IRegularGrid2DComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IRegularGrid3DComponentDataItem>);
}

// ============================================================================
// Missing spatiotemporal interface abstract / virtual destructor tests
// ============================================================================

TEST(SpatioTemporalInterfaceTest, AdditionalInterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<ITimeSeriesTINComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ITimeSeriesRasterComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ITimeRegularGrid2DComponentDataItem>);
    EXPECT_TRUE(std::is_abstract_v<ITimeRegularGrid3DComponentDataItem>);
}

TEST(SpatioTemporalInterfaceTest, AdditionalInterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeSeriesTINComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeSeriesRasterComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeRegularGrid2DComponentDataItem>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITimeRegularGrid3DComponentDataItem>);
}

// ============================================================================
// Inheritance relationship tests (is_base_of_v)
// ============================================================================

TEST(InheritanceTest, CoreInheritanceChain)
{
    // IPropertyChanged -> IDescription -> IIdentity
    EXPECT_TRUE((std::is_base_of_v<IPropertyChanged, IDescription>));
    EXPECT_TRUE((std::is_base_of_v<IDescription, IIdentity>));

    // IIdentity -> IComponentInfo -> IModelComponentInfo
    EXPECT_TRUE((std::is_base_of_v<IIdentity, IComponentInfo>));
    EXPECT_TRUE((std::is_base_of_v<IComponentInfo, IModelComponentInfo>));

    // IIdentity -> IModelComponent -> ICloneableModelComponent / ICheckpointableModelComponent
    EXPECT_TRUE((std::is_base_of_v<IIdentity, IModelComponent>));
    EXPECT_TRUE((std::is_base_of_v<IModelComponent, ICloneableModelComponent>));
    EXPECT_TRUE((std::is_base_of_v<IModelComponent, ICheckpointableModelComponent>));

    // IDescription -> IValueDefinition -> IQuality / IQuantity
    EXPECT_TRUE((std::is_base_of_v<IDescription, IValueDefinition>));
    EXPECT_TRUE((std::is_base_of_v<IValueDefinition, IQuality>));
    EXPECT_TRUE((std::is_base_of_v<IValueDefinition, IQuantity>));

    // IDescription -> IUnitDimensions, IUnit
    EXPECT_TRUE((std::is_base_of_v<IDescription, IUnitDimensions>));
    EXPECT_TRUE((std::is_base_of_v<IDescription, IUnit>));

    // IIdentity -> IDimension
    EXPECT_TRUE((std::is_base_of_v<IIdentity, IDimension>));
}

TEST(InheritanceTest, DataItemChain)
{
    // IIdentity -> IComponentDataItem -> IArgument
    EXPECT_TRUE((std::is_base_of_v<IIdentity, IComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IArgument>));

    // IComponentDataItem -> IExchangeItem -> IOutput / IInput
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IExchangeItem>));
    EXPECT_TRUE((std::is_base_of_v<IExchangeItem, IOutput>));
    EXPECT_TRUE((std::is_base_of_v<IExchangeItem, IInput>));

    // IOutput -> IAdaptedOutput
    EXPECT_TRUE((std::is_base_of_v<IOutput, IAdaptedOutput>));

    // IInput -> IMultiInput
    EXPECT_TRUE((std::is_base_of_v<IInput, IMultiInput>));

    // IComponentDataItem -> IIdBasedComponentDataItem
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IIdBasedComponentDataItem>));

    // IAdaptedOutputFactory -> IAdaptedOutputFactoryComponent
    EXPECT_TRUE((std::is_base_of_v<IIdentity, IAdaptedOutputFactory>));
    EXPECT_TRUE((std::is_base_of_v<IAdaptedOutputFactory, IAdaptedOutputFactoryComponent>));

    // IComponentInfo -> IAdaptedOutputFactoryComponentInfo / IWorkflowComponentInfo
    EXPECT_TRUE((std::is_base_of_v<IComponentInfo, IAdaptedOutputFactoryComponentInfo>));
    EXPECT_TRUE((std::is_base_of_v<IComponentInfo, IWorkflowComponentInfo>));
}

TEST(InheritanceTest, SpatialGeometryHierarchy)
{
    // IGeometry -> IPoint -> IVertex
    EXPECT_TRUE((std::is_base_of_v<IGeometry, IPoint>));
    EXPECT_TRUE((std::is_base_of_v<IPoint, IVertex>));

    // IGeometry -> IGeometryCollection -> IMultiPoint
    EXPECT_TRUE((std::is_base_of_v<IGeometry, IGeometryCollection>));
    EXPECT_TRUE((std::is_base_of_v<IGeometryCollection, IMultiPoint>));

    // IGeometry -> ICurve -> ILineString -> ILine, ILinearRing
    EXPECT_TRUE((std::is_base_of_v<IGeometry, ICurve>));
    EXPECT_TRUE((std::is_base_of_v<ICurve, ILineString>));
    EXPECT_TRUE((std::is_base_of_v<ILineString, ILine>));
    EXPECT_TRUE((std::is_base_of_v<ILineString, ILinearRing>));

    // IGeometryCollection -> IMultiCurve -> IMultiLineString
    EXPECT_TRUE((std::is_base_of_v<IGeometryCollection, IMultiCurve>));
    EXPECT_TRUE((std::is_base_of_v<IMultiCurve, IMultiLineString>));

    // IGeometry -> ISurface -> IPolygon -> ITriangle
    EXPECT_TRUE((std::is_base_of_v<IGeometry, ISurface>));
    EXPECT_TRUE((std::is_base_of_v<ISurface, IPolygon>));
    EXPECT_TRUE((std::is_base_of_v<IPolygon, ITriangle>));

    // IGeometryCollection -> IMultiSurface -> IMultiPolygon
    EXPECT_TRUE((std::is_base_of_v<IGeometryCollection, IMultiSurface>));
    EXPECT_TRUE((std::is_base_of_v<IMultiSurface, IMultiPolygon>));

    // ISurface -> IPolyhedralSurface -> ITIN
    EXPECT_TRUE((std::is_base_of_v<ISurface, IPolyhedralSurface>));
    EXPECT_TRUE((std::is_base_of_v<IPolyhedralSurface, ITIN>));
}

TEST(InheritanceTest, SpatialComponentDataItemChains)
{
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IGeometryComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, INetworkComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IPolyhedralSurfaceComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IRasterComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IRegularGrid2DComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IRegularGrid3DComponentDataItem>));
}

TEST(InheritanceTest, SpatioTemporalMultipleInheritance)
{
    // ITimeGeometryComponentDataItem inherits from both temporal and spatial
    EXPECT_TRUE((std::is_base_of_v<HydroCouple::Temporal::ITimeSeriesComponentDataItem, ITimeGeometryComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<Spatial::IGeometryComponentDataItem, ITimeGeometryComponentDataItem>));

    EXPECT_TRUE((std::is_base_of_v<HydroCouple::Temporal::ITimeSeriesComponentDataItem, ITimeNetworkComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<Spatial::INetworkComponentDataItem, ITimeNetworkComponentDataItem>));

    EXPECT_TRUE((std::is_base_of_v<HydroCouple::Temporal::ITimeSeriesComponentDataItem, ITimeSeriesPolyhedralSurfaceComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<Spatial::IPolyhedralSurfaceComponentDataItem, ITimeSeriesPolyhedralSurfaceComponentDataItem>));

    EXPECT_TRUE((std::is_base_of_v<ITimeSeriesPolyhedralSurfaceComponentDataItem, ITimeSeriesTINComponentDataItem>));

    EXPECT_TRUE((std::is_base_of_v<HydroCouple::Temporal::ITimeSeriesComponentDataItem, ITimeSeriesRasterComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<Spatial::IRasterComponentDataItem, ITimeSeriesRasterComponentDataItem>));

    EXPECT_TRUE((std::is_base_of_v<HydroCouple::Temporal::ITimeSeriesComponentDataItem, ITimeRegularGrid2DComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<Spatial::IRegularGrid2DComponentDataItem, ITimeRegularGrid2DComponentDataItem>));

    EXPECT_TRUE((std::is_base_of_v<HydroCouple::Temporal::ITimeSeriesComponentDataItem, ITimeRegularGrid3DComponentDataItem>));
    EXPECT_TRUE((std::is_base_of_v<Spatial::IRegularGrid3DComponentDataItem, ITimeRegularGrid3DComponentDataItem>));
}

// ============================================================================
// Distributed interface tests
// ============================================================================

using namespace HydroCouple::Distributed;

TEST(DistributedInterfaceTest, InterfacesAreAbstract)
{
    EXPECT_TRUE(std::is_abstract_v<IExchangeRequest>);
    EXPECT_TRUE(std::is_abstract_v<ITransport>);
    EXPECT_TRUE(std::is_abstract_v<IDistributedModelComponent>);
    EXPECT_TRUE(std::is_abstract_v<IProxyModelComponent>);
    EXPECT_TRUE(std::is_abstract_v<IPartitionedComponentDataItem>);
}

TEST(DistributedInterfaceTest, InterfacesHaveVirtualDestructors)
{
    EXPECT_TRUE(std::has_virtual_destructor_v<IExchangeRequest>);
    EXPECT_TRUE(std::has_virtual_destructor_v<ITransport>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IDistributedModelComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IProxyModelComponent>);
    EXPECT_TRUE(std::has_virtual_destructor_v<IPartitionedComponentDataItem>);
}

TEST(DistributedInterfaceTest, InheritanceChain)
{
    EXPECT_TRUE((std::is_base_of_v<IModelComponent, IDistributedModelComponent>));
    EXPECT_TRUE((std::is_base_of_v<IDistributedModelComponent, IProxyModelComponent>));
    EXPECT_TRUE((std::is_base_of_v<IModelComponent, IProxyModelComponent>));
    EXPECT_TRUE((std::is_base_of_v<IComponentDataItem, IPartitionedComponentDataItem>));
}

TEST(DistributedInterfaceTest, EndpointDefaults)
{
    ITransport::Endpoint e;
    EXPECT_TRUE(e.address.empty());
    EXPECT_EQ(e.rank, -1);
}

// ============================================================================
// IMeshView tests
// ============================================================================

TEST(MeshViewTest, IsAbstractWithVirtualDestructor)
{
    EXPECT_TRUE(std::is_abstract_v<Spatial::IMeshView>);
    EXPECT_TRUE(std::has_virtual_destructor_v<Spatial::IMeshView>);
}

// ============================================================================
// Hyperslab parameter acceptance tests over the typed data plane
// ============================================================================

// Minimal concrete implementation of IComponentDataItem capturing hyperslab
// selections for verification.
class StubComponentDataItem final : public IComponentDataItem
{
public:
    mutable std::vector<int64_t> lastStart;
    mutable std::vector<int64_t> lastCount;
    mutable DataKind lastBufferKind = DataKind::Unknown;
    mutable void *lastBufferData = nullptr;

    std::vector<int64_t> shape() const override { return {4, 5}; }
    DataKind dataKind() const override { return DataKind::Float64; }

    bool getValuesInto(const BufferDescriptor &destination,
                       std::span<const int64_t> start,
                       std::span<const int64_t> count,
                       std::string * = nullptr) const override
    {
        lastStart.assign(start.begin(), start.end());
        lastCount.assign(count.begin(), count.end());
        lastBufferKind = destination.kind;
        lastBufferData = destination.data;
        return true;
    }

    bool setValuesFrom(const BufferDescriptor &source,
                       std::span<const int64_t> start,
                       std::span<const int64_t> count,
                       std::string * = nullptr) override
    {
        lastStart.assign(start.begin(), start.end());
        lastCount.assign(count.begin(), count.end());
        lastBufferKind = source.kind;
        lastBufferData = source.data;
        return true;
    }

    // Stubs for remaining pure virtuals
    IModelComponent *modelComponent() const override { return nullptr; }
    std::vector<IDimension *> dimensions() const override { return {}; }
    IValueDefinition *valueDefinition() const override { return nullptr; }

    // IIdentity / IDescription stubs
    const std::string &id() const override { static std::string s; return s; }
    const std::string &caption() const override { static std::string s; return s; }
    void setCaption(const std::string &) override {}
    const std::string &description() const override { static std::string s; return s; }
    void setDescription(const std::string &) override {}

    // ISignal / ISlot stubs
    void connect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<const std::shared_ptr<IComponentDataItemValueChanged> &>> &) override {}
    void blockSignals(bool) override {}
    void emit(const std::shared_ptr<IComponentDataItemValueChanged> &) override {}

    // IPropertyChanged stubs
    void connect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void disconnect(const std::shared_ptr<ISlot<std::string>> &) override {}
    void emit(std::string) override {}
};

TEST(HyperslabParameterTest, AcceptsStdArraySelections)
{
    StubComponentDataItem stub;
    std::array<int64_t, 2> start = {1, 2};
    std::array<int64_t, 2> count = {2, 3};
    double buffer[6] = {};

    EXPECT_TRUE(stub.getValuesInto(
        makeContiguous(buffer, DataKind::Float64, count), start, count));
    ASSERT_EQ(stub.lastStart.size(), 2u);
    EXPECT_EQ(stub.lastStart[0], 1);
    EXPECT_EQ(stub.lastStart[1], 2);
    ASSERT_EQ(stub.lastCount.size(), 2u);
    EXPECT_EQ(stub.lastCount[0], 2);
    EXPECT_EQ(stub.lastCount[1], 3);
    EXPECT_EQ(stub.lastBufferKind, DataKind::Float64);
    EXPECT_EQ(stub.lastBufferData, buffer);
}

TEST(HyperslabParameterTest, AcceptsStdVectorSelections)
{
    StubComponentDataItem stub;
    std::vector<int64_t> start = {0, 0};
    std::vector<int64_t> count = {4, 5};
    std::vector<double> buffer(20);

    EXPECT_TRUE(stub.setValuesFrom(
        makeContiguous(buffer.data(), DataKind::Float64, count), start, count));
    EXPECT_EQ(stub.lastStart, start);
    EXPECT_EQ(stub.lastCount, count);
}

TEST(HyperslabParameterTest, ScalarTemplateHelperRoutesThroughBulkPath)
{
    StubComponentDataItem stub;
    double value = 0.0;
    std::array<int64_t, 2> index = {3, 4};

    EXPECT_TRUE(getValue(stub, value, index));
    ASSERT_EQ(stub.lastStart.size(), 2u);
    EXPECT_EQ(stub.lastStart[0], 3);
    EXPECT_EQ(stub.lastStart[1], 4);
    ASSERT_EQ(stub.lastCount.size(), 2u);
    EXPECT_EQ(stub.lastCount[0], 1);
    EXPECT_EQ(stub.lastCount[1], 1);
    EXPECT_EQ(stub.lastBufferKind, DataKind::Float64);
    EXPECT_EQ(stub.lastBufferData, &value);

    const double toWrite = 9.5;
    EXPECT_TRUE(setValue(stub, toWrite, index));
    EXPECT_EQ(stub.lastBufferKind, DataKind::Float64);
}

TEST(HyperslabParameterTest, SpanTemplateHelperRoutesThroughBulkPath)
{
    StubComponentDataItem stub;
    std::vector<double> dest(6);
    std::array<int64_t, 2> start = {0, 1};
    std::array<int64_t, 2> count = {2, 3};

    EXPECT_TRUE(getValues(stub, std::span<double>(dest), start, count));
    EXPECT_EQ(stub.lastBufferKind, DataKind::Float64);
    EXPECT_EQ(stub.lastBufferData, dest.data());

    std::vector<double> src(6, 1.5);
    EXPECT_TRUE(setValues(stub, std::span<const double>(src), start, count));
    EXPECT_EQ(stub.lastBufferData, const_cast<double *>(src.data()));
}
