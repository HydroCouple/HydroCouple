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

#ifndef IHPOLYHEDRALSURFACE_H
#define IHPOLYHEDRALSURFACE_H

#include <isurface.h>
#include <imultipolygon.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{

			//!A PolyhedralSurface is a contiguous collection of polygons, which share common boundary segments.
			/*!
			
			<para>
			For each pair of IHPolygons that “touch”, the common boundary shall be expressible as a finite collection of IHLineStrings
			such IHLineString shall be part of the boundary of at most 2 IHPolygon patches. A IHTIN (triangulated irregular network)
			is a IHPolyhedralSurface consisting only of IHTriangle patches.
			</para>

			<para>
			For any two IHPolygons that share a common boundary, the “top” of the IHPolygon shall be consistent. This means
			that when two IHLinearRings from these two IHPolygons traverse the common boundary segment, they do so in
			opposite directions. Since the IHPolyhedral surface is contiguous, all IHPolygons will be thus consistently oriented.
			This means that a non-oriented surface (such as Möbius band) shall not have single surface representations.
			They may be represented by a IHMultiSurface.
			</para>


			<para>
			If each such IHLineString is the boundary of exactly 2 IHPolygon patches, then the IHPolyhedralSurface is a simple,
			closed polyhedron and is topologically isomorphic to the surface of a sphere. By the Jordan Surface Theorem
			(Jordan’s Theorem for 2-spheres), such polyhedrons enclose a solid topologically isomorphic to the interior of a
			sphere; the ball. In this case, the “top” of the surface will either point inward or outward of the enclosed finite solid.
			If outward, the surface is the exterior boundary of the enclosed surface. If inward, the surface is the interior of the
			infinite complement of the enclosed solid. A Ball with some number of voids (holes) inside can thus be presented
			as one exterior boundary shell, and some number in interior boundary shells.
			</para>

			*/
			class IPolyhedralSurface : public virtual ISurface
			{
				Q_PROPERTY(int PatchCount READ patchCount)
				Q_PROPERTY(bool IsClosed READ isClosed)


			public:
				virtual ~IPolyhedralSurface(){}

				//!Returns the number of including polygons
				virtual int patchCount() const = 0;

				//!Returns a polygon in this surface, the order is arbitrary.
				virtual IPolygon* patch(int index) const = 0;

				//!Returns the collection of polygons in this surface that bounds the given polygon "polygon" for any polygon "polygon" in the surface.
				virtual IMultiPolygon* boundingPolygons(const IPolygon* polygon) const = 0;

				//!Returns 1 (True) if the polygon closes on itself, and thus has no boundary andencloses a solid
				virtual bool isClosed() const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IPolyhedralSurface, "HydroCouple::Data::Spatial::IPolyhedralSurface/1.0")

#endif // IHPOLYHEDRALSURFACE_H
