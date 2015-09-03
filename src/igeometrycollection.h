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

#ifndef IGEOMETRYCOLLECTION_H
#define IGEOMETRYCOLLECTION_H

#include "igeometry.h"

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A GeometryCollection is a geometric object that is a collection of some number of geometric objects.
			/*!
			<para>
			All the elements in a GeometryCollection shall be in the same Spatial Reference System. This is also the Spatial
			Reference System for the GeometryCollection.
			</para>

			<para>
			GeometryCollection places no other constraints on its elements. Subclasses of GeometryCollection may restrict
			membership based on dimension and may also place other constraints on the degree of spatial overlap between
			elements.
			</para>
			*/
			class IGeometryCollection : public virtual IGeometry
			{
				//Q_PROPERTY(int GeometryCount READ geometryCount)

			public:
				virtual ~IGeometryCollection(){}

				//!Returns the number of geometries in this IHGeometryCollection.
				virtual int geometryCount() const = 0;

				//!Returns the index th geometry in this IHGeometryCollection.
				virtual IGeometry* geometry(int index) const  = 0;

			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IGeometryCollection, "HydroCouple::Data::Spatial::IGeometryCollection/1.0")

#endif // IGEOMETRYCOLLECTION_H
