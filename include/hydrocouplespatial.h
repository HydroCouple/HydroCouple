/*! \file   hydrocouplespatial.h
 *  \author Caleb Amoa Buahin <caleb.buahin@gmail.com>
 *  \version   1.0.0.0
 *  \section   Description
 *  This header files contains interface definitions that have a spatial component for the
 *  HydroCouple component-cased modeling framework.
 *  \section License
 *  hydrocouplespatial.h, associated files and libraries are free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  Lesser GNU General Public License as published by the Free Software Foundation;
 *  either version 3 of the License, or (at your option) any later version.
 *  hydrocouplespatial.h its associated files is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 *  \date 2014-2016
 *  \pre
 *  \bug
 *  \warning
 */

/*!
*/
#ifndef HYDROCOUPLESPATIAL
#define HYDROCOUPLESPATIAL

#include "hydrocouple.h"

namespace HydroCouple
{
   /*!
    * \brief HydroCouple's interfaces that have a spatial component.
    */
   namespace Spatial
   {
      /*!
       * \brief The type of IGeometry.
       */
      enum GeometryType
      {
         Geometry = 0,
         Point = 1,
         LineString = 2,
         Polygon = 3,
         MultiPoint = 4,
         MultiLineString = 5,
         MultiPolygon = 6,
         GeometryCollection = 7,
         CircularString  = 8,
         CompoundCurve = 9,
         CurvePolygon = 10,
         MultiCurve = 11,
         MultiSurface = 12,
         Curve = 13,
         Surface = 14,
         PolyhedralSurface = 15,
         TIN =16,


         GeometryZ = 1000,
         PointZ = 1001,
         LineStringZ = 1002,
         PolygonZ = 1003,
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
       * \brief The data type associated with a raster.
       */
      enum RasterDataType
      {
         //!Unknown or unspecified type
         Unknown,
         //!Eight bit unsigned integer
         Byte,
         //!Sixteen bit unsigned integer
         UInt16,
         //!Sixteen bit signed integer
         Int16,
         //!Thirty two bit unsigned integer
         UInt32,
         //!Thirty two bit signed integer
         Int32,
         //!Thirty two bit floating point
         Float32,
         //!Sixty four bit floating point
         Float64,
         //!Complex Int16
         CInt16,
         //!Complex Int32
         CInt32,
         //!Complex Float32
         CFloat32,
         //!Complex Float64
         CFloat64
      };

      /*!
       * \brief Spatial Reference System
       */
      class ISpatialReferenceSystem : public virtual IIdentity
      {

         public:
            virtual ~ISpatialReferenceSystem(){}

            /*!
             * \brief Returns the Spatial Reference System ID for a geometric object.
             * This will normally be a foreign key to an index of reference systems
             * stored in either the same or some other datastore.
             */
            virtual int authSRID() const = 0;

            /*!
             * \brief The Authority Specific Spatial Reference System Identifier.
             */
            virtual QString authName() const = 0;

            /*!
             * Well-known Text description of the Spatial Reference System.
             */
            virtual QString srText() const = 0;
      };

      /*!
       * \brief IGeometry is the root class of the hierarchy.
       */
      class IGeometry : public virtual IIdentity
      {
         public:
            virtual ~IGeometry(){}


            /*!
             * \brief index of the geometry if it is part of a collection.
             * \return index of the geometry in a collection.
             */
            virtual int index() const = 0;

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
             * Spatial reference system of goemetric object.
             */
            virtual ISpatialReferenceSystem* spatialReferenceSystem() const = 0;

            /*!
             * \brief The minimum bounding box for this Geometry, returned as a IGeometry.
             *
             * \details The polygon is defined by the corner points of the bounding box [(MINX, MINY), (MAXX, MINY), (MAXX, MAXY),
             * (MINX, MAXY), (MINX, MINY)]. Minimums for Z and M may be added. The simplest representation of an
             * Envelope is as two direct positions, one containing all the minimums, and another all the maximums. In some
             * cases, this coordinate will be outside the range of validity for the Spatial Reference System.
             */
            virtual IGeometry* envelope() const = 0;

            /*!
             * Exports this geometric object to a specific Well-known Text Representation of Geometry.
             */
            virtual QString wkt() const = 0;

            /*!
             * Exports this geometric object to a specific Well-known byte Representation of Geometry.
             */
            virtual unsigned char* wkb(int & size) const = 0;


            //!Returns <code>true</code> if this geometric object is the empty Geometry.
            /*!
              If true, then this geometric object represents the empty point set ∅ for the coordinate space.
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
            virtual IGeometry* boundary() const = 0;

            /** @name Query
              *Query functions
              */
            ///@{

            /*!
             * \returns <code>true</code> if this geometric object is spatially equal to geom.
             */
            virtual bool equals(const IGeometry* geom) const = 0;

            /*!
             * \returns <code>true</code> if this geometric object is spatially disjoint to geom.
             */
            virtual bool disjoint(const IGeometry*  geom) const = 0;

            /*!
             * \returns <code>true</code> if this geometric object is spatially intersects to geom.
             */
            virtual bool intersects(const IGeometry*  geom) const = 0;

            /*!
             * \returns <code>true</code> if this geometric object is spatially touches to geom.
             */
            virtual bool touches(const IGeometry* geom) const = 0;

            /*!
             * \returns <code>true</code> if this geometric object is spatially crosses to geom.
             */
            virtual bool crosses(const IGeometry* geom) const = 0;

            /*!
             * \returns <code>true</code> if this geometric object is spatially within to geom.
             */
            virtual bool within(const IGeometry* geom) const = 0;

            /*!
             * \returns <code>true</code> if this geometric object is spatially contains to geom.
             */
            virtual bool contains(const IGeometry* geom) const = 0;

            /*!
             * \returns <code>true</code> if this geometric object is spatially overlaps to geom.
             */
            virtual bool overlaps(const IGeometry* geom) const = 0;

            /*!
             * \details This returns <code>false</code> if all the tested intersections
             * are empty except exterior (this) intersect exterior (another).
             *
             * \returns <code>true</code> if this geometric object is spatially related
             * to geom by testing for intersections between the interior, boundary and
             * exterior of the two geometric objects as specified by the values in the intersectionPatternMatrix.
             *
             */
            virtual bool relate(const IGeometry* geom) const = 0;

            /*!
             * \returns a derived geometry collection value that matches the specified m coordinate value.
             */
            virtual IGeometry* loocateAlong(double value) const = 0;

            /*!
             * \returns a derived geometry collection value that matches the specified range of m coordinate values inclusively.
             */
            virtual IGeometry* locateBetween(double mStart, double mEnd) const = 0;

            ///@}


            /** @name Spatial Analysis
             *Spatial analysis functions
             */
            ///@{

            /*!
             * \brief Returns the shortest distance between any two
             * Points in the two geometric objects as calculated in
             * the spatial reference system of this geometric object.
             *
             * \details Because the geometries are closed, it is possible to
             * find a point on each geometric object involved, such that the distance
             * between these 2 points is the returned distance between their geometric objects.
             */
            virtual double distance(const IGeometry* geom) const = 0;

            /*!
             * \brief Returns a geometric object that represents all Points whose distance
             * from this geometric object is less than or equal to distance.
             *
             * \details Calculations are in the spatial reference system of this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
             * small error in this distance, but it should be near the resolution of the coordinates used.
             */
            virtual IGeometry* buffer(double buffer) const = 0;

            /*!
             * \returns a geometric object that represents the convex hull of this geometric object.
             *
             * \details Convex hulls, being dependent on straight lines, can be accurately represented
             * in linear interpolations for any geometry restricted to linear interpolations.
             */
            virtual IGeometry* convexHull() const = 0;

            /*!
             * \returns a geometric object that represents the Point set intersection of this geometric object with geom.
             */
            virtual IGeometry* intersection(const IGeometry* geom) const = 0;

            /*!
             * \returns a geometric object that represents the Point set union of this geometric object with geom.
             */
            virtual IGeometry* uniong(const IGeometry* geom) const = 0;

            /*!
             * \returns a geometric object that represents the Point set difference of this geometric object with geom.
             */
            virtual IGeometry* difference(const IGeometry* geom) const = 0;

            /*!
             * \returns a geometric object that represents the Point set symmetric difference of this geometric object with geom.
             */
            virtual IGeometry* symmetricDifference(const IGeometry* geom) const = 0;

            ///@}
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
            virtual ~IGeometryCollection(){}

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
            virtual IGeometry* geometry(int index) const  = 0;

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
            virtual ~IMultiPoint(){}

            /*!
             * \returns the index sup(th) IPoint in this IGeometryCollection.
             */
            virtual IPoint* point(int index) const = 0;
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
            virtual ~IPoint(){}

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

      class IEdge;

      /*!
       * \brief The IVertex class is an IPoint of a topologically aware IGeometry
       */
      class IVertex : public virtual IPoint
      {
         public:
            virtual ~IVertex(){}

            /*!
             * \brief An arbitrary outgoing IEdge from this vertex.
             * \returns An edge whose origin is this vertex;
             *    null if isolated
             */
            virtual IEdge *edge() const = 0;

            /*!
             * \brief Add a given outgoing edge to this vertex.
             * \param edge -> an edge whose origin is this vertex;
             *         must be nonnull
             */
            virtual void addEdge(IEdge *edge) = 0;

            /*!
             * \brief Remove a given outgoing IEdge from this IVertex.
             * \param edge -> an edge whose origin is no longer at this vertex;
             *         must be nonnull
             */
            virtual void removeEdge(IEdge *edge) = 0;

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
            virtual ~ICurve(){}

            /*!
             * \brief The length of this ICurve in its associated ISpatialReferenceSystem.
             */
            virtual double length() const = 0;

            /*!
             * \brief The start IPoint of this ICurve.
             */
            virtual IPoint* startPoint() const = 0;

            /*!
             * \brief The end IPoint of this ICurve.
             */
            virtual IPoint* endPoint() const = 0;

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
            virtual ~IMultiCurve(){}

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
       * \brief An IMultiLineString is an IMultiCurve whose elements are ILineStrings.
       */
      class IMultiLineString : public virtual IMultiCurve
      {
         public:
            virtual ~IMultiLineString(){}

            //!Returns the ILineString at index
            virtual ILineString* element(int index) const = 0;
      };

      /*!
       * \brief An ILineString is a Curve with linear interpolation between Points.
       * Each consecutive pair of Points defines a Line segment.
       */
      class ILineString : public virtual ICurve
      {

         public:
            virtual ~ILineString(){}

            /*!
             * \brief The number of IPoints in this ILineString.
             */
            virtual int pointCount() const = 0;

            /*!
             * \returns the specified IPoint at index in this ILineString.
             */
            virtual IPoint* point(int index) const = 0;

      };

      /*!
       * \brief The ILine class is an ILineString with exactly 2 IPoints.
       */
      class ILine : public virtual ILineString
      {
         public:
            virtual ~ILine(){}
      };

      /*!
       * \brief An ILinearRing is an ILineString that is both isClosed() and isSimple().
       */
      class ILinearRing : public virtual ILineString
      {
         public:
            virtual ~ILinearRing(){}
      };

      class IPolygon;

      /*!
       * \brief A directed edge from one vertex to another, adjacent to two faces.
       * Based on Dani Lischinski's code from Graphics Gems IV.
       * Original quad-edge data structure due to Guibas and Stolfi (1985).
       */
      class IEdge : public virtual ILine
      {
         public:
            virtual ~IEdge(){}

            /*!
             * \brief the id number assigned to the edge.
             * \returns the id of this edge. This id() will be positive.
             */
            virtual int index()  const = 0;

            /*!
             * \brief Change the id of this edge.
             * \param id is the new id for this edge; must be positive.
             */
            virtual void setId(unsigned int id) = 0;

            /*!
             * \brief The origin IVertex of this IEdge.
             * \returns the origin of this IEdge; NULL if currently unknown.
             */
            virtual IVertex *orig() = 0;

            /*!
             * \brief Change the origin IVertex of this IEdge to a given IVertex.
             * \param orig  -> the new origin IVertex of this IEdge; NULL if currently unknown.
             */
            virtual void setOrig(IVertex *orig) = 0;

            /*!
             * \brief The destination IVertex of this IEdge.
             * \returns the destination of this IEdge; NULL if currently unknown.
             */
            virtual IVertex *dest() = 0;

            /*!
             * \brief Change the destination IVertex of this IEdge to a given IVertex.
             * \param dest  -> the new destination IVertex of this IEdge;
             *         null if currently unknown
             */
            virtual void setDest(IVertex *dest) = 0;

            /*!
             * \brief The left face of this edge.
             * \returns the left face of this edge;
             *  Null if currently unknown
             */
            virtual IPolygon *left() const  = 0 ;

            /*!
             * \brief The right face of this edge.
             * \returns The right face of this edge;
             *    null if currently unknown
             */
            virtual IPolygon *right() const = 0;

            /*!
             * \brief Change the left face of this edge to a given face.
             * \param left  -> the new left face of this edge;
             *          null if currently unknown
             */
            virtual void setLeft(IPolygon *left) = 0;

            /*!
             * \brief Change the right face of this edge to a given face.
             * \param right -> the new right face of this edge;
             *          null if currently unknown
             */
            virtual void setRight(IPolygon *right) = 0;

            /*!
             * \brief The dual of this edge, directed from its right to its left.
             * \returns The right to left dual of this edge;
             *    will be nonnull
             */
            virtual IEdge *rot() const = 0;

            /*!
             * \brief The dual of this IEdge, directed from its left to its right.
             * \returns The left to right dual of this edge;
             *    will be nonnull
             */
            virtual IEdge *invRot() const = 0;

            /*!
             * \brief The IEdge from the destination to the origin of this IEdge.
             * \returns The symmetric of this edge;
             *    will be nonnull
             */
            virtual IEdge *sym() const = 0;

            /*!
             * \brief The next ccw edge around (from) the origin of this IEdge.
             * \returns The next edge from the origin;
             *    will be nonnull
             */
            virtual IEdge *origNext() const = 0;

            /*!
             * \brief The next cw edge around (from) the origin of this edge.
             * \returns The previous edge from the origin;
             *    will be nonnull
             */
            virtual IEdge *origPrev() const = 0;

            /*!
             * \brief The next ccw edge around (into) the destination of this edge.
             * \returns The next edge to the destination;
             *    will be nonnull
             */
            virtual IEdge *destNext() const  = 0;

            /*!
             * \brief The next cw edge around (into) the destination of this edge.
             * \returns The previous edge to the destination;
             *    will be nonnull
             */
            virtual IEdge* destPrev() const = 0;

            /*!
             * \brief The ccw edge around the left face following this edge.
             * \returns The next left face edge;
             *    will be nonnull
             */
            virtual IEdge* leftNext() const = 0;

            /*!
             * \brief The ccw edge around the left face before this edge.
             * \returns The previous left face edge;
             *    will be nonnull
             */
            virtual IEdge* leftPrev() const = 0;

            /*!
             * \brief The edge around the right face ccw following this edge.
             * \returns The next right face edge;
             *    will be nonnull
             */
            virtual IEdge* rightNext() const  = 0;

            /*!
             * \brief The IEdge around the right face ccw before this IEdge.
             * \returns The previous right face edge;
             *    will be nonnull
             */
            virtual IEdge* rightPrev() const =  0;

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
            virtual ~ISurface(){}

            /*!
             * \brief The area of this ISurface, as measured in
             * the spatial reference system of this ISurface.
             */
            virtual double area() const = 0;

            /*!
             * \brief The mathematical centroid for this ISurface as a Point.
             * The result is not guaranteed to be on this ISurface.
             */
            virtual IPoint* centroid() const = 0;

            /*!
             * \brief The mathematical centroid for this ISurface as a Point.
             * The result is not guaranteed to be on this ISurface.
             */

            virtual IPoint* pointOnSurface() const = 0;

            //!
            virtual IMultiCurve* boundaryMultiCurve() const = 0;
      };

      /*!
       * \brief A MultiSurface is a 2-dimensional GeometryCollection whose elements
       * are Surfaces, all using coordinates from the same coordinate reference system.
       */
      class IMultiSurface : public virtual IGeometryCollection
      {

         public:
            virtual ~IMultiSurface(){}

            /*!
             * \brief The area of this ISurface, as measured in
             * the spatial reference system of this ISurface.
             */
            virtual double area() const = 0;

            /*!
             * \brief The mathematical centroid for this ISurface as an IPoint.
             * The result is not guaranteed to be on this ISurface.
             */
            virtual IPoint* centroid() const = 0;

            /*!
             * \brief The mathematical centroid for this ISurface as an IPoint.
             * The result is not guaranteed to be on this ISurface.
             */
            virtual IPoint* pointOnSurface() const = 0;

      };

      class IPolyhedralSurface;

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
            virtual ~IPolygon(){}

            /*!
             * \returns the exterior ring of this IPolygon.
             */
            virtual ILineString* exteriorRing() const = 0;

            /*!
             * \returns the number of interior rings in this IPolygon.
             */
            virtual int interiorRingCount() const = 0;

            /*!
             * \returns the index - th interior ring for this IPolygon as a ILineString.
             */
            virtual ILineString* interiorRing(int index) const = 0;

            /*!
             * \brief An arbitrary adjacent edge for this IPolygon.
             * \returns An edge that is adjacent to this face;
             *    null if degenerate
             */
            virtual IEdge *edge() const = 0;

            /*!
             * \brief Add a given adjacent edge to this face.
             * \param edge -> an edge that is adjacent to this face;
             *         must be nonnull
             */
            virtual void addEdge(IEdge *edge) = 0;

            /*!
             * \brief Remove a given adjacent from this face.
             * \param edge -> an edge that is no longer adjacent to this face;
             *         must be nonnull
             */
            virtual void removeEdge(IEdge *edge) = 0;

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
            virtual ~IMultiPolygon(){}

            /*!
               * \returns the index sup(th) polygon in this IMultiPolygon/IGeometryCollection.
               */
            virtual IPolygon* element(int index) const = 0;
      };

      /*!
       * \brief An ITriangle is a IPolygon with 3 distinct,
       * non-collinear vertices and no interior boundary.
       */
      class ITriangle : public virtual IPolygon
      {
         public:
            virtual ~ITriangle(){}

            virtual IVertex* p1() const = 0;

            virtual IVertex* p2() const = 0;

            virtual IVertex* p3() const = 0;

      };

      //************************************************************************

      /*!
       * \brief The INetwork class.
       */
      class INetwork : public virtual IMultiLineString
      {
         public:
            /*!
             * \brief ~Network
             */
            virtual ~INetwork(){}

            /*!
             * \brief An arbitrary vertex from which traversal can be conducted.
             */
            virtual IVertex* vertex() const ;
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
            virtual ~IPolyhedralSurface(){}

            /*!
             * \returns The number of including polygons.
             */
            virtual int patchCount() const = 0;

            /*!
             * \returns An IPolygon in this surface, the order is arbitrary.
             * \param index of the IPolygon patch.
             */
            virtual IPolygon* patch(int index) const = 0;

            /*!
             * \returns The collection of polygons in this surface that
             * bounds the given polygon "polygon" for any polygon "polygon" in the surface.
             */
            virtual IMultiPolygon* boundingPolygons(const IPolygon* polygon) const = 0;

            /*!
             * \returns 1 (True) if the polygon closes on itself,
             * and thus has no boundary and encloses a solid.
             */
            virtual bool isClosed() const = 0;

            /*!
             * \brief A new IEdge formed by splitting a given vertex between a given pair
             * of faces.
             *
             * \details A new IVertex is introduced at the destination of the new edge.
             * The new edge has _left_ along its left and _right_ along its right.
             *
             * \param vertex -> the IVertex to split to make the new edge;
             *                must be nonnull; must share an edge with both _left_ and _right_.
             *
             * \param left -> the left face adjacent to the new edge;
             *                must be nonnull;
             *                must share an edge with _vertex_.
             *
             * \param right -> the right face adjacent to the new edge;
             *                must be nonnull;
             *                must share an edge with _vertex_.
             *
             * \returns the new edge; will be nonnull.
             *
             */
            virtual IEdge *makeVertexEdge(IVertex *vertex, IPolygon *left, IPolygon *right) = 0;

            /*!
             * \brief Delete a given edge from this cell, along with its destination vertex.
             * \param edge -> the edge to delete;
             *         must be nonnull
             */
            virtual void deleteVertexEdge(IEdge *edge) = 0;

            /*!
             * \brief A new IEdge formed by splitting a given face through a given pair
             * of vertices.
             *
             * \details A new IPolygon is introduced to the right of the new edge.
             * The new IEdge has _orig_ as its origin and _dest_ as its destination.
             *
             * \param face  -> the face to divide to make the new edge;
             *              must be nonnull;
             *              must have both _org_ and _dest_ on its perimiter
             *
             * \param orig -> the vertices for the endpoints of the new IEdge;
             *              must be nonnull;
             *              must be located on the perimiter of _face_
             *
             * \param dest -> the vertices for the endpoints of the new IEdge;
             *              must be nonnull;
             *              must be located on the perimiter of _face_
             *
             * \returns The new IEdge; will be nonnull.
             */
            virtual IEdge *makeFaceEdge(IPolygon *face, IVertex *orig, IVertex *dest) = 0;

            /*!
             * \brief Delete a given IEdge from this IPolyhedralSurface, along with its right IPolygon.
             * \param edge -> the IEdge to delete; must be nonnull.
             */
            virtual void deleteFaceEdge(IEdge *edge) = 0;

      };

      /*!
       * \brief An ITIN is a triangulated irregular network
       * IPolyhedralSurface consisting only of ITriangle patches.
       */
      class ITIN : public virtual IPolyhedralSurface
      {
         public:
            virtual ~ITIN(){}

            /*!
               * \returns an ITriangle in this surface, the order is arbitrary.
               */
            virtual ITriangle* patch(int index) const = 0;
      };

      //************************************************************************

      class IRasterBand;

      /*!
       * \brief A Raster spatial feature.
       */
      class IRaster : public virtual IIdentity
      {

         public:
            virtual ~IRaster(){}

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
            virtual ISpatialReferenceSystem* spatialReferenceSystem() const = 0;

            /*!
             * \brief Fetches the affine transformation coefficients. It is an array of size 6.
             * \details Fetches the coefficients for transforming between pixel/line (P,L)
             * raster space, and projection coordinates (Xp,Yp) space.
             * Xp = transformationMatrix[0] + P*transformationMatrix[1] + L*transformationMatrix[2];
             * Yp = transformationMatrix[3] + P*transformationMatrix[4] + L*transformationMatrix[5];
             * In a north up image, transformationMatrix[1] is the pixel width, and transformationMatrix[5] is the pixel height. The upper left corner of the upper left pixel is at position (transformationMatrix[0],transformationMatrix[3]).
             */
            virtual void geoTransformation(double* transformationMatrix) = 0;

            /*!
             * \brief Gets the IRasterBand for the band with index bandIndex.
             */
            virtual IRasterBand* getRasterBand(int bandIndex) const = 0;

      };

      /*!
       * \brief IRasterBand is a single raster band for an IRaster.
       */
      class IRasterBand : public virtual IIdentity
      {

         public:
            virtual ~IRasterBand(){}

            //!Number of pixels in the x direction
            virtual int xSize() const = 0;

            //!Number of pixels in y direction
            virtual int ySize() const = 0;

            //!Parent IRaster of this IRasterBand
            virtual IRaster* raster() const = 0;

            //!Raster data type
            virtual RasterDataType dataType() const = 0;

            /*!
               * \brief Reads data into the image block.
               * \param xOffset is the pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.</param>
               * \param yOffset is the line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.</param>
               * \param xSize is the width of the region of the band to be accessed in pixels.</param>
               * \param ySize is the height of the region of the band to be accessed in lines.</param>
               * \param image is the pointer to where data is to be written. Delete after use.
               */
            virtual void read(int xOffset, int yOffset, int xSize, int ySize, void* image) const = 0;

            /*!
               * \brief Writes image into the raster band.
               * \param xOffset is the pixel offset to the top left corner of the region of the band to be accessed. This would be zero to start from the left side.</param>
               * \param yOffset is the line offset to the top left corner of the region of the band to be accessed. This would be zero to start from the top.</param>
               * \param xSize is the width of the region of the band to be accessed in pixels.</param>
               * \param ySize is the height of the region of the band to be accessed in lines.</param>
               * \param image is the pointer to where data is to be written. Delete after use.
              */
            virtual void write(int xOffset, int yOffset, int xSize, int ySize, const void* image) = 0;

            /*!
               * The nodata value for this IRasterBand.
               */
            virtual double noData() const = 0;
      };

      //************************************************************************

      /*!
       * \brief The types of regular grids.
       */
      enum RegularGridType
      {
         //!Cartesian grid.
         Cartesian,
         //!Rectilinear grid.
         Rectilinear,
         //!Curvilinear grid.
         Curvilinear
      };

      /*!
       * \brief The IRegularGrid2D class
       */
      class IRegularGrid2D : public virtual IIdentity
      {
         public:
            virtual ~IRegularGrid2D(){}

            virtual RegularGridType gridType() const  = 0;

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
            virtual double xNodeLocation(int xNodeIndex , int yNodeIndex) const = 0;


            /*!
             * \brief yNodeLocation provides the x location coordinate for the x-node and y-node indexes.
             * \param xNodeIndex the x-node index.
             * \param yNodeIndex the y-node index
             * \return returns y location coordinate for the x-node and y-node provided
             */
            virtual double yNodeLocation(int xNodeIndex , int yNodeIndex) const = 0;

            /*!
             * \brief isActive is a bool indicating whether a cell is active.
             * \param xCellIndex the x cell index for the cell. Must be less than numXNodes() - 1.
             * \param yCellIndex the y cell index for the cell. Must be less than numYNodes() - 1.
             * \return a bool indicating whether a cell is active.
             */
            virtual bool isActive(int xCellIndex , int yCellIndex) const = 0;
      };

      /*!
       * \brief The IRegularGrid3D class
       */
      class IRegularGrid3D : public virtual IRegularGrid2D
      {
         public:
            virtual ~IRegularGrid3D(){}

            /*!
             * \brief numZNodes represents the number of nodes in the z direction.
             * \return number of nodes in the z direction.
             */
            virtual int numZNodes() const = 0;

            /*!
             * \brief zNodeLocation provides the z location coordinate for the x-node, y-node, and z-node indexes.
             * \param xNodeIndex the x-node index.
             * \param yNodeIndex the y-node index
             * \param zNodeIndex the z-node index
             * \return returns z location coordinate for the x-node, y-node, and z-node indexes.
             */
            virtual double zNodeLocation(int xNode , int yNode , int zNode) const = 0;

            /*!
             * \brief isActive is a bool indicating whether a cell is active.
             * \param xCellIndex the x cell index for the cell. Must be less than numXNodes() - 1.
             * \param yCellIndex the y cell index for the cell. Must be less than numYNodes() - 1.
             * \param zCellIndex the z cell index for the cell. Must be less than numZNodes() - 1.
             * \return a bool indicating whether a cell is active.
             */
            virtual bool isActive(int xCellIndex , int yCellIndex , int zCellIndex) const = 0;
      };

      //************************************************************************

      /*!
       * \brief IGeometryComponentItem represents IGeometryCollection IComponentItem.
       */
      class IGeometryComponentItem : public virtual IComponentItem
      {
         public:
            virtual ~IGeometryComponentItem(){}

            /*!
             * \returns The IDimension for this IGeometryComponentItem.
             */
            virtual IDimension* geometryDimension() const = 0;

            /*!
             * \returns An IGeometryCollection associated with this IGeometryComponentItem.
             */
            virtual IGeometryCollection* geometryCollection() const = 0;

            /*!
             * \returns An IGeometry for the geometry index.
             */
            virtual IGeometry* geometry(int geometryIndex) const = 0;
      };

      /*!
       * \brief The IGeometryValueSet class
       */
      class IGeometryValueSet : public virtual IValueSet
      {
         public:
            virtual ~IGeometryValueSet(){}

            /*!
             * \returns The IGeometryComponentItem associated with this IGeometryValueSet.
             */
            virtual IGeometryComponentItem* geometryComponentItem() const = 0;

            //!Gets a single value for given geometry dimension index.
            /*!
             * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int geometryDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension geometry dimension index and size for a hyperslab.
            /*!
             * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int geometryDimensionIndex, int stride,  QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given geometry dimension index and size for a hyperslab.
            /*!
             * \param geometryDimensionIndex is the geometry dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(int geometryDimensionIndex, int stride,  void* data) const = 0;

            //!Sets a single value for given geometry dimension index.
            /*!
             * \param geometryDimensionIndex is the geometry dimension index where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int geometryDimensionIndex, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given geometry dimension index and size for a hyperslab.
            /*!
             * \param geometryDimensionIndex is the geometry dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int geometryDimensionIndex, int stride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given geometry dimension index and size for a hyperslab.
            /*!
             * \param geometryDimensionIndex is the geometry dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int geometryDimensionIndex , int stride, const void* data) = 0;

      };

      /*!
       * \brief An IGeometryArgument class is an IArgument class with a geometry component.
       */
      class IGeometryArgument : public virtual IArgument , public virtual IGeometryComponentItem
      {

         public:
            virtual ~IGeometryArgument(){}

            /*!
             * \brief Sets an IGeometryCollection associated with this IGeometryArgument.
             * Update Appropriate dimensions.
             *
             * \param geometry to set.
             */
            virtual void setGeometryCollection(IGeometryCollection* geomtery) = 0;

            /*!
             * \brief IGeometryValueSet objects associated with this IGeometryArgument.
             * \returns A list of IGeometryValueSet objects associated with this IGeometryArgument.
             */
            virtual QList<IGeometryValueSet*> geometryValueSets() const = 0;

      };

      /*!
       * \brief An IExchangeItem class with a geometry component.
       */
      class IGeometryExchangeItem : public virtual IExchangeItem, public virtual IGeometryComponentItem
      {
         public:
            /*!
             * \brief ~IGeometryExchangeItem
             */
            virtual ~IGeometryExchangeItem(){}

            /*!
             * \brief IGeometryValueSet  associated with this IGeometryExchangeItem.
             * \returns An IGeometryValueSet associated with this IGeometryExchangeItem.
             */
            virtual IGeometryValueSet* geometryValues() const = 0;
      };

      //************************************************************************

      /*!
       * \brief ITINComponentItem represents ITIN IComponentItem.
       */
      class ITINComponentItem : public virtual IComponentItem
      {
         public:
            virtual ~ITINComponentItem(){}

            /*!
             * \returns The IDimension for this ITINComponentItem triangles.
             */
            virtual IDimension* patchDimension() const = 0;

            /*!
             * \returns The ITriangle associated with this ITINComponentItem.
             */
            virtual ITriangle* polyhedralSurface() const = 0;

            /*!
             * \returns An IPolygon for the patch index.
             */
            virtual ITriangle* geometry(int patchIndex) const = 0;
      };

      /*!
       * \brief The ITINValueSet class.
       */
      class ITINValueSet : public virtual IValueSet
      {
         public:
            virtual ~ITINValueSet(){}

            /*!
             * \returns The ITINComponentItem associated with this ITINValueSet .
             */
            virtual ITINComponentItem* TINComponentItem() const = 0;

            //!Gets a single value for given patch dimension index.
            /*!
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int patchDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int patchDimensionIndex, int stride,  QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(int patchDimensionIndex, int stride,  void* data) const = 0;

            //!Sets a single value for given patch dimension index.
            /*!
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int patchDimensionIndex, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int patchDimensionIndex, int stride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int patchDimensionIndex , int stride, const void* data) = 0;

      };

      /*!
       * \brief An ITINArgument class is an IArgument class with an ITIN component.
       */
      class ITINArgument : public virtual IArgument , public virtual ITINComponentItem
      {
         public:

            /*!
             * \brief ~IPolyhedralSurfaceArgument.
             */
            virtual ~ITINArgument(){}

            /*!
             * \brief Sets an IPolyhedralSurface associated with this IPolyhedralSurfaceArgument.
             * Update Appropriate dimensions.
             *
             */
            virtual void setTIN(ITIN* TINSurface) = 0;

            /*!
             * \brief IPolyhedralSurfaceValueSet objects associated with this IPolyhedralSurfaceArgument.
             * \returns A list of IPolyhedralSurfaceValueSet objects associated with this IPolyhedralSurfaceArgument.
             */
            virtual QList<ITINValueSet*> patchValueSets() const = 0;
      };

      /*!
       * \brief An ITINExchangeItem class with an TIN component.
       */
      class ITINExchangeItem : public virtual IExchangeItem, public virtual ITINComponentItem
      {
         public:
            virtual ~ITINExchangeItem(){}

            /*!
             * \brief ITINValueSet associated with this ITINExchangeItem.
             * \returns A ITINValueSet objects associated with this ITINExchangeItem.
             */
            virtual ITINValueSet* patchValues() const = 0;
      };


      //************************************************************************

      /*!
       * \brief IPolyhedralSurfaceComponentItem represents IPolyhedralSurface IComponentItem.
       */
      class IPolyhedralSurfaceComponentItem : public virtual IComponentItem
      {
         public:
            virtual ~IPolyhedralSurfaceComponentItem(){}

            /*!
             * \returns The IDimension for this IPolyhedralSurfaceComponentItem patches.
             */
            virtual IDimension* patchDimension() const = 0;

            /*!
             * \returns The IPolyhedralSurface associated with this IPolyhedralSurfaceComponentItem.
             */
            virtual IPolyhedralSurface* polyhedralSurface() const = 0;

            /*!
             * \returns An IPolygon for the patch index.
             */
            virtual IPolygon* geometry(int patchIndex) const = 0;
      };

      /*!
       * \brief The IPolyhedralSurfaceValueSet class.
       */
      class IPolyhedralSurfaceValueSet : public virtual IValueSet
      {
         public:
            virtual ~IPolyhedralSurfaceValueSet(){}

            /*!
             * \returns The IPolyhedralSurfaceComponentItem associated with this IPolyhedralSurfaceValueSet .
             */
            virtual IPolyhedralSurfaceComponentItem* polyhedralSurfaceComponentItem() const = 0;

            //!Gets a single value for given patch dimension index.
            /*!
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int patchDimensionIndex, QVariant& data) const = 0;

            //!Gets a multi-dimensional array of values for given dimension patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int patchDimensionIndex, int stride,  QVariant* data) const = 0;

            //!Gets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index from where to obtain the requested data.
             * \param stride is the size for hyperslab from which to copy data.
             * \param data is a multi dimensional array where data is to be written. Must be allocated beforehand with the correct data type.
             */
            virtual void getValues(int patchDimensionIndex, int stride,  void* data) const = 0;

            //!Sets a single value for given patch dimension index.
            /*!
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int patchDimensionIndex, const QVariant& data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int patchDimensionIndex, int stride, const QVariant* data) = 0;

            //!Sets a multi-dimensional array of values for given patch dimension index and size for a hyperslab.
            /*!
             * \param patchDimensionIndex is the patch dimension index where data is to be written.
             * \param stride is the size for hyperslab where data is to be written.
             * \param data is the input multi dimensional array to be written.
             */
            virtual void setValues(int patchDimensionIndex , int stride, const void* data) = 0;

      };

      /*!
       * \brief An IPolyhedralSurfaceArgument class is an IArgument class with an IPolyhedralSurface component.
       */
      class IPolyhedralSurfaceArgument : public virtual IArgument , public virtual IPolyhedralSurfaceComponentItem
      {
         public:

            /*!
             * \brief ~IPolyhedralSurfaceArgument.
             */
            virtual ~IPolyhedralSurfaceArgument(){}

            /*!
             * \brief Sets an IPolyhedralSurface associated with this IPolyhedralSurfaceArgument.
             * Update Appropriate dimensions.
             *
             */
            virtual void setPolyhedralSurface(IPolyhedralSurface* geomtery) = 0;

            /*!
             * \brief IPolyhedralSurfaceValueSet objects associated with this IPolyhedralSurfaceArgument.
             * \returns A list of IPolyhedralSurfaceValueSet objects associated with this IPolyhedralSurfaceArgument.
             */
            virtual QList<IPolyhedralSurfaceValueSet*> patchValueSets() const = 0;
      };

      /*!
       * \brief An IPolyhedralSurfaceExchangeItem class with an IPolyhedralSurface component.
       */
      class IPolyhedralSurfaceExchangeItem : public virtual IExchangeItem, public virtual IPolyhedralSurfaceComponentItem
      {
         public:
            virtual ~IPolyhedralSurfaceExchangeItem(){}

            /*!
             * \brief IPolyhedralSurfaceValueSet associated with this IPolyhedralSurfaceExchangeItem.
             * \returns A IPolyhedralSurfaceValueSet objects associated with this IPolyhedralSurfaceExchangeItem.
             */
            virtual IPolyhedralSurfaceValueSet* patchValues() const = 0;
      };


      //************************************************************************

      /*!
       * \brief An IRasterComponentItem represents an IRaster IComponentItem.
       */
      class IRasterComponentItem : public virtual IComponentItem
      {

         public:
            virtual ~IRasterComponentItem(){}

            /*!
             * \brief IRaster associated with this IComponentItem.
             */
            virtual IDimension* rasterDimension() const = 0;

            /*!
             * \brief IDimension for xDirection.
             */
            virtual IDimension* xDimension() const = 0;

            /*!
             * \brief IDimension for yDirection.
             */
            virtual IDimension* yDimension() const = 0;

            /*!
             * \brief IDimension for IRasterBands.
             */
            virtual IDimension* bandDimension() const = 0;

            /*!
             * \brief IRaster associated with this IRasterComponentItem.
             */
            virtual IRaster* raster() const = 0;

      };

      /*!
       * \brief The IRasterValueSet class.
       */
      class IRasterValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ~IRasterValueSet
             */
            virtual ~IRasterValueSet(){}

            /*!
             * \returns The IRasterComponentItem associated with this IRasterValueSet .
             */
            virtual IRasterComponentItem* rasterComponentItem() const = 0;


            /*!
             * \brief Gets a single value for given x, y, raster band indexes.
             * \param xindex is the x dimension index from where to obtain the requested data.
             * \param yindex is the y dimension index from where to obtain the requested data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param data is the output QVariant where data is to be written.
             */
            virtual void getValue(int xindex, int yindex, int band, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xindex is the x dimension index from where to obtain the requested data.
             * \param yindex is the y dimension index from where to obtain the requested data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param xstride is the x size for hyperslab from which to copy data.
             * \param ystride is the x size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int xindex, int yindex, int band, int xstride, int ystride,  QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xindex is the x dimension index from where to obtain the requested data.
             * \param yindex is the y dimension index from where to obtain the requested data.
             * \param band is the band dimension index from where to obtain the requested data.
             * \param xstride is the x size for hyperslab from which to copy data.
             * \param ystride is the x size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int xindex, int yindex, int band, int xstride, int ystride,  void* data) const = 0;

            /*!
             * \brief Sets a single value for given x, y, raster band indexes.
             * \param xindex is the x dimension index where to set data.
             * \param yindex is the y dimension index where to set data.
             * \param band is the band dimension index where to set data.
             * \param data is the input QVariant to be written.
             */
            virtual void setValue(int xindex, int yindex, int band, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xindex is the x dimension index where to set data.
             * \param yindex is the y dimension index where to set data.
             * \param band is the band dimension index where to set data.
             * \param xstride is the x size for hyperslab where data is to be written.
             * \param ystride is the y size for hyperslab where data is to be written.
             * \param data is the input QVariant array to be written.
             */
            virtual void setValues(int xindex, int yindex, int band, int xstride, int ystride,  const QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xindex is the x dimension index where to set data.
             * \param yindex is the y dimension index where to set data.
             * \param band is the band dimension index where to set data.
             * \param xstride is the x size for hyperslab where data is to be written.
             * \param ystride is the y size for hyperslab where data is to be written.
             * \param data is the input array to be written.
             */
            virtual void setValues(int xindex, int yindex, int band, int xstride, int ystride,  const void* data) = 0;

      };

      /*!
       * \brief An  IRasterArgument is an IArgument with an IRaster.
       */
      class IRasterArgument : public virtual IArgument, public virtual IRasterComponentItem
      {
         public:
            virtual ~IRasterArgument(){}

            /*!
             * \brief Set IRaster associated with IRasterArgument.
             *
             * \param raster IRaster to set.
             */
            virtual void setRaster(IRaster* raster) = 0;

            /*!
             * \brief IRasterValueSet objects associated with this IRasterArgument.
             * \returns A list of IRasterValueSet objects associated with this IRasterArgument.
             */
            virtual QList<IRasterValueSet*> rasterValueSets() const = 0;
      };

      /*!
       * \brief An IRasterExhangeItem is an IExchangeItem with an IRaster.
       */
      class IRasterExhangeItem : public virtual IExchangeItem, public virtual IRasterComponentItem
      {
         public:

            /*!
             * \brief ~IRasterExhangeItem.
             */
            virtual ~IRasterExhangeItem(){}

            /*!
             * \brief IRasterValueSet associated with this IRasterExhangeItem.
             * \returns An IRasterValueSet object associated with this IRasterExhangeItem.
             */
            virtual IRasterValueSet* rasterValues() const = 0;
      };

      //************************************************************************

      /*!
       * \brief An IRegularGrid2DComponentItem represents an IRegularGrid2D IComponentItem
       */
      class IRegularGrid2DComponentItem : public virtual IComponentItem
      {

         public:

            /*!
             * \brief ~IRegularGrid2DComponentItem.
             */
            virtual ~IRegularGrid2DComponentItem(){}

            /*!
             * \brief Number of X cells IDimension.
             */
            virtual IDimension* numXCellsDimension() const = 0;

            /*!
             * \brief Number of Y cells IDimension.
             */
            virtual IDimension* numYCellsDimension() const = 0;

            /*!
             * \brief IRegularGrid2D grid associated with this IRegularGrid2DComponentItem.
            */
            virtual IRegularGrid2D* grid() const = 0;

      };

      /*!
       * \brief The IRegularGrid2DValueSet class.
       */
      class IRegularGrid2DValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ~IRegularGrid2DValueSet.
             */
            virtual ~IRegularGrid2DValueSet(){}

            /*!
             * \brief regularGrid2DComponentItem
             * \return
             */
            virtual IRegularGrid2DComponentItem* regularGrid2DComponentItem() const = 0;

            /*!
             * \brief Gets the value for the xcell index and ycell index.
             * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
             * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
             * \param data is output into which the requested data is to be written.
             */
            virtual void getValue(int xCellIndex, int yCellIndex, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param xCellStride is the xcell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int xCellIndex, int yCellIndex, int xCellStride, int yCellStride, QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param xCellStride is the xcell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int xCellIndex, int yCellIndex, int xCellStride, int yCellStride, void* data) const = 0;

            /*!
             * \brief Sets the value for the xcell index and ycell index.
             * \param xCellIndex the x dimension index for the cell from which data is to be written.
             * \param yCellIndex the y dimension index for the cell from which data is to be written.
             * \param data is input data to be written.
             */
            virtual void setValue(int xCellIndex, int yCellIndex, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param xCellStride is the xcell size for hyperslab where data is to be written.
             * \param yCellStride is the ycell size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int xCellIndex, int yCellIndex, int xCellStride, int yCellStride, QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param xCellStride is the xcell size for hyperslab where data is to be written.
             * \param yCellStride is the ycell size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int xCellIndex, int yCellIndex, int xCellStride, int yCellStride, void* data) = 0;
      };

      /*!
       * \brief An  IRegularGrid2DArgument is an IArgument with an IRegularGrid2DComponentItem.
       */
      class IRegularGrid2DArgument : public virtual IArgument, public virtual IRegularGrid2DComponentItem
      {
         public:
            virtual ~IRegularGrid2DArgument(){}

            /*!
             * \brief Set IRegularGrid2D associated with IRegularGrid2DArgument.
              */
            virtual void setGrid(IRegularGrid2D* grid) = 0;

            /*!
             * \brief IRegularGrid2DValueSet objects associated with this IRegularGrid2DArgument.
             * \returns A list of IRegularGrid2DValueSet objects associated with this IRegularGrid2DArgument.
             */
            virtual QList<IRegularGrid2DValueSet*> regular2DValueSets() const = 0;
      };

      /*!
       * \brief An IRegularGrid2DExhangeItem is an IExchangeItem with an IRegularGrid2DComponentItem.
       */
      class IRegularGrid2DExhangeItem : public virtual IExchangeItem, public virtual IRegularGrid2DComponentItem
      {
         public:
            virtual ~IRegularGrid2DExhangeItem(){}

            /*!
             * \brief IRegularGrid2DValueSet associated with this IRegularGrid2DExhangeItem.
             * \returns An IRegularGrid2DValueSet object associated with this IRegularGrid2DExhangeItem.
             */
            virtual IRegularGrid2DValueSet* regular2DValues() const = 0;

      };

      //************************************************************************

      /*!
       * \brief An IRegularGrid3DComponentItem represents an IRegularGrid3D IComponentItem
       */
      class IRegularGrid3DComponentItem : public virtual IRegularGrid2DComponentItem
      {

         public:
            virtual ~IRegularGrid3DComponentItem(){}

            /*!
             * \brief Number of Z cells IDimension.
             */
            virtual IDimension* numZCellsDimension() const = 0;

            /*!
             * \brief  IRegularGrid3D grid associated with this IRegularGrid3DComponentItem.
            */
            virtual IRegularGrid3D* grid() const = 0;

      };

      /*!
       * \brief The IRegularGrid3DValueSet class.
       */
      class IRegularGrid3DValueSet : public virtual IValueSet
      {
         public:

            /*!
             * \brief ~IRegularGrid3DValueSet.
             */
            virtual ~IRegularGrid3DValueSet(){}

            /*!
             * \brief regularGrid2DComponentItem
             * \return
             */
            virtual IRegularGrid3DComponentItem* regularGrid3DComponentItem() const = 0;

            /*!
             * \brief Gets the value for the xcell index and ycell index.
             * \param xCellIndex the x dimension index for the cell from which data is to be retrieved.
             * \param yCellIndex the y dimension index for the cell from which data is to be retrieved.
             * \param zCellIndex the z dimension index for the cell from which data is to be retrieved.
             * \param data is output into which the requested data is to be written.
             */
            virtual void getValue(int xCellIndex, int yCellIndex, int zCellIndex, QVariant& data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param zCellIndex is the z dimension index from where to obtain the requested data.
             * \param xCellStride is the xcell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param zCellStride is the zcell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int xCellIndex, int yCellIndex, int zCellIndex, int xCellStride, int yCellStride, int zCellStride, QVariant* data) const = 0;

            /*!
             * \brief Gets a multi-dimensional array of values for given dimension for a hyperslab.
             * \param xCellIndex is the x dimension index from where to obtain the requested data.
             * \param yCellIndex is the y dimension index from where to obtain the requested data.
             * \param zCellIndex is the z dimension index from where to obtain the requested data.
             * \param xCellStride is the xcell size for hyperslab from which to copy data.
             * \param yCellStride is the ycell size for hyperslab from which to copy data.
             * \param zCellStride is the zcell size for hyperslab from which to copy data.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void getValues(int xCellIndex, int yCellIndex, int zCellIndex, int xCellStride, int yCellStride, int zCellStride,  void* data) const = 0;

            /*!
             * \brief Sets the value for the xcell index and ycell index.
             * \param xCellIndex the x dimension index for the cell from which data is to be written.
             * \param yCellIndex the y dimension index for the cell from which data is to be written.
             * \param zCellIndex the z dimension index for the cell from which data is to be written.
             * \param data is input data to be written.
             */
            virtual void setValue(int xCellIndex, int yCellIndex, int zCellIndex, const QVariant& data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param zCellIndex is the z dimension index where to write data.
             * \param xCellStride is the xcell size for hyperslab where data is to be written.
             * \param yCellStride is the ycell size for hyperslab where data is to be written.
             * \param zCellStride is the zcell size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int xCellIndex, int yCellIndex, int zCellIndex, int xCellStride, int yCellStride, int zCellStride, QVariant* data) = 0;

            /*!
             * \brief Sets a multi-dimensional array of values for given dimensions of a hyperslab.
             * \param xCellIndex is the x dimension index where to write data.
             * \param yCellIndex is the y dimension index where to write data.
             * \param zCellIndex is the z dimension index where to write data.
             * \param xCellStride is the xcell size for hyperslab where data is to be written.
             * \param yCellStride is the ycell size for hyperslab where data is to be written.
             * \param zCellStride is the zcell size for hyperslab where data is to be written.
             * \param data is the multi dimensional array where data is to be written. Must be allocated beforehand.
             */
            virtual void setValues(int xCellIndex, int yCellIndex, int zCellIndex, int xCellStride, int yCellStride, int zCellStride, void* data) = 0;
      };

      /*!
       * \brief An  IRegularGrid3DArgument is an IArgument with an IRegularGrid3DComponentItem.
       */
      class IRegularGrid3DArgument : public virtual IArgument, public virtual IRegularGrid3DComponentItem
      {
         public:
            virtual ~IRegularGrid3DArgument(){}

            /*!
             * \brief Set IRegularGrid2D associated with IRegularGrid2DArgument.
              */
            virtual void setGrid(IRegularGrid3D* grid) = 0;


            /*!
             * \brief IRegularGrid3DValueSet objects associated with this IRegularGrid3DArgument.
             * \returns A list of IRegularGrid2DValueSet objects associated with this IRegularGrid3DArgument.
             */
            virtual QList<IRegularGrid3DValueSet*> regular3DValueSets() const = 0;
      };

      /*!
       * \brief An IRegularGrid3DExhangeItem is an IExchangeItem with an IRegularGrid3DComponentItem.
       */
      class IRegularGrid3DExhangeItem : public virtual IExchangeItem, public virtual IRegularGrid3DComponentItem
      {
         public:
            virtual ~IRegularGrid3DExhangeItem(){}

            /*!
             * \brief IRegularGrid3DValueSet associated with this IRegularGrid3DExhangeItem.
             * \returns An IRegularGrid3DValueSet object associated with this IRegularGrid3DExhangeItem.
             */
            virtual IRegularGrid3DValueSet* regular3DValues() const = 0;

      };

   }
}

Q_DECLARE_INTERFACE(HydroCouple::Spatial::ISpatialReferenceSystem, "HydroCouple::Spatial::ISpatialReferenceSystem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IGeometry, "HydroCouple::Spatial::IGeometry/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IGeometryCollection, "HydroCouple::Spatial::IGeometryCollection/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IMultiPoint, "HydroCouple::Spatial::IMultiPoint/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IPoint, "HydroCouple::Spatial::IPoint/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IVertex, "HydroCouple::Spatial::IVertex/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ICurve, "HydroCouple::Spatial::ICurve/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IMultiCurve, "HydroCouple::Spatial::IMultiCurve/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IMultiLineString, "HydroCouple::Spatial::IMultiLineString/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ILineString, "HydroCouple::Spatial::ILineString/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ILine, "HydroCouple::Spatial::ILine/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ILinearRing, "HydroCouple::Spatial::ILinearRing/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IEdge, "HydroCouple::Spatial::IEdge/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ISurface, "HydroCouple::Spatial::ISurface/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IMultiSurface, "HydroCouple::Spatial::IMultiSurface/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IPolygon, "HydroCouple::Spatial::IPolygon/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IMultiPolygon, "HydroCouple::Spatial::IMultiPolygon/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ITriangle, "HydroCouple::Spatial::ITriangle/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::INetwork, "HydroCouple::Spatial::INetwork/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IPolyhedralSurface, "HydroCouple::Spatial::IPolyhedralSurface/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ITIN, "HydroCouple::Spatial::ITIN/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRaster, "HydroCouple::Spatial::IRaster/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRasterBand, "HydroCouple::Spatial::IRasterBand/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid2D, "HydroCouple::Spatial::IRegularGrid2D/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid3D, "HydroCouple::Spatial::IRegularGrid3D/1.0")


Q_DECLARE_INTERFACE(HydroCouple::Spatial::IGeometryComponentItem, "HydroCouple::Spatial::IGeometryComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IGeometryValueSet, "HydroCouple::Spatial::IGeometryValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IGeometryArgument, "HydroCouple::Spatial::IGeometryArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IGeometryExchangeItem, "HydroCouple::Spatial::IGeometryExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::Spatial::ITINComponentItem, "HydroCouple::Spatial::ITINComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ITINValueSet, "HydroCouple::Spatial::ITINValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ITINArgument, "HydroCouple::Spatial::ITINArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::ITINExchangeItem, "HydroCouple::Spatial::ITINExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::Spatial::IPolyhedralSurfaceComponentItem, "HydroCouple::Spatial::IPolyhedralSurfaceComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IPolyhedralSurfaceValueSet, "HydroCouple::Spatial::IPolyhedralSurfaceValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IPolyhedralSurfaceArgument, "HydroCouple::Spatial::IPolyhedralSurfaceArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IPolyhedralSurfaceExchangeItem, "HydroCouple::Spatial::IPolyhedralSurfaceExchangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRasterComponentItem, "HydroCouple::Spatial::IRasterComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRasterValueSet, "HydroCouple::Spatial::IRasterValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRasterArgument, "HydroCouple::Spatial::IRasterArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRasterExhangeItem, "HydroCouple::Spatial::IRasterExhangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid2DComponentItem, "HydroCouple::Spatial::IRegularGrid2DComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid2DValueSet, "HydroCouple::Spatial::IRegularGrid2DValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid2DArgument, "HydroCouple::Spatial::IRegularGrid2DArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid2DExhangeItem, "HydroCouple::Spatial::IRegularGrid2DExhangeItem/1.0")

Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid3DComponentItem, "HydroCouple::Spatial::IRegularGrid3DComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid3DValueSet, "HydroCouple::Spatial::IRegularGrid3DValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid3DArgument, "HydroCouple::Spatial::IRegularGrid3DArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::Spatial::IRegularGrid3DExhangeItem, "HydroCouple::Spatial::IRegularGrid3DExhangeItem/1.0")

#endif // HYDROCOUPLESPATIAL

