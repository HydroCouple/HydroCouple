/*!
 * \file hydrocouplespatial.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0-alpha.1
 * \brief Geospatial interface definitions for the HydroCouple component-based modeling framework.
 * \details This header file contains the geospatial interface definitions for the
 * HydroCouple component-based modeling framework. It defines interfaces following the
 * OGC Simple Features Access specification for geometry types, spatial reference systems,
 * mesh/network data structures, raster data, regular grids, and their corresponding
 * component data item interfaces.
 * \license
 * This file and its associated files and libraries are free software.
 * You can redistribute them and/or modify them under the terms of the
 * MIT License. They are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the MIT License for details.
 * \copyright Copyright 2014-2026, Caleb Buahin, All rights reserved.
 * \date 2014-2026
 */

#ifndef HYDROCOUPLESPATIAL_H
#define HYDROCOUPLESPATIAL_H

#include "hydrocouple.h"


namespace HydroCouple
{
  /*!
   * \brief HydroCouple's interfaces that have a spatial component.
   */
  namespace Spatial
  {
    class IEdge;
    class IMeshView;
    class IRasterBand;
    class IPolygon;
    class IPolyhedralSurface;

    /*!
     * \brief The MeshDataObjectType enum describes the part of the
     * geometry of the mesh that data corresponds to.
     */
    enum class MeshDataObjectType
    {

      /*!
       * \brief The data corresponds to mesh cell.
       */
      Cell,

      /*!
       * \brief The data corresponds to the vertex of mesh edge vertex.
       */
      Vertex,

      /*!
       * \brief The data corresponds to the edges of the mesh.
       */
      Edge,

      /*!
       * \brief The data corresponds to the faces of the mesh.
       */
      Face,
    };

    /*!
     * \brief The types of data available in a network.
     */
    enum class NetworkDataObjectType
    {
      /*!
       * \brief The data corresponds to the nodes of the network.
       */
      Node,

      /*!
       * \brief The data corresponds to the edges of the network.
       */
      Edge,
    };

    /*!
     * \brief SpatialDataType describes the type of spatial data
     * (applicable to meshes, networks, and other spatial structures).
     */
    enum class SpatialDataType
    {
      //! Single scalar value.
      Scalar,
      //! Multiple scalar values.
      MultiScalar,
      //! Vector values.
      Vector,
      //! Tensor values.
      Tensor,
    };

    /*!
     * \brief The types of regular grids.
     */
    enum class RegularGridType
    {
      //! Cartesian grid.
      Cartesian,
      //! Rectilinear grid.
      Rectilinear,
      //! Curvilinear grid.
      Curvilinear
    };

    /*!
     * \brief ISpatialReferenceSystem describes the coordinate reference system
     * for spatial geometric objects.
     */
    class ISpatialReferenceSystem
    {

    public:
      /*!
       * \brief ISpatialReferenceSystem destructor.
       */
      virtual ~ISpatialReferenceSystem() = default;

      /*!
       * \brief Returns the Spatial Reference System ID (SRID) for a geometric object.
       * This will normally be a foreign key to an index of reference systems
       * stored in either the same or some other datastore.
       */
      [[nodiscard]] virtual int authSRID() const = 0;

      /*!
       * \brief The Authority Specific Spatial Reference System Identifier.
       */
      [[nodiscard]] virtual const std::string &authName() const = 0;

      /*!
       * Well-known Text description of the Spatial Reference System.
       */
      [[nodiscard]] virtual const std::string &srText() const = 0;

      /*!
       * \brief The measurement distance units for the Spatial Reference System.
       */
      [[nodiscard]] virtual HydroCouple::IUnit::DistanceUnits distanceUnits() const = 0;
    };

    /*!
     * \brief IEnvelope represents the minimum bounding box of a geometry.
     */
    class IEnvelope
    {
    public:
      /*!
       * \brief IEnvelope destructor.
       */
      virtual ~IEnvelope() = default;

      /*!
       * \brief The minimum x-coordinate value for this IEnvelope.
       * \return The minimum x-coordinate.
       */
      [[nodiscard]] virtual double minX() const = 0;

      /*!
       * \brief The maximum x-coordinate value for this IEnvelope.
       * \return The maximum x-coordinate.
       */
      [[nodiscard]] virtual double maxX() const = 0;

      /*!
       * \brief The minimum y-coordinate value for this IEnvelope.
       * \return The minimum y-coordinate.
       */
      [[nodiscard]] virtual double minY() const = 0;

      /*!
       * \brief The maximum y-coordinate value for this IEnvelope.
       * \return The maximum y-coordinate.
       */
      [[nodiscard]] virtual double maxY() const = 0;

      /*!
       * \brief The minimum z-coordinate value for this IEnvelope.
       * \return The minimum z-coordinate.
       */
      [[nodiscard]] virtual double minZ() const = 0;

      /*!
       * \brief The maximum z-coordinate value for this IEnvelope.
       * \return The maximum z-coordinate.
       */
      [[nodiscard]] virtual double maxZ() const = 0;
    };

    /*!
     * \brief IGeometry is the root class of the geometry hierarchy.
     */
    class IGeometry
    {
    public:
      /*!
       * \brief The type of IGeometry.
       */
      enum class GeometryType
      {
        Geometry = 0,
        Point = 1,
        LineString = 2,
        Polygon = 3,
        Triangle = 17,
        MultiPoint = 4,
        MultiLineString = 5,
        MultiPolygon = 6,
        GeometryCollection = 7,
        CircularString = 8,
        CompoundCurve = 9,
        CurvePolygon = 10,
        MultiCurve = 11,
        MultiSurface = 12,
        Curve = 13,
        Surface = 14,
        PolyhedralSurface = 15,
        TIN = 16,

        GeometryZ = 1000,
        PointZ = 1001,
        LineStringZ = 1002,
        PolygonZ = 1003,
        TriangleZ = 1017,
        MultiPointZ = 1004,
        MultiLineStringZ = 1005,
        MultiPolygonZ = 1006,
        GeometryCollectionZ = 1007,
        CircularStringZ = 1008,
        CompoundCurveZ = 1009,
        CurvePolygonZ = 1010,
        MultiCurveZ = 1011,
        MultiSurfaceZ = 1012,
        CurveZ = 1013,
        SurfaceZ = 1014,
        PolyhedralSurfaceZ = 1015,
        TINZ = 1016,

        GeometryM = 2000,
        PointM = 2001,
        LineStringM = 2002,
        PolygonM = 2003,
        TriangleM = 2017,
        MultiPointM = 2004,
        MultiLineStringM = 2005,
        MultiPolygonM = 2006,
        GeometryCollectionM = 2007,
        CircularStringM = 2008,
        CompoundCurveM = 2009,
        CurvePolygonM = 2010,
        MultiCurveM = 2011,
        MultiSurfaceM = 2012,
        CurveM = 2013,
        SurfaceM = 2014,
        PolyhedralSurfaceM = 2015,
        TINM = 2016,

        GeometryZM = 3000,
        PointZM = 3001,
        LineStringZM = 3002,
        PolygonZM = 3003,
        TriangleZM = 3017,
        MultiPointZM = 3004,
        MultiLineStringZM = 3005,
        MultiPolygonZM = 3006,
        GeometryCollectionZM = 3007,
        CircularStringZM = 3008,
        CompoundCurveZM = 3009,
        CurvePolygonZM = 3010,
        MultiCurveZM = 3011,
        MultiSurfaceZM = 3012,
        CurveZM = 3013,
        SurfaceZM = 3014,
        PolyhedralSurfaceZM = 3015,
        TINZM = 3016,
      };

      /*!
       * \brief IGeometry destructor.
       */
      virtual ~IGeometry() = default;

      /*!
       * \brief id of the geometry.
       * \return id of the geometry.
       */
      [[nodiscard]] virtual const std::string &id() const = 0;

      /*!
       * \brief index of the geometry if it is part of a collection.
       * \return index of the geometry in a collection.
       */
      [[nodiscard]] virtual unsigned int index() const = 0;

      /*!
       * \brief The inherent dimension of this geometric object, which must be less than or equal to the coordinate dimension.
       *  In non - homogeneous collections, this will return the largest topological dimension of the contained objects.
       *
       * \returns 0 for points, 1 for lines and 2 for surfaces.
       *
       */
      [[nodiscard]] virtual int dimension() const = 0;

      /*!
       * \brief Get the dimension of the coordinates in this object.
       *
       * \returns In practice this will return 2 or 3. It can also return 0 in the case of an empty point.
       */
      [[nodiscard]] virtual int coordinateDimension() const = 0;

      /*!
       * \brief Gets the geometry type of this object.
       *
       * \returns A GeometryType enum value representing the instantiable subtype of Geometry
       * of which this geometric object is a member.
       */
      [[nodiscard]] virtual GeometryType geometryType() const = 0;

      /*!
       * \brief Spatial reference system of geometric object.
       */
      [[nodiscard]] virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

      /*!
       * \brief The minimum bounding box for this Geometry, returned as a IGeometry. Recalculated at the time of the call
       * \returns The minimum bounding box for this Geometry.
       */
      [[nodiscard]] virtual IEnvelope *envelope() const = 0;

      /*!
       * \brief Exports this geometric object to a specific Well-known Text Representation of Geometry.
       * \returns Well-known Text Representation of Geometry.
       */
      [[nodiscard]] virtual std::string getWKT() const = 0;

      /*!
       * \brief Exports this geometric object to a specific Well-known byte Representation of Geometry.
       */
      [[nodiscard]] virtual std::vector<unsigned char> getWKB() const = 0;

      /*!
       * \brief If true, then this geometric object represents the empty point set ∅ for the coordinate space.
       * \returns <code>true</code> if this geometric object is the empty Geometry.
       */
      [[nodiscard]] virtual bool isEmpty() const = 0;

      /*!
       * \brief Returns <code>true</code> if this geometric object has no anomalous
       * geometric points, such as self intersection or self tangency.
       *
       * \details The description of each instantiable geometric class will include the specific
       * conditions that cause an instance of that class to be classified as not simple.
       */
      [[nodiscard]] virtual bool isSimple() const = 0;

      /*!
       * \returns <code>true</code> if this geometric object has z coordinate values.
       */
      [[nodiscard]] virtual bool is3D() const = 0;

      /*!
       * \returns <code>true</code> if this geometric object has m coordinate values.
       */
      [[nodiscard]] virtual bool isMeasured() const = 0;

      /*!
       * \brief Returns the closure of the combinatorial boundary of
       * this geometric object (Reference [1], section 3.12.2).
       *
       * \details Because the result of this function is a closure, and hence topologically
       * closed, the resulting boundary can be represented using representational Geometry primitives (Reference [1],
       * section 3.12.2). The return type is integer, but is interpreted as Boolean, TRUE=1, FALSE=0.
       *
       */
      [[nodiscard]] virtual IGeometry *boundary() const = 0;

      /** @name Query
       *Query functions
       */
      //@{

      /*!
       * \returns <code>true</code> if this geometric object is spatially equal to geom.
       */
      [[nodiscard]] virtual bool equals(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially disjoint to geom.
       */
      [[nodiscard]] virtual bool disjoint(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially intersects to geom.
       */
      [[nodiscard]] virtual bool intersects(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially touches to geom.
       */
      [[nodiscard]] virtual bool touches(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially crosses to geom.
       */
      [[nodiscard]] virtual bool crosses(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially within to geom.
       */
      [[nodiscard]] virtual bool within(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially contains to geom.
       */
      [[nodiscard]] virtual bool contains(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially overlaps to geom.
       */
      [[nodiscard]] virtual bool overlaps(const IGeometry &geom) const = 0;

      /*!
       * \details This returns <code>false</code> if all the tested intersections
       * are empty except exterior (this) intersect exterior (another).
       *
       * \returns <code>true</code> if this geometric object is spatially related
       * to geom by testing for intersections between the interior, boundary and
       * exterior of the two geometric objects as specified by the values in the intersectionPatternMatrix.
       *
       */
      [[nodiscard]] virtual bool relate(const IGeometry &geom) const = 0;

      /*!
       * \returns a derived geometry collection value that matches the specified m coordinate value.
       */
      [[nodiscard]] virtual IGeometry *locateAlong(double value) const = 0;

      /*!
       * \returns a derived geometry collection value that matches the specified range of m coordinate values inclusively.
       */
      [[nodiscard]] virtual IGeometry *locateBetween(double mStart, double mEnd) const = 0;

      ///@}

      /** @name Spatial Analysis
       *Spatial analysis functions
       */
      //@{
      /*!
       * \brief Returns the shortest distance between any two
       * Points in the two geometric objects as calculated in
       * the spatial reference system of this geometric object.
       *
       * \details Because the geometries are closed, it is possible to
       * find a point on each geometric object involved, such that the distance
       * between these 2 points is the returned distance between their geometric objects.
       */
      [[nodiscard]] virtual double distance(const IGeometry &geom) const = 0;

      /*!
       * \brief Returns a geometric object that represents all Points whose distance
       * from this geometric object is less than or equal to distance.
       *
       * \details Calculations are in the spatial reference system of this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
       * small error in this distance, but it should be near the resolution of the coordinates used.
       */
      [[nodiscard]] virtual IGeometry *buffer(double bufferDistance) const = 0;

      /*!
       * \returns a geometric object that represents the convex hull of this geometric object.
       *
       * \details Convex hulls, being dependent on straight lines, can be accurately represented
       * in linear interpolations for any geometry restricted to linear interpolations.
       */
      [[nodiscard]] virtual IGeometry *convexHull() const = 0;

      /*!
       * \returns a geometric object that represents the Point set intersection of this geometric object with geom.
       */
      [[nodiscard]] virtual IGeometry *intersection(const IGeometry &geom) const = 0;

      /*!
       * \returns a geometric object that represents the Point set union of this geometric object with geom.
       */
      [[nodiscard]] virtual IGeometry *unionG(const IGeometry &geom) const = 0;

      /*!
       * \returns a geometric object that represents the Point set difference of this geometric object with geom.
       */
      [[nodiscard]] virtual IGeometry *difference(const IGeometry &geom) const = 0;

      /*!
       * \returns a geometric object that represents the Point set symmetric difference of this geometric object with geom.
       */
      [[nodiscard]] virtual IGeometry *symmetricDifference(const IGeometry &geom) const = 0;

      //@}
    };

    /*!
     * \brief An IGeometryCollection is a geometric object that is a
     * collection of some number of IGeometry objects.
     *
     * \details All the elements in a IGeometryCollection shall be in
     * the same Spatial Reference System. This is also the ISpatialReferenceSystem
     * for the GeometryCollection.
     *
     * \details GeometryCollection places no other constraints on its elements. Subclasses of GeometryCollection may restrict
     * membership based on dimension and may also place other constraints on the degree of spatial overlap between
     * elements.
     *
     */
    class IGeometryCollection : public virtual IGeometry
    {

    public:
      /*!
       * \brief IGeometryCollection destructor.
       */
      virtual ~IGeometryCollection() = default;

      /*!
       * \brief The number of geometries in this IGeometryCollection
       * \returns The number of geometries in this IGeometryCollection.
       */
      [[nodiscard]] virtual int geometryCount() const = 0;

      /*!
       * \brief The IGeometry object associated with a specified index.
       * \param index of the geometry in this IGeometryCollection.
       * \return The IGeometry object associated with this index.
       */
      [[nodiscard]] virtual IGeometry *geometry(int index) const = 0;
    };

    /*!
     * \brief An IPoint is a 0-dimensional geometric object
     * and represents a single location in coordinate space.
     *
     * \details A IPoint has an x-coordinate value, a y-coordinate value.
     *  If called for by the associated Spatial Reference System, it may also
     *  have coordinate values for z and m.
     *
     * \details The boundary of a Point is the empty set.
     */
    class IPoint : public virtual IGeometry
    {
    public:
      /*!
       * \brief IPoint destructor.
       */
      virtual ~IPoint() = default;

      /*!
       * \brief The x-coordinate value for this IPoint.
       */
      [[nodiscard]] virtual double x() const = 0;

      /*!
       * \brief The y-coordinate value for this IPoint.
       */
      [[nodiscard]] virtual double y() const = 0;

      /*!
       * \brief The z-coordinate value for this IPoint. Returns NIL otherwise.
       */
      [[nodiscard]] virtual double z() const = 0;

      /*!
       * \brief The m-coordinate value for this IPoint. Returns NIL otherwise.
       */
      [[nodiscard]] virtual double m() const = 0;
    };

    /*!
     * \brief An IMultiPoint is a 0-dimensional IGeometryCollection.
     *
     * \details The elements of a IMultiPoint are restricted to IPoints.
     * The IPoints are not connected or ordered in any semantically
     * important way (see the discussion at IGeometryCollection).
     *
     * \details An IMultiPoint is simple if no two IPoints in the
     * MultiPoint are equal (have identical coordinate values in X and Y).
     * Every IMultiPoint is spatially equal under the definition in Clause 6.1.15.3 to a simple IMultipoint.
     * The boundary of an IMultiPoint is the empty set.
     *
     */
    class IMultiPoint : public virtual IGeometryCollection
    {

    public:
      /*!
       * \brief IMultiPoint destructor.
       */
      virtual ~IMultiPoint() = default;

      /*!
       * \returns the index sup(th) IPoint in this IGeometryCollection.
       */
      [[nodiscard]] virtual IPoint *point(int index) const = 0;
    };

    /*!
     * \brief The IVertex class is an IPoint of a topologically aware IGeometry
     */
    class IVertex : public virtual IPoint
    {
    public:
      /*!
       * \brief IVertex destructor.
       */
      virtual ~IVertex() = default;

      /*!
       * \brief unique index identifier
       * \return
       */
      [[nodiscard]] virtual unsigned int index() const = 0;

      /*!
       * \brief An arbitrary outgoing IEdge from this vertex.
       * \returns An edge whose origin is this vertex;
       *    null if isolated
       */
      [[nodiscard]] virtual IEdge *edge() const = 0;
    };

    /*!
     * \brief A Curve is a 1-dimensional geometric object usually stored as a sequence of IPoints,
     * with the subtype of ICurve specifying the form of the interpolation between IPoints.
     *
     * \details This standard defines only one subclass of Curve, LineString,
     * which uses linear interpolation between Points.
     *
     * \details A Curve is a 1-dimensional geometric object that is the homeomorphic image of a real, closed, interval:
     * D = [a, b] = {t∈ℜ⏐ a ≤ t ≤ b} under a mapping f :[a, b] → ℜn
     * where n is the coordinate dimension of the underlying Spatial Reference System.
     * A Curve is simple if it does not pass through the same Point twice with the possible exception of the two end
     * points (Reference [1], section 3.12.7.3):
     *
     * \details ∀ c ∈ Curve, [a, b] = c.Domain, c =: f :[a, b] → ℜ n
     * c.IsSimple ⇔ ∀ x1, x2 ∈ [a, b]: [ f(x1)=f(x2) ∧ x1<x2] ⇒ [x1=a ∧ x2=b]
     * A Curve is closed if its start Point is equal to its end Point (Reference [1], section 3.12.7.3).
     * c.IsClosed ⇔ [f(a) = f(b)]
     *
     * \details The boundary of a closed Curve is empty.
     * c.IsClosed ⇔ [c.boundary = ∅] A Curve that is simple and closed is a Ring.
     * The boundary of a non-closed Curve consists of its two end Points (Reference [1], section 3.12.3.2).
     * A Curve is defined as topologically closed, that is, it contains its endpoints f(a) and f(b).
     */
    class ICurve : public virtual IGeometry
    {

    public:
      /*!
       * \brief ICurve destructor.
       */
      virtual ~ICurve() = default;

      /*!
       * \brief The length of this ICurve in its associated ISpatialReferenceSystem.
       */
      [[nodiscard]] virtual double length() const = 0;

      /*!
       * \brief The start IPoint of this ICurve.
       */
      [[nodiscard]] virtual IPoint *startPoint() const = 0;

      /*!
       * \brief The end IPoint of this ICurve.
       */
      [[nodiscard]] virtual IPoint *endPoint() const = 0;

      /*!
       * \returns True if this ICurve is closed[startPoint() = endPoint()].
       */
      [[nodiscard]] virtual bool isClosed() const = 0;

      /*!
       * \returns True if this ICurve is closed [startPoint() = endPoint()]
       * and this ICurve is simple(does not pass through the same Point more than once).
       */
      [[nodiscard]] virtual bool isRing() const = 0;
    };

    /*!
     * \brief An IMultiCurve is a 1-dimensional IGeometryCollection whose elements are ICurves.
     *
     * \details IMultiCurve is a non-instantiable class in this standard;
     * it defines a set of methods for its subclasses and is
     * included for reasons of extensibility.
     *
     * \details An IMultiCurve is simple if and only if all of its elements
     * are simple and the only intersections between any two
     * elements occur at Points that are on the boundaries of both elements.
     *
     * \details The boundary of a IMultiCurve is obtained by applying
     * the "mod 2" union rule: A Point is in the boundary of a
     * IMultiCurve if it is in the boundaries of an
     * odd number of elements of the IMultiCurve
     *
     * \details A MultiCurve is closed if all of its elements are closed.
     * The boundary of a closed MultiCurve is always empty.
     *
     * \details A MultiCurve is defined as topologically closed.
     */
    class IMultiCurve : public virtual IGeometryCollection
    {
    public:
      virtual ~IMultiCurve() = default;

      /*!
       * \returns 1 (TRUE) if this MultiCurve is
       * closed[startPoint ( ) = endPoint ( ) for each ICurve in this IMultiCurve].
       */
      [[nodiscard]] virtual bool isClosed() const = 0;

      /*!
       * \brief The Length of this IMultiCurve which is equal to the sum
       * of the lengths of the element ICurves.
       */
      [[nodiscard]] virtual double length() const = 0;
    };

    /*!
     * \brief An ILineString is a Curve with linear interpolation between Points.
     * Each consecutive pair of Points defines a Line segment.
     */
    class ILineString : public virtual ICurve
    {
    public:
      /*!
       * \brief ILineString destructor.
       */
      virtual ~ILineString() = default;

      /*!
       * \brief The number of IPoints in this ILineString.
       */
      [[nodiscard]] virtual int pointCount() const = 0;

      /*!
       * \returns the specified IPoint at index in this ILineString.
       */
      [[nodiscard]] virtual IPoint *point(int index) const = 0;
    };

    /*!
     * \brief An IMultiLineString is an IMultiCurve whose elements are ILineStrings.
     */
    class IMultiLineString : public virtual IMultiCurve
    {
    public:
      /*!
       * \brief IMultiLineString destructor.
       */
      virtual ~IMultiLineString() = default;

      //! Returns the ILineString at index
      [[nodiscard]] virtual ILineString *lineString(int index) const = 0;
    };

    /*!
     * \brief The ILine class is an ILineString with exactly 2 IPoints.
     */
    class ILine : public virtual ILineString
    {
    public:
      /*!
       * \brief ILine destructor.
       */
      virtual ~ILine() = default;
    };

    /*!
     * \brief An ILinearRing is an ILineString that is both isClosed() and isSimple().
     */
    class ILinearRing : public virtual ILineString
    {
    public:
      /*!
       * \brief ILinearRing destructor.
       */
      virtual ~ILinearRing() = default;
    };

    /*!
     * \brief A directed edge from one vertex to another, adjacent to two faces.
     * Based on Dani Lischinski's code from Graphics Gems IV.
     * Original quad-edge data structure due to Guibas and Stolfi (1985). Does not inherit from geometry to reduce size.
     * since each edge has 4 pointers.
     */
    class IEdge
    {
    public:
      /*!
       * \brief IEdge destructor.
       */
      virtual ~IEdge() = default;

      /*!
       * \brief unique index identifier
       * \return
       */
      [[nodiscard]] virtual unsigned int index() const = 0;

      /*!
       * \brief The origin IVertex of this IEdge.
       * \returns the origin of this IEdge; NULL if currently unknown.
       */
      [[nodiscard]] virtual IVertex *orig() const = 0;

      /*!
       * \brief The destination IVertex of this IEdge.
       * \returns the destination of this IEdge; NULL if currently unknown.
       */
      [[nodiscard]] virtual IVertex *dest() const = 0;

      /*!
       * \brief The left face of this edge.
       * \returns the left face of this edge;
       *  Null if currently unknown
       */
      [[nodiscard]] virtual IPolygon *left() const = 0;

      /*!
       * \brief The right face of this edge.
       * \returns The right face of this edge;
       *    null if currently unknown
       */
      [[nodiscard]] virtual IPolygon *right() const = 0;

      /*!
       * \brief The target face of this edge, if dual. Otherwise null if not dual.
       * \return The target face polygon, or null if not dual.
       */
      [[nodiscard]] virtual IPolygon *face() const = 0;

      /*!
       * \brief The dual of this edge, directed from its right to its left.
       * \returns The right to left dual of this edge;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *rot() const = 0;

      /*!
       * \brief The dual of this IEdge, directed from its left to its right.
       * \returns The left to right dual of this edge;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *invRot() const = 0;

      /*!
       * \brief The IEdge from the destination to the origin of this IEdge.
       * \returns The symmetric of this edge;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *sym() const = 0;

      /*!
       * \brief The next ccw edge around (from) the origin of this IEdge.
       * \returns The next edge from the origin;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *origNext() const = 0;

      /*!
       * \brief The next cw edge around (from) the origin of this edge.
       * \returns The previous edge from the origin;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *origPrev() const = 0;

      /*!
       * \brief The next ccw edge around (into) the destination of this edge.
       * \returns The next edge to the destination;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *destNext() const = 0;

      /*!
       * \brief The next cw edge around (into) the destination of this edge.
       * \returns The previous edge to the destination;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *destPrev() const = 0;

      /*!
       * \brief The ccw edge around the left face following this edge.
       * \returns The next left face edge;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *leftNext() const = 0;

      /*!
       * \brief The ccw edge around the left face before this edge.
       * \returns The previous left face edge;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *leftPrev() const = 0;

      /*!
       * \brief The edge around the right face ccw following this edge.
       * \returns The next right face edge;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *rightNext() const = 0;

      /*!
       * \brief The IEdge around the right face ccw before this IEdge.
       * \returns The previous right face edge;
       *    will be nonnull
       */
      [[nodiscard]] virtual IEdge *rightPrev() const = 0;
    };

    /*!
     * \brief A ISurface is a 2-dimensional IGeometry object.
     *
     * \details A simple Surface may consists of a single patch that is associated with one exterior boundary and 0 or more
     * interior boundaries. A single such Surface patch in 3-dimensional space is isometric to planar Surfaces, by a
     * simple affine rotation matrix that rotates the patch onto the plane z = 0. If the patch is not vertical,
     * the projection onto the same plane is an isomorphism, and can be represented as a linear transformation, i.e. an affine.
     *
     * \details Polyhedral Surfaces are formed by stitching together such simple Surfaces patches along their common
     * boundaries. Such polyhedral Surfaces in a 3-dimensional space may not be planar as a whole, depending on the
     * orientation of their planar normals (Reference [1], sections 3.12.9.1, and 3.12.9.3). If all the patches are in
     * alignment (their normals are parallel), then the whole stitched polyhedral surface is co-planar and can be
     * represented as a single patch if it is connected.
     *
     * \details The boundary of a simple Surface is the set of closed Curves corresponding to its exterior and interior�
     * boundaries (Reference [1], section 3.12.9.4).
     *
     * \details The only instantiable subclasses of ISurface defined in this standard are IPolygon and IPolyhedralSurface. A
     * IPolygon is a simple Surface that is planar. A IPolyhedralSurface is a simple surface, consisting of some number of
     * IPolygon patches or facets. If a IPolyhedralSurface is closed, then it bounds a solid. A IMultiSurface containing a set
     * of closed IPolyhedralSurfaces can be used to represent a ISolid object with holes.
     */
    class ISurface : public virtual IGeometry
    {
    public:
      /*!
       * \brief ISurface destructor.
       */
      virtual ~ISurface() = default;

      /*!
       * \brief The area of this ISurface, as measured in
       * the spatial reference system of this ISurface.
       */
      [[nodiscard]] virtual double area() const = 0;

      /*!
       * \brief The mathematical centroid for this ISurface as a Point.
       * The result is not guaranteed to be on this ISurface.
       */
      [[nodiscard]] virtual IPoint *centroid() const = 0;

      /*!
       * \brief A Point guaranteed to be on this Surface.
       */
      [[nodiscard]] virtual IPoint *pointOnSurface() const = 0;

      /*!
       * \brief Gets the boundary of this surface as a multi-curve.
       * \return The boundary multi-curve of this surface.
       */
      [[nodiscard]] virtual IMultiCurve *boundaryMultiCurve() const = 0;
    };

    /*!
     * \brief A MultiSurface is a 2-dimensional GeometryCollection whose elements
     * are Surfaces, all using coordinates from the same coordinate reference system.
     */
    class IMultiSurface : public virtual IGeometryCollection
    {

    public:
      /*!
       * \brief IMultiSurface destructor.
       */
      virtual ~IMultiSurface() = default;

      /*!
       * \brief The area of this ISurface, as measured in
       * the spatial reference system of this ISurface.
       */
      [[nodiscard]] virtual double area() const = 0;

      /*!
       * \brief The mathematical centroid for this ISurface as an IPoint.
       * The result is not guaranteed to be on this ISurface.
       */
      [[nodiscard]] virtual IPoint *centroid() const = 0;

      /*!
       * \brief A Point guaranteed to be on this ISurface.
       */
      [[nodiscard]] virtual IPoint *pointOnSurface() const = 0;
    };

    /*!
     * \brief A IPolygon is a planar ISurface defined by 1
     * exterior boundary and 0 or more interior boundaries.
     *
     * \details Each interior boundary defines a hole in the IPolygon.
     *
     * \details The exterior boundary LinearRing defines the “top” of
     * the surface which is the side of the surface from which the
     * exterior boundary appears to traverse the boundary in a counter
     * clockwise direction. The interior LinearRings will
     * have the opposite orientation, and appear as clockwise when viewed from the “top”,
     *
     * \details The assertions for Polygons (the rules that define valid Polygons) are as follows:
     * a) Polygons are topologically closed;
     * b) The boundary of a Polygon consists of a set of ILinearRings that make up its exterior and interior boundaries;
     * c) No two Rings in the boundary cross and the Rings in the boundary of a IPolygon may intersect at a Point but
     * only as a tangent, e.g.
     *
     * ∀ P ∈ Polygon, ∀ c1,c2∈P.Boundary(), c1≠c2,
     * ∀ p, q ∈Point, p, q ∈ c1, p ≠ q ,
     * [p ∈ c2] ⇒ [∃ δ > 0 ∋ [|p-q|<δ] ⇒ [q ∉ c2] ];
     *
     * \note This last condition says that at a point common to the two curves,
     * nearby points cannot be common. This
     * forces each common point to be a point of tangency.
     *
     * d) A Polygon may not have cut lines, spikes or punctures e.g.:
     * ∀ P ∈ Polygon, P = P.Interior.Closure;
     * e) The interior of every Polygon is a connected point set;
     * f) The exterior of a Polygon with 1 or more holes is not
     * connected. Each hole defines a connected component of
     * the exterior.
     * In the above assertions, interior, closure and exterior
     * have the standard topological definitions. The combination
     * of (a) and (c) makes a Polygon a regular closed IPoint set.
     * IPolygons are simple geometric objects.
     */
    class IPolygon : public virtual ISurface
    {
    public:
      /*!
       * \brief IPolygon destructor.
       */
      virtual ~IPolygon() = default;

      /*!
       * \returns the exterior ring of this IPolygon.
       */
      [[nodiscard]] virtual ILineString *exteriorRing() const = 0;

      /*!
       * \returns the number of interior rings in this IPolygon.
       */
      [[nodiscard]] virtual int interiorRingCount() const = 0;

      /*!
       * \returns the index - th interior ring for this IPolygon as a ILineString.
       */
      [[nodiscard]] virtual ILineString *interiorRing(int index) const = 0;

      /*!
       * \brief An arbitrary adjacent edge for this IPolygon.
       * \returns An edge that is adjacent to this face;
       *    null if degenerate
       */
      [[nodiscard]] virtual IEdge *edge() const = 0;

      /*!
       * \brief Gets the polyhedral surface this polygon belongs to.
       * \returns The surface this IPolygon belongs to, or null if not associated with a surface.
       */
      [[nodiscard]] virtual IPolyhedralSurface *polyhedralSurface() const = 0;
    };

    /*!
     * \brief An IMultiPolygon is an IMultiSurface whose elements are IPolygons.
     */
    class IMultiPolygon : public virtual IMultiSurface
    {

    public:
      virtual ~IMultiPolygon() = default;

      /*!
       * \returns the index sup(th) polygon in this IMultiPolygon/IGeometryCollection.
       */
      [[nodiscard]] virtual IPolygon *polygon(int index) const = 0;
    };

    /*!
     * \brief An ITriangle is a IPolygon with 3 distinct,
     * non-collinear vertices and no interior boundary.
     */
    class ITriangle : public virtual IPolygon
    {
    public:
      /*!
       * \brief ITriangle destructor.
       */
      virtual ~ITriangle() = default;

      /*!
       * \brief The first vertex of this ITriangle.
       */
      [[nodiscard]] virtual IVertex *vertex1() const = 0;

      /*!
       * \brief The second vertex of this ITriangle.
       */
      [[nodiscard]] virtual IVertex *vertex2() const = 0;

      /*!
       * \brief The third vertex of this ITriangle.
       */
      [[nodiscard]] virtual IVertex *vertex3() const = 0;

      /*!
       * \brief The vertex of this ITriangle at the specified index.
       */
      [[nodiscard]] virtual IVertex *vertex(int index) const = 0;
    };

    /*!
     * \brief INetwork represents a graph structure of connected vertices and edges.
     */
    class INetwork : public virtual IIdentity
    {
    public:
      /*!
       * \brief INetwork destructor.
       */
      virtual ~INetwork() = default;

      /*!
       * \brief The number of edges in the network.
       */
      [[nodiscard]] virtual int64_t edgeCount() const = 0;

      /*!
       * \brief The IEdge at the specified index.
       * \param[in] index of the edge to retrieve.
       */
      [[nodiscard]] virtual IEdge *edge(int64_t index) const = 0;

      /*!
       * \brief The number of vertices in the network.
       */
      [[nodiscard]] virtual int64_t vertexCount() const = 0;

      /*!
       * \brief The IVertex at the specified index.
       * \param[in] index of the vertex to retrieve.
       */
      [[nodiscard]] virtual IVertex *vertex(int64_t index) const = 0;

      /*!
       * \brief Bulk structure-of-arrays view of this network's geometry and connectivity.
       * \details This is the accessor partitioners, interpolating adapters, IO writers,
       * and device staging must use; the per-entity object accessors above are a
       * convenience for spot queries and editing.
       * \returns The IMeshView over this network; never nullptr.
       */
      [[nodiscard]] virtual const IMeshView *meshView() const = 0;
    };

    /*!
     * \brief An IPolyhedralSurface is a contiguous collection of polygon patches or facets
     * stitched together along their shared boundary edges.
     * \details If a IPolyhedralSurface is closed it bounds a solid. Patches are
     * consistently oriented; boundary edges are shared by at most two patches.
     */
    class IPolyhedralSurface : public virtual ISurface
    {
    public:
      /*!
       * \brief IPolyhedralSurface destructor.
       */
      virtual ~IPolyhedralSurface() = default;

      /*!
       * \brief The number of polygon patches in this surface.
       */
      [[nodiscard]] virtual int64_t patchCount() const = 0;

      /*!
       * \brief The IPolygon patch at the specified index.
       * \param[in] index of the patch to retrieve.
       */
      [[nodiscard]] virtual IPolygon *patch(int64_t index) const = 0;

      /*!
       * \brief The number of vertices in this surface.
       */
      [[nodiscard]] virtual int64_t vertexCount() const = 0;

      /*!
       * \brief The IVertex at the specified index.
       * \param[in] index of the vertex to retrieve.
       */
      [[nodiscard]] virtual IVertex *vertex(int64_t index) const = 0;

      /*!
       * \brief The collection of polygons in this surface that bound the given polygon.
       * \param[in] polygon whose bounding polygons are requested.
       */
      [[nodiscard]] virtual IMultiPolygon *boundingPolygons(const IPolygon *polygon) const = 0;

      /*!
       * \brief Checks whether this surface is closed and therefore bounds a solid.
       */
      [[nodiscard]] virtual bool isClosed() const = 0;

      /*!
       * \brief Bulk structure-of-arrays view of this surface's geometry and connectivity.
       * \details This is the accessor partitioners, interpolating adapters, IO writers,
       * and device staging must use; the per-entity object accessors above are a
       * convenience for spot queries and editing.
       * \returns The IMeshView over this surface; never nullptr.
       */
      [[nodiscard]] virtual const IMeshView *meshView() const = 0;
    };

    /*!
     * \brief An ITIN is a triangulated irregular network
     * IPolyhedralSurface consisting only of ITriangle patches.
     */
    class ITIN : public virtual IPolyhedralSurface
    {
    public:
      /*!
       * \brief ITIN destructor.
       */
      virtual ~ITIN() = default;

      /*!
       * \brief The ITriangle patch at the specified index.
       * \param[in] index of the triangle to retrieve.
       */
      [[nodiscard]] virtual ITriangle *triangle(int64_t index) const = 0;
    };

    /*!
     * \brief A Raster spatial feature.
     */
    class IRaster : public virtual IIdentity
    {

    public:
      /*!
       * \brief The data type associated with a raster.
       */
      enum class RasterDataType
      {
        //! Unknown or unspecified type
        Unknown,
        //! Eight bit unsigned integer
        Byte,
        //! Sixteen bit unsigned integer
        UInt16,
        //! Sixteen bit signed integer
        Int16,
        //! Thirty two bit unsigned integer
        UInt32,
        //! Thirty two bit signed integer
        Int32,
        //! Thirty two bit floating point
        Float32,
        //! Sixty four bit floating point
        Float64,
        //! Complex Int16
        CInt16,
        //! Complex Int32
        CInt32,
        //! Complex Float32
        CFloat32,
        //! Complex Float64
        CFloat64,
        //! Color, alpha, red, green, blue, 4 bytes the same as QImage::Format_ARGB32
        ARGB32,
        //! Color, alpha, red, green, blue, 4 bytes  the same as QImage::Format_ARGB32_Premultiplied
        ARGB32_Premultiplied,
      };

      /*!
       * \brief IRaster destructor.
       */
      virtual ~IRaster() = 0;

      /*!
       * \brief Number of pixels in the x direction.
       */
      [[nodiscard]] virtual int xSize() const = 0;

      /*!
       * \brief Number of pixels in y direction.
       */
      [[nodiscard]] virtual int ySize() const = 0;

      /*!
       * \brief Number of raster bands.
       */
      [[nodiscard]] virtual int rasterBandCount() const = 0;

      /*!
       * \brief Adds a new IRasterBand.
       */
      virtual void addRasterBand(RasterDataType dataType) = 0;

      /*!
       * \brief The ISpatialReferenceSystem represents the spatial reference system of this raster.
       */
      [[nodiscard]] virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

      /*!
       * \brief Fetches the affine transformation coefficients. It is an array of size 6.
       * \details Fetches the coefficients for transforming between pixel/line (P,L)
       * raster space, and projection coordinates (Xp,Yp) space.
       * Xp = transformationMatrix[0] + P*transformationMatrix[1] + L*transformationMatrix[2];
       * Yp = transformationMatrix[3] + P*transformationMatrix[4] + L*transformationMatrix[5];
       * In a north up image, transformationMatrix[1] is the pixel width, and transformationMatrix[5] is the pixel height. The upper left corner of the upper left pixel is at position (transformationMatrix[0],transformationMatrix[3]).
       */
      virtual void geoTransformation(double *transformationMatrix) = 0;

      /*!
       * \brief Gets the IRasterBand for the band with index bandIndex.
       */
      [[nodiscard]] virtual IRasterBand *getRasterBand(int bandIndex) const = 0;
    };

    /*!
     * \brief IRasterBand is a single raster band for an IRaster.
     */
    class IRasterBand : public virtual IIdentity
    {

    public:
      /*!
       * \brief IRasterBand destructor.
       */
      virtual ~IRasterBand() = default;

      //! Number of pixels in the x direction
      [[nodiscard]] virtual int xSize() const = 0;

      //! Number of pixels in y direction
      [[nodiscard]] virtual int ySize() const = 0;

      //! Parent IRaster of this IRasterBand
      [[nodiscard]] virtual IRaster *raster() const = 0;

      //! Raster data type
      [[nodiscard]] virtual IRaster::RasterDataType dataType() const = 0;

      /*!
       * \brief Reads data into the image block.
       * \param[in] xOffset is the pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
       * \param[in] yOffset is the line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
       * \param[in] xSize is the width of the region of the band to be accessed in pixels.
       * \param[in] ySize is the height of the region of the band to be accessed in lines.
       * \param[out] image is the pointer to where data is to be written. Must be pre-allocated with the correct size.
       */
      virtual void read(int xOffset, int yOffset, int xSize, int ySize, void *image) const = 0;

      /*!
       * \brief Writes image into the raster band.
       * \param[in] xOffset is the pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.
       * \param[in] yOffset is the line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.
       * \param[in] xSize is the width of the region of the band to be accessed in pixels.
       * \param[in] ySize is the height of the region of the band to be accessed in lines.
       * \param[in] image is the pointer to the image data to be written to the raster band.
       */
      virtual void write(int xOffset, int yOffset, int xSize, int ySize, const void *image) = 0;

      /*!
       * The nodata value for this IRasterBand.
       */
      [[nodiscard]] virtual double noData() const = 0;
    };

    /*!
     * \brief IRegularGrid2D represents a two-dimensional structured grid
     * of nodes and cells.
     */
    class IRegularGrid2D : public virtual IIdentity
    {
    public:
      /*!
       * \brief IRegularGrid2D destructor.
       */
      virtual ~IRegularGrid2D() = default;

      /*!
       * \brief The ISpatialReferenceSystem represents the spatial reference system of this geometric object.
       */
      [[nodiscard]] virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

      /*!
       * \brief Gets the type of regular grid.
       * \return The RegularGridType of this 2D grid.
       */
      [[nodiscard]] virtual RegularGridType gridType() const = 0;

      /*!
       * \brief numXNodes represents the number of nodes in the x direction.
       * \return number of nodes in the x direction.
       */
      [[nodiscard]] virtual int numXNodes() const = 0;

      /*!
       * \brief numYNodes represents the number of nodes in the y direction.
       * \return number of nodes in the y direction.
       */
      [[nodiscard]] virtual int numYNodes() const = 0;

      /*!
       * \brief xNodeLocation provides the x location coordinate for the x-node and y-node indexes.
       * \details Per-node convenience accessor; bulk consumers must use nodeXs().
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns x location coordinate for the x-node and y-node provided
       */
      [[nodiscard]] virtual double xNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief yNodeLocation provides the y location coordinate for the x-node and y-node indexes.
       * \details Per-node convenience accessor; bulk consumers must use nodeYs().
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns y location coordinate for the x-node and y-node provided
       */
      [[nodiscard]] virtual double yNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief Bulk x coordinates of all nodes, row-major [yNode][xNode].
       * \details The span has numYNodes()*numXNodes() elements and remains valid until
       * the grid geometry changes.
       */
      [[nodiscard]] virtual std::span<const double> nodeXs() const = 0;

      /*!
       * \brief Bulk y coordinates of all nodes, row-major [yNode][xNode].
       * \details The span has numYNodes()*numXNodes() elements and remains valid until
       * the grid geometry changes.
       */
      [[nodiscard]] virtual std::span<const double> nodeYs() const = 0;

      /*!
       * \brief isActive is a bool indicating whether a cell is active.
       * \details Per-cell convenience accessor; bulk consumers must use activeCells().
       * \param xCellIndex the x cell index for the cell. Must be less than numXNodes() - 1.
       * \param yCellIndex the y cell index for the cell. Must be less than numYNodes() - 1.
       * \return a bool indicating whether a cell is active.
       */
      [[nodiscard]] virtual bool isActive(int xCellIndex, int yCellIndex) const = 0;

      /*!
       * \brief Bulk activity mask of all cells, row-major [yCell][xCell]; nonzero means active.
       * \details The span has (numYNodes()-1)*(numXNodes()-1) elements and remains valid
       * until the grid changes.
       */
      [[nodiscard]] virtual std::span<const uint8_t> activeCells() const = 0;
    };

    /*!
     * \brief IRegularGrid3D represents a three-dimensional structured grid
     * of nodes and cells.
     */
    class IRegularGrid3D : public virtual IIdentity
    {
    public:
      /*!
       * \brief IRegularGrid3D destructor.
       */
      virtual ~IRegularGrid3D() = default;

      /*!
       * \brief The ISpatialReferenceSystem represents the spatial reference system of this grid.
       */
      [[nodiscard]] virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

      /*!
       * \brief Gets the type of regular grid.
       * \return The RegularGridType of this 3D grid.
       */
      [[nodiscard]] virtual RegularGridType gridType() const = 0;

      /*!
       * \brief numXNodes represents the number of nodes in the x direction.
       * \return number of nodes in the x direction.
       */
      [[nodiscard]] virtual int numXNodes() const = 0;

      /*!
       * \brief numYNodes represents the number of nodes in the y direction.
       * \return number of nodes in the y direction.
       */
      [[nodiscard]] virtual int numYNodes() const = 0;

      /*!
       * \brief numZNodes represents the number of nodes in the z direction.
       * \return number of nodes in the z direction.
       */
      [[nodiscard]] virtual int numZNodes() const = 0;

      /*!
       * \brief xNodeLocation provides the x location coordinate for the x-node and y-node indexes.
       * \details Per-node convenience accessor; bulk consumers must use nodeXs().
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns x location coordinate for the x-node and y-node provided
       */
      [[nodiscard]] virtual double xNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief yNodeLocation provides the y location coordinate for the x-node and y-node indexes.
       * \details Per-node convenience accessor; bulk consumers must use nodeYs().
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns y location coordinate for the x-node and y-node provided
       */
      [[nodiscard]] virtual double yNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief zNodeLocation provides the z location coordinate for the x-node, y-node, and z-node indexes.
       * \details Per-node convenience accessor; bulk consumers must use nodeZs().
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \param zNodeIndex the z-node index
       * \return returns z location coordinate for the x-node, y-node, and z-node indexes.
       */
      [[nodiscard]] virtual double zNodeLocation(int xNodeIndex, int yNodeIndex, int zNodeIndex) const = 0;

      /*!
       * \brief Bulk x coordinates of all nodes in a horizontal layer, row-major [yNode][xNode].
       * \details The span has numYNodes()*numXNodes() elements (plan coordinates are
       * layer-invariant) and remains valid until the grid geometry changes.
       */
      [[nodiscard]] virtual std::span<const double> nodeXs() const = 0;

      /*!
       * \brief Bulk y coordinates of all nodes in a horizontal layer, row-major [yNode][xNode].
       * \details The span has numYNodes()*numXNodes() elements (plan coordinates are
       * layer-invariant) and remains valid until the grid geometry changes.
       */
      [[nodiscard]] virtual std::span<const double> nodeYs() const = 0;

      /*!
       * \brief Bulk z coordinates of all nodes, row-major [zNode][yNode][xNode].
       * \details The span has numZNodes()*numYNodes()*numXNodes() elements and remains
       * valid until the grid geometry changes.
       */
      [[nodiscard]] virtual std::span<const double> nodeZs() const = 0;

      /*!
       * \brief isActive is a bool indicating whether a cell is active.
       * \details Per-cell convenience accessor; bulk consumers must use activeCells().
       * \param xCellIndex the x cell index for the cell. Must be less than numXNodes() - 1.
       * \param yCellIndex the y cell index for the cell. Must be less than numYNodes() - 1.
       * \param zCellIndex the z cell index for the cell. Must be less than numZNodes() - 1.
       * \return a bool indicating whether a cell is active.
       */
      [[nodiscard]] virtual bool isActive(int xCellIndex, int yCellIndex, int zCellIndex) const = 0;

      /*!
       * \brief Bulk activity mask of all cells, row-major [zCell][yCell][xCell]; nonzero means active.
       * \details The span has (numZNodes()-1)*(numYNodes()-1)*(numXNodes()-1) elements
       * and remains valid until the grid changes.
       */
      [[nodiscard]] virtual std::span<const uint8_t> activeCells() const = 0;
    };

    /*!
     * \brief IMeshView is a bulk, structure-of-arrays view of an unstructured mesh or
     * network: flat coordinate spans plus CSR (compressed sparse row) connectivity.
     *
     * \details The layout is deliberately congruent with the UGRID conventions so that
     * persistence (face_node_connectivity, node coordinate arrays), message packing,
     * and device staging can all consume the view without transformation. All spans
     * remain valid until the underlying mesh topology or geometry changes. Obtained
     * from INetwork::meshView(), IPolyhedralSurface::meshView(), and their
     * specializations.
     */
    class IMeshView
    {
    public:
      /*!
       * \brief IMeshView destructor.
       */
      virtual ~IMeshView() = default;

      /*!
       * \brief The number of nodes (vertices) in the mesh.
       */
      [[nodiscard]] virtual int64_t nodeCount() const = 0;

      /*!
       * \brief The number of edges in the mesh.
       */
      [[nodiscard]] virtual int64_t edgeCount() const = 0;

      /*!
       * \brief The number of faces (patches/cells) in the mesh; 0 for a pure network.
       */
      [[nodiscard]] virtual int64_t faceCount() const = 0;

      /*!
       * \brief x coordinates of all nodes; the span has nodeCount() elements.
       */
      [[nodiscard]] virtual std::span<const double> nodeX() const = 0;

      /*!
       * \brief y coordinates of all nodes; the span has nodeCount() elements.
       */
      [[nodiscard]] virtual std::span<const double> nodeY() const = 0;

      /*!
       * \brief z coordinates of all nodes; the span has nodeCount() elements, or is empty for a 2D mesh.
       */
      [[nodiscard]] virtual std::span<const double> nodeZ() const = 0;

      /*!
       * \brief CSR row offsets into faceNodes(): the nodes of face f are
       * faceNodes()[faceNodeOffsets()[f] ... faceNodeOffsets()[f+1]).
       * \details The span has faceCount() + 1 elements; it is empty for a pure network.
       */
      [[nodiscard]] virtual std::span<const int64_t> faceNodeOffsets() const = 0;

      /*!
       * \brief Concatenated node indexes of all faces, ordered counter-clockwise per face.
       */
      [[nodiscard]] virtual std::span<const int64_t> faceNodes() const = 0;

      /*!
       * \brief Node index pairs of all edges: edge e connects edgeNodes()[2*e] and edgeNodes()[2*e + 1].
       * \details The span has 2 * edgeCount() elements.
       */
      [[nodiscard]] virtual std::span<const int64_t> edgeNodes() const = 0;
    };

    /*!
     * \brief IGeometryComponentDataItem is an IComponentDataItem whose data is associated
     * with a collection of IGeometry objects. This class must be implemented as an abstract class.
     * \details Canonical dimension ordering: the geometry dimension is dimension 0 of
     * shape(); any additional dimensions follow. Data access uses the inherited
     * getValuesInto()/setValuesFrom() hyperslab API.
     */
    class IGeometryComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief IGeometryComponentDataItem destructor.
       */
      virtual ~IGeometryComponentDataItem() = default;

      /*!
       * \brief The type of the geometries this data item is associated with.
       */
      [[nodiscard]] virtual IGeometry::GeometryType geometryType() const = 0;

      /*!
       * \brief The number of geometries associated with this data item.
       */
      [[nodiscard]] virtual int64_t geometryCount() const = 0;

      /*!
       * \brief The IGeometry at the specified index.
       * \param[in] geometryIndex of the geometry to retrieve.
       */
      [[nodiscard]] virtual IGeometry *geometry(int64_t geometryIndex) const = 0;

      /*!
       * \brief The IDimension of the geometries (dimension 0 of shape()).
       */
      [[nodiscard]] virtual HydroCouple::IDimension *geometryDimension() const = 0;

      /*!
       * \brief The IEnvelope bounding all geometries of this data item.
       */
      [[nodiscard]] virtual HydroCouple::Spatial::IEnvelope *envelope() const = 0;
    };

    /*!
     * \brief INetworkComponentDataItem is an IComponentDataItem whose data is
     * associated with the edges and/or vertices of an INetwork.
     * \details Canonical dimension ordering: the entity dimension selected by
     * networkDataType() (edge or vertex) is dimension 0 of shape(); any additional
     * dimensions follow. Data access uses the inherited getValuesInto()/setValuesFrom()
     * hyperslab API.
     */
    class INetworkComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief INetworkComponentDataItem destructor.
       */
      virtual ~INetworkComponentDataItem() = default;

      /*!
       * \brief The INetwork this data item is associated with.
       */
      [[nodiscard]] virtual INetwork *network() const = 0;

      /*!
       * \brief The kind of network object this data item's values describe.
       */
      [[nodiscard]] virtual NetworkDataObjectType networkDataObjectType() const = 0;

      /*!
       * \brief The mesh entity (edge or vertex) this data item's values are attached to.
       */
      [[nodiscard]] virtual SpatialDataType networkDataType() const = 0;

      /*!
       * \brief The IDimension of the network edges.
       */
      [[nodiscard]] virtual IDimension *edgeDimension() const = 0;

      /*!
       * \brief The IDimension of the network vertices.
       */
      [[nodiscard]] virtual IDimension *vertexDimension() const = 0;
    };

    /*!
     * \brief IPolyhedralSurfaceComponentDataItem is an IComponentDataItem whose data
     * is associated with the patches, edges, or vertices of an IPolyhedralSurface.
     * \details Canonical dimension ordering: the entity dimension selected by
     * meshDataType() (patch, edge, or vertex) is dimension 0 of shape(); any additional
     * dimensions follow. Data access uses the inherited getValuesInto()/setValuesFrom()
     * hyperslab API.
     */
    class IPolyhedralSurfaceComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief IPolyhedralSurfaceComponentDataItem destructor.
       */
      virtual ~IPolyhedralSurfaceComponentDataItem() = default;

      /*!
       * \brief The kind of mesh object this data item's values describe.
       */
      [[nodiscard]] virtual MeshDataObjectType meshDataObjectType() const = 0;

      /*!
       * \brief The mesh entity (patch, edge, or vertex) this data item's values are attached to.
       */
      [[nodiscard]] virtual SpatialDataType meshDataType() const = 0;

      /*!
       * \brief The IPolyhedralSurface this data item is associated with.
       */
      [[nodiscard]] virtual IPolyhedralSurface *polyhedralSurface() const = 0;

      /*!
       * \brief The IDimension of the surface patches.
       */
      [[nodiscard]] virtual IDimension *patchDimension() const = 0;

      /*!
       * \brief The IDimension of the surface edges.
       */
      [[nodiscard]] virtual IDimension *edgeDimension() const = 0;

      /*!
       * \brief The IDimension of the surface vertices.
       */
      [[nodiscard]] virtual IDimension *vertexDimension() const = 0;
    };

    /*!
     * \brief ITINComponentDataItem is an IPolyhedralSurfaceComponentDataItem whose
     * surface is an ITIN.
     */
    class ITINComponentDataItem : public virtual IPolyhedralSurfaceComponentDataItem
    {
    public:
      /*!
       * \brief ITINComponentDataItem destructor.
       */
      virtual ~ITINComponentDataItem() = default;

      /*!
       * \brief The ITIN this data item is associated with.
       */
      [[nodiscard]] virtual ITIN *TIN() const = 0;
    };

    /*!
     * \brief IRasterComponentDataItem is an IComponentDataItem whose data is
     * associated with an IRaster.
     * \details Canonical dimension ordering: band is dimension 0, y (row) is
     * dimension 1, x (column) is dimension 2 of shape(). Data access uses the
     * inherited getValuesInto()/setValuesFrom() hyperslab API.
     */
    class IRasterComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief IRasterComponentDataItem destructor.
       */
      virtual ~IRasterComponentDataItem() = default;

      /*!
       * \brief The IRaster this data item is associated with.
       */
      [[nodiscard]] virtual IRaster *raster() const = 0;

      /*!
       * \brief The IDimension of the raster columns (dimension 2 of shape()).
       */
      [[nodiscard]] virtual IDimension *xDimension() const = 0;

      /*!
       * \brief The IDimension of the raster rows (dimension 1 of shape()).
       */
      [[nodiscard]] virtual IDimension *yDimension() const = 0;

      /*!
       * \brief The IDimension of the raster bands (dimension 0 of shape()).
       */
      [[nodiscard]] virtual IDimension *bandDimension() const = 0;
    };

    /*!
     * \brief IRegularGrid2DComponentDataItem is an IComponentDataItem whose data is
     * associated with the cells of an IRegularGrid2D.
     * \details Canonical dimension ordering: y-cell is dimension 0, x-cell is
     * dimension 1 of shape(); the optional cell edge and cell vertex dimensions
     * follow. Data access uses the inherited getValuesInto()/setValuesFrom()
     * hyperslab API, so "one field over all cells" is a contiguous slab.
     */
    class IRegularGrid2DComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief IRegularGrid2DComponentDataItem destructor.
       */
      virtual ~IRegularGrid2DComponentDataItem() = default;

      /*!
       * \brief The IRegularGrid2D this data item is associated with.
       */
      [[nodiscard]] virtual IRegularGrid2D *grid() const = 0;

      /*!
       * \brief The kind of mesh object this data item's values describe.
       */
      [[nodiscard]] virtual MeshDataObjectType meshDataObjectType() const = 0;

      /*!
       * \brief The IDimension of the cells in the x direction (dimension 1 of shape()).
       */
      [[nodiscard]] virtual IDimension *xCellDimension() const = 0;

      /*!
       * \brief The IDimension of the cells in the y direction (dimension 0 of shape()).
       */
      [[nodiscard]] virtual IDimension *yCellDimension() const = 0;

      /*!
       * \brief The IDimension of the edges of a cell, when values are attached to cell edges.
       */
      [[nodiscard]] virtual IDimension *cellEdgeDimension() const = 0;

      /*!
       * \brief The IDimension of the vertices of a cell, when values are attached to cell vertices.
       */
      [[nodiscard]] virtual IDimension *cellVertexDimension() const = 0;
    };

    /*!
     * \brief IRegularGrid3DComponentDataItem is an IComponentDataItem whose data is
     * associated with the cells of an IRegularGrid3D.
     * \details Canonical dimension ordering: z-cell is dimension 0, y-cell is
     * dimension 1, x-cell is dimension 2 of shape(); the optional cell face and cell
     * vertex dimensions follow. Data access uses the inherited
     * getValuesInto()/setValuesFrom() hyperslab API.
     */
    class IRegularGrid3DComponentDataItem : public virtual IComponentDataItem
    {
    public:
      /*!
       * \brief IRegularGrid3DComponentDataItem destructor.
       */
      virtual ~IRegularGrid3DComponentDataItem() = default;

      /*!
       * \brief The IRegularGrid3D this data item is associated with.
       */
      [[nodiscard]] virtual IRegularGrid3D *grid() const = 0;

      /*!
       * \brief The kind of mesh object this data item's values describe.
       */
      [[nodiscard]] virtual MeshDataObjectType meshDataObjectType() const = 0;

      /*!
       * \brief The IDimension of the cells in the x direction (dimension 2 of shape()).
       */
      [[nodiscard]] virtual IDimension *xCellDimension() const = 0;

      /*!
       * \brief The IDimension of the cells in the y direction (dimension 1 of shape()).
       */
      [[nodiscard]] virtual IDimension *yCellDimension() const = 0;

      /*!
       * \brief The IDimension of the cells in the z direction (dimension 0 of shape()).
       */
      [[nodiscard]] virtual IDimension *zCellDimension() const = 0;

      /*!
       * \brief The IDimension of the faces of a cell, when values are attached to cell faces.
       */
      [[nodiscard]] virtual IDimension *cellFaceDimension() const = 0;

      /*!
       * \brief The IDimension of the vertices of a cell, when values are attached to cell vertices.
       */
      [[nodiscard]] virtual IDimension *cellVertexDimension() const = 0;
    };
  }
}

#endif // HYDROCOUPLESPATIAL_H
