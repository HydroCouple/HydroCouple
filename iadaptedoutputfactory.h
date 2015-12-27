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
#ifndef IADAPTEDOUTPUTFACTORY_H
#define IADAPTEDOUTPUTFACTORY_H

#include "iidentity.h"
#include "iadaptedoutput.h"

namespace HydroCouple
{
	namespace Data
	{
		//! IAdaptedOutputFactory is used to create instances of IAdaptedProducerExchangeItems .
		/*!
		This class can be internal to an IModelComponent by calling IModel::adaptedProducterExchangeItemFactories() or can be generated
		from an HydroCouple::IAdaptedOutputFactoryComponent;
		*/
		class IAdaptedOutputFactory : public virtual IIdentity
		{

		public:
			//! Get a list of IIdentity objects representing the list
			//! of the available IAdaptedOutput that can make the <param name="producer"/> match the <param name="consumer"/>.
			/*!
			If the <paramref name="consumer"/>is<code>null</code>, the identifiers of all IAdaptedOutput s that can adapt the <paramref name="producer"/> are returned.
			\param IOutput to adapt
			<param name="consumer"> IInput to adapt the producer to, can be <code>null</code>.</param>
			<returns>List of identifiers for the available IAdaptedOutputs.</returns>
			*/
			virtual QList<IIdentity*> getAvailableAdaptedOutputIds(const IOutput* provider, const IInput* consumer) = 0;

			//! Creates a IAdaptedOutput that adapts the <paramref name="producer"/> so that it fits the consumer.
			/*!
			<para>
			The adaptedProducerId used must be one of the IHIdentifier instances returned by the createAdaptedProducerItem method.
			</para>
			<para>
			The returned IAdaptedOutputs will already be registered with the <paramref name="producer"/>.
			</para>
			<param name="adaptedProducerId">The identifier of the IAdaptedOutput to create.</param>
			<param name="producer">IOutput to adapt.</param>
			<param name="consumer">IInput to adapt the adaptee to, can be <code>null</code>.</param>
			<returns>IAdaptedOutput</returns>
			*/
			virtual IAdaptedOutput* createAdaptedOutput(IIdentity* adaptedProviderId, const IOutput* provider,const IInput* consumer) = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IAdaptedOutputFactory, "HydroCouple::Data::IAdaptedOutputFactory/1.0")

#endif // IADAPTEDOUTPUTFACTORY_H
