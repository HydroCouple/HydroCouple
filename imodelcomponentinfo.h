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

#ifndef IMODELCOMPONENTINFO_H
#define IMODELCOMPONENTINFO_H

#include <icomponentinfo.h>

namespace HydroCouple
{
	class IModelComponent;
	//!IModelComponentInfo interface class provides detailed metadata about an IModelComponent
	/*!
	The IModelComponentInfo interface is used to provide metadata on a component and create new instances of a component
	*/
	class IModelComponentInfo : public virtual IComponentInfo
	{

	public:
		//!Creates new IModelComponent instance
		/*!
		*/
		virtual IModelComponent* createComponentInstance() = 0;
	};
}
Q_DECLARE_INTERFACE(HydroCouple::IModelComponentInfo, "HydroCouple::IModelComponentInfo/1.0")

#endif // IMODELCOMPONENTINFO_H
