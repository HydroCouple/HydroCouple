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

#ifndef ITIME_H
#define ITIME_H

namespace HydroCouple
{
	namespace Data
	{
		//!Time related entities
		namespace Temporal
		{
			//! IHTime interface based on a Modified Julian Date (number and fraction of days since 00:00 November 17, 1858).
			class ITime
			{
				//Q_PROPERTY(double DateTime READ dateTime);

			public:
				virtual ~ITime(){}

				//!DateTime as a modified julian day value.
				virtual double dateTime() const = 0;

			};
		}
	}
}

using namespace HydroCouple::Data::Temporal;
Q_DECLARE_INTERFACE(ITime, "HydroCouple::Data::Temporal::ITime/1.0")

#endif // ITIME_H
