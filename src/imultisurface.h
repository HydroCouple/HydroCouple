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

#ifndef IHMULTISURFACE_H
#define IHMULTISURFACE_H

#include <igeometrycollection.h>
#include <ipoint.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A MultiSurface is a 2-dimensional GeometryCollection whose elements are Surfaces, all using coordinates from	the same coordinate reference system.
			class IMultiSurface : public virtual IGeometryCollection
			{
			/*	Q_PROPERTY(double Area READ area)
				Q_PROPERTY(IHPoint* Centroid READ centroid)*/

			public:
				virtual ~IMultiSurface(){}

				//!The area of this IHSurface, as measured in the spatial reference system of this IHSurface.
				virtual double area() const = 0;

				//!The mathematical centroid for this IHSurface as a Point. The result is not guaranteed to be on this IHSurface.
				virtual IPoint* centroid() const = 0;

				//!The mathematical centroid for this IHSurface as a Point. The result is not guaranteed to be on this IHSurface.
				virtual IPoint* pointOnSurface() const = 0;

			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IMultiSurface, "HydroCouple::Data::Spatial::IMultiSurface/1.0")

#endif // IHMULTISURFACE_H
