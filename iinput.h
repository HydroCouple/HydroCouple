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

#ifndef IINPUT_H
#define IINPUT_H

#include "iexchangeitem.h"

namespace HydroCouple
{
	namespace Data
	{
		class IOutput;

		//!An IInput item that can accept values for an IModelComponent.
		class IInput : public virtual IExchangeItem
		{

		public:
			//! Gets the producer this consumer should get its values from.
			virtual IOutput* getProvider() const = 0;

			//! Sets the producer this consumer should get its values from.
			virtual void setProvider(IOutput* provider) = 0;

			//! Returns true if this IInput can consume this producer.
			virtual bool canConsume(const IOutput* provider) const = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IInput, "HydroCouple::Data::IInput/1.0");

#endif // IINPUT_H
