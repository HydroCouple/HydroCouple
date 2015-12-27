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

#ifndef IOUTPUT_H
#define IOUTPUT_H

#include "iinput.h"

namespace HydroCouple
{
	namespace Data
	{
		class IAdaptedOutput;
		/// <summary>
		/// An output exchange item that can deliver values from an IModelComponent.
		/// 
		/*!
		<para>
		If an output does not provide the data in the way a consumer would like to have it the output can be adapted by an IAdaptedOutput,
		which can transform the data according to the consumer's wishes. E.g. by performing interpolation in time, spatial aggregation, etc.).
		</para>
		*/
		class IOutput : public virtual IExchangeItem
		{
		public:
			/// <summary>
			/// Input items that will consume the values, by calling the GetValues() method. Every input item
			/// that will call this method, needs to call the AddConsumer method first. If the input item is not
			/// interested any longer in calling the GetValues, it should remove itself by calling the RemoveConsumer method.
			/// <para>
			/// The list is readonly. Add and remove from the list by using <see cref="AddConsumer"/>
			/// and <see cref="RemoveConsumer"/>.
			/// </para>
			/// </summary>
			/// <remarks>
			/// Please be aware that the "unadulterated" values in the output item, provided by the read only Values property,
			/// may be called anyway, even if there are no values available.
			/// </remarks>
			virtual QList<IInput*> consumers() const = 0;


			///<summary>
			/// Add a consumer to this output item. Every input item that wants to call the GetValues() method,
			/// needs to add itself as a consumer first.
			/// <para>If a consumer is added that can not be handled, or that is incompatible with the already
			/// added consumers, an exception will be thrown.</para>
			/// <para>The AddConsumer method must and will automatically set the consumer's
			/// Provider (see <see cref="IBaseInput.Provider"/>)</para>
			/// <param name="consumer">consumer that has to be added</param>
			///</summary>
			virtual void addConsumer(IInput* consumer) = 0;

			///<summary>
			/// Remove a consumer. If an input item is not interested any longer in calling the GetValues method,
			/// it should remove itself by calling RemoveConsumer.
			/// <param name="consumer">consumer that has to be removed</param>
			///</summary>
			virtual bool removeConsumer(IInput* consumer) = 0;

			///<summary>
			/// The adaptedOutputs that have this current output item as <code>Adaptee</code>.
			/// As soon as the output item's values have been updated, for each adaptedOutput its 
			/// <see cref="IBaseAdaptedOutput.Refresh"/> method must be called.
			/// <para>
			/// The list is readonly. Add and remov from the list by using <see cref="AddAdaptedOutput"/>
			/// and <see cref="RemoveAdaptedOutput"/>.
			/// </para>
			///</summary>
			virtual QList<IAdaptedOutput*> adaptedOutputs() const = 0;

			///<summary>
			/// Add a <see cref="IBaseAdaptedOutput"/> to this output item. 
			/// Every adaptedOutput that uses data from this output item,
			/// needs to add itself as a consumer first.
			/// <para>If a adaptedOutput is added that can not be handled, or that is 
			/// incompatible with the already added adaptedOutputs, an exception will be thrown.</para>
			/// <param name="adaptedOutput">consumer that has to be added</param>
			///</summary>
			virtual void addAdaptedOutput(IAdaptedOutput* adaptedOutput) = 0;

			///<summary>
			/// Remove a <see cref="IBaseAdaptedOutput"/>. If a adaptedOutput is not interested 
			/// any longer in this output item data,
			/// it should remove itself by calling RemoveConsumer.
			/// <param name="adaptedOutput">consumer that has to be removed</param>
			///</summary>
			virtual bool removeAdaptedOutput(IAdaptedOutput* adaptedOutput) = 0;

			///<summary>
			/// Provides the values matching the value definition specified by the
			/// <param name="querySpecifier">. Extensions can overwrite this base version to include
			/// more details in the query, e.g. time and space.
			///</summary>
			/// <remarks>
			/// One might expect to be the querySpecifier to be of the type IInput, because every input item that calls
			/// the GetValues method needs to add itself as a consumer first.
			/// <para>However, the <see cref="IBaseExchangeItem"/> suffices to  specify what is required. Therefore,
			/// to have the flexibility to loosen the "always register as consumer" approach, it is chosen to provide
			/// an <see cref="IBaseExchangeItem"/> as argument.</para>
			/// </remarks>
			virtual void updateValues(IInput* querySpecifier) = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IOutput, "HydroCouple::Data::IOutput/1.0");

#endif // IOUTPUT_H
