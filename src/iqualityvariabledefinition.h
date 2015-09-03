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
#ifndef IQUALITYVARIABLEDEFINITION_H
#define IQUALITYVARIABLEDEFINITION_H

#include <ivariabledefinition.h>
#include <icategory.h>

namespace HydroCouple
{
	namespace Data
	{
		//! Qualitative data described items in terms of some quality or categorization that may be 'informal' or may use relatively ill-defined characteristics such as warmth and flavour. However, qualitative data can include well-defined aspects such as gender, nationality or commodity type.
		/*!
		<para>
		An IHQualityValueDefinition describes qualitative data, where a value is specified as one category within a number of predefined (possible) categories. These categories can be ordered or not.
		</para>

		<para>
		For qualitative data the IHComponentItem data exchanged between IHComponents contains one of the possible IHCategory instances per element in the  IHComponentItem involved.
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
		class IQualityVariableDefinition : public virtual IVariableDefinition
		{
			//Q_PROPERTY(QList<IHCategory*> Categories READ categories)
			//Q_PROPERTY(bool IsOrdered READ isOrdered)

		public:
			virtual ~IQualityVariableDefinition(){}

			//! Returns a list of the possible IHCategory allowed for this IQualityValueDefinition. If the quality is not ordered the list contains the IHCategory's in an unspecified order. When it is ordered the list contains the IHCategory's in the same sequence.
			virtual QList<ICategory*> categories() const = 0;

			//! Checks if the IQualityValueDefinition is defined by an ordered set of IHCategory or not.
			virtual bool isOrdered() const = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IQualityVariableDefinition, "HydroCouple::Data::IQualityVariableDefinition/1.0")

#endif // IQUALITYVARIABLEDEFINITION_H
