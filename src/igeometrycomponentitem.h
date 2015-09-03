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
#ifndef IGEOMETRYCOMPONENTITEM_H
#define IGEOMETRYCOMPONENTITEM_H

#include "icomponentitem.h"
#include "igeometrycollection.h"


namespace HydroCouple
{
	namespace Data
	{
		namespace Spatial
		{
			//!IGeometryComponentItem represents IHGeometryCollection IHComponentItem
			class IGeometryComponentItem : public virtual IComponentItem
			{
				//Q_PROPERTY(IGeometryComponentItem* GeometryCollection READ geometryCollection)

			public:
				virtual ~IGeometryComponentItem(){}

				//!Returns IGeometryDimension for IGeometryComponentItem
				virtual IDimension* geometryDimension() const = 0;

				//!IGeometryCollection associated with this IGeometryComponentItem
				virtual IGeometryCollection* geometryCollection() const = 0;

				//!Returns IGeometry for index
				virtual IGeometry* geometry(int geometryIndex) const = 0;
			};
		}
	}
}
using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IGeometryComponentItem, "HydroCouple::Data::Spatial::IGeometryComponentItem/1.0")
#endif // IGEOMETRYCOMPONENTITEM_H
