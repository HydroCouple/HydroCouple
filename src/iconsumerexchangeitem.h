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
#ifndef ICONSUMEREXCHANGEITEM_H
#define ICONSUMEREXCHANGEITEM_H

#include "icomponentexchangeitem.h"

namespace HydroCouple
{
	namespace Data
	{
		class IProducerExchangeItem;

		//!An input item that can accept values for an IHModelComponent.
		class IConsumerExchangeItem : public virtual IComponentExchangeItem
		{
			Q_PROPERTY(IHProducerItem* Producer READ getProducer WRITE setProducer)

		public:
			virtual ~IConsumerExchangeItem(){}

			//! Gets the producer this consumer should get its values from.
			virtual IProducerExchangeItem* getProducer() const = 0;

			//! Sets the producer this consumer should get its values from.
			virtual void setProducer(IProducerExchangeItem* const producer) = 0;

			//! Returns true if this IHConsumerItem can consume this producer.
			virtual bool canConsume(IProducerExchangeItem* const producer) const = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IConsumerExchangeItem, "HydroCouple::Data::IConsumerExchangeItem/1.0")

#endif // ICONSUMEREXCHANGEITEM_H
