/*!
 * \file hydrocouplespatialwkb.h
 * \author Caleb Amoa Buahin <caleb.buahin@gmail.com>
 * \version 1.0.0
 * \description
 * This header file contains the structures for the OGC wkb SFA specifications.
 * \license
 * This file and its associated files, and libraries are free software.
 * You can redistribute it and/or modify it under the terms of the
 * Lesser GNU Lesser General Public License as published by the Free Software Foundation;
 * either version 3 of the License, or (at your option) any later version.
 * This file and its associated files is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 * \copyright Copyright 2014-2018, Caleb Buahin, All rights reserved.
 * \date 2014-2018
 * \pre
 * \bug
 * \warning
 * \todo
 */

#ifndef HYDROCOUPLESPATIALWKB_H
#define HYDROCOUPLESPATIALWKB_H

/*!
 * \brief The Point struct
 */
struct Point
{
    double x;
    double y;
};

/*!
 * \brief The PointZ struct
 */
struct PointZ
{
    double x;
    double y;
    double z;
};

/*!
 * \brief The PointM struct
 */
struct PointM
{
    double x;
    double y;
    double m;
};

/*!
 * \brief The PointZM struct
 */
struct PointZM
{
    double x;
    double y;
    double z;
    double m;
};

/*!
 * \brief The LinearRing struct
 */
struct LinearRing
{
    uint32_t numPoints;
    Point *points;
};

/*!
 * \brief The LinearRingZ struct
 */
struct LinearRingZ
{
    uint32_t numPoints;
    PointZ *points;
};

/*!
 * \brief The LinearRingM struct
 */
struct LinearRingM
{
    uint32_t numPoints;
    PointM *points;
};

/*!
 * \brief The LinearRingZM struct
 */
struct LinearRingZM
{
    uint32_t numPoints;
    PointZM *points;
};

/*!
 * \brief The WKBByteOrder enum
 */
enum WKBByteOrder
{
  wkbXDR = 0, // Big Endian
  wkbNDR = 1 // Little Endian
};

/*!
 * \brief The WKBGeometryType enum
 */
enum WKBGeometryType
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
  wkbTrianglez = 1017,
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
 * \brief The WKBPoint struct
 */
struct WKBPoint
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1;
    Point point;
};

/*!
 * \brief The WKBPointZ struct
 */
struct WKBPointZ
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1001;
    PointZ point;
};

/*!
 * \brief The WKBPointM struct
 */
struct WKBPointM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2001;
    PointM point;
};

/*!
 * \brief The WKBPointZM struct
 */
struct WKBPointZM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3001;
    PointZM point;
};

/*!
 * \brief The WKBLineString struct
 */
struct WKBLineString
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2;
    uint32_t numPoints;
    Point *points;
};

/*!
 * \brief The WKBLineStringZ struct
 */
struct WKBLineStringZ
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1002;
    uint32_t numPoints;
    PointZ *points;
};

/*!
 * \brief The WKBLineStringM struct
 */
struct WKBLineStringM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2002;
    uint32_t numPoints;
    PointM *points;
};

/*!
 * \brief The WKBLineStringZM struct
 */
struct WKBLineStringZM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3002;
    uint32_t numPoints;
    PointZM points;
};

/*!
 * \brief The WKBPolygon struct
 */
struct WKBPolygon
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3;
    uint32_t numRings;
    LinearRing *rings;
};

/*!
 * \brief The WKBPolygonZ struct
 */
struct WKBPolygonZ
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1003;
    uint32_t numRings;
    LinearRingZ *rings;
};

/*!
 * \brief The WKBPolygonM struct
 */
struct WKBPolygonM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2003;
    uint32_t numRings;
    LinearRingM *rings;
};

/*!
 * \brief The WKBPolygonZM struct
 */
struct WKBPolygonZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3003;
    uint32_t numRings;
    LinearRingZM *rings;
};

/*!
 * \brief The WKBTriangle struct
 */
struct WKBTriangle {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 17;
    uint32_t numRings;
    LinearRing *rings;
};

/*!
 * \brief The WKBTriangleZ struct
 */
struct WKBTriangleZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1017;
    uint32_t numRings;
    LinearRingZ *rings;
};

/*!
 * \brief The WKBTriangleM struct
 */
struct WKBTriangleM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2017;
    uint32_t numRings;
    LinearRingM *rings;
};

/*!
 * \brief The WKBTriangleZM struct
 */
struct WKBTriangleZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3017;
    uint32_t numRings;
    LinearRingZM *rings;
};

/*!
 * \brief The WKBPolyhedralSurface struct
 */
struct WKBPolyhedralSurface {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 15;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

/*!
 * \brief The WKBPolyhedralSurfaceZ struct
 */
struct WKBPolyhedralSurfaceZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=1015;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

/*!
 * \brief The WKBPolyhedralSurfaceM struct
 */
struct WKBPolyhedralSurfaceM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=2015;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

/*!
 * \brief The WKBPolyhedralSurfaceZM struct
 */
struct WKBPolyhedralSurfaceZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=3015;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

/*!
 * \brief The WKBTIN struct
 */
struct WKBTIN {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 16;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

/*!
 * \brief The WKBTINZ struct
 */
struct WKBTINZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=1016;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

/*!
 * \brief The WKBTINM struct
 */
struct WKBTINM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=2016;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

/*!
 * \brief The WKBTINZM struct
 */
struct WKBTINZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=3016;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

/*!
 * \brief The WKBMultiPoint struct
 */
struct WKBMultiPoint {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=4;
    uint32_t numPoints;
    WKBPoint *points;
};

/*!
 * \brief The WKBMultiPointZ struct
 */
struct WKBMultiPointZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=1004;
    uint32_t numPoints;
    WKBPointZ *points;
};

/*!
 * \brief The WKBMultiPointM struct
 */
struct WKBMultiPointM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=2004;
    uint32_t numPoints;
    WKBPointM *points;
};

/*!
 * \brief The WKBMultiPointZM struct
 */
struct WKBMultiPointZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=3004;
    uint32_t numPoints;
    WKBPointZM *points;
};

/*!
 * \brief The WKBMultiLineString struct
 */
struct WKBMultiLineString {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 5;
    uint32_t numLineStrings;
    WKBLineString *lineStrings;
};

/*!
 * \brief The WKBMultiLineStringZ struct
 */
struct WKBMultiLineStringZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1005;
    uint32_t numLineStrings;
    WKBLineStringZ *lineStrings;
};

/*!
 * \brief The WKBMultiLineStringM struct
 */
struct WKBMultiLineStringM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2005;
    uint32_t numLineStrings;
    WKBLineStringM *lineStrings;
};

/*!
 * \brief The WKBMultiLineStringZM struct
 */
struct WKBMultiLineStringZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3005;
    uint32_t numLineStrings;
    WKBLineStringZM *lineStrings;
};

/*!
 * \brief The WKBMultiPolygon struct
 */
struct WKBMultiPolygon {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 6;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

/*!
 * \brief The WKBMultiPolygonZ struct
 */
struct WKBMultiPolygonZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1006;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

/*!
 * \brief The WKBMultiPolygonM struct
 */
struct WKBMultiPolygonM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2006;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

/*!
 * \brief The WKBMultiPolygonZM struct
 */
struct WKBMultiPolygonZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3006;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

/*!
 * \brief The WKBGeometryCollection struct
 */
struct WKBGeometryCollection {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 7;
    uint32_t numGeometries;
    WKBGeometry *geometries;
};

/*!
 * \brief The WKBGeometryCollectionZ struct
 */
struct WKBGeometryCollectionZ {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 1007;
    uint32_t numGeometries;
    WKBGeometryZ *geometries;
};

/*!
 * \brief The WKBGeometryCollectionM struct
 */
struct WKBGeometryCollectionM {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 2007;
    uint32_t numGeometries;
    WKBGeometryM *geometries;
};

/*!
 * \brief The WKBGeometryCollectionZM struct
 */
struct WKBGeometryCollectionZM {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 3007;
    uint32_t numGeometries;
    WKBGeometryZM *geometries;
};

/*!
 * \brief The WKBGeometry union
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
 * \brief The WKBGeometryZ union
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
 * \brief The WKBGeometryM union
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
 * \brief The WKBGeometryZM union
 */
union WKBGeometryZM
{
    WKBPointZM pointzm;
    WKBLineStringZM linestringzm;
    WKBPolygonZM polygonzm;
    WKBTriangleZM trianglezm;
    WKBPolyhedralSurfaceM Polyhedralsurfacezm;
    WKBTINZM tinzm;
    WKBMultiPointZM mpointzm;
    WKBMultiLineStringZM mlinestringzm;
    WKBMultiPolygonZ mpolygonzm;
    WKBGeometryCollectionZM collectionzm;
};

#endif // HYDROCOUPLESPATIALWKB_H
