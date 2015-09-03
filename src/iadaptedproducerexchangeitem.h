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
#ifndef IADAPTEDPRODUCEREXCHANGEITEM_H
#define IADAPTEDPRODUCEREXCHANGEITEM_H

#include "iargument.h"
#include "iproducerexchangeitem.h"

namespace HydroCouple
{
	namespace Data
	{
		class IAdaptedProducerExchangeItemFactory;


        //! An IAdaptedProducerExchangeItem adds one or more data operations on top of an IProducerExhangeItem.

        /*!

         It is in itself an IProducerExchangeItem. The IAdaptedProducerExchangeItem extends an output
		 item with functionality as spatial interpolation, temporal interpolation, unit conversion etc.

		 <para>
         IAdaptedProducerExchangeItem instances are created by means of an IAdaptedProducerExchangeItemFactory.
		 </para>
		 
		 <para>
         The IAdaptedProducerExchangeItem is based on the adaptor design pattern. It adapts
         an IProducerExchangeItem or another IAdaptedProducerExchangeItem to make it
		 suitable for new use or purpose. The object being adapted is typically called the 
		 adaptee. The IAdaptedProducerExchangeItem replaces the DataOperation that was
         used in OpenMI Standard version 1.x.
		 </para>
		
		*/
		class IAdaptedProducerExchangeItem : public virtual IProducerExchangeItem
		{

		public:
			virtual ~IAdaptedProducerExchangeItem(){}


            //! IAdaptedProducerExchangeItemFactory that generated this IAdaptedProducerExchangeItem.
            /*!
            <returns>IAdaptedProducerExchangeItemFactory parent</returns>
            */
            virtual const IAdaptedProducerExchangeItemFactory* adaptedProducerExchangeItemFactory() const = 0;

            //! IArgument represents input parameters needed for this IAdaptedProducerExchangeItem.
			
			/*!
            An unmodifiable list of the (modifiable) IArguments should be returned that can be used to
            get information on an IArgument and to modify its values. Validation of changes is done when they occur (e.g. using notifications).

            <returns>Unmodifiable list of IArgument for the adapted output</returns>
			*/
			virtual QList<IArgument*> arguments() const = 0;

            //!Sets an IArgument for the argument list of this IAdaptedProducerExchangeItem.Returns true if successful. Takes ownership of the pointer
            /*!
            \param setArgument
            */
            virtual bool setArgument(IArgument * const argument) = 0;


			//! Let the adapted output initialize itself, based on the current values specified by the arguments. 
			/*!
			Only after initialize is called the refresh method might be called.
			<para>
			A component must invoke the initialize() method of all its
			adapted outputs at the end of the component's Prepare phase.
			In case of stacked adapted outputs, the adaptee must be initialized first.
			</para>
			*/
			virtual void initialize() = 0;


            //! IProducerExchangeItem that this IAdaptedProducerExchangeItem extracts content from. In the adapter design pattern, it is the item being adapted.
            /*!

            \returns an IProducerExchangeItem that is being modified by this IAdaptedProducerExchangeItem
            */
			virtual IProducerExchangeItem* adaptee() const = 0;


            //! Request the IAdaptedProducerExchangeItem to refresh itself and perform any necessary calculations.
            /*!
            This method will be called by the adaptee(), when it has been refreshed/updated. In the
            implementation of the refresh method the adapted output should update its contents according to the changes in the adaptee.

             <para>
             After updating itself the IAdaptedProducerExchangeItem must call refresh on all
             its IAdaptedProducerExchangeItem children, so the chain of IProducerExchangeItem 's refreshes themselves.
            </para>
            */
			virtual void refresh() = 0;
		};
	}
}

using namespace HydroCouple::Data;
Q_DECLARE_INTERFACE(IAdaptedProducerExchangeItem, "HydroCouple::Data::IAdaptedProducerExchangeItem/1.0")

#endif // IADAPTEDPRODUCEREXCHANGEITEM_H
