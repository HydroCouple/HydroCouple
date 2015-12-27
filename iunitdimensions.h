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
#ifndef IUNITDIMENSIONS_H
#define IUNITDIMENSIONS_H

#include "idescription.h"

namespace HydroCouple
{
	namespace Data
	{
		enum FundamentalDimension
		{
			//! Fundamental dimension for length.
			Length,

			//! Fundamental dimension for mass.
			Mass,

			//! Fundamental dimension for time.
			Time,

			//! Fundamental dimension for electric current.
			ElectricCurrent,

			//! Fundamental dimension for temperature.
			Temperature,

			//! Fundamental dimension for amount of substance.
			AmountOfSubstance,

			//! Fundamental dimension for luminous intensity.
			LuminousIntensity,

			//! Fundamental dimension for currency.
			Currency
		};


		//! Defines the order of dimension in each FundamentalDimension for a unit
		class IUnitDimensions : public virtual IDescription
		{
			//Q_ENUMS(FundamentalDimension);

		public:
			//!Returns the power for the requested dimension
			/*!
			<example>
			 <para>
			    For a quantity such as flow, which may have the unit m3/s, the getPower method must work as follows:
			 </para>
			 
			 <para>myDimension.getPower(FundamentalDimension::AmountOfSubstance) --> returns 0</para>
			 <para>myDimension.getPower(FundamentalDimension::Currency)          --> returns 0</para>
			 <para>myDimension.getPower(FundamentalDimension::ElectricCurrent)   --> returns 0</para>
			 <para>myDimension.getPower(FundamentalDimension::Length)            --> returns 3</para>
			 <para>myDimension.getPower(FundamentalDimension::LuminousIntensity) --> returns 0</para>
			 <para>myDimension.getPower(FundamentalDimension::Mass)              --> returns 0</para>
			 <para>myDimension.getPower(FundamentalDimension::Temperature)       --> returns 0</para>
			 <para>myDimension.getPower(FundamentalDimension::Time)              --> returns -1</para>
			 </example>
			*/
			virtual double getPower(FundamentalDimension dimension) const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IUnitDimensions, "HydroCouple::Data::IUnitDimensions/1.0")

#endif // IUNITDIMENSIONS_H
