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
#ifndef IADAPTEDPRODUCEREXCHANGEITEMFACTORY_H
#define IADAPTEDPRODUCEREXCHANGEITEMFACTORY_H

#include "iidentity.h"
#include "iadaptedproducerexchangeitem.h"


using namespace HydroCouple::Data;

namespace HydroCouple
{
		//! IAdaptedProducerExchangeItemFactory is used to create instances of IAdaptedProducerExchangeItems .
        /*!
        This class can be internal to an IModelComponent by calling IModel::adaptedProducterExchangeItemFactories() or can be generated
        from an HydroCouple::IAdaptedProducerExchangeItemFactoryComponent;
        */
	    class IAdaptedProducerExchangeItemFactory : public virtual IIdentity
		{

		public:
			virtual ~IAdaptedProducerExchangeItemFactory(){}

            //! Get a list of IIdentity objects representing the list
            //! of the available IAdaptedProducerExchangeItem that can make the <param name="producer"/> match the <param name="consumer"/>.
			/*!
            If the <paramref name="consumer"/>is<code>null</code>, the identifiers of all IAdaptedProducerExchangeItem s that can adapt the <paramref name="producer"/> are returned.
            \param IHProducerItem to adapt
			<param name="consumer"> IHConsumerItem to adapt the producer to, can be <code>null</code>.</param>
			<returns>List of identifiers for the available IHAdaptedProducerItems.</returns>
			*/
			virtual QList<IIdentity*> getAvailableAdaptedOutputIds(const IProducerExchangeItem * const producer, const IConsumerExchangeItem * const consumer) = 0;

			//! Creates a IHAdaptedProducerItem that adapts the <paramref name="producer"/> so that it fits the consumer.
			/*!
			<para>
			 The adaptedProducerId used must be one of the IHIdentifier instances returned by the createAdaptedProducerItem method.
			 </para>
			 <para>
			 The returned IHAdaptedProducerItems will already be registered with the <paramref name="producer"/>.
			 </para>
			 <param name="adaptedProducerId">The identifier of the IHAdaptedProducerItem to create.</param>
			 <param name="producer">IHProducerItem to adapt.</param>
			 <param name="consumer">IHConsumerItem to adapt the adaptee to, can be <code>null</code>.</param>
			 <returns>IHAdaptedProducerItem</returns>
			*/
			virtual IAdaptedProducerExchangeItem* createAdaptedProducerItem(IIdentity* const adaptedProducerId, IProducerExchangeItem* const producer, IConsumerExchangeItem* const consumer)  = 0;
		};
}

using namespace HydroCouple;
Q_DECLARE_INTERFACE(IAdaptedProducerExchangeItemFactory, "HydroCouple::IAdaptedProducerExchangeItemFactory/1.0")

#endif // IADAPTEDPRODUCEREXCHANGEITEMFACTORY_H
