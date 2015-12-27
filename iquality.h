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
#ifndef IQUALITY_H
#define IQUALITY_H

#include <ivariabledefinition.h>
#include <icategory.h>

namespace HydroCouple
{
	namespace Data
	{
		//! Qualitative data described items in terms of some quality or categorization that may be 'informal' or may use relatively ill-defined characteristics such as warmth and flavour. However, qualitative data can include well-defined aspects such as gender, nationality or commodity type.
		/*!
		<para>
		An IQuality describes qualitative data, where a value is specified as one category within a number of predefined (possible) categories. These categories can be ordered or not.
		</para>

		<para>
		For qualitative data the IComponentItem data exchanged between IComponents contains one of the possible ICategory instances per element in the  IComponentItem involved.
		</para>

		<para>
		<example>
		Examples:
		<list>
		<item>Colors: red, green, blue</item>
		<item>Land use: nature, recreation, industry, infrastructure</item>
		<item>Rating: worse, same, better</item>
		</list>
		</example>
		</para>
		*/
		class IQuality : public virtual IVariableDefinition
		{
		public:
			//! Returns a list of the possible ICategory allowed for this IQuality If the quality is not ordered the list contains the ICategory's in an unspecified order. When it is ordered the list contains the ICategory's in the same sequence.
			virtual QList<ICategory*> categories() const = 0;

			//! Checks if the IQuality is defined by an ordered set of ICategory or not.
			virtual bool isOrdered() const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IQuality, "HydroCouple::Data::IQuality/1.0")

#endif // IQUALITY_H
