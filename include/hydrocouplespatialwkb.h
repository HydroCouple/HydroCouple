/*!
 * \file hydrocouplespatialwkb.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0
 * \brief Well-Known Binary (WKB) structures for OGC Simple Features Access geometry serialization.
 * \details This header file contains the C-compatible structures for parsing and
 * constructing OGC WKB (Well-Known Binary) geometry representations as defined by
 * the Simple Features Access (SFA) specification. It includes Point, LineString,
 * Polygon, Triangle, PolyhedralSurface, TIN, and collection types with 2D, Z, M,
 * and ZM coordinate variants, as well as Structure-of-Arrays coordinate buffers
 * for efficient batch processing.
 * \license
 * This file and its associated files and libraries are free software.
 * You can redistribute it and/or modify it under the terms of the
 * MIT License as published by the Free Software Foundation.
 * This file and its associated files are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the MIT License for details.
 * \copyright Copyright 2014-2025, Caleb Buahin, All rights reserved.
 * \date 2014-2025
 */

#ifndef HYDROCOUPLESPATIALWKB_H
#define HYDROCOUPLESPATIALWKB_H

#include <stdint.h>
#include <type_traits>

union WKBGeometry;
union WKBGeometryZ;
union WKBGeometryM;
union WKBGeometryZM;

/*!
 * \brief A 2D point with x and y coordinates.
 */
struct Point
{
    double x; //!< The x-coordinate.
    double y; //!< The y-coordinate.
};

/*!
 * \brief A 3D point with x, y, and z coordinates.
 */
struct PointZ
{
    double x; //!< The x-coordinate.
    double y; //!< The y-coordinate.
    double z; //!< The z-coordinate (elevation).
};

/*!
 * \brief A 2D point with x, y, and an associated measure (m) value.
 */
struct PointM
{
    double x; //!< The x-coordinate.
    double y; //!< The y-coordinate.
    double m; //!< The measure value.
};

/*!
 * \brief A 3D point with x, y, z coordinates and an associated measure (m) value.
 */
struct PointZM
{
    double x; //!< The x-coordinate.
    double y; //!< The y-coordinate.
    double z; //!< The z-coordinate (elevation).
    double m; //!< The measure value.
};

/*!
 * \brief A closed sequence of 2D Points forming a ring boundary.
 */
struct LinearRing
{
    uint32_t numPoints;
    Point *points;
};

/*!
 * \brief A closed sequence of 3D PointZ values forming a ring boundary.
 */
struct LinearRingZ
{
    uint32_t numPoints;
    PointZ *points;
};

/*!
 * \brief A closed sequence of PointM values forming a ring boundary with measures.
 */
struct LinearRingM
{
    uint32_t numPoints;
    PointM *points;
};

/*!
 * \brief A closed sequence of PointZM values forming a ring boundary with z and measure.
 */
struct LinearRingZM
{
    uint32_t numPoints;
    PointZM *points;
};

/*!
 * \brief The WKBByteOrder enum class indicates the byte order of serialized WKB data.
 */
enum class WKBByteOrder : uint8_t
{
  wkbXDR = 0, //!< Big Endian
  wkbNDR = 1  //!< Little Endian
};

/*!
 * \brief The WKBGeometryType enum class enumerates OGC WKB geometry type codes.
 */
enum class WKBGeometryType : uint32_t
{
  wkbPoint = 1,
  wkbLineString = 2,
  wkbPolygon = 3,
  wkbTriangle = 17,
  wkbMultiPoint = 4,
  wkbMultiLineString = 5,
  wkbMultiPolygon = 6,
  wkbGeometryCollection = 7,
  wkbPolyhedralSurface = 15,
  wkbTIN = 16,
  wkbPointZ = 1001,
  wkbLineStringZ = 1002,
  wkbPolygonZ = 1003,
  wkbTriangleZ = 1017,
  wkbMultiPointZ = 1004,
  wkbMultiLineStringZ = 1005,
  wkbMultiPolygonZ = 1006,
  wkbGeometryCollectionZ = 1007,
  wkbPolyhedralSurfaceZ = 1015,
  wkbTINZ = 1016,
  wkbPointM = 2001,
  wkbLineStringM = 2002,
  wkbPolygonM = 2003,
  wkbTriangleM = 2017,
  wkbMultiPointM = 2004,
  wkbMultiLineStringM = 2005,
  wkbMultiPolygonM = 2006,
  wkbGeometryCollectionM = 2007,
  wkbPolyhedralSurfaceM = 2015,
  wkbTINM = 2016,
  wkbPointZM = 3001,
  wkbLineStringZM = 3002,
  wkbPolygonZM = 3003,
  wkbTriangleZM = 3017,
  wkbMultiPointZM = 3004,
  wkbMultiLineStringZM = 3005,
  wkbMultiPolygonZM = 3006,
  wkbGeometryCollectionZM = 3007,
  wkbPolyhedralSurfaceZM = 3015,
  wkbTinZM = 3016,
};

/*!
 * \brief WKB-encoded 2D point geometry (type code 1).
 */
struct WKBPoint
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 1;
    Point point;
};

/*!
 * \brief WKB-encoded 3D point geometry with z-coordinate (type code 1001).
 */
struct WKBPointZ
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 1001;
    PointZ point;
};

/*!
 * \brief WKB-encoded 2D point geometry with measure (type code 2001).
 */
struct WKBPointM
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 2001;
    PointM point;
};

/*!
 * \brief WKB-encoded 3D point geometry with z-coordinate and measure (type code 3001).
 */
struct WKBPointZM
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 3001;
    PointZM point;
};

/*!
 * \brief WKB-encoded 2D line string geometry (type code 2).
 */
struct WKBLineString
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 2;
    uint32_t numPoints;
    Point *points;
};

/*!
 * \brief WKB-encoded 3D line string geometry with z-coordinates (type code 1002).
 */
struct WKBLineStringZ
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 1002;
    uint32_t numPoints;
    PointZ *points;
};

/*!
 * \brief WKB-encoded 2D line string geometry with measures (type code 2002).
 */
struct WKBLineStringM
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 2002;
    uint32_t numPoints;
    PointM *points;
};

/*!
 * \brief WKB-encoded 3D line string geometry with z-coordinates and measures (type code 3002).
 */
struct WKBLineStringZM
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 3002;
    uint32_t numPoints;
    PointZM *points;
};

/*!
 * \brief WKB-encoded 2D polygon geometry defined by linear rings (type code 3).
 */
struct WKBPolygon
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 3;
    uint32_t numRings;
    LinearRing *rings;
};

/*!
 * \brief WKB-encoded 3D polygon geometry with z-coordinates (type code 1003).
 */
struct WKBPolygonZ
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 1003;
    uint32_t numRings;
    LinearRingZ *rings;
};

/*!
 * \brief WKB-encoded 2D polygon geometry with measures (type code 2003).
 */
struct WKBPolygonM
{
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 2003;
    uint32_t numRings;
    LinearRingM *rings;
};

/*!
 * \brief WKB-encoded 3D polygon geometry with z-coordinates and measures (type code 3003).
 */
struct WKBPolygonZM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 3003;
    uint32_t numRings;
    LinearRingZM *rings;
};

/*!
 * \brief WKB-encoded 2D triangle geometry with exactly one exterior ring of four points (type code 17).
 */
struct WKBTriangle {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 17;
    uint32_t numRings;
    LinearRing *rings;
};

/*!
 * \brief WKB-encoded 3D triangle geometry with z-coordinates (type code 1017).
 */
struct WKBTriangleZ {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 1017;
    uint32_t numRings;
    LinearRingZ *rings;
};

/*!
 * \brief WKB-encoded 2D triangle geometry with measures (type code 2017).
 */
struct WKBTriangleM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 2017;
    uint32_t numRings;
    LinearRingM *rings;
};

/*!
 * \brief WKB-encoded 3D triangle geometry with z-coordinates and measures (type code 3017).
 */
struct WKBTriangleZM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 3017;
    uint32_t numRings;
    LinearRingZM *rings;
};

/*!
 * \brief WKB-encoded 2D polyhedral surface as a collection of polygons (type code 15).
 */
struct WKBPolyhedralSurface {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 15;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

/*!
 * \brief WKB-encoded 3D polyhedral surface with z-coordinates (type code 1015).
 */
struct WKBPolyhedralSurfaceZ {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=1015;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

/*!
 * \brief WKB-encoded 2D polyhedral surface with measures (type code 2015).
 */
struct WKBPolyhedralSurfaceM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=2015;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

/*!
 * \brief WKB-encoded 3D polyhedral surface with z-coordinates and measures (type code 3015).
 */
struct WKBPolyhedralSurfaceZM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=3015;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

/*!
 * \brief WKB-encoded 2D Triangulated Irregular Network (type code 16).
 */
struct WKBTIN {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 16;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

/*!
 * \brief WKB-encoded 3D Triangulated Irregular Network with z-coordinates (type code 1016).
 */
struct WKBTINZ {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=1016;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

/*!
 * \brief WKB-encoded 2D Triangulated Irregular Network with measures (type code 2016).
 */
struct WKBTINM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=2016;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

/*!
 * \brief WKB-encoded 3D Triangulated Irregular Network with z-coordinates and measures (type code 3016).
 */
struct WKBTINZM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=3016;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

/*!
 * \brief WKB-encoded 2D multi-point collection (type code 4).
 */
struct WKBMultiPoint {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=4;
    uint32_t numPoints;
    WKBPoint *points;
};

/*!
 * \brief WKB-encoded 3D multi-point collection with z-coordinates (type code 1004).
 */
struct WKBMultiPointZ {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=1004;
    uint32_t numPoints;
    WKBPointZ *points;
};

/*!
 * \brief WKB-encoded 2D multi-point collection with measures (type code 2004).
 */
struct WKBMultiPointM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=2004;
    uint32_t numPoints;
    WKBPointM *points;
};

/*!
 * \brief WKB-encoded 3D multi-point collection with z-coordinates and measures (type code 3004).
 */
struct WKBMultiPointZM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType=3004;
    uint32_t numPoints;
    WKBPointZM *points;
};

/*!
 * \brief WKB-encoded 2D multi-line-string collection (type code 5).
 */
struct WKBMultiLineString {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 5;
    uint32_t numLineStrings;
    WKBLineString *lineStrings;
};

/*!
 * \brief WKB-encoded 3D multi-line-string collection with z-coordinates (type code 1005).
 */
struct WKBMultiLineStringZ {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 1005;
    uint32_t numLineStrings;
    WKBLineStringZ *lineStrings;
};

/*!
 * \brief WKB-encoded 2D multi-line-string collection with measures (type code 2005).
 */
struct WKBMultiLineStringM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 2005;
    uint32_t numLineStrings;
    WKBLineStringM *lineStrings;
};

/*!
 * \brief WKB-encoded 3D multi-line-string collection with z-coordinates and measures (type code 3005).
 */
struct WKBMultiLineStringZM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 3005;
    uint32_t numLineStrings;
    WKBLineStringZM *lineStrings;
};

/*!
 * \brief WKB-encoded 2D multi-polygon collection (type code 6).
 */
struct WKBMultiPolygon {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 6;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

/*!
 * \brief WKB-encoded 3D multi-polygon collection with z-coordinates (type code 1006).
 */
struct WKBMultiPolygonZ {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 1006;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

/*!
 * \brief WKB-encoded 2D multi-polygon collection with measures (type code 2006).
 */
struct WKBMultiPolygonM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 2006;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

/*!
 * \brief WKB-encoded 3D multi-polygon collection with z-coordinates and measures (type code 3006).
 */
struct WKBMultiPolygonZM {
    WKBByteOrder byteOrder;
    static constexpr uint32_t wkbType = 3006;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

/*!
 * \brief WKB-encoded heterogeneous 2D geometry collection (type code 7).
 */
struct WKBGeometryCollection {
    WKBByteOrder byte_order;
    static constexpr uint32_t wkbType = 7;
    uint32_t numGeometries;
    WKBGeometry *geometries;
};

/*!
 * \brief WKB-encoded heterogeneous 3D geometry collection with z-coordinates (type code 1007).
 */
struct WKBGeometryCollectionZ {
    WKBByteOrder byte_order;
    static constexpr uint32_t wkbType = 1007;
    uint32_t numGeometries;
    WKBGeometryZ *geometries;
};

/*!
 * \brief WKB-encoded heterogeneous 2D geometry collection with measures (type code 2007).
 */
struct WKBGeometryCollectionM {
    WKBByteOrder byte_order;
    static constexpr uint32_t wkbType = 2007;
    uint32_t numGeometries;
    WKBGeometryM *geometries;
};

/*!
 * \brief WKB-encoded heterogeneous 3D geometry collection with z-coordinates and measures (type code 3007).
 */
struct WKBGeometryCollectionZM {
    WKBByteOrder byte_order;
    static constexpr uint32_t wkbType = 3007;
    uint32_t numGeometries;
    WKBGeometryZM *geometries;
};

/*!
 * \brief Type-punning union over all 2D WKB geometry structs.
 * \details Allows interpreting a raw WKB byte stream as the appropriate
 * concrete geometry type after inspecting the wkbType field.
 */
union WKBGeometry
{
    WKBPoint point;
    WKBLineString linestring;
    WKBPolygon polygon;
    WKBTriangle triangle;
    WKBPolyhedralSurface polyhedralsurface;
    WKBTIN tin;
    WKBMultiPoint mpoint;
    WKBMultiLineString mlinestring;
    WKBMultiPolygon mpolygon;
    WKBGeometryCollection collection;
};

/*!
 * \brief Type-punning union over all 3D (Z) WKB geometry structs.
 */
union WKBGeometryZ {

    WKBPointZ pointz;
    WKBLineStringZ linestringz;
    WKBPolygonZ polygonz;
    WKBTriangleZ trianglez;
    WKBPolyhedralSurfaceZ Polyhedralsurfacez;
    WKBTINZ tinz;
    WKBMultiPointZ mpointz;
    WKBMultiLineStringZ mlinestringz;
    WKBMultiPolygonZ mpolygonz;
    WKBGeometryCollectionZ collectionz;
};

/*!
 * \brief Type-punning union over all 2D+M WKB geometry structs.
 */
union WKBGeometryM
{
    WKBPointM pointm;
    WKBLineStringM linestringm;
    WKBPolygonM polygonm;
    WKBTriangleM trianglem;
    WKBPolyhedralSurfaceM Polyhedralsurfacem;
    WKBTINM tinm;
    WKBMultiPointM mpointm;
    WKBMultiLineStringM mlinestringm;
    WKBMultiPolygonM mpolygonm;
    WKBGeometryCollectionM collectionm;
};

/*!
 * \brief Type-punning union over all 3D+M WKB geometry structs.
 */
union WKBGeometryZM
{
    WKBPointZM pointzm;
    WKBLineStringZM linestringzm;
    WKBPolygonZM polygonzm;
    WKBTriangleZM trianglezm;
    WKBPolyhedralSurfaceZM Polyhedralsurfacezm;
    WKBTINZM tinzm;
    WKBMultiPointZM mpointzm;
    WKBMultiLineStringZM mlinestringzm;
    WKBMultiPolygonZM mpolygonzm;
    WKBGeometryCollectionZM collectionzm;
};

// ============================================================================
// Structure-of-Arrays (SoA) coordinate buffers for cache-efficient bulk processing.
// These provide non-owning views into contiguous coordinate component arrays,
// enabling SIMD-friendly iteration over individual coordinate dimensions.
// ============================================================================

/*!
 * \brief Non-owning SoA view over 2D coordinate data (x, y stored in separate arrays).
 * \details Enables cache-efficient iteration over individual coordinate components
 * for SIMD and data-oriented processing patterns. The caller is responsible
 * for managing the lifetime and allocation of the underlying arrays.
 */
struct CoordinateBuffer2D
{
    double *x;        //!< Pointer to contiguous array of x-coordinates.
    double *y;        //!< Pointer to contiguous array of y-coordinates.
    uint32_t count;   //!< Number of coordinate tuples.
};

/*!
 * \brief Non-owning SoA view over 3D coordinate data (x, y, z stored in separate arrays).
 */
struct CoordinateBuffer3D
{
    double *x;        //!< Pointer to contiguous array of x-coordinates.
    double *y;        //!< Pointer to contiguous array of y-coordinates.
    double *z;        //!< Pointer to contiguous array of z-coordinates.
    uint32_t count;   //!< Number of coordinate tuples.
};

/*!
 * \brief Non-owning SoA view over 2D+M coordinate data (x, y, m stored in separate arrays).
 */
struct CoordinateBufferM
{
    double *x;        //!< Pointer to contiguous array of x-coordinates.
    double *y;        //!< Pointer to contiguous array of y-coordinates.
    double *m;        //!< Pointer to contiguous array of measure values.
    uint32_t count;   //!< Number of coordinate tuples.
};

/*!
 * \brief Non-owning SoA view over 3D+M coordinate data (x, y, z, m stored in separate arrays).
 */
struct CoordinateBuffer3DM
{
    double *x;        //!< Pointer to contiguous array of x-coordinates.
    double *y;        //!< Pointer to contiguous array of y-coordinates.
    double *z;        //!< Pointer to contiguous array of z-coordinates.
    double *m;        //!< Pointer to contiguous array of measure values.
    uint32_t count;   //!< Number of coordinate tuples.
};

// ============================================================================
// Compile-time type property assertions
// ============================================================================

static_assert(std::is_trivially_copyable_v<Point>, "Point must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointZ>, "PointZ must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointM>, "PointM must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointZM>, "PointZM must be trivially copyable");
static_assert(std::is_trivially_copyable_v<LinearRing>, "LinearRing must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CoordinateBuffer2D>, "CoordinateBuffer2D must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CoordinateBuffer3D>, "CoordinateBuffer3D must be trivially copyable");
static_assert(std::is_trivially_copyable_v<CoordinateBuffer3DM>, "CoordinateBuffer3DM must be trivially copyable");

static_assert(std::is_standard_layout_v<Point>, "Point must be standard layout");
static_assert(std::is_standard_layout_v<PointZ>, "PointZ must be standard layout");
static_assert(std::is_standard_layout_v<PointM>, "PointM must be standard layout");
static_assert(std::is_standard_layout_v<PointZM>, "PointZM must be standard layout");
static_assert(std::is_standard_layout_v<CoordinateBuffer2D>, "CoordinateBuffer2D must be standard layout");
static_assert(std::is_standard_layout_v<CoordinateBuffer3D>, "CoordinateBuffer3D must be standard layout");
static_assert(std::is_standard_layout_v<CoordinateBuffer3DM>, "CoordinateBuffer3DM must be standard layout");

static_assert(sizeof(Point) == 2 * sizeof(double), "Point must have no padding");
static_assert(sizeof(PointZ) == 3 * sizeof(double), "PointZ must have no padding");
static_assert(sizeof(PointM) == 3 * sizeof(double), "PointM must have no padding");
static_assert(sizeof(PointZM) == 4 * sizeof(double), "PointZM must have no padding");

#endif // HYDROCOUPLESPATIALWKB_H
