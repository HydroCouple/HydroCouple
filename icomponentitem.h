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

#ifndef ICOMPONENTITEM_H
#define ICOMPONENTITEM_H

#include "imodelcomponent.h"
#include "idimension.h"

namespace HydroCouple
{
	//! Data related entities
	namespace Data
	{
		//!IComponentItem is a fundamental unit of data for a component.
		/*!
		This interface is not to be implemented directly, any class is to implement either the IInput or IOutput.
		*/
		class IComponentItem : public virtual IIdentity
		{

		public:
			//! Gets the owner IModelComponent of this IComponentItem. For an IOutput component item this is the component responsible for providing the content of the IOutput.
			/*!
			It is possible for an IComponentItem to have no owner, in this case the method will return nullptr.
			*/
			virtual IModelComponent* modelComponent() const = 0;

			//! Gets IDimension s associated with this IComponentItem.
			/*!
			*/
			virtual QList<IDimension*> dimensions() const = 0;

		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IComponentItem, "HydroCouple::Data::IComponentItem/1.0")

#endif // ICOMPONENTITEM_H
