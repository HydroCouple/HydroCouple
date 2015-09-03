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
#ifndef ICOMPONENTITEMCHANGEEVENT_H
#define ICOMPONENTITEMCHANGEEVENT_H

#include <QString>

namespace HydroCouple
{
	namespace Data
	{
		class IComponentItem;
		//!The IComponentItemChangeEvent contains the information that will be passed when the IHComponentItem fires the componentItemChanged signal.
		/*!
		 <para>
		 Sending exchange item events is optional, so it should not be used as a
		 mechanism to build critical functionality upon.
		 </para>
		*/
		class IComponentItemChangeEvent
		{
			//Q_PROPERTY(IComponentItem* ComponentItem READ componentItem);
			//Q_PROPERTY(QString Message READ getMessage WRITE setMessage);

		public:
			virtual ~IComponentItemChangeEvent(){}

			//!IHComponentItem which fired the signal
			virtual IComponentItem* componentItem() const = 0;

			//!Gets message associated with the event
			virtual QString getMessage() const = 0;

			//!Sets message associated with the event
			virtual void setMessage(const QString & message) = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IComponentItemChangeEvent, "HydroCouple::Data::IComponentItemChangeEvent/1.0")

#endif // ICOMPONENTITEMCHANGEEVENT_H
