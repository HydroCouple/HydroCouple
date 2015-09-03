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

#ifndef ITIMECOMPONENTITEM_H
#define ITIMECOMPONENTITEM_H

#include "icomponentitem.h"
#include "idimension.h"
#include "itimespan.h"

namespace HydroCouple
{
	namespace Data
	{
		namespace Temporal
		{
			class ITimeComponentItem : public virtual IComponentItem
			{

			public:
				virtual ~ITimeComponentItem(){}

				//!Duration of the timespan in days
				virtual IDimension* timeDimension() const = 0;

				//! ITimes associated with this dimension.
				virtual QList<ITime*> times() const = 0;

				//! ITimeSpan associated with this dimension.
				virtual ITimeSpan* timeSpan() const = 0;

				//! Return ITime for index.
				virtual ITime* time(int timeIndex) const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Temporal;
Q_DECLARE_INTERFACE(ITimeComponentItem, "HydroCouple::Data::Temporal::ITimeComponentItem/1.0")

#endif // ITIMECOMPONENTITEM_H
