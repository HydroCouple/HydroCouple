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

#ifndef IHSURFACE_H
#define IHSURFACE_H

#include <ipoint.h>
#include <imulticurve.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A IHSurface is a 2-dimensional geometric object.
			/*!
			<para>
			A simple Surface may consists of a single “patch” that is associated with one “exterior boundary” and 0 or more
			“interior” boundaries. A single such Surface patch in 3-dimensional space is isometric to planar Surfaces, by a
			simple affine rotation matrix that rotates the patch onto the plane z = 0. If the patch is not vertical, the projection
			onto the same plane is an isomorphism, and can be represented as a linear transformation, i.e. an affine.
			</para>

			<para>
			Polyhedral Surfaces are formed by “stitching” together such simple Surfaces patches along their common
			boundaries. Such polyhedral Surfaces in a 3-dimensional space may not be planar as a whole, depending on the
			orientation of their planar normals (Reference [1], sections 3.12.9.1, and 3.12.9.3). If all the patches are in
			alignment (their normals are parallel), then the whole stitched polyhedral surface is co-planar and can be
			represented as a single patch if it is connected.
			</para>

			<para>
			The boundary of a simple Surface is the set of closed Curves corresponding to its “exterior” and “interior”
			boundaries (Reference [1], section 3.12.9.4).
			</para>

			<para>
			The only instantiable subclasses of IHSurface defined in this standard are IHPolygon and IHPolyhedralSurface. A
			IHPolygon is a simple Surface that is planar. A IHPolyhedralSurface is a simple surface, consisting of some number of
			IHPolygon patches or facets. If a IHPolyhedralSurface is closed, then it bounds a solid. A IHMultiSurface containing a set
			of closed IHPolyhedralSurfaces can be used to represent a IHSolid object with holes.
			</para>

			*/
			class ISurface : public virtual IGeometry
			{
				Q_PROPERTY(double Area READ area)
				Q_PROPERTY(IHPoint* Centroid READ centroid)

			public:
				virtual ~ISurface(){}

				//!The area of this IHSurface, as measured in the spatial reference system of this IHSurface.
				virtual double area() const = 0;

				//!The mathematical centroid for this IHSurface as a Point. The result is not guaranteed to be on this IHSurface.
				virtual IPoint* centroid() const = 0;

				//!The mathematical centroid for this IHSurface as a Point. The result is not guaranteed to be on this IHSurface.
				virtual IPoint* pointOnSurface() const = 0;

				//!
				virtual IMultiCurve* boundaryMultiCurve() const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(ISurface, "HydroCouple::Data::Spatial::ISurface/1.0")

#endif // IHSURFACE_H
