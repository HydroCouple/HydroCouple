/*!
 * \file hydrocouplespatial.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0
 * \description
 * This header file contains the geospatial interface definitions for the
 * HydroCouple component-based modeling definitions.
 * \license
 * This file and its associated files, and libraries are free software.
 * You can redistribute it and/or modify it under the terms of the
 * MIT License as published by the Free Software Foundation.
 * This file and its associated files is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 * \copyright Copyright 2014-2025, Caleb Buahin, All rights reserved.
 * \date 2014-2025
 * \pre
 * \bug
 * \warning
 * \todo
 */

#ifndef HYDROCOUPLESPATIAL_H
#define HYDROCOUPLESPATIAL_H

#include "hydrocouple.h"

using namespace std;

namespace HydroCouple
{
  /*!
   * \brief HydroCouple's interfaces that have a spatial component.
   */
  namespace Spatial
  {
    class IEdge;
    class IRasterBand;
    class IPolygon;
    class IPolyhedralSurface;

    /*!
     * \brief The MeshDataType enum describes the part of the
     * geometry of the mesh that data corresponds to.
     */
    enum MeshDataObjectType
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
       * \brief The data corresponds to the edges of the mesh.
       */
      Face,
    };

    /*!
     * \brief The types of data available in a network.
     */
    enum NetworkDataObjectType
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
     * \brief The types of data available in a mesh.
     */
    enum MeshDataType
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
     * \brief The types of data available in a network.
     */
    enum NetworkDataType
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
    enum RegularGridType
    {
      //! Cartesian grid.
      Cartesian,
      //! Rectilinear grid.
      Rectilinear,
      //! Curvilinear grid.
      Curvilinear
    };

    /*!
     * \brief Spatial Reference System
     */
    class ISpatialReferenceSystem
    {

    public:
      /*!
       * \brief The Well-known Text Representation of the Spatial Reference System.
       */
      virtual ~ISpatialReferenceSystem() = default;

      /*!
       * \brief Returns the Spatial Reference System ID (SRID) for a geometric object.
       * This will normally be a foreign key to an index of reference systems
       * stored in either the same or some other datastore.
       */
      virtual int authSRID() const = 0;

      /*!
       * \brief The Authority Specific Spatial Reference System Identifier.
       */
      virtual string authName() const = 0;

      /*!
       * Well-known Text description of the Spatial Reference System.
       */
      virtual string srText() const = 0;

      /*!
       * \brief The measurement distance units for the Spatial Reference System.
       */
      virtual HydroCouple::IUnit::DistanceUnits distanceUnits() const = 0;
    };

    /*!
     * \brief The IEnvelope class
     */
    class IEnvelope
    {
    public:
      /*!
       * \brief The minimum x-coordinate value for this IEnvelope.
       */
      virtual ~IEnvelope() = default;

      /*!
       * \brief minX The minimum x-coordinate value for this IEnvelope.
       * \return
       */
      virtual double minX() const = 0;

      /*!
       * \brief maxX The maximum x-coordinate value for this IEnvelope.
       * \return
       */
      virtual double maxX() const = 0;

      /*!
       * \brief minY The minimum y-coordinate value for this IEnvelope.
       * \return
       */
      virtual double minY() const = 0;

      /*!
       * \brief maxY The maximum y-coordinate value for this IEnvelope.
       * \return
       */
      virtual double maxY() const = 0;

      /*!
       * \brief minZ The minimum z-coordinate value for this IEnvelope.
       * \return
       */
      virtual double minZ() const = 0;

      /*!
       * \brief maxZ The maximum z-coordinate value for this IEnvelope.
       * \return
       */
      virtual double maxZ() const = 0;
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
      enum GeometryType
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
       * \brief The type of IGeometry.
       */
      virtual ~IGeometry() = default;

      /*!
       * \brief id of the geometry.
       * \return id of the geometry.
       */
      virtual string id() const = 0;

      /*!
       * \brief index of the geometry if it is part of a collection.
       * \return index of the geometry in a collection.
       */
      virtual unsigned int index() const = 0;

      /*!
       * \brief The inherent dimension of this geometric object, which must be less than or equal to the coordinate dimension.
       *  In non - homogeneous collections, this will return the largest topological dimension of the contained objects.
       *
       * \returns 0 for points, 1 for lines and 2 for surfaces.
       *
       */
      virtual int dimension() const = 0;

      /*!
       * \brief Get the dimension of the coordinates in this object.
       *
       * \returns In practice this will return 2 or 3. It can also return 0 in the case of an empty point.
       */
      virtual int coordinateDimension() const = 0;

      /*!
       * \brief Get the dimension of the coordinates in this object.
       *
       * \details Returns an enum representing the instantiable subtype of Geometry of which this
       * geometric object is an instantiable member.The name of the subtype of Geometry is returned as a string.
       *
       * \returns In practice this will return 2 or 3. It can also return 0 in the case of an empty point.
       */
      virtual GeometryType geometryType() const = 0;

      /*!
       * \brief Spatial reference system of geometric object.
       */
      virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

      /*!
       * \brief The minimum bounding box for this Geometry, returned as a IGeometry. Recalculated at the time of the call
       * \returns The minimum bounding box for this Geometry.
       */
      virtual IEnvelope *envelope() const = 0;

      /*!
       * \brief Exports this geometric object to a specific Well-known Text Representation of Geometry.
       * \returns Well-known Text Representation of Geometry.
       */
      virtual string getWKT() const = 0;

      /*!
       * \brief Exports this geometric object to a specific Well-known byte Representation of Geometry.
       */
      virtual unsigned char *getWKB(int &size) const = 0;

      /*!
       * \brief If true, then this geometric object represents the empty point set ∅ for the coordinate space.
       * \returns <code>true</code> if this geometric object is the empty Geometry.
       */
      virtual bool isEmpty() const = 0;

      /*!
       * \brief Returns <code>true</code> if this geometric object has no anomalous
       * geometric points, such as self intersection or self tangency.
       *
       * \details The description of each instantiable geometric class will include the specific
       * conditions that cause an instance of that class to be classified as not simple.
       */
      virtual bool isSimple() const = 0;

      /*!
       * \returns <code>true</code> if this geometric object has z coordinate values.
       */
      virtual bool is3D() const = 0;

      /*!
       * \returns <code>true</code> if this geometric object has m coordinate values.
       */
      virtual bool isMeasured() const = 0;

      /*!
       * \brief Returns the closure of the combinatorial boundary of
       * this geometric object (Reference [1], section 3.12.2).
       *
       * \details Because the result of this function is a closure, and hence topologically
       * closed, the resulting boundary can be represented using representational Geometry primitives (Reference [1],
       * section 3.12.2). The return type is integer, but is interpreted as Boolean, TRUE=1, FALSE=0.
       *
       */
      virtual IGeometry *boundary() const = 0;

      /** @name Query
       *Query functions
       */
      //@{

      /*!
       * \returns <code>true</code> if this geometric object is spatially equal to geom.
       */
      virtual bool equals(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially disjoint to geom.
       */
      virtual bool disjoint(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially intersects to geom.
       */
      virtual bool intersects(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially touches to geom.
       */
      virtual bool touches(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially crosses to geom.
       */
      virtual bool crosses(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially within to geom.
       */
      virtual bool within(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially contains to geom.
       */
      virtual bool contains(const IGeometry &geom) const = 0;

      /*!
       * \returns <code>true</code> if this geometric object is spatially overlaps to geom.
       */
      virtual bool overlaps(const IGeometry &geom) const = 0;

      /*!
       * \details This returns <code>false</code> if all the tested intersections
       * are empty except exterior (this) intersect exterior (another).
       *
       * \returns <code>true</code> if this geometric object is spatially related
       * to geom by testing for intersections between the interior, boundary and
       * exterior of the two geometric objects as specified by the values in the intersectionPatternMatrix.
       *
       */
      virtual bool relate(const IGeometry &geom) const = 0;

      /*!
       * \returns a derived geometry collection value that matches the specified m coordinate value.
       */
      virtual IGeometry *locateAlong(double value) const = 0;

      /*!
       * \returns a derived geometry collection value that matches the specified range of m coordinate values inclusively.
       */
      virtual IGeometry *locateBetween(double mStart, double mEnd) const = 0;

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
      virtual double distance(const IGeometry &geom) const = 0;

      /*!
       * \brief Returns a geometric object that represents all Points whose distance
       * from this geometric object is less than or equal to distance.
       *
       * \details Calculations are in the spatial reference system of this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
       * small error in this distance, but it should be near the resolution of the coordinates used.
       */
      virtual IGeometry *buffer(double bufferDistance) const = 0;

      /*!
       * \returns a geometric object that represents the convex hull of this geometric object.
       *
       * \details Convex hulls, being dependent on straight lines, can be accurately represented
       * in linear interpolations for any geometry restricted to linear interpolations.
       */
      virtual IGeometry *convexHull() const = 0;

      /*!
       * \returns a geometric object that represents the Point set intersection of this geometric object with geom.
       */
      virtual IGeometry *intersection(const IGeometry &geom) const = 0;

      /*!
       * \returns a geometric object that represents the Point set union of this geometric object with geom.
       */
      virtual IGeometry *unionG(const IGeometry &geom) const = 0;

      /*!
       * \returns a geometric object that represents the Point set difference of this geometric object with geom.
       */
      virtual IGeometry *difference(const IGeometry &geom) const = 0;

      /*!
       * \returns a geometric object that represents the Point set symmetric difference of this geometric object with geom.
       */
      virtual IGeometry *symmetricDifference(const IGeometry &geom) const = 0;

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
      virtual int geometryCount() const = 0;

      /*!
       * \brief The IGeometry object associated with a specified index.
       * \param index of the geometry in this IGeometryCollection.
       * \return The IGeometry object associated with this index.
       */
      virtual IGeometry *geometry(int index) const = 0;
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
      virtual double x() const = 0;

      /*!
       * \brief The y-coordinate value for this IPoint.
       */
      virtual double y() const = 0;

      /*!
       * \brief The x-coordinate value for this IPoint. Returns NIL otherwise.
       */
      virtual double z() const = 0;

      /*!
       * \brief The x-coordinate value for this IPoint. Returns NIL otherwise.
       */
      virtual double m() const = 0;
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
      virtual IPoint *point(int index) const = 0;
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
      virtual unsigned int index() const = 0;

      /*!
       * \brief An arbitrary outgoing IEdge from this vertex.
       * \returns An edge whose origin is this vertex;
       *    null if isolated
       */
      virtual IEdge *edge() const = 0;
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
      virtual double length() const = 0;

      /*!
       * \brief The start IPoint of this ICurve.
       */
      virtual IPoint *startPoint() const = 0;

      /*!
       * \brief The end IPoint of this ICurve.
       */
      virtual IPoint *endPoint() const = 0;

      /*!
       * \returns True if this ICurve is closed[startPoint() = endPoint()].
       */
      virtual bool isClosed() const = 0;

      /*!
       * \returns True if this ICurve is closed [startPoint() = endPoint()]
       * and this ICurve is simple(does not pass through the same Point more than once).
       */
      virtual bool isRing() const = 0;
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
      virtual bool isClosed() const = 0;

      /*!
       * \brief The Length of this IMultiCurve which is equal to the sum
       * of the lengths of the element ICurves.
       */
      virtual double length() const = 0;
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
      virtual int pointCount() const = 0;

      /*!
       * \returns the specified IPoint at index in this ILineString.
       */
      virtual IPoint *point(int index) const = 0;
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
      virtual ILineString *lineString(int index) const = 0;
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
      virtual unsigned int index() const = 0;

      /*!
       * \brief The origin IVertex of this IEdge.
       * \returns the origin of this IEdge; NULL if currently unknown.
       */
      virtual IVertex *orig() = 0;

      /*!
       * \brief The destination IVertex of this IEdge.
       * \returns the destination of this IEdge; NULL if currently unknown.
       */
      virtual IVertex *dest() = 0;

      /*!
       * \brief The left face of this edge.
       * \returns the left face of this edge;
       *  Null if currently unknown
       */
      virtual IPolygon *left() = 0;

      /*!
       * \brief The right face of this edge.
       * \returns The right face of this edge;
       *    null if currently unknown
       */
      virtual IPolygon *right() = 0;

      /*!
       * \brief The target face of this edge, if dual. Otherwise null if not dual.
       * \return
       */
      virtual IPolygon *face() = 0;

      /*!
       * \brief The dual of this edge, directed from its right to its left.
       * \returns The right to left dual of this edge;
       *    will be nonnull
       */
      virtual IEdge *rot() = 0;

      /*!
       * \brief The dual of this IEdge, directed from its left to its right.
       * \returns The left to right dual of this edge;
       *    will be nonnull
       */
      virtual IEdge *invRot() = 0;

      /*!
       * \brief The IEdge from the destination to the origin of this IEdge.
       * \returns The symmetric of this edge;
       *    will be nonnull
       */
      virtual IEdge *sym() = 0;

      /*!
       * \brief The next ccw edge around (from) the origin of this IEdge.
       * \returns The next edge from the origin;
       *    will be nonnull
       */
      virtual IEdge *origNext() = 0;

      /*!
       * \brief The next cw edge around (from) the origin of this edge.
       * \returns The previous edge from the origin;
       *    will be nonnull
       */
      virtual IEdge *origPrev() = 0;

      /*!
       * \brief The next ccw edge around (into) the destination of this edge.
       * \returns The next edge to the destination;
       *    will be nonnull
       */
      virtual IEdge *destNext() = 0;

      /*!
       * \brief The next cw edge around (into) the destination of this edge.
       * \returns The previous edge to the destination;
       *    will be nonnull
       */
      virtual IEdge *destPrev() = 0;

      /*!
       * \brief The ccw edge around the left face following this edge.
       * \returns The next left face edge;
       *    will be nonnull
       */
      virtual IEdge *leftNext() = 0;

      /*!
       * \brief The ccw edge around the left face before this edge.
       * \returns The previous left face edge;
       *    will be nonnull
       */
      virtual IEdge *leftPrev() = 0;

      /*!
       * \brief The edge around the right face ccw following this edge.
       * \returns The next right face edge;
       *    will be nonnull
       */
      virtual IEdge *rightNext() = 0;

      /*!
       * \brief The IEdge around the right face ccw before this IEdge.
       * \returns The previous right face edge;
       *    will be nonnull
       */
      virtual IEdge *rightPrev() = 0;
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
      virtual double area() const = 0;

      /*!
       * \brief The mathematical centroid for this ISurface as a Point.
       * The result is not guaranteed to be on this ISurface.
       */
      virtual IPoint *centroid() const = 0;

      /*!
       * \brief A Point guaranteed to be on this Surface.
       */
      virtual IPoint *pointOnSurface() const = 0;

      /*!
       * \brief boundaryMultiCurve
       * \return
       */
      virtual IMultiCurve *boundaryMultiCurve() const = 0;
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
      virtual double area() const = 0;

      /*!
       * \brief The mathematical centroid for this ISurface as an IPoint.
       * The result is not guaranteed to be on this ISurface.
       */
      virtual IPoint *centroid() const = 0;

      /*!
       * \brief The mathematical centroid for this ISurface as an IPoint.
       * The result is not guaranteed to be on this ISurface.
       */
      virtual IPoint *pointOnSurface() const = 0;
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
      virtual ILineString *exteriorRing() const = 0;

      /*!
       * \returns the number of interior rings in this IPolygon.
       */
      virtual int interiorRingCount() const = 0;

      /*!
       * \returns the index - th interior ring for this IPolygon as a ILineString.
       */
      virtual ILineString *interiorRing(int index) const = 0;

      /*!
       * \brief An arbitrary adjacent edge for this IPolygon.
       * \returns An edge that is adjacent to this face;
       *    null if degenerate
       */
      virtual IEdge *edge() const = 0;

      /*!
       * \brief polyhydralSurface
       * \returns the surface this IPolygon belongs to. Otherwise returns Null.
       */
      virtual IPolyhedralSurface *polyhydralSurface() const = 0;
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
      virtual IPolygon *polygon(int index) const = 0;
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
      virtual IVertex *vertex1() const = 0;

      /*!
       * \brief The second vertex of this ITriangle.
       */
      virtual IVertex *vertex2() const = 0;

      /*!
       * \brief The third vertex of this ITriangle.
       */
      virtual IVertex *vertex3() const = 0;

      /*!
       * \brief The vertex of this ITriangle at the specified index.
       */
      virtual IVertex *vertex(int index) const = 0;
    };

    /*!
     * \brief The INetwork class.
     */
    class INetwork : public virtual IIdentity
    {
    public:
      /*!
       * \brief ~Network destructor.
       */
      virtual ~INetwork() = default;

      /*!
       * \brief edgeCount represents the number of all the edges in the network.
       * \return Count of all the edges in the network.
       */
      virtual int edgeCount() const = 0;

      /*!
       * \brief edge
       * \param index of the edge in the network.
       * \return The edge at the specified index.
       */
      virtual IEdge *edge(int index) const = 0;

      /*!
       * \brief vertexCount
       * \return Number of vertices in the network.
       */
      virtual int vertexCount() const = 0;

      /*!
       * \brief vertex
       * \param index of the vertex in the network.
       * \return The vertex at the specified index.
       */
      virtual IVertex *vertex(int index) const = 0;
    };

    /*!
     * \brief An IPolyhedralSurface is a contiguous collection of polygons,
     * which share common boundary segments.
     *
     * \details For each pair of IPolygons that "touch", the common
     * boundary shall be expressible as a finite collection of ILineStrings
     * such ILineString shall be part of the boundary of at most 2 IPolygon patches.
     * A ITIN (triangulated irregular network)
     * is a IPolyhedralSurface consisting only of ITriangle patches.
     *
     * \details For any two IPolygons that share a common boundary,
     * the "top" of the IPolygon shall be consistent. This means
     * that when two ILinearRings from these two IPolygons
     * traverse the common boundary segment, they do so in
     * opposite directions. Since the IPolyhedral surface
     * is contiguous, all IPolygons will be thus consistently oriented.
     * This means that a non-oriented surface (such as Möbius band) shall not
     * have single surface representations.
     * They may be represented by a IMultiSurface.
     *
     * \details  If each such ILineString is the boundary of exactly 2 IPolygon patches,
     * then the IPolyhedralSurface is a simple,
     * closed polyhedron and is topologically isomorphic to the surface of a sphere.
     * By the Jordan Surface Theorem (Jordan's Theorem for 2-spheres),
     * such polyhedrons enclose a solid topologically isomorphic to the interior of a
     * sphere; the ball. In this case, the "top" of the surface will
     * either point inward or outward of the enclosed finite solid.
     * If outward, the surface is the exterior boundary of the
     * enclosed surface. If inward, the surface is the interior of the
     * infinite complement of the enclosed solid. A Ball with
     * some number of voids (holes) inside can thus be presented
     * as one exterior boundary shell, and some number in interior boundary shells.
     *
     */
    class IPolyhedralSurface : public virtual ISurface
    {
    public:
      /*!
       * \brief IPolyhedralSurface destructor.
       */
      virtual ~IPolyhedralSurface() = default;

      /*!
       * \brief The number of polygons in this surface.
       * \returns The number of including polygons.
       */
      virtual int patchCount() const = 0;

      /*!
       * \returns An IPolygon in this surface, the order is arbitrary.
       * \param index of the IPolygon patch.
       */
      virtual IPolygon *patch(int index) const = 0;

      /*!
       * \brief vertexCount represents the number of all the vertices shared by the patches.
       * \return
       */
      virtual int vertexCount() const = 0;

      /*!
       * \brief vertex of the network with the specified index.
       * \param index of the vertex in the network.
       * \return The vertex at the specified index.
       */
      virtual IVertex *vertex(int index) const = 0;

      /*!
       * \returns The collection of polygons in this surface that
       * bounds the given polygon "polygon" for any polygon "polygon" in the surface.
       */
      virtual IMultiPolygon *boundingPolygons(const IPolygon *polygon) const = 0;

      /*!
       * \returns 1 (True) if the polygon closes on itself,
       * and thus has no boundary and encloses a solid.
       */
      virtual bool isClosed() const = 0;
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
       * \returns an ITriangle in this surface, the order is arbitrary.
       */
      virtual ITriangle *triangle(int index) const = 0;
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
      enum RasterDataType
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
      virtual int xSize() const = 0;

      /*!
       * \brief Number of pixels in y direction.
       */
      virtual int ySize() const = 0;

      /*!
       * \brief Number of raster bands.
       */
      virtual int rasterBandCount() const = 0;

      /*!
       * \brief Adds a new IRasterBand.
       */
      virtual void addRasterBand(RasterDataType dataType) = 0;

      /*!
       * \brief The ISpatialReferenceSystem represents the spatial reference system of goemetric object.
       */
      virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

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
      virtual IRasterBand *getRasterBand(int bandIndex) const = 0;
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
      virtual int xSize() const = 0;

      //! Number of pixels in y direction
      virtual int ySize() const = 0;

      //! Parent IRaster of this IRasterBand
      virtual IRaster *raster() const = 0;

      //! Raster data type
      virtual IRaster::RasterDataType dataType() const = 0;

      /*!
       * \brief Reads data into the image block.
       * \param xOffset is the pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.</param>
       * \param yOffset is the line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.</param>
       * \param xSize is the width of the region of the band to be accessed in pixels.</param>
       * \param ySize is the height of the region of the band to be accessed in lines.</param>
       * \param image is the pointer to where data is to be written. Delete after use.
       */
      virtual void read(int xOffset, int yOffset, int xSize, int ySize, void *image) const = 0;

      /*!
       * \brief Writes image into the raster band.
       * \param xOffset is the pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.</param>
       * \param yOffset is the line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.</param>
       * \param xSize is the width of the region of the band to be accessed in pixels.</param>
       * \param ySize is the height of the region of the band to be accessed in lines.</param>
       * \param image is the pointer to where data is to be written. Delete after use.
       */
      virtual void write(int xOffset, int yOffset, int xSize, int ySize, const void *image) = 0;

      /*!
       * The nodata value for this IRasterBand.
       */
      virtual double noData() const = 0;
    };

    /*!
     * \brief The IRegularGrid2D class
     */
    class IRegularGrid2D : public virtual IIdentity
    {
    public:
      /*!
       * \brief IRegularGrid2D destructor.
       */
      virtual ~IRegularGrid2D() = default;

      /*!
       * \brief The ISpatialReferenceSystem represents the spatial reference system of goemetric object.
       */
      virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

      /*!
       * \brief gridType
       * \return
       */
      virtual RegularGridType gridType() const = 0;

      /*!
       * \brief numXNodes represents the number of nodes in the x direction.
       * \return number of nodes in the x direction.
       */
      virtual int numXNodes() const = 0;

      /*!
       * \brief numYNodes represents the number of nodes in the y direction.
       * \return number of nodes in the y direction.
       */
      virtual int numYNodes() const = 0;

      /*!
       * \brief xNodeLocation provides the x location coordinate for the x-node and y-node indexes.
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns x location coordinate for the x-node and y-node provided
       */
      virtual double xNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief yNodeLocation provides the x location coordinate for the x-node and y-node indexes.
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns y location coordinate for the x-node and y-node provided
       */
      virtual double yNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief isActive is a bool indicating whether a cell is active.
       * \param xCellIndex the x cell index for the cell. Must be less than numXNodes() - 1.
       * \param yCellIndex the y cell index for the cell. Must be less than numYNodes() - 1.
       * \return a bool indicating whether a cell is active.
       */
      virtual bool isActive(int xCellIndex, int yCellIndex) const = 0;
    };

    /*!
     * \brief The IRegularGrid3D class
     */
    class IRegularGrid3D : public virtual IIdentity
    {
    public:
      /*!
       * \brief IRegularGrid3D destructor.
       */
      virtual ~IRegularGrid3D() = default;

      /*!
       * \brief The ISpatialReferenceSystem represents the spatial reference system of goemetric object.
       */
      virtual ISpatialReferenceSystem *spatialReferenceSystem() const = 0;

      /*!
       * \brief gridType
       * \return
       */
      virtual RegularGridType gridType() const = 0;

      /*!
       * \brief numXNodes represents the number of nodes in the x direction.
       * \return number of nodes in the x direction.
       */
      virtual int numXNodes() const = 0;

      /*!
       * \brief numYNodes represents the number of nodes in the y direction.
       * \return number of nodes in the y direction.
       */
      virtual int numYNodes() const = 0;

      /*!
       * \brief numZNodes represents the number of nodes in the z direction.
       * \return number of nodes in the z direction.
       */
      virtual int numZNodes() const = 0;

      /*!
       * \brief xNodeLocation provides the x location coordinate for the x-node and y-node indexes.
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns x location coordinate for the x-node and y-node provided
       */
      virtual double xNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief yNodeLocation provides the x location coordinate for the x-node and y-node indexes.
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \return returns y location coordinate for the x-node and y-node provided
       */
      virtual double yNodeLocation(int xNodeIndex, int yNodeIndex) const = 0;

      /*!
       * \brief zNodeLocation provides the z location coordinate for the x-node, y-node, and z-node indexes.
       * \param xNodeIndex the x-node index.
       * \param yNodeIndex the y-node index
       * \param zNodeIndex the z-node index
       * \return returns z location coordinate for the x-node, y-node, and z-node indexes.
       */
      virtual double zNodeLocation(int xNodeIndex, int yNodeIndex, int zNodeIndex) const = 0;

      /*!
       * \brief isActive is a bool indicating whether a cell is active.
       * \param xCellIndex the x cell index for the cell. Must be less than numXNodes() - 1.
       * \param yCellIndex the y cell index for the cell. Must be less than numYNodes() - 1.
       * \param zCellIndex the z cell index for the cell. Must be less than numZNodes() - 1.
       * \return a bool indicating whether a cell is active.
       */
      virtual bool isActive(int xCellIndex, int yCellIndex, int zCellIndex) const = 0;
    };

    /*!
     * \brief IGeometryComponentItem represents IGeometryCollection IComponentItem. This class must be implemented as an abstract class
     */
    class IGeometryComponentDataItem : public virtual IComponentDataItem
    {

    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief IGeometryComponentItem destructor.
       */
      virtual ~IGeometryComponentDataItem() = default;

      /*!
       * \brief geometryType
       * \return
       */
      virtual IGeometry::GeometryType geometryType() const = 0;

      /*!
       * \brief geometryCount
       * \return
       */
      virtual int geometryCount() const = 0;

      /*!
       * \brief geometry
       * \param geometryIndex
       * \return
       */
      virtual IGeometry *geometry(int geometryIndex) const = 0;

      /*!
       * \returns The dimension attributes for the data with the geometry. This can be the field name
       * for an attribute for a shapefile. Must be the first dimension in the dimensions() list
       */
      virtual HydroCouple::IDimension *geometryDimension() const = 0;

      /*!
       * \brief envelope
       * \return
       */
      virtual HydroCouple::Spatial::IEnvelope *envelope() const = 0;

      /*!
       * \brief Gets value for given geometry dimension index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for additional dimension for the data to be obtained. Empty vector if no additional dimensions.
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int geometryDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given geometry dimension index and size for a hyperslab.
       * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
       * \param geometryDimensionIndex is the start geometry dimension index from where to obtain the requested data.
       * \param dimensionIndexes are the indexes for the data to be obtained.
       * \param geometryDimensionLength is the length of the geometry dimension for the data to be obtained.
       * \param dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int geometryDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int geometryDimensionLength = 1,
          const initializer_list<int> &dimensionLength = {}) const = 0;

      /*!
       * \brief Sets value for given geometry dimension index.
       * \param data is a pointer data thata to is to be copied
       * \param geometryDimensionIndex is the geometry dimension index from where to write data.
       * \param dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void setValue(
          const hydrocouple_variant *data,
          int geometryDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given geometry dimension index and size for a hyperslab.
       * \param data is the input multi dimensional array to be written.
       * \param geometryDimensionIndex is the start geometry dimension index from where to write data.
       * \param dimensionIndexes are the indexes for the data to be obtained.
       * \param geometryDimensionLength is the length of the geometry dimension for the data to be obtained.
       * \param dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int geometryDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int geometryDimensionLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief The INetworkComponentDataItem class
     */
    class INetworkComponentDataItem : public virtual IComponentDataItem
    {

    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief INetworkComponentDataItem destructor.
       */
      virtual ~INetworkComponentDataItem() = default;

      /*!
       * \brief network associated with this INetworkComponentDataItem.
       * \return The network associated with this INetworkComponentDataItem.
       */
      virtual INetwork *network() const = 0;

      /*!
       * \brief MeshDataObject that represents the artifact of network that is associated with the data.
       * \return The MeshDataObject that represents the artifact of network that is associated with the data.
       */
      virtual NetworkDataObjectType networkDataObjectType() const = 0;

      /*!
       * \brief meshDataType represents the type of mesh data type stored in the network.
       * \return The type of mesh data type stored in the network.
       */
      virtual NetworkDataType networkDataType() const = 0;

      /*!
       * \brief edgeDimension represents the dimension for the edges.
       * \return The dimension for the edges.
       */
      virtual IDimension *edgeDimension() const = 0;

      /*!
       * \brief nodeDimension represents the dimension for the vertices.
       * \return The dimension for the vertices.
       */
      virtual IDimension *vertexDimension() const = 0;

      /*!
       * \brief getValue for given edge dimension index and node dimension index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given edge dimension index and node dimension index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for any additional dimensions for the data to be obtained.
       * \param[in] edgeDimensionIndexLength is the length of the edge dimension for the data to be obtained.
       * \param[in] vertexDimensionIndexLength is the length of the node dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given edge dimension index and node dimension index and data.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] edgeDimensionIndex is the edge dimension index from where to write data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given edge dimension index and node dimension index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] edgeDimensionIndex is the edge dimension index from where to write data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       * \param[in] edgeDimensionIndexLength is the length of the edge dimension for the data to be obtained.
       * \param[in] vertexDimensionIndexLength is the length of the node dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief IPolyhedralSurfaceComponentItem represents IPolyhedralSurface IComponentItem.
     */
    class IPolyhedralSurfaceComponentDataItem : public virtual IComponentDataItem
    {

    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief IPolyhedralSurfaceComponentItem destructor.
       */
      virtual ~IPolyhedralSurfaceComponentDataItem() = default;

      /*!
       * \brief meshDataType represents the type of mesh data.
       * \return The type of mesh data.
       */
      virtual MeshDataObjectType meshDataObjectType() const = 0;

      /*!
       * \brief polyhedralSurface
       * \return The IPolyhedralSurface associated with this IPolyhedralSurfaceComponentItem.
       */
      virtual MeshDataType meshDataType() const = 0;

      /*!
       * \returns The IPolyhedralSurface associated with this IPolyhedralSurfaceDimension.
       */
      virtual IPolyhedralSurface *polyhedralSurface() const = 0;

      /*!
       * \brief patchDimension represents the dimension for the patches associated with the IPolyhedralSurface.
       * \returns The dimension for the patches.
       */
      virtual IDimension *patchDimension() const = 0;

      /*!
       * \brief edgeDimension represents the dimension for the edges of patches associated with the IPolyhedralSurface.`
       * \return The dimension for the edges.
       */
      virtual IDimension *edgeDimension() const = 0;

      /*!
       * \brief vertexDimension represents the dimension for the nodes of the edges of patches associated with the IPolyhedralSurface.
       * \return The dimension for the vertices.
       */
      virtual IDimension *vertexDimension() const = 0;

      /*!
       * \brief getValue for given cell dimension index, edge dimension index, and node dimension index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] patchDimensionIndex is the cell dimension index from where to obtain the requested data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to obtain the requested data.
       * \param[in] vertexDimensionIndex is the node dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given cell dimension index, edge dimension index and node dimension index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] patchDimensionIndex are the cell dimension indexes from where to obtain the requested data.
       * \param[in] edgeDimensionIndex are the edge dimension indexes from where to obtain the requested data. If user wants to
       * only obtain cell data for MeshDataType::Cell, this vector should be empty.
       * \param[in] vertexDimensionIndex are the node dimension indexes from where to obtain the requested data. If user wants to
       * only obtain cell or edge data for MeshDataType::Cell or MeshDataType::Edge, this vector should be empty.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] patchDimensionIndexLength are the lengths of the cell dimensions for the data to be obtained.
       * \param[in] edgeDimensionIndexLength are the lengths of the edge dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       * \param[in] vertexDimensionIndexLength are the lengths of the node dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int patchDimensionIndexLength = 1,
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValues for given cell dimension index, edge dimension index, and node dimension index.
       * \param[in] data is a pointer data thata to is to be copied to the mesh.
       * \param[in] patchDimensionIndex is the face dimension index from where to write data.
       * \param[in] edgeDimensionIndex is the edge dimension index from where to write data.
       * \param[in] vertexDimensionIndex is the edge dimension index from where to obtain the requested data. If user wants to
       * only obtain cell data for MeshDataType::Cell, this vector should be empty.
       * \param dimensionIndexes are the indexes for the data to be obtained.
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given cell dimension index, edge dimension index, and node dimension index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied to the mesh.
       * \param[in] patchDimensionIndex are the cell dimension indexes from where to write data.
       * \param[in] edgeDimensionIndex are the edge dimension indexes from where to write data.
       * \param[in] vertexDimensionIndex are the node dimension indexes from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       * \param[in] patchDimensionIndexLength are the lengths of the cell dimensions for the data to be obtained.
       * \param[in] edgeDimensionIndexLength are the lengths of the edge dimensions for the data to be obtained.
       * \param[in] vertexDimensionIndexLength are the lengths of the node dimensions for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the other dimensions for the data to be obtained.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int patchDimensionIndex,
          int edgeDimensionIndex,
          int vertexDimensionIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int patchDimensionIndexLength = 1,
          int edgeDimensionIndexLength = 1,
          int vertexDimensionIndexLength = 1,
          const initializer_list<int> &dimensionLengths) = 0;
    };

    /*!
     * \brief ITINComponentDataItem represents ITIN IComponentDataItem.
     */
    class ITINComponentDataItem : public virtual IComponentDataItem,
                                  public virtual IPolyhedralSurfaceComponentDataItem
    {

    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;
      using IPolyhedralSurfaceComponentDataItem::getValue;
      using IPolyhedralSurfaceComponentDataItem::getValues;
      using IPolyhedralSurfaceComponentDataItem::setValue;
      using IPolyhedralSurfaceComponentDataItem::setValues;

      /*!
       * \brief ITINComponentDataItem destructor.
       */
      virtual ~ITINComponentDataItem() = default;

      /*!
       * \returns The ITIN associated with this ITINComponentDataItem.
       */
      virtual ITIN *TIN() const = 0;
    };

    /*!
     * \brief An IRasterComponentDataItem represents an IRaster IComponentItem.
     */
    class IRasterComponentDataItem : public virtual IComponentDataItem
    {

    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief IRasterComponentItem destructor.
       */
      virtual ~IRasterComponentDataItem() = default;

      /*!
       * \brief IRaster associated with this IRasterComponentDataItem.
       */
      virtual IRaster *raster() const = 0;

      /*!
       * \brief IDimension for xDirection.
       */
      virtual IDimension *xDimension() const = 0;

      /*!
       * \brief IDimension for yDirection.
       */
      virtual IDimension *yDimension() const = 0;

      /*!
       * \brief IDimension for IRasterBands.
       */
      virtual IDimension *bandDimension() const = 0;

      /*!
       * \brief getValue for given x dimension index, y dimension index, and band dimension index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] xIndex is the x dimension index from where to obtain the requested data.
       * \param[in] yIndex is the y dimension index from where to obtain the requested data.
       * \param[in] bandIndex is the band dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int xIndex,
          int yIndex,
          int bandIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
       * \param[out] data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] xIndex is the x dimension index from where to obtain the requested data.
       * \param[in] yIndex is the y dimension index from where to obtain the requested data.
       * \param[in] bandIndex is the band dimension index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       * \param[in] xIndexLength is the length of the x dimension for the data to be obtained.
       * \param[in] yIndexLength is the length of the y dimension for the data to be obtained.
       * \param[in] bandIndexLength is the length of the band dimension for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the other dimensions for the data to be obtained.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int xIndex,
          int yIndex,
          int bandIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int xIndexLength = 1,
          int yIndexLength = 1,
          int bandIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given x dimension index, y dimension index, and band dimension index.
       * \param[in] data  is a pointer data thata to is to be copied
       * \param[in] xIndex is the x dimension index from where to write data.
       * \param[in] yIndex is the y dimension index from where to write data.
       * \param[in] band  is the band dimension index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int xIndex,
          int yIndex,
          int band,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
       * \param[in] data is the input array to be written.
       * \param[in] xIndex is the x dimension index where to set data.
       * \param[in] yIndex is the y dimension index where to set data.
       * \param[in] bandIndex is the band dimension index where to set data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       * \param[in] xIndexLength is the x size for hyperslab where data is to be written.
       * \param[in] yIndexLength is the y size for hyperslab where data is to be written.
       * \param[in] bandIndexLength is the band size for hyperslab where data is to be written.
       * \param[in] dimensionLengths are the lengths of the other dimensions for the data to be obtained.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int xIndex,
          int yIndex,
          int bandIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int xIndexLength = 1,
          int yIndexLength = 1,
          int bandIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief An IRegularGrid2DComponentDataItem represents an IRegularGrid2D IComponentItem
     */
    class IRegularGrid2DComponentDataItem : public virtual IComponentDataItem
    {

    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief ~IRegularGrid2DComponentItem.
       */
      virtual ~IRegularGrid2DComponentDataItem() = default;

      /*!
       * \brief IRegularGrid2D grid associated with this IRegularGrid2DComponentItem.
       * \return The IRegularGrid2D grid associated with this IRegularGrid2DComponentItem.
       */
      virtual IRegularGrid2D *grid() const = 0;

      /*!
       * \brief meshDataType represents the type of mesh data type stored in the grid.
       * \return The type of mesh data type stored in the grid.
       */
      virtual MeshDataObjectType meshDataObjectType() const = 0;

      /*!
       * \brief Number of X cells IDimension.
       * \return IDimension for the x cells.
       */
      virtual IDimension *xCellDimension() const = 0;

      /*!
       * \brief Number of Y cells IDimension.
       * \return IDimension for the y cells.
       */
      virtual IDimension *yCellDimension() const = 0;

      /*!
       * \brief cellEdgeDimension. Edge indices start from the bottom and go in a counter clockwise order.
       * \return IDimension for the cell edges.
       */
      virtual IDimension *cellEdgeDimension() const = 0;

      /*!
       * \brief cellVertexDimension. Vertex indices start from the bottom left and go in a counter clockwise order.
       * \return IDimension for the cell vertices.
       */
      virtual IDimension *cellVertexDimension() const = 0;

      /*!
       * \brief getValue for given x cell index, y cell index, edge index, and node index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] cellEdgeIndex is the cell edge index from where to obtain the requested data.
       * \param[in] cellNodeIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.  If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given x cell index, y cell index, edge index, and node index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] cellEdgeIndex is the cell edge index from where to obtain the requested data.
       * \param[in] cellVertexIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] cellEdgeIndexLength is the length of the cell edge index for the data to be obtained.
       * \param[in] cellVertexIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int xCellIndexLength = 1,
          int yCellIndexLength = 1,
          int cellEdgeIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given x cell index, y cell index, edge index, and node index.
       * \param[in] data is a pointer data thata to is to be copied to the mesh.
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] cellEdgeIndex is the cell edge index from where to write data.
       * \param[in] cellVertexIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given x cell index, y cell index, edge index, and node index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] cellEdgeIndex is the cell edge index from where to write data.
       * \param[in] cellNodeIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] cellEdgeIndexLength is the length of the cell edge index for the data to be obtained.
       * \param[in] cellNodeIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int xCellIndex,
          int yCellIndex,
          int cellEdgeIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int xCellIndexLength = 1,
          int yCellIndexLength = 1,
          int cellEdgeIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

    /*!
     * \brief An IRegularGrid3DComponentItem represents an IRegularGrid3D IComponentItem
     */
    class IRegularGrid3DComponentDataItem : public virtual IComponentDataItem
    {

    public:
      using IComponentDataItem::getValue;
      using IComponentDataItem::getValues;
      using IComponentDataItem::setValue;
      using IComponentDataItem::setValues;

      /*!
       * \brief ~IRegularGrid3DComponentItem.
       */
      virtual ~IRegularGrid3DComponentDataItem() = default;

      /*!
       * \brief  IRegularGrid3D grid associated with this IRegularGrid3DComponentItem.
       */
      virtual IRegularGrid3D *grid() const = 0;

      /*!
       * \brief meshDataType
       * \return The type of mesh data.
       */
      virtual MeshDataObjectType meshDataObjectType() const = 0;

      /*!
       * \brief Number of X cells IDimension.
       */
      virtual IDimension *xCellDimension() const = 0;

      /*!
       * \brief Number of Y cells IDimension.
       */
      virtual IDimension *yCellDimension() const = 0;

      /*!
       * \brief Number of Z cells IDimension.
       */
      virtual IDimension *zCellDimension() const = 0;

      /*!
       * \brief cellFaceDimension 0 = Top , 1 = Bottom, 2 = left , 3 = Right, Up = 4, Down = 5
       * \return The dimension for the cell faces.
       */
      virtual IDimension *cellFaceDimension() const = 0;

      /*!
       * \brief cellVertexDimension Node indices start from the bottom left and go in a counter clockwise order.
       * \return The dimension for the cell nodes.
       */
      virtual IDimension *cellVertexDimension() const = 0;

      /*!
       * \brief getValue for given x cell index, y cell index, z cell index, face index, and node index.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] zCellIndex is the z cell index from where to obtain the requested data.
       * \param[in] cellFaceIndex is the cell face index from where to obtain the requested data.
       * \param[in] cellVertexIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void getValue(
          hydrocouple_variant &data,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) const = 0;

      /*!
       * \brief getValues for given x cell index, y cell index, z cell index, face index, and node index and size for a hyperslab.
       * \param[out] data is a pointer to data that is to be written. Must be allocated beforehand with the correct data type.
       * \param[in] xCellIndex is the x cell index from where to obtain the requested data.
       * \param[in] yCellIndex is the y cell index from where to obtain the requested data.
       * \param[in] zCellIndex is the z cell index from where to obtain the requested data.
       * \param[in] cellFaceIndex is the cell face index from where to obtain the requested data.
       * \param[in] cellVertexIndex is the cell node index from where to obtain the requested data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] zCellIndexLength is the length of the z cell index for the data to be obtained.
       * \param[in] cellFaceIndexLength is the length of the cell face index for the data to be obtained.
       * \param[in] cellVertexIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void getValues(
          hydrocouple_variant *data,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {},
          int xCellIndexLength = 1,
          int yCellIndexLength = 1,
          int zCellIndexLength = 1,
          int cellFaceIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) const = 0;

      /*!
       * \brief setValue for given x cell index, y cell index, z cell index, face index, and node index.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] zCellIndex is the z cell index from where to write data.
       * \param[in] cellFaceIndex is the cell face index from where to write data.
       * \param[in] cellVertexIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained. If there are additional dimensions
       * the length of the vector must be equal to the number of other dimensions
       */
      virtual void setValue(
          const hydrocouple_variant &data,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}) = 0;

      /*!
       * \brief setValues for given x cell index, y cell index, z cell index, face index, and node index and size for a hyperslab.
       * \param[in] data is a pointer data thata to is to be copied
       * \param[in] xCellIndex is the x cell index from where to write data.
       * \param[in] yCellIndex is the y cell index from where to write data.
       * \param[in] zCellIndex is the z cell index from where to write data.
       * \param[in] cellFaceIndex is the cell face index from where to write data.
       * \param[in] cellVertexIndex is the cell node index from where to write data.
       * \param[in] dimensionIndexes are the indexes for the data to be obtained.
       * \param[in] xCellIndexLength is the length of the x cell index for the data to be obtained.
       * \param[in] yCellIndexLength is the length of the y cell index for the data to be obtained.
       * \param[in] zCellIndexLength is the length of the z cell index for the data to be obtained.
       * \param[in] cellFaceIndexLength is the length of the cell face index for the data to be obtained.
       * \param[in] cellVertexIndexLength is the length of the cell node index for the data to be obtained.
       * \param[in] dimensionLengths are the lengths of the dimensions for the data to be obtained. If empty a single value is returned,
       * otherwise the length of the vector must be equal to the number of dimensions.
       */
      virtual void setValues(
          const hydrocouple_variant *data,
          int xCellIndex,
          int yCellIndex,
          int zCellIndex,
          int cellFaceIndex,
          int cellVertexIndex,
          const initializer_list<int> &dimensionIndexes = {}, 
          int xCellIndexLength  = 1,
          int yCellIndexLength = 1,
          int zCellIndexLength  = 1,
          int cellFaceIndexLength = 1,
          int cellVertexIndexLength = 1,
          const initializer_list<int> &dimensionLengths = {}) = 0;
    };

  }
}

#endif // HYDROCOUPLESPATIAL_H
