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
#ifndef IHADAPTEDPRODUCEREXCHANGEITEMFACTORYCOMPONENT_H
#define IHADAPTEDPRODUCEREXCHANGEITEMFACTORYCOMPONENT_H

#include "iadaptedproducerexchangeitemfactory.h"
#include "iadaptedproducerexchangeitemfactorycomponentinfo.h"

namespace HydroCouple
{
	class IAdaptedProducerExchangeItemFactoryComponent : public virtual IAdaptedProducerExchangeItemFactory
	{
		//Q_PROPERTY(IAdaptedProducerItemFactoryComponentInfo* ComponentInfo READ componentInfo);

	public:
		virtual ~IAdaptedProducerExchangeItemFactoryComponent(){}

		//!Contains the metadata about the IHModelComponent
		/*!
		This information includes the developer, component version number, contact url etc.
		*/
		virtual IAdaptedProducerExchangeItemFactoryComponentInfo* componentInfo() const = 0;
	};
}

using namespace HydroCouple;
Q_DECLARE_INTERFACE(IAdaptedProducerExchangeItemFactoryComponent, "HydroCouple::IAdaptedProducerExchangeItemFactoryComponent/1.0")

#endif // IHADAPTEDPRODUCEREXCHANGEITEMFACTORYCOMPONENT_H
