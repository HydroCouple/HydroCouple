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

#ifndef IHMULTIPOLYGON_H
#define IHMULTIPOLYGON_H

#include <imultisurface.h>
#include <ipolygon.h>

namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!A MultiPolygon is a MultiSurface whose elements are Polygons.
			class IMultiPolygon : public virtual IMultiSurface
			{

			public:
				virtual ~IMultiPolygon(){}

				//!Returns the index th polygon in this IHMultiPolygon/IHGeometryCollection.
				virtual IPolygon* element(int index) const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IMultiPolygon, "HydroCouple::Data::Spatial::IMultiPolygon/1.0")

#endif // IHMULTIPOLYGON_H
