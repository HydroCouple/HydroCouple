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

#ifndef IEXCHANGEITEM_H
#define IEXCHANGEITEM_H

#include "icomponentitem.h"
//#include "ivariable.h"
#include "iexchangeitemchangeeventargs.h"

namespace HydroCouple
{
	namespace Data
	{
		class IExchangeItem : public virtual IComponentItem
		{

		public:
			//! IVariable associated with this IExchangeItem.
			//virtual IVariable* variable() const = 0;

			//! The componentItemChanged event is fired when the content of an IComponentItem has changed.
			/*!
			\param statusChangedEvent provides information about the status change
			*/
			virtual void itemChanged(const IExchangeItemChangeEventArgs& statusChangedEvent) = 0;
		};

	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IExchangeItem, "HydroCouple::Data::IExchangeItem/1.0")

#endif // IEXCHANGEITEM_H
