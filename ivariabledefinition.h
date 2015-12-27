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
#ifndef IVALUEDEFINITION_H
#define IVALUEDEFINITION_H

#include <QVariant>
#include "idescription.h"

namespace HydroCouple
{
	namespace Data
	{
		//! IVariableDefinition describes a value returned by the getValues function of IVariable
		/*! 
		 This interface is not meant to be implemented directly.
		 Instead, implement either IQuality or IQuantity or a custom derived vale definition interface.
		*/
		class IVariableDefinition : public virtual IDescription
		{
		public:
			//!The object types of value that will be available and is returned by the GetValues function.
			virtual QVariant::Type type() const = 0;

			//!The value representing that data is missing.
			virtual QVariant missingData() const = 0;

			//!Gets the default value of the argument
			virtual QVariant defaultValue() const = 0;

			//!Gets list of possible allowed values 
			/*!
			If for integral types or component specific types all possible values are allowed, invalid qvariant is returned.
			A list with length 0 indicates that there is indeed a limitation on the possible values, but that currently
			no values are possible. Effectively this means that the values will not and cannot be set.
			*/
			virtual QList<QVariant> possibleValues() const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IVariableDefinition, "HydroCouple::Data::IVariableDefinition/1.0")

#endif // IVALUEDEFINITION_H
