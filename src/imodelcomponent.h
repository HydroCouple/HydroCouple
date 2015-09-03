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

#ifndef IMODELCOMPONENT_H
#define IMODELCOMPONENT_H

#include "hydrocouple.h"
#include "imodelcomponentinfo.h"
#include "iargument.h"
//#include "iadaptedproducerexchangeitemfactory.h"

namespace HydroCouple
{


	class IComponentStatusChangeEvent;
	class IConsumerExchangeItem;
	class IProducerExchangeItem;
	class IAdaptedProducerExchangeItemFactory;

	//!IModelComponent interface class is the core interface in the HydroCouple standard defining a component
	/*
	*/
	class IModelComponent : public virtual IIdentity
	{

        //Properties must be implemented in derived classes
		//Q_PROPERTY(const IModelComponentInfo* ComponentInfo READ componentInfo);
		//Q_PROPERTY(QList<IHArgument*> Arguments READ arguments);
		//Q_PROPERTY(ComponentStatus Status READ status);
		//Q_PROPERTY(QList<IHConsumerItem*> Consumers READ consumers);
		//Q_PROPERTY(QList<IHProducerItem*> Producers READ producers);
		//Q_PROPERTY(QList<IHAdaptedProducerItemFactory*> AdaptedProducterItemFactories READ adaptedProducterItemFactories);
		//Q_PROPERTY(IModelComponent* Parent READ parent);
		//Q_PROPERTY(bool HasProgressMonitor READ hasProgressMonitor);
		//Q_PROPERTY(int PercentProgress READ percentProgress);

	public:

		virtual ~IModelComponent(){}

		//!Contains the metadata about the IModelComponent
		/*!
		This information includes the developer, component version number, contact url etc.
		*/
		virtual const IModelComponentInfo* componentInfo() const = 0;

		//!Arguments needed to let the component do its work. An unmodifiable list of (modifiable) arguments must be returned that is to be used to get information about the arguments and to set argument values. 
		/*!

		<para>
		Validation of changes can be done either when they occur (e.g. using notifications) or
		when the initialize method is called. Initialize will always be called before any call to the update method of the IModelComponent.
		</para>

		<para>
		This property must be available as soon is the IModelComponent instance is created.
		Arguments describes the arguments that can be set before the initialize() method is called.
		</para>

		*/
		virtual QList<IArgument*> arguments() const = 0;

		//!Adds an argument to the argument list of this component and validates.Returns true if successful. Takes ownership of the pointer
		/*!
		\param setArgument
		*/
		virtual bool setArgument(IArgument * const argument) = 0;

		//!Defines current status of the IModelComponent. See HydroCouple::Componentstatus for the possible values
		/*!
		<para>
		The first status that a component sets is Created,
		as soon after it has been created. In this status, Arguments is the
		only property that may be accessed.
		</para>
		*/
        virtual ComponentStatus status() const = 0;

		//! The list of consumer items for which a component can recieve values.
		/*!
		 <para>This property must be accessible after the initialize() method has been
		 invoked and until the validate() method has been invoked. If this property
		 is accessed before the initialize() method has been invoked or after the
		 validate() method has been invoked and the IModelComponent cannot handle
		 this an exception must be thrown.</para>
		 <para>This method basically returns references to IHConsumerItems items.
		 There is no guarantee that the list of objects is not altered by other components
		 after it has been returned. It is the responsibility of the IModelComponent
		 to make sure that such possible alterations do not subsequently corrupt
		 the IModelComponent.
		 </para>
		 */
		virtual QList<IConsumerExchangeItem*> consumers() const = 0;

		//! The list of IHProducerItems for which a component can produce results.
		/*!
		 <para>
		 This property must be accessible after the initialize() method has been
		 invoked and until the validate() method has been invoked. If this property
		 is accessed before the initialize() method has been invoked or after the
		 validate() method has been invoked and the IModelComponent cannot handle
		 this an exception must be thrown.
		 </para>

		 <para>
		 The list only contains the core IHProducerItems of the IModelComponent, not
		 the IHBaseAdaptedProducerItem derived from each IHProducerItem (etc.). To get a complete
		 list of producers traverse the chain of IHBaseAdaptedProducerItem that start with the
		 IHProducerItem returned in the list.
		 </para>

		 <para>
		 This method basically returns references to IHProducerItems.
		 There is no guarantee that the list of objects is not altered by other components
		 after it has been returned. It is the responsibility of the IModelComponents
		 to make sure that such possible alterations do not subsequently corrupt
		 the IModelComponents.
		 </para>
		 */
		virtual QList<IProducerExchangeItem*> producers() const = 0;


		///<summary>
		/// Get a list of <see cref="IAdaptedOutputFactory"/>, each allowing to 
		/// create <see cref="IBaseAdaptedOutput"/> item for making outputs fit to inputs
		/// in case they do not already do so.
		/// <para>
		/// </para>
		/// Factories can be added to and removed from the list so that third-party
		/// factories and IBaseAdaptedOutput classes can be introduced.
		///</summary>
		virtual QList<IAdaptedProducerExchangeItemFactory*> adaptedProducterExchangeItemFactories() const = 0;


		//!Initializes the IModelComponent
		/*!
		<para>The initialize() method must be invoked before any other
		method or property in the IModelComponent interface is invoked or accessed, except
		for the Arguments property.</para>

		<para>Immediately after the method is invoked, it changes the IModelComponent's status to Initializing.</para>

		<para>When the method is executed and an error occurs, the status of the component will change to Failed, and an exception will	be thrown. If the component initializes succesfully, the  status is changed to Initialized.</para>

		<para>When the initialize() method has been finished and the status	is Initialized, the properties Id,	Caption, Description, Inputs, Outputs, have been set,
		and the method Validate can be called.</para>

		<para>It is only required that the method initialize() can be invoked once.
		If the initialize() method is invoked more than once and the IModelComponent
		cannot handle this; an exception must be thrown.</para>

		<para>REMARKS:</para>
		<para>The method will typically populate the component based on the
		values specified in its arguments, which can be retrieved with
		arguments. Settings can be used to read input files, allocate memory,
		and organize input and output exchange items.</para>
		*/
		virtual void initialize() = 0;

		//!Validates the populated instance of the IModelComponent
		/*!
		<para>
		This method must be accessible after the initialize() method has been
		invoked and until the Finish method has been invoked. If this property
		is accessed before the initialize() method has been invoked or after the
		Finish method has been invoked and the IModelComponent cannot handle
		this an exception must be thrown.
		</para>

		<para>
		The method will and must be invoked after the various provider/consumer relations between
		this component's exchange items and the exchange items of other components present
		in the composition.
		</para>

		<para>
		Immediatly after the method is invoked, it changes the IModelComponent's status to Validating.
		</para>

		<para>
		When the Validate method has finished, the status of the IModelComponent has changed to either Valid or Invalid.
		</para>

		<returns>
		Returns null or an array of strings of length null if there are no messages
		at all. If there are messages while the components status is Valid, the messages are purely informative.
		If there are messages while the components status is HydroCouple::Componentstatus.Invalid, at least one of the messages
		indicates a fatal error.
		</returns>
		*/
		virtual QList<QString> validate() const = 0;



		//!Prepares the IModelComponent for calls to the Update method
		/*!
		<para>Before prepare() is called, the IModelComponent are not required to honor
		any type of action that retrieves values from the IModelComponent. After Prepare
		is called, the IModelComponent must be ready for providing values.
		</para>

		<para>This method must be accessible after the initialize() method has been
		invoked and until the finish() method has been invoked. If this property
		is accessed before the initialize() method has been invoked or after the
		finish() method has been invoked and the IModelComponent cannot handle
		this an exception must be thrown.</para>

		<para>Immediatly after the method is invoked, it changes the IModelComponent's status to Preparing.</para>

		<para>When the method has finished, the status of the IModelComponent has changed
		to either Updated or Failed.</para>

		<para>It is only required that the prepare() method can be invoked once. If the
		prepare() method is invoked more that once and the LinkableComponent cannot handle
		this an exception must be thrown.</para>
		*/
		virtual void prepare() = 0;

		//! This method is called to let the component update itself, thus reaching its next state.
		/*!
		 <para>
		 Immediately after the method is invoked, it changes the component's status() to Updating.
		 </para>

		 <para>
		 The type of actions a component takes during the <see cref="Update"/> method depends
		 on the type of component. A numerical model that progresses in time will typically
		 compute a time step. A database whould typically look at the consumers of its output items,
		 and perform one or more queries to be able to provide the values that the consumers require.
		 A GIS system would typically re-evaluate the values in a grid coverage, so that its output
		 output items can provide up-to-date values.
		 </para>

		 <para>If the Update method is performed succesfully, the component sets its state to
		 <see cref="LinkableComponentStatus.Updated"/>, unless after this Update action the
		 component is at the end of its computation, in which case it will be set its State
		 to <see cref="LinkableComponentStatus.Done"/>.</para>

		 <para>If during the Update method a problem arises, the component sets its state to
		 <see cref="LinkableComponentStatus.Failed"/>, and throws an exception.</para>

		 </summary>
		 <param name="requiredProducers">
		 This optional parameter lets the caller specify the specific
		 producer items that should be updated. If the length is 0, the component
		 will at least update its producer items that have consumers, or all its output items,
		 depending on the component's implementation.
		 </param>
		 */
		virtual void update(const QList<IProducerExchangeItem*> & requiredProducers = QList<IProducerExchangeItem*>()) = 0;

		//!The finish() must be invoked as the last of any methods in the IModelComponent interface
		/*!
		<para>
		This method must be accessible after the prepare() method has been invoked.
		If this method is invoked before the prepare() method has been invoked an exception must be thrown by the IModelComponent.
		</para>

		<para>
		Immediately after the method is invoked, it changes the IModelComponent's status() to ComponentStatus::Finishing.
		Once the finishing is completed, the component changes its status() to ComponentStatus::Finished if it can not be restarted,
		or ComponentStatus::Created if it can.
		</para>
		*/
		virtual void finish() = 0;

		//!Parent IModelComponent from which current component was cloned from.
		/*!
		*/
		virtual IModelComponent* parent() const = 0;


		//!Deep clones itself including cloning its iharguments
		/*!
		*/
		virtual IModelComponent* clone() = 0;

		//!Components cloned from this IModelComponent instance
		/*!
		*/
		virtual QList<IModelComponent*> childModelComponents() const = 0;

		//!True is IModelComponent has a progress monitor
		/*!
		*/
		virtual bool hasProgressMonitor() const = 0;

		//!Number between 0 and 100 indicating the progress made by a component in its simulation.
		/*!
		*/
		virtual int percentProgress() const = 0;

		//! The componentStatusChanged() function must be emitted when Status of the component changes.
		/*! See ComponentStatus for the possible states
		*/
	signals:
		virtual void componentStatusChanged(const IComponentStatusChangeEvent& statusChangedEvent) = 0;
	};
}

using namespace HydroCouple;
Q_DECLARE_INTERFACE(IModelComponent, "HydroCouple::IModelComponent/1.0")
#endif // IMODELCOMPONENT_H
