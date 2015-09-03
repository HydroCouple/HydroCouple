/****************************************************************************
**
**  Copyright (C) 2014 Caleb Amoa Buahin
**  Contact: calebgh@gmail.com
**
**  This file is part of HydroCouple.dll
**
**  HydroCouple.dll and its associated files is free software; you can redistribute it and/or modify
**  it under the terms of the Lesser GNU General Public License as published by
**  the Free Software Foundation; either version 3 of the License, or
**  (at your option) any later version.
**
**  HydroCouple.dll and its associated files is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  Lesser GNU General Public License for more details.
**
**  You should have received a copy of the Lesser GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>
**
****************************************************************************/

#ifndef IGEOMETRY_H
#define IGEOMETRY_H

#include <QString>
#include <ispatial.h>
#include <ispatialreferencesystem.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!IHGeometry is the root class of the hierarchy.
			class IGeometry : public virtual IIdentity
			{
				//Q_PROPERTY(int Dimension READ dimension)
				//Q_PROPERTY(int SpatialDimension READ spatialDimension)
				//Q_PROPERTY(int CoordinateDimension READ coordinateDimension)
				//Q_PROPERTY(GeometryType GeometryType READ geometryType)
				//Q_PROPERTY(IHSpatialReferenceSystem* SpatialReferenceSystem READ spatialReferenceSystem)
				//Q_PROPERTY(IHGeometry* Envelope READ envelope)
				//Q_PROPERTY(QString WellKnownText READ wkt)
				//Q_PROPERTY(bool IsEmpty READ isEmpty)
				//Q_PROPERTY(bool IsSimple READ isSimple)
				//Q_PROPERTY(bool Is3D READ is3D)
				//Q_PROPERTY(bool IsMeasured READ isMeasured)
				//Q_PROPERTY(IHGeometry* Boundary READ boundary)

			public:
				virtual ~IGeometry(){}

				//!The inherent dimension of this geometric object, which must be less than or equal to the coordinate dimension. In non - homogeneous collections, this will return the largest topological dimension of the contained objects
				/*!
				<returns>
				0 for points, 1 for lines and 2 for surfaces.
				</returns>
				*/
				virtual int dimension() const = 0;

				//!Get the dimension of the coordinates in this object.
				/*!
				<returns>
				In practice this will return 2 or 3. It can also return 0 in the case of an empty point.
				</returns>
				*/
				virtual int coordinateDimension() const = 0;

				//virtual int spatialDimension() const = 0;

				//!Returns an enum representing the instantiable subtype of Geometry of which this geometric object is an instantiable member.The name of the subtype of Geometry is returned as a string
				virtual GeometryType geometryType() const = 0;

				//!Spatial reference system of goemetric object
				virtual ISpatialReferenceSystem* spatialReferenceSystem() const = 0;

				//!The minimum bounding box for this Geometry, returned as a Geometry.
				/*!
				The polygon is defined by the corner points of the bounding box [(MINX, MINY), (MAXX, MINY), (MAXX, MAXY),
				(MINX, MAXY), (MINX, MINY)]. Minimums for Z and M may be added. The simplest representation of an
				Envelope is as two direct positions, one containing all the minimums, and another all the maximums. In some
				cases, this coordinate will be outside the range of validity for the Spatial Reference System.
				*/
				virtual IGeometry* envelope() const = 0;

				//!Exports this geometric object to a specific Well-known Text Representation of Geometry.
				virtual QString wkt() const = 0;


				//!Exports this geometric object to a specific Well-known byte Representation of Geometry.
				virtual unsigned char* wkb(int & size) const = 0;


				//!Returns <code>true</code> if this geometric object is the empty Geometry. 
				/*!
				If true, then this geometric object represents the empty point set ∅ for the coordinate space.
				*/
				virtual bool isEmpty() const = 0;

				//!Returns <code>true</code> if this geometric object has no anomalous geometric points, such as self intersection or self tangency.
				/*!
				The description of each instantiable geometric class will include the specific conditions that cause an instance of that class to be classified as not simple.
				*/
				virtual bool isSimple() const = 0;

				//!Returns <code>true</code> if this geometric object has z coordinate values.
				virtual bool is3D() const = 0;

				//!Returns <code>true</code> if this geometric object has m coordinate values.
				virtual bool isMeasured() const = 0;

				//!Returns the closure of the combinatorial boundary of this geometric object (Reference [1], section 3.12.2)
				/*!
				Because the result of this function is a closure, and hence topologically
				closed, the resulting boundary can be represented using representational Geometry primitives (Reference [1],
				section 3.12.2). The return type is integer, but is interpreted as Boolean, TRUE=1, FALSE=0.
				*/
				virtual IGeometry* boundary() const = 0;

				/** @name Query
				*Query functions
				*/
				///@{

				//! Returns <code>true</code> if this geometric object is spatially equal to geom 
				virtual bool equals(const IGeometry* const geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially disjoint to geom
				virtual bool disjoint(const IGeometry* const  geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially intersects to geom
				virtual bool intersects(const IGeometry* const  geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially touches to geom
				virtual bool touches(const IGeometry* const  geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially crosses to geom
				virtual bool crosses(const IGeometry* const  geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially within to geom
				virtual bool within(const IGeometry* const  geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially contains to geom
				virtual bool contains(const IGeometry* const  geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially overlaps to geom
				virtual bool overlaps(const IGeometry* const  geom) const = 0;

				//! Returns <code>true</code> if this geometric object is spatially related to geom by testing for intersections between the interior, boundary and exterior of the two geometric objects as specified by the values in the intersectionPatternMatrix.
				/*!
				This returns <code>false</code> if all the tested intersections are empty except exterior (this) intersect exterior (another)
				*/
				virtual bool relate(const IGeometry* const  geom) const = 0;

				//! Returns a derived geometry collection value that matches the specified m coordinate value
				virtual IGeometry* loocateAlong(double value) const = 0;

				//! Returns a derived geometry collection value that matches the specified range of m coordinate values inclusively 
				virtual IGeometry* locateBetween(double mStart, double mEnd) const = 0;

				///@}


				/** @name Spatial Analysis
				*Spatial analysis functions
				*/
				///@{

				//!Returns the shortest distance between any two Points in the two geometric objects as calculated in the spatial reference system of this geometric object.
				/*!
				Because the	geometries are closed, it is possible to find a point on each geometric object involved, such that the distance
				between these 2 points is the returned distance between their geometric objects.
				*/
				virtual double distance(const IGeometry*  const geom) const = 0;

				//!Returns a geometric object that represents all Points whose distance	from this geometric object is less than or equal to distance.
				/*!
				Calculations are in the spatial reference system of this geometric object. Because of the limitations of linear interpolation, there will often be some relatively
				small error in this distance, but it should be near the resolution of the coordinates used.
				*/
				virtual IGeometry* buffer(double buffer) const = 0;

				//!Returns a geometric object that represents the convex hull of this geometric	object.
				/*!
			    Convex hulls, being dependent on straight lines, can be accurately represented in linear interpolations for any geometry restricted to linear interpolations.
				*/
				virtual IGeometry* convexHull() const = 0;

				//!Returns a geometric object that represents the Point set intersection of this geometric object with geom.
				virtual IGeometry* intersection(const IGeometry*  const geom) const = 0;

				//!Returns a geometric object that represents the Point set union of this geometric object with geom.
				virtual IGeometry* unionp(const IGeometry*  const geom) const = 0;

				//!Returns a geometric object that represents the Point set difference of this geometric object with geom.
				virtual IGeometry* difference(const IGeometry*  const geom) const = 0;

				//!Returns a geometric object that represents the Point set symmetric difference of this geometric object with geom.
				virtual IGeometry* symmetricDifference(const IGeometry*  const geom) const = 0;

				///@}
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IGeometry, "HydroCouple::Data::Spatial::IGeometry/1.0")

#endif // IGEOMETRY_H
