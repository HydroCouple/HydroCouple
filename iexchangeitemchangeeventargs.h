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
#ifndef IEXCHANGEITEMCHANGEEVENTARGS_H
#define IEXCHANGEITEMCHANGEEVENTARGS_H

#include <QString>

namespace HydroCouple
{
	namespace Data
	{
		class IExchangeItem;
		//!The IExchangeItemChangeEventArgs contains the information that will be passed when the IComponentItem fires the componentItemChanged signal.
		/*!
		 <para>
		 Sending exchange item events is optional, so it should not be used as a mechanism to build critical functionality upon.
		 </para>
		*/
		class IExchangeItemChangeEventArgs
		{
		public:
			//!IExchangeItem which fired the signal
			virtual IExchangeItem* exchangeItem() const = 0;

			//!Gets message associated with the event
			virtual QString message() const = 0;

		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IExchangeItemChangeEventArgs, "HydroCouple::Data::IExchangeItemChangeEventArgs/1.0")

#endif // IEXCHANGEITEMCHANGEEVENTARGS_H
