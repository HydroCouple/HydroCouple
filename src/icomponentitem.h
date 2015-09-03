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
#include "icomponentitemchangeevent.h"
#include "idimension.h"

namespace HydroCouple
{
	//! Data related entities
	namespace Data
	{
		//! IComponentItem is a fundamental unit of data for a component.
		/*!
		<para>
		This interface is not to be implemented directly, any class is to implement either the IHConsumerItem or IHProducerItem.
		</para>
		*/
		class IComponentItem : public virtual IIdentity
		{
			//Q_PROPERTY(IHValueDefinition* ValueDefinition READ valueDefinition)
			//Q_PROPERTY(IHModelComponent* Component READ component)

		public:
			virtual ~IComponentItem(){}

			//! Gets the owner of the exchange item. For an output exchange item this is the component responsible for providing the content of the output item.
			/*!
			It is possible for an exchange item to have no owner, in this case the method will return null.
			*/
			virtual IModelComponent* modelComponent() const = 0;

			//! Gets dimensions associated with this ComponentItem.
			/*!
			*/
			virtual QList<IDimension*> dimensions() const = 0;

		signals:
			//! The componentItemChanged event is fired when the content of an IHComponentItem has changed.
            /*!
            \param  statusChangedEvent event provides information about the status change
            */
             virtual void componentItemChanged(const IComponentItemChangeEvent& statusChangedEvent) = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IComponentItem, "HydroCouple::Data::IComponentItem/1.0")

#endif // ICOMPONENTITEM_H
