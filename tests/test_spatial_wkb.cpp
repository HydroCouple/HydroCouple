/*!
 * \file test_spatial_wkb.cpp
 * \brief Tests for WKB data structures, SoA coordinate buffers, and data-oriented type properties.
 */

#include <gtest/gtest.h>

#include "hydrocouplespatialwkb.h"

#include <cstring>
#include <type_traits>
#include <vector>

// ============================================================================
// WKBByteOrder enum class tests
// ============================================================================

TEST(WKBByteOrderTest, Values)
{
    EXPECT_EQ(static_cast<uint8_t>(WKBByteOrder::wkbXDR), 0);
    EXPECT_EQ(static_cast<uint8_t>(WKBByteOrder::wkbNDR), 1);
}

TEST(WKBByteOrderTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<WKBByteOrder>);
    // enum class should NOT be implicitly convertible to int
    EXPECT_FALSE((std::is_convertible_v<WKBByteOrder, int>));
}

TEST(WKBByteOrderTest, UnderlyingType)
{
    EXPECT_TRUE((std::is_same_v<std::underlying_type_t<WKBByteOrder>, uint8_t>));
}

// ============================================================================
// WKBGeometryType enum class tests
// ============================================================================

TEST(WKBGeometryTypeTest, IsEnumClass)
{
    EXPECT_TRUE(std::is_enum_v<WKBGeometryType>);
    EXPECT_FALSE((std::is_convertible_v<WKBGeometryType, int>));
}

TEST(WKBGeometryTypeTest, UnderlyingType)
{
    EXPECT_TRUE((std::is_same_v<std::underlying_type_t<WKBGeometryType>, uint32_t>));
}

TEST(WKBGeometryTypeTest, BaseTypeValues)
{
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPoint), 1u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbLineString), 2u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPolygon), 3u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbMultiPoint), 4u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbMultiLineString), 5u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbMultiPolygon), 6u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbGeometryCollection), 7u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPolyhedralSurface), 15u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbTIN), 16u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbTriangle), 17u);
}

TEST(WKBGeometryTypeTest, ZTypeValues)
{
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPointZ), 1001u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbLineStringZ), 1002u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPolygonZ), 1003u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbTriangleZ), 1017u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPolyhedralSurfaceZ), 1015u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbTINZ), 1016u);
}

TEST(WKBGeometryTypeTest, MTypeValues)
{
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPointM), 2001u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbLineStringM), 2002u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPolygonM), 2003u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbTriangleM), 2017u);
}

TEST(WKBGeometryTypeTest, ZMTypeValues)
{
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPointZM), 3001u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbLineStringZM), 3002u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbPolygonZM), 3003u);
    EXPECT_EQ(static_cast<uint32_t>(WKBGeometryType::wkbTriangleZM), 3017u);
}

// ============================================================================
// Point struct type property tests
// ============================================================================

TEST(PointTest, Layout)
{
    EXPECT_EQ(sizeof(Point), 2 * sizeof(double));
    EXPECT_TRUE(std::is_trivially_copyable_v<Point>);
    EXPECT_TRUE(std::is_standard_layout_v<Point>);
    EXPECT_TRUE(std::is_trivially_default_constructible_v<Point>);
}

TEST(PointZTest, Layout)
{
    EXPECT_EQ(sizeof(PointZ), 3 * sizeof(double));
    EXPECT_TRUE(std::is_trivially_copyable_v<PointZ>);
    EXPECT_TRUE(std::is_standard_layout_v<PointZ>);
}

TEST(PointMTest, Layout)
{
    EXPECT_EQ(sizeof(PointM), 3 * sizeof(double));
    EXPECT_TRUE(std::is_trivially_copyable_v<PointM>);
    EXPECT_TRUE(std::is_standard_layout_v<PointM>);
}

TEST(PointZMTest, Layout)
{
    EXPECT_EQ(sizeof(PointZM), 4 * sizeof(double));
    EXPECT_TRUE(std::is_trivially_copyable_v<PointZM>);
    EXPECT_TRUE(std::is_standard_layout_v<PointZM>);
}

TEST(PointTest, FieldAccess)
{
    Point p{1.0, 2.0};
    EXPECT_DOUBLE_EQ(p.x, 1.0);
    EXPECT_DOUBLE_EQ(p.y, 2.0);
}

TEST(PointZTest, FieldAccess)
{
    PointZ p{1.0, 2.0, 3.0};
    EXPECT_DOUBLE_EQ(p.x, 1.0);
    EXPECT_DOUBLE_EQ(p.y, 2.0);
    EXPECT_DOUBLE_EQ(p.z, 3.0);
}

TEST(PointZMTest, FieldAccess)
{
    PointZM p{1.0, 2.0, 3.0, 4.0};
    EXPECT_DOUBLE_EQ(p.x, 1.0);
    EXPECT_DOUBLE_EQ(p.y, 2.0);
    EXPECT_DOUBLE_EQ(p.z, 3.0);
    EXPECT_DOUBLE_EQ(p.m, 4.0);
}

TEST(PointTest, ContiguousArrayLayout)
{
    // Verify Point arrays can be treated as contiguous double arrays (AoS pattern)
    Point points[3] = {{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};
    const double *raw = reinterpret_cast<const double *>(points);
    EXPECT_DOUBLE_EQ(raw[0], 1.0); // points[0].x
    EXPECT_DOUBLE_EQ(raw[1], 2.0); // points[0].y
    EXPECT_DOUBLE_EQ(raw[2], 3.0); // points[1].x
    EXPECT_DOUBLE_EQ(raw[3], 4.0); // points[1].y
    EXPECT_DOUBLE_EQ(raw[4], 5.0); // points[2].x
    EXPECT_DOUBLE_EQ(raw[5], 6.0); // points[2].y
}

// ============================================================================
// LinearRing struct tests
// ============================================================================

TEST(LinearRingTest, TypeProperties)
{
    EXPECT_TRUE(std::is_trivially_copyable_v<LinearRing>);
    EXPECT_TRUE(std::is_standard_layout_v<LinearRing>);
}

TEST(LinearRingTest, FieldAccess)
{
    Point pts[3] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}};
    LinearRing ring{3, pts};
    EXPECT_EQ(ring.numPoints, 3u);
    EXPECT_DOUBLE_EQ(ring.points[0].x, 0.0);
    EXPECT_DOUBLE_EQ(ring.points[1].x, 1.0);
    EXPECT_DOUBLE_EQ(ring.points[2].y, 1.0);
}

// ============================================================================
// WKB struct constexpr type tag tests
// ============================================================================

TEST(WKBTypeTagTest, PointTypes)
{
    EXPECT_EQ(WKBPoint::wkbType, 1u);
    EXPECT_EQ(WKBPointZ::wkbType, 1001u);
    EXPECT_EQ(WKBPointM::wkbType, 2001u);
    EXPECT_EQ(WKBPointZM::wkbType, 3001u);
}

TEST(WKBTypeTagTest, LineStringTypes)
{
    EXPECT_EQ(WKBLineString::wkbType, 2u);
    EXPECT_EQ(WKBLineStringZ::wkbType, 1002u);
    EXPECT_EQ(WKBLineStringM::wkbType, 2002u);
    EXPECT_EQ(WKBLineStringZM::wkbType, 3002u);
}

TEST(WKBTypeTagTest, PolygonTypes)
{
    EXPECT_EQ(WKBPolygon::wkbType, 3u);
    EXPECT_EQ(WKBPolygonZ::wkbType, 1003u);
    EXPECT_EQ(WKBPolygonM::wkbType, 2003u);
    EXPECT_EQ(WKBPolygonZM::wkbType, 3003u);
}

TEST(WKBTypeTagTest, TriangleTypes)
{
    EXPECT_EQ(WKBTriangle::wkbType, 17u);
    EXPECT_EQ(WKBTriangleZ::wkbType, 1017u);
    EXPECT_EQ(WKBTriangleM::wkbType, 2017u);
    EXPECT_EQ(WKBTriangleZM::wkbType, 3017u);
}

TEST(WKBTypeTagTest, PolyhedralSurfaceTypes)
{
    EXPECT_EQ(WKBPolyhedralSurface::wkbType, 15u);
    EXPECT_EQ(WKBPolyhedralSurfaceZ::wkbType, 1015u);
    EXPECT_EQ(WKBPolyhedralSurfaceM::wkbType, 2015u);
    EXPECT_EQ(WKBPolyhedralSurfaceZM::wkbType, 3015u);
}

TEST(WKBTypeTagTest, TINTypes)
{
    EXPECT_EQ(WKBTIN::wkbType, 16u);
    EXPECT_EQ(WKBTINZ::wkbType, 1016u);
    EXPECT_EQ(WKBTINM::wkbType, 2016u);
    EXPECT_EQ(WKBTINZM::wkbType, 3016u);
}

TEST(WKBTypeTagTest, MultiPointTypes)
{
    EXPECT_EQ(WKBMultiPoint::wkbType, 4u);
    EXPECT_EQ(WKBMultiPointZ::wkbType, 1004u);
    EXPECT_EQ(WKBMultiPointM::wkbType, 2004u);
    EXPECT_EQ(WKBMultiPointZM::wkbType, 3004u);
}

TEST(WKBTypeTagTest, MultiLineStringTypes)
{
    EXPECT_EQ(WKBMultiLineString::wkbType, 5u);
    EXPECT_EQ(WKBMultiLineStringZ::wkbType, 1005u);
    EXPECT_EQ(WKBMultiLineStringM::wkbType, 2005u);
    EXPECT_EQ(WKBMultiLineStringZM::wkbType, 3005u);
}

TEST(WKBTypeTagTest, MultiPolygonTypes)
{
    EXPECT_EQ(WKBMultiPolygon::wkbType, 6u);
    EXPECT_EQ(WKBMultiPolygonZ::wkbType, 1006u);
    EXPECT_EQ(WKBMultiPolygonM::wkbType, 2006u);
    EXPECT_EQ(WKBMultiPolygonZM::wkbType, 3006u);
}

TEST(WKBTypeTagTest, GeometryCollectionTypes)
{
    EXPECT_EQ(WKBGeometryCollection::wkbType, 7u);
    EXPECT_EQ(WKBGeometryCollectionZ::wkbType, 1007u);
    EXPECT_EQ(WKBGeometryCollectionM::wkbType, 2007u);
    EXPECT_EQ(WKBGeometryCollectionZM::wkbType, 3007u);
}

TEST(WKBTypeTagTest, TypeTagsAreConstexpr)
{
    // These must be usable in constexpr contexts
    constexpr uint32_t pt = WKBPoint::wkbType;
    constexpr uint32_t ls = WKBLineString::wkbType;
    constexpr uint32_t pg = WKBPolygon::wkbType;
    EXPECT_EQ(pt, 1u);
    EXPECT_EQ(ls, 2u);
    EXPECT_EQ(pg, 3u);
}

// ============================================================================
// WKB struct data access tests
// ============================================================================

TEST(WKBPointTest, DataAccess)
{
    WKBPoint wkb;
    wkb.byteOrder = WKBByteOrder::wkbNDR;
    wkb.point = {1.5, 2.5};

    EXPECT_EQ(wkb.byteOrder, WKBByteOrder::wkbNDR);
    EXPECT_DOUBLE_EQ(wkb.point.x, 1.5);
    EXPECT_DOUBLE_EQ(wkb.point.y, 2.5);
}

TEST(WKBLineStringTest, DataAccess)
{
    Point pts[2] = {{0.0, 0.0}, {1.0, 1.0}};
    WKBLineString wkb;
    wkb.byteOrder = WKBByteOrder::wkbXDR;
    wkb.numPoints = 2;
    wkb.points = pts;

    EXPECT_EQ(wkb.byteOrder, WKBByteOrder::wkbXDR);
    EXPECT_EQ(wkb.numPoints, 2u);
    EXPECT_DOUBLE_EQ(wkb.points[0].x, 0.0);
    EXPECT_DOUBLE_EQ(wkb.points[1].y, 1.0);
}

TEST(WKBPolygonTest, DataAccess)
{
    Point pts[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {0.0, 0.0}};
    LinearRing ring{4, pts};
    WKBPolygon wkb;
    wkb.byteOrder = WKBByteOrder::wkbNDR;
    wkb.numRings = 1;
    wkb.rings = &ring;

    EXPECT_EQ(wkb.numRings, 1u);
    EXPECT_EQ(wkb.rings[0].numPoints, 4u);
    EXPECT_DOUBLE_EQ(wkb.rings[0].points[2].y, 1.0);
}

// ============================================================================
// SoA Coordinate Buffer tests
// ============================================================================

TEST(CoordinateBuffer2DTest, TypeProperties)
{
    EXPECT_TRUE(std::is_trivially_copyable_v<CoordinateBuffer2D>);
    EXPECT_TRUE(std::is_standard_layout_v<CoordinateBuffer2D>);
    EXPECT_TRUE(std::is_trivially_default_constructible_v<CoordinateBuffer2D>);
}

TEST(CoordinateBuffer2DTest, SoAAccess)
{
    std::vector<double> xs = {1.0, 2.0, 3.0, 4.0};
    std::vector<double> ys = {5.0, 6.0, 7.0, 8.0};
    CoordinateBuffer2D buf{xs.data(), ys.data(), 4};

    EXPECT_EQ(buf.count, 4u);
    for (uint32_t i = 0; i < buf.count; i++)
    {
        EXPECT_DOUBLE_EQ(buf.x[i], static_cast<double>(i + 1));
        EXPECT_DOUBLE_EQ(buf.y[i], static_cast<double>(i + 5));
    }
}

TEST(CoordinateBuffer3DTest, TypeProperties)
{
    EXPECT_TRUE(std::is_trivially_copyable_v<CoordinateBuffer3D>);
    EXPECT_TRUE(std::is_standard_layout_v<CoordinateBuffer3D>);
}

TEST(CoordinateBuffer3DTest, SoAAccess)
{
    std::vector<double> xs = {1.0, 2.0, 3.0};
    std::vector<double> ys = {4.0, 5.0, 6.0};
    std::vector<double> zs = {7.0, 8.0, 9.0};
    CoordinateBuffer3D buf{xs.data(), ys.data(), zs.data(), 3};

    EXPECT_EQ(buf.count, 3u);
    EXPECT_DOUBLE_EQ(buf.x[0], 1.0);
    EXPECT_DOUBLE_EQ(buf.y[1], 5.0);
    EXPECT_DOUBLE_EQ(buf.z[2], 9.0);
}

TEST(CoordinateBufferMTest, TypeProperties)
{
    EXPECT_TRUE(std::is_trivially_copyable_v<CoordinateBufferM>);
    EXPECT_TRUE(std::is_standard_layout_v<CoordinateBufferM>);
}

TEST(CoordinateBuffer3DMTest, TypeProperties)
{
    EXPECT_TRUE(std::is_trivially_copyable_v<CoordinateBuffer3DM>);
    EXPECT_TRUE(std::is_standard_layout_v<CoordinateBuffer3DM>);
}

TEST(CoordinateBuffer3DMTest, SoAAccess)
{
    std::vector<double> xs = {1.0, 2.0};
    std::vector<double> ys = {3.0, 4.0};
    std::vector<double> zs = {5.0, 6.0};
    std::vector<double> ms = {7.0, 8.0};
    CoordinateBuffer3DM buf{xs.data(), ys.data(), zs.data(), ms.data(), 2};

    EXPECT_EQ(buf.count, 2u);
    EXPECT_DOUBLE_EQ(buf.x[0], 1.0);
    EXPECT_DOUBLE_EQ(buf.y[0], 3.0);
    EXPECT_DOUBLE_EQ(buf.z[0], 5.0);
    EXPECT_DOUBLE_EQ(buf.m[0], 7.0);
    EXPECT_DOUBLE_EQ(buf.x[1], 2.0);
    EXPECT_DOUBLE_EQ(buf.m[1], 8.0);
}

TEST(CoordinateBuffer2DTest, SoAVsAoSIterationPattern)
{
    // Demonstrate that SoA enables per-component iteration
    // which is more cache-friendly than AoS for single-component access
    constexpr uint32_t N = 100;
    std::vector<double> xs(N), ys(N);
    for (uint32_t i = 0; i < N; i++)
    {
        xs[i] = static_cast<double>(i);
        ys[i] = static_cast<double>(i * 2);
    }
    CoordinateBuffer2D buf{xs.data(), ys.data(), N};

    // Sum only x coordinates - this iterates a contiguous double array (cache-friendly)
    double sum_x = 0.0;
    for (uint32_t i = 0; i < buf.count; i++)
    {
        sum_x += buf.x[i];
    }
    EXPECT_DOUBLE_EQ(sum_x, (N - 1) * N / 2.0);

    // Compare: AoS iteration through Point[] would interleave x,y in cache
    std::vector<Point> points(N);
    for (uint32_t i = 0; i < N; i++)
    {
        points[i] = {static_cast<double>(i), static_cast<double>(i * 2)};
    }
    double sum_x_aos = 0.0;
    for (uint32_t i = 0; i < N; i++)
    {
        sum_x_aos += points[i].x;
    }
    EXPECT_DOUBLE_EQ(sum_x, sum_x_aos);
}

// ============================================================================
// WKB Union tests
// ============================================================================

TEST(WKBGeometryUnionTest, PointAccess)
{
    WKBGeometry geom;
    geom.point.byteOrder = WKBByteOrder::wkbNDR;
    geom.point.point = {42.0, 84.0};

    EXPECT_DOUBLE_EQ(geom.point.point.x, 42.0);
    EXPECT_DOUBLE_EQ(geom.point.point.y, 84.0);
}
