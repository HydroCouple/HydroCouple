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

#ifndef ICATEGORY_H
#define ICATEGORY_H

#include <idescription.h>
#include <QVariant>

namespace HydroCouple
{
	namespace Data
	{
		//! The ICategory describes one item of a possible categorization. It is used by the IQuality interface for describing qualitative data.
		/*!
		<para>
		For qualitative data the <see cref="IBaseValueSet"/> exchanged between <see cref="IBaseLinkableComponent"/>s
		contains one of the possible ICategory instances per data element.
		</para>
		<para>
		A category defines one "class" within a "set of classes".
		</para>
		*/
		class ICategory : public virtual IDescription
		{
		public:
			//! value for this category.
			/*!
			<example>
			"blue" in a "red"/"green"/"blue" set.
			</example>
			*/
			virtual QVariant value() const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::ICategory, "HydroCouple::Data::ICategory/1.0")
#endif // ICATEGORY_H
