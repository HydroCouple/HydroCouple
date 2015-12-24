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

#ifndef IPROPERTYCHANGED_H
#define IPROPERTYCHANGED_H

#include <QObject>

namespace HydroCouple
{
	//!IPropertyChanged interface is used to emit signal/event when a property of an object changes
	/*!
	*/
	class IPropertyChanged
	{

	public:
		virtual ~IPropertyChanged(){}

	signals:
		//!called to emit signal/event when property of child class changes
		/*!
        \param propertyName is a string representing the name of the property
		\param value is a QVariant representing the value of the property
		*/
		virtual void propertyChanged(const QString& propertyName, const QVariant& value) = 0;

	};
}

Q_DECLARE_INTERFACE(HydroCouple::IPropertyChanged, "HydroCouple::IPropertyChanged/1.0");

#endif // IPROPERTYCHANGED_H
