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
#ifndef IHPOLYGON_H
#define IHPOLYGON_H

#include <isurface.h>
#include <ilinestring.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A IHPolygon is a planar IHSurface defined by 1 exterior boundary and 0 or more interior boundaries.
			/*!
			<para>
			Each interior boundary defines a hole in the IHPolygon.
			</para>

			<para>
			The exterior boundary LinearRing defines the “top” of the surface which is the side of the surface from which the
			exterior boundary appears to traverse the boundary in a counter clockwise direction. The interior LinearRings will
			have the opposite orientation, and appear as clockwise when viewed from the “top”,
			</para>

			<para>
			The assertions for Polygons (the rules that define valid Polygons) are as follows:
			a) Polygons are topologically closed;
			b) The boundary of a Polygon consists of a set of LinearRings that make up its exterior and interior boundaries;
			c) No two Rings in the boundary cross and the Rings in the boundary of a Polygon may intersect at a Point but
			only as a tangent, e.g.

			∀ P ∈ Polygon, ∀ c1,c2∈P.Boundary(), c1≠c2,
			∀ p, q ∈Point, p, q ∈ c1, p ≠ q ,
			[p ∈ c2] ⇒ [∃ δ > 0 ∋ [|p-q|<δ] ⇒ [q ∉ c2] ];

			Note: This last condition says that at a point common to the two curves, nearby points cannot be common. This
			forces each common point to be a point of tangency.
			d) A Polygon may not have cut lines, spikes or punctures e.g.:
			∀ P ∈ Polygon, P = P.Interior.Closure;
			e) The interior of every Polygon is a connected point set;
			f) The exterior of a Polygon with 1 or more holes is not connected. Each hole defines a connected component of
			the exterior.
			In the above assertions, interior, closure and exterior have the standard topological definitions. The combination
			of (a) and (c) makes a Polygon a regular closed Point set. Polygons are simple geometric objects.

			</para>

			*/
			class IPolygon : public virtual ISurface
			{
				Q_PROPERTY(ILineString* ExteriorRing READ exteriorRing)
				Q_PROPERTY(int InteriorRingCount READ interiorRingCount)

			public:
				virtual ~IPolygon(){}

				//!Returns the exterior ring of this IHPolygon.
				virtual ILineString* exteriorRing() const = 0;

				//!Returns the number of interior rings in this IHPolygon.
				virtual int interiorRingCount() const = 0;

				//!Returns the index - th interior ring for this Polygon as a LineString.
				virtual ILineString* interiorRing(int index) const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IPolygon, "HydroCouple::Data::Spatial::IPolygon/1.0")

#endif // IHPOLYGON_H
