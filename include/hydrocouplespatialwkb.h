#ifndef HYDROCOUPLESPATIALWKB_H
#define HYDROCOUPLESPATIALWKB_H

#include  <cstdint>
// Basic Type definitions
// WKBByteOrder : 1 WKBByteOrder
// uint32_t : 32 bit unsigned integer (4 bytes)
// double : double precision number (8 bytes)
// Building Blocks : Coordinate, LinearRing

struct Point
{
    double x;
    double y;
};

struct PointZ
{
    double x;
    double y;
    double z;
};

struct PointM
{
    double x;
    double y;
    double m;
};

struct PointZM
{
    double x;
    double y;
    double z;
    double m;
};

struct LinearRing
{
    uint32_t numPoints;
    Point *points;
};

struct LinearRingZ
{
    uint32_t numPoints;
    PointZ *points;
};

struct LinearRingM
{
    uint32_t numPoints;
    PointM *points;
};

struct LinearRingZM
{
    uint32_t numPoints;
    PointZM *points;
};

enum WKBByteOrder
{
  wkbXDR = 0, // Big Endian
  wkbNDR = 1 // Little Endian
};

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

struct WKBPoint
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1;
    Point point;
};

struct WKBPointZ
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1001;
    PointZ point;
};

struct WKBPointM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2001;
    PointM point;
};

struct WKBPointZM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3001;
    PointZM point;
};

struct WKBLineString
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2;
    uint32_t numPoints;
    Point *points;
};

struct WKBLineStringZ
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1002;
    uint32_t numPoints;
    PointZ *points;
};

struct WKBLineStringM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2002;
    uint32_t numPoints;
    PointM *points;
};

struct WKBLineStringZM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3002;
    uint32_t numPoints;
    PointZM points;
};

struct WKBPolygon
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3;
    uint32_t numRings;
    LinearRing *rings;
};

struct WKBPolygonZ
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1003;
    uint32_t numRings;
    LinearRingZ *rings;
};


struct WKBPolygonM
{
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2003;
    uint32_t numRings;
    LinearRingM *rings;
};

struct WKBPolygonZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3003;
    uint32_t numRings;
    LinearRingZM *rings;
};

struct WKBTriangle {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 17;
    uint32_t numRings;
    LinearRing *rings;
};

struct WKBTriangleZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1017;
    uint32_t numRings;
    LinearRingZ *rings;
};

struct WKBTriangleM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2017;
    uint32_t numRings;
    LinearRingM *rings;
};

struct WKBTriangleZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3017;
    uint32_t numRings;
    LinearRingZM *rings;
};

struct WKBPolyhedralSurface {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 15;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

struct WKBPolyhedralSurfaceZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=1015;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

struct WKBPolyhedralSurfaceM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=2015;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

struct WKBPolyhedralSurfaceZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=3015;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

struct WKBTIN {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 16;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

struct WKBTINZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=1016;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

struct WKBTINM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=2016;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

struct WKBTINZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=3016;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};

struct WKBMultiPoint {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=4;
    uint32_t numPoints;
    WKBPoint *points;
};

struct WKBMultiPointZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=1004;
    uint32_t numPoints;
    WKBPointZ *points;
};

struct WKBMultiPointM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=2004;
    uint32_t numPoints;
    WKBPointM *points;
};


struct WKBMultiPointZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType=3004;
    uint32_t numPoints;
    WKBPointZM *points;
};

struct WKBMultiLineString {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 5;
    uint32_t numLineStrings;
    WKBLineString *lineStrings;
};

struct WKBMultiLineStringZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1005;
    uint32_t numLineStrings;
    WKBLineStringZ *lineStrings;
};

struct WKBMultiLineStringM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2005;
    uint32_t numLineStrings;
    WKBLineStringM *lineStrings;
};

struct WKBMultiLineStringZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3005;
    uint32_t numLineStrings;
    WKBLineStringZM *lineStrings;
};

struct WKBMultiPolygon {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 6;
    uint32_t numPolygons;
    WKBPolygon *polygons;
};

struct WKBMultiPolygonZ {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 1006;
    uint32_t numPolygons;
    WKBPolygonZ *polygons;
};

struct WKBMultiPolygonM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 2006;
    uint32_t numPolygons;
    WKBPolygonM *polygons;
};

struct WKBMultiPolygonZM {
    WKBByteOrder byteOrder;
    static uint32_t wkbType = 3006;
    uint32_t numPolygons;
    WKBPolygonZM *polygons;
};



struct WKBGeometryCollection {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 7;
    uint32_t numGeometries;
    WKBGeometry *geometries;
};

struct WKBGeometryCollectionZ {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 1007;
    uint32_t numGeometries;
    WKBGeometryZ *geometries;
};

struct WKBGeometryCollectionM {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 2007;
    uint32_t numGeometries;
    WKBGeometryM *geometries;
};

struct WKBGeometryCollectionZM {
    WKBByteOrder byte_order;
    static uint32_t wkbType = 3007;
    uint32_t numGeometries;
    WKBGeometryZM *geometries;
};

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

union WKBGeometryZ {

    WKBPointZ pointz;
    WKBLineStringZ linestringz;
    WKBPolygonZ polygonz;
    WKBTriangleZ trianglez;
    WKBPolyhedralSurfaceZ Polyhedralsurfacez;
    WKBTinZ tinz;
    WKBMultiPointZ mpointz;
    WKBMultiLineStringZ mlinestringz;
    WKBMultiPolygonZ mpolygonz;
    WKBGeometryCollectionZ collectionz;
};

union WKBGeometryM
{
    WKBPointM pointm;
    WKBLineStringM linestringm;
    WKBPolygonM polygonm;
    WKBTriangleM trianglem;
    WKBPolyhedralSurfaceM Polyhedralsurfacem;
    WKBTinM tinm;
    WKBMultiPointM mpointm;
    WKBMultiLineStringM mlinestringm;
    WKBMultiPolygonM mpolygonm;
    WKBGeometryCollectionM collectionm;
};

union WKBGeometryZM
{
    WKBPointZM pointzm;
    WKBLineStringZM linestringzm;
    WKBPolygonZM polygonzm;
    WKBTriangleZM trianglezm;
    WKBPolyhedralSurfaceM Polyhedralsurfacezm;
    WKBTinZM tinzm;
    WKBMultiPointZM mpointzm;
    WKBMultiLineStringZM mlinestringzm;
    WKBMultiPolygonZ mpolygonzm;
    WKBGeometryCollectionZM collectionzm;
};

#endif // HYDROCOUPLESPATIALWKB_H
