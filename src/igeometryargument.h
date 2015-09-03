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
#ifndef IGEOMETRYARGUMENT_H
#define IGEOMETRYARGUMENT_H

#include "iargument.h"
#include "igeometrycomponentitem.h"

namespace HydroCouple
{
	namespace Data
	{

		class IGeometryArgument : public virtual IArgument , public virtual IGeometryComponentItem
		{

		public:
			virtual ~IGeometryArgument(){}

			//!Sets IGeometryCollection associated with this IGeometryArgument. Update Appropriate dimensions
			virtual void setGeometryCollection(IGeometryCollection* geomtery) = 0;

		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IGeometryArgument, "HydroCouple::Data::IGeometryArgument/1.0")
#endif // IGEOMETRYARGUMENT_H
