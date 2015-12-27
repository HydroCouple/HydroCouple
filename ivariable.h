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

#ifndef IVARIABLE_H
#define IVARIABLE_H

#include "idescription.h"
#include "icomponentitem.h"
#include "ivariabledefinition.h"


namespace HydroCouple
{
	namespace Data
	{
		class IVariable : public virtual IDescription
		{

		public:
			//!IComponentItem associated with this IVariable
			/*!
			*/
			virtual IComponentItem* componentItem() const = 0;

			//!IDimension s related to this IVariable
			/*!
			*/
			virtual QList<IDimension*> dimensions() const = 0;

			//!IDimension s related to this IVariable
			/*!
			\param dimensionIndexes for the IDimension to retrieve the length
			*/
			virtual int getDimensionLength(const QList<int>& dimensionIndexes) const = 0;
	
			//!Gets value for given dimension indexes
			/*!
			\param dimensionIndexes for the IDimension to retrieve the length
			*/
            virtual QVariant getValue(int dimensionIndexes[]) const = 0;

			//!sets value for given dimension indexes
			/*!
			\param dimensionIndexes for the IDimension to retrieve the length
			\param value to set 
			*/
			virtual void setValue(int dimensionIndexes[], const QVariant& value) = 0;

			//!IVariableDefinition for this IVariable defines the variable type associated with this object
			/*!
			*/
			virtual IVariableDefinition variableDefinition() const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IVariable, "HydroCouple::Data::IVariable/1.0");

#endif // IVARIABLE_H