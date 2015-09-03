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
#ifndef IARGUMENT_H
#define IARGUMENT_H

#include <QVariant>
#include "icomponentitem.h"


class IVariable;

namespace HydroCouple
{
	namespace Data
	{
		//!IArgument interface class used to set the arguments  
		/*!
		IArgument is primarily used to set arguments of IHModelComponent and IHAdaptedProducer
		*/
		class IArgument : public virtual IComponentItem
		{
			//!Properties must be implemented in derived classes
			//Q_PROPERTY(QVariant::Type ValueType READ valueType)
			//Q_PROPERTY(bool IsOptional READ isOptional)
			//Q_PROPERTY(bool IsReadOnly READ isReadOnly)
			//Q_PROPERTY(QVariant Value READ getValue WRITE setValue)
			//Q_PROPERTY(QVariant DefaultValue READ defaultValue)
			//Q_PROPERTY(QList<QVariant> PossibleValues READ possibleValues)

		public:
			virtual ~IArgument(){}

			////!The type of the value of the argument
			///*!
			//Examples include an integral type like string, integer or double, or a non integral type, such as a time series object
			//*/
			//virtual QVariant::Type valueType() const = 0;

			//!Specifies whether the argument is optional or not.
			/*!
			If the getValue property returns null and isOptional == false, a value has to be set before the argument can be used
			*/
			virtual bool isOptional() const = 0;

			//!Defines whether the Values property may be edited
			/*!
			This is used to let a IBaseLinkableComponent or an IBaseAdaptedOutput
			present the actual value of an argument that can not be changed by the user,
			but is needed to determine the values of other arguments or is informative
			in any other way.
			*/
			virtual bool isReadOnly() const = 0;

			//!Deep clones the IArgument*
			/*!
			For example if the argument is a file, it makes a copy of the file with a different file name;
			*/
			virtual IArgument* clone() const = 0;

			//!Variables for this IArgument
			/*!
			*/
			virtual QList<IVariable*> variables() const = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IArgument, "HydroCouple::Data::IArgument/1.0")

#endif // IARGUMENT_H
