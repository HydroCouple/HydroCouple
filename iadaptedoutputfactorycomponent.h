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
#ifndef IADAPTEDOUTPUTFACTORYCOMPONENT_H
#define IADAPTEDOUTPUTFACTORYCOMPONENT_H

#include "iadaptedoutputfactory.h"
#include "iadaptedoutputfactorycomponentinfo.h"

namespace HydroCouple
{
	namespace Data
	{
		class IAdaptedOutputFactoryComponent : public virtual IAdaptedOutputFactory
		{

		public:
			//!Contains the metadata about the IModelComponent
			/*!
			This information includes the developer, component version number, contact url etc.
			*/
			virtual IAdaptedOutputFactoryComponentInfo* componentInfo() const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IAdaptedOutputFactoryComponent, "HydroCouple::IAdaptedOutputFactoryComponent/1.0")

#endif // IADAPTEDOUTPUTFACTORYCOMPONENT_H
