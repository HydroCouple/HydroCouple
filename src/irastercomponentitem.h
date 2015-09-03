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

#ifndef IHRASTERCOMPONENTITEM_H
#define IHRASTERCOMPONENTITEM_H

#include "icomponentitem.h"
#include "idimension.h"
#include "iraster.h"


namespace HydroCouple
{
	namespace Data
	{

		namespace Spatial
		{

			//!IRasterComponentItem represents an IHRaster IHComponentItem
			class IRasterComponentItem : public virtual IComponentItem
			{
				//Q_PROPERTY(IRaster* Raster READ raster)

			public:
				virtual ~IRasterComponentItem(){}

				//!IHRaster associated with this IHComponentItem = band * row + column
				virtual IDimension* rasterDimension() const = 0;

				//!Dimension for xDirection.
				/*!
				*/
				virtual IDimension* xDimension() const = 0;

				//!Dimension for yDirection.
				/*!
				*/
				virtual IDimension* yDimension() const = 0;

				//!Dimension for bands.
				/*!
				*/
				virtual IDimension* bandDimension() const = 0;

				//!Raster associated with this IRasterComponentItem
				/*!
				*/
				virtual IRaster* raster() const = 0;

			};
		}
	}
}

using namespace HydroCouple::Data::Spatial;
Q_DECLARE_INTERFACE(IRasterComponentItem, "HydroCouple::Data::Spatial::IRasterComponentItem/1.0")

#endif // IHRASTERCOMPONENTITEM_H
