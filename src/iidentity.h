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

#ifndef IIDENTITY_H
#define IIDENTITY_H

#include "idescription.h"

namespace HydroCouple
{
	//!IIdentity interface class defines a method to get the Id of an HydroCouple entity. 
	/*!
	IIdentity extends the IDescribable interface class, and therefore has, next to the id, a caption and a description
	*/
	class IIdentity : public virtual IDescription
	{
		//!Properties must be implemented in derived classes
		//! Gets a unique identifier for the entity.
		//Q_PROPERTY(QString Id READ Id);

	public:
		virtual ~IIdentity(){}

		//! Gets a unique identifier for the entity.
		/*!
		Returns the Id as a String. The Id must be unique within its context but
		does not need to be globally unique. E.g. the id of an input exchange
		item must be unique in the list of inputs of a IHModelComponent, but a
		similar Id might be used by an exchange item of another IHModelComponent.
		*/
		virtual QString Id() const = 0;

	};
}

using namespace HydroCouple;
Q_DECLARE_INTERFACE(IIdentity, "HydroCouple::IIdentity/1.0")

#endif // IIDENTITY_H
