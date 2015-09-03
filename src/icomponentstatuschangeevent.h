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

#ifndef ICOMPONENTSTATUSCHANGEEVENT_H
#define ICOMPONENTSTATUSCHANGEEVENT_H

#include "hydrocouple.h"
#include "imodelcomponent.h"


namespace HydroCouple
{

		//!The IComponentStatusChangeEvent contains the information that will be passed when the IHModelComponent fires the IHModelComponent::statusChanged signal.
		/*!
		 <para>
		 Sending exchange item events is optional, so it should not be used as a mechanism to build critical functionality upon.
		 </para>
		*/
		class IComponentStatusChangeEvent
		{ 
			//!Properties must be implemented in derived classes
			//Q_PROPERTY(IHModelComponent* Component READ component)
			//Q_PROPERTY(IHModelComponent::ComponentStatus OldStatus READ getOldStatus WRITE setOldStatus)
			//Q_PROPERTY(IHModelComponent::ComponentStatus NewStatus READ getNewStatus WRITE setNewStatus)
			//Q_PROPERTY(IHModelComponent::ComponentStatus Message READ getMessage WRITE setMessage)
			
		public:
			virtual ~IComponentStatusChangeEvent(){}

			//!Gets the IHModelComponent that fired the event
			virtual IModelComponent* modelComponent() const = 0;

			//!Gets the IHModelComponent's status before the status change.
            virtual ComponentStatus oldStatus() const = 0;

			//!Gets the IHModelComponent's status after the status change.
            virtual ComponentStatus newStatus() const = 0;

			//!Gets additional information about the status change.
			virtual QString message() const = 0;

		};
}

using namespace HydroCouple;
Q_DECLARE_INTERFACE(IComponentStatusChangeEvent, "HydroCouple::IComponentStatusChangeEvent/1.0")

#endif // ICOMPONENTSTATUSCHANGEEVENT_H
