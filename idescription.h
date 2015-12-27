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

#ifndef IDESCRIPTION_H
#define IDESCRIPTION_H

#include "ipropertychanged.h"

namespace HydroCouple
{
	//!IDescription interface class provides descriptive information on an HydroCouple entity
	/*!
	An entity that is describable has a caption (title or heading)
	and a description. These are not to be used for identification (see IIdentifier)
	*/
	class IDescription : public virtual IPropertyChanged
	{

	public:
		//!Gets caption for the entity.
		/*!
		\returns string representing caption for entity
		*/
		virtual QString getCaption() const = 0;

		//!Sets caption for the entity
		/*!
		\param caption is a string representing the caption for the entity
		*/
		virtual void setCaption(const QString & caption) = 0;

		//!Gets additional descriptive information for the entity.
		/*!
		\returns description of entity
		*/
		virtual QString getDescription() const = 0;

		//!Gets additional descriptive information for the entity.
		/*!
		\param description is a string for describing an entity
		*/
		virtual void setDescription(const QString& decription) = 0;

	};
}

Q_DECLARE_INTERFACE(HydroCouple::IDescription, "HydroCouple::IDescription/1.0")

#endif // IDESCRIPTION_H
