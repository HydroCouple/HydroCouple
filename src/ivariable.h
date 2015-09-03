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

#include "iidentity.h"
#include "icomponentitem.h"
#include "ivariabledefinition.h"


namespace HydroCouple
{
	namespace Data
	{
		class IVariable : public virtual IIdentity
		{

		public:
			virtual ~IVariable(){}
			
			//!ComponentItem
			/*!
			*/
			virtual IComponentItem* componentItem() const = 0;

			//!Dimensions
			/*!
			*/
			virtual QList<IDimension*> dimensions() const = 0;

			//!Gets uniqueID for combination of dimension indexes
			/*!
			*/
            virtual QString getIdFromDimensions(int dimensions []) const = 0;

			//!Gets value for given dimension indexes
			/*!
			*/
            virtual QVariant getValue(int dimensions []) const = 0;


			//!Gets value for given id
			/*!
			*/
			virtual QVariant getValue(const QString& id) const = 0;

			//!Gets multidimensional values for given id for a certain length. Reciever is reponsible for deletion
			/*!
			*/
			virtual QVariant* getValues(int startDimIndexes [], int length []) const = 0;

			//!sets value for given dimension indexes
			/*!
			*/
			virtual void setValue(int dimIndexes [], const QVariant& value) = 0;

			//!sets value for given dimension id
			/*!
			*/
			virtual void setValue(const QString& id, const QVariant& value) = 0;

			//!sets values for given dimension and length and deletes value after success full
			/*!
			Implement at your own 
			*/
			virtual void setValues(int startDimIndexes [], int length [],  QVariant* const value) = 0;

			//!gets the variableDefinition
			/*!
			*/
			virtual IVariableDefinition variableDefinition() const = 0;

			//!gets the variableDefinition
			/*!
			*/
			virtual QString valuesToString() const = 0;

			//!gets the variableDefinition
			/*!
			*/
			virtual void valuesFromString(const QString& values)  = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IVariable, "HydroCouple::Data::IVariable/1.0")

#endif // IVARIABLE_H
