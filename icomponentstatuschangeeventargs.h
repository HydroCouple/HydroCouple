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

#ifndef ICOMPONENTSTATUSCHANGEEVENTARGS_H
#define ICOMPONENTSTATUSCHANGEEVENTARGS_H

#include "hydrocouple.h"
#include "imodelcomponent.h"


namespace HydroCouple
{

		//!The IComponentStatusChangeEventArgs contains the information that will be passed when the IModelComponent fires the IModelComponent::statusChanged signal.
		/*!
		 <para>
		 Sending exchange item events is optional, so it should not be used as a mechanism to build critical functionality upon.
		 </para>
		*/
		class IComponentStatusChangeEventArgs
		{ 
			//!Properties must be implemented in derived classes
			//Q_PROPERTY(IModelComponent* Component READ component)
			//Q_PROPERTY(IModelComponent::ComponentStatus OldStatus READ getOldStatus WRITE setOldStatus)
			//Q_PROPERTY(IModelComponent::ComponentStatus NewStatus READ getNewStatus WRITE setNewStatus)
			//Q_PROPERTY(IModelComponent::ComponentStatus Message READ getMessage WRITE setMessage)
			
		public:
			//!Gets the IModelComponent that fired the event
			virtual IModelComponent* modelComponent() const = 0;

			//!Gets the IModelComponent's status before the status change.
            virtual ComponentStatus oldStatus() const = 0;

			//!Gets the IModelComponent's status after the status change.
            virtual ComponentStatus newStatus() const = 0;

			//!Gets additional information about the status change.
			virtual QString message() const = 0;

			/*!Return true if status has a percent progress included in its definition/ otherwise progress bar shows busy
			*/
			virtual bool hasProgressMonitor() const = 0;

			//!Number between 0 and 100 indicating the progress made by a component in its simulation.
			/*!
			*/
			virtual int percentProgress() const = 0;
		};
}
Q_DECLARE_INTERFACE(HydroCouple::IComponentStatusChangeEventArgs, "HydroCouple::IComponentStatusChangeEventArgs/1.0")

#endif // ICOMPONENTSTATUSCHANGEEVENTARGS_H
