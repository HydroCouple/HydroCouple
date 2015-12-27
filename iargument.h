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
		IArgument is primarily used to set arguments of IModelComponent and IAdaptedOutput
		*/
		class IArgument : public virtual IComponentItem
		{

		public:
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
			This is used to let a IModelComponent or an IAdaptedOutput
			present the actual value of an argument that can not be changed by the user,
			but is needed to determine the values of other arguments or is informative
			in any other way.
			*/
			virtual bool isReadOnly() const = 0;

			//!Deep clones this IArgument
			/*!
			For example if the argument is a file, it makes a copy of the file with a different file name and reads
			*/
			virtual IArgument* clone() const = 0;

			//!Variables for this IArgument
			/*!
			*/
			virtual QList<IVariable*> variables() const = 0;

			//!Variables for this IArgument
			/*!
			*/
			virtual QString toString() const = 0;


			virtual void setFromString(const QString& value) = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IArgument, "HydroCouple::Data::IArgument/1.0")

#endif // IARGUMENT_H
