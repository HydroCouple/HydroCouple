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
#ifndef IUNIT_H
#define IUNIT_H

#include "idescription.h"
#include "iunitdimensions.h"

namespace HydroCouple
{
	namespace Data
	{
		/// IUnit interface, describing the physical unit of a IHQuantity.
		class IUnit : public virtual IDescription
		{

		public:
			virtual ~IUnit(){}

			//!Fundamental dimensions of the unit
			virtual IUnitDimensions* dimensions() const = 0;

			//! Conversion factor to SI ('A' in: SI-value = A * quant-value + B)
			virtual double  conversionFactorToSI() const = 0;

			//! OffSet to SI ('B' in: SI-value = A * quant-value + B)
			virtual double offSetToSI() const = 0;
		};
	}
}
using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IUnit, "HydroCouple::Data::IUnit/1.0")

#endif // IUnit_H
