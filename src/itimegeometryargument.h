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

#ifndef ITIMEGEOMETRYARGUMENT_H
#define ITIMEGEOMETRYARGUMENT_H

#include "iargument.h"
#include "itimegeometrycomponentitem.h"

namespace HydroCouple
{
	namespace Data
	{
		//! Temporal and Spatial related entities
		namespace TimeSpace
		{
			class ITimeGeometryArgument : public virtual IArgument , public virtual ITimeGeometryComponentItem
			{

			public:
				~ITimeGeometryArgument(){}


				//!Sets IGeometryCollection associated with this IGeometryArgument. Update Appropriate dimensions
				virtual void setGeometryCollection(IGeometryCollection* geomtery) = 0;

				//!Set ITimes associated with this ITimeArgument. Dont forget to update timeSpace and timeDimension accordingly
				virtual void  setTimes(const QList<ITime*>& times) = 0;

			};
		}
	}
}

using namespace HydroCouple::Data::TimeSpace;
Q_DECLARE_INTERFACE(ITimeGeometryArgument, "HydroCouple::Data::TimeSpace::ITimeGeometryArgument/1.0")

#endif // ITIMEGEOMETRYARGUMENT_H
