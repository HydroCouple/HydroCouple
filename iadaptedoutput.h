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
#ifndef IADAPTEDOUTPUT_H
#define IADAPTEDOUTPUT_H

#include "iargument.h"
#include "ioutput.h"

namespace HydroCouple
{
	namespace Data
	{
		class IAdaptedOutputFactory;


        //! An IAdaptedOutput adds one or more data operations on top of an IOutput.

        /*!

         It is in itself an IOutput. The IAdaptedOutput extends an output
		 item with functionality as spatial interpolation, temporal interpolation, unit conversion etc.

		 <para>
         IAdaptedOutput instances are created by means of an IAdaptedOutputFactory.
		 </para>
		 
		 <para>
         The IAdaptedOutput is based on the adaptor design pattern. It adapts
         an IOutput or another IAdaptedOutput to make it
		 suitable for new use or purpose. The object being adapted is typically called the 
		 adaptee. The IAdaptedOutput replaces the DataOperation that was
         used in OpenMI Standard version 1.x.
		 </para>
		
		*/
		class IAdaptedOutput : public virtual IOutput
		{

		public:
            //! IAdaptedOutputFactory that generated this IAdaptedOutput.
            /*!
            <returns>IAdaptedOutputFactory parent</returns>
            */
            virtual const IAdaptedOutputFactory* adaptedOutputFactory() const = 0;

            //! IArgument represents input parameters needed for this IAdaptedOutput.
			
			/*!
            An unmodifiable list of the (modifiable) IArguments should be returned that can be used to
            get information on an IArgument and to modify its values. Validation of changes is done when they occur (e.g. using notifications).

            <returns>Unmodifiable list of IArgument for the adapted output</returns>
			*/
			virtual QList<IArgument*> arguments() const = 0;

            //!Sets an IArgument for the argument list of this IAdaptedOutput.Returns true if successful. Takes ownership of the pointer
            /*!
            \param setArgument
            */
            virtual bool setArgument(IArgument * argument) = 0;


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


            //! IOutput that this IAdaptedOutput extracts content from. In the adapter design pattern, it is the item being adapted.
            /*!

            \returns an IOutput that is being modified by this IAdaptedOutput
            */
			virtual IOutput* adaptee() const = 0;


            //! Request the IAdaptedOutput to refresh itself and perform any necessary calculations.
            /*!
            This method will be called by the adaptee(), when it has been refreshed/updated. In the
            implementation of the refresh method the adapted output should update its contents according to the changes in the adaptee.

             <para>
             After updating itself the IAdaptedOutput must call refresh on all
             its IAdaptedOutput children, so the chain of IOutput 's refreshes themselves.
            </para>
            */
			virtual void refresh() = 0;
		};
	}
}

Q_DECLARE_INTERFACE(HydroCouple::Data::IAdaptedOutput, "HydroCouple::Data::IAdaptedOutput/1.0")

#endif // IADAPTEDOUTPUT_H
