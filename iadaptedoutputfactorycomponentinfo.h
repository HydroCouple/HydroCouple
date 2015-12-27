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

#ifndef IADAPTEDOUTPUTFACTORYCOMPONENTINFO_H
#define IADAPTEDOUTPUTFACTORYCOMPONENTINFO_H

#include "icomponentinfo.h"

namespace HydroCouple
{
	namespace Data
	{
		class IAdaptedOutputFactoryComponent;

		//!IAdaptedOutputFactoryComponentInfo interface class provides information about an IAdaptedOutputFactoryComponent.
		/*!
		IAdaptedOutputFactoryComponentInfo used to provide metadata on a producer item factory component and create instances of it.
		*/
		class IAdaptedOutputFactoryComponentInfo : public virtual IComponentInfo
		{

		public:
			//!New IAdaptedOutputFactoryComponent instance
			/*!
			*/
			virtual IAdaptedOutputFactoryComponent* createComponentInstance() const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IAdaptedOutputFactoryComponentInfo, "HydroCouple::IAdaptedOutputFactoryComponentInfo/1.0")

#endif // IADAPTEDOUTPUTFACTORYCOMPONENTINFO_H
