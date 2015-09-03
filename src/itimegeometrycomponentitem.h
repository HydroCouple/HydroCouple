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
#ifndef ITIMEGEOMETRYCOMPONENTITEM_H
#define ITIMEGEOMETRYCOMPONENTITEM_H

#include  "igeometrycomponentitem.h"
#include "itimecomponentitem.h"

namespace HydroCouple
{
	namespace Data
	{
		namespace TimeSpace
		{
			//!IHTimeComponentItem represents geometries with associated time series IHComponentItem
			class ITimeGeometryComponentItem : public virtual ITimeComponentItem, public virtual IGeometryComponentItem
			{
				//Q_PROPERTY(IHTimeSpan* TimeSpan READ timeSpan)
				//	Q_PROPERTY(QList<IHTime*>  Times READ times)
				//	Q_PROPERTY(IHGeometryCollection* GeometryCollection READ geometryCollection)

			public:
				virtual ~ITimeGeometryComponentItem(){}

			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(ITimeGeometryComponentItem, "HydroCouple::DataSpatial::ITimeGeometryComponentItem/1.0")

#endif // IHTIMEGEOMETRYCOMPONENTITEM_H
