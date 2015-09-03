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

#ifndef ITIMESPAN_H
#define ITIMESPAN_H

#include <itime.h>

namespace HydroCouple
{
	namespace Data
	{
		//!Time related entities
		namespace Temporal
		{
			//!IHTimeSpan specifies a time duration
			class ITimeSpan : public virtual ITime
			{
				//Q_PROPERTY(double Duration READ duration);

			public:
				virtual ~ITimeSpan(){}

				//!Duration of the timespan in days
				virtual double duration() const = 0;
			};
		}
	}
}

using namespace HydroCouple::Data::Temporal;
Q_DECLARE_INTERFACE(ITimeSpan, "HydroCouple::Data::Temporal::ITimeSpan/1.0")

#endif // ITIMESPAN_H
