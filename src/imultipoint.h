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

#ifndef IHMULTIPOINT_H
#define IHMULTIPOINT_H

#include <igeometrycollection.h>
#include <ipoint.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A MultiPoint is a 0-dimensional GeometryCollection.
			/*!
			<para>
			The elements of a IHMultiPoint are restricted to IHPoints. The
			IHPoints are not connected or ordered in any semantically important way (see the discussion at
			IHGeometryCollection).
			</para>

			<para>
			A MultiPoint is simple if no two Points in the MultiPoint are equal (have identical coordinate values in X and Y).
			Every MultiPoint is spatially equal under the definition in Clause 6.1.15.3 to a simple Multipoint.
			The boundary of a MultiPoint is the empty set.
			</para>
			*/
			class IMultiPoint : public virtual IGeometryCollection
			{

			public:
				virtual ~IMultiPoint(){}

				//!Returns the index th HPoint in this IHGeometryCollection.
				virtual IPoint* point(int index) const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IMultiPoint, "HydroCouple::Data::Spatial::IMultiPoint/1.0")

#endif // IHMULTIPOINT_H
