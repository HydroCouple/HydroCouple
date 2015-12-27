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

#ifndef IDIMENSION_H
#define IDIMENSION_H

#include <idescription.h>

namespace HydroCouple
{
	namespace Data
	{
		enum DimensionType
		{
			Constant,
			Dynamic
		};

		class IDimension : public virtual IDescription
		{

		public:
			//! Gets length of dimension
			/*!
			*/
			virtual int length() const = 0;

			//! Gets the dimension length type
			/*!
			*/
			virtual DimensionType lengthType() const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IDimension, "HydroCouple::Data::IDimension/1.0")
#endif // IDIMENSION_H
