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

#ifndef IQUANTITYVARIABLEDEFINITION_H
#define IQUANTITYVARIABLEDEFINITION_H

#include <ivariabledefinition.h>
#include <iunit.h>

namespace HydroCouple
{
	namespace Data
	{
		//!A Quantity specifies values as an amount of some unit, usually as a floating point number.
		class IQuantityVariableDefinition : public virtual IVariableDefinition
		{
			//Q_PROPERTY(IHUnit* Unit READ unit)

		public:
			virtual ~IQuantityVariableDefinition(){}

			//!Unit of quantity
			virtual IUnit* unit() const = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IQuantityVariableDefinition, "HydroCouple::Data::IQuantityVariableDefinition/1.0")

#endif // IQUANTITYVARIABLEDEFINITION_H
