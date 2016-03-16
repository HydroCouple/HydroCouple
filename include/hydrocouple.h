/*! \file   hydrocouple.h
 *  \author Caleb Amoa Buahin <caleb.buahin@gmail.com>
 *  \version   1.0.0.0
 *  \section   Description
 *  This header files contains the core interface definitions for the
 *  HydroCouple Component-Based modeling framework.
 *  \section License
 *  hydrocouple.h, associated files and libraries are free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  Lesser GNU General Public License as published by the Free Software Foundation;
 *  either version 3 of the License, or (at your option) any later version.
 *  hydrocouple.h its associated files is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 *  \date 2014-2016
 *  \pre
 *  \bug
 *  \warning
 */

/*!
*/
#ifndef HYDROCOUPLE_H
#define HYDROCOUPLE_H

#include <QtCore/qglobal.h>
#include <QVariant>

#ifdef HYDROCOUPLE_LIB
# define HYDROCOUPLE_EXPORT Q_DECL_EXPORT
#else
# define HYDROCOUPLE_EXPORT Q_DECL_IMPORT
#endif


/*!
 * \brief HydroCouple namespace contains the core interface specifications
 * for the HydroCouple component-based modeling framework interface specification.
 */
namespace HydroCouple
{
   //!Forward declarations
   class IModelComponent;
   class IAdaptedOutputFactory;
   class IArgument;
   class IInput;
   class IOutput;
   class IExchangeItem;
   class IAdaptedOutput;
   class IAdaptedOutputFactory;
   class IAdaptedOutputFactoryComponent;
   
   
   /*!
    * \brief HydroCouple::ComponentStatus is an enumerator that describes the status of
    *a component over the course of its lifetime.
    */
   enum ComponentStatus
   {
      /*!
       * \brief The IModelComponent instance has just been created.
       * This status must and will be followed by HydroCouple::Initializing.
       */
      Created,
      
      /*!
       * \brief The IModelComponent is initializing itself.
       * This status will end in a status change to HydroCouple::Initialized or HydroCouple::Failed.
       */
      Initializing,
      
      /*!
       * \brief The IModelComponent has succesfully initialized itself by calliing
       * IModelComponent::initialize(). The connections between its inputs/outputs and those of
       * other components can be established.
       *
       */
      Initialized,
      
      /*!
       * \brief After links between an IModelComponent's inputs/outputs and
       * those of other components have been established,
       * the IModelComponent is HydroCouple::Validating whether
       * its required input will be available when it updates itself,
       * and whether indeed it will be able to provide the required output during this update.
       * This Validating status will when the IModelComponent::status()
       * changes to HydroCouple::Valid or HydroCouple::Invalid.
       */
      Validating,
      
      /*!
       * \brief The IModelComponent is in a HydroCouple::Valid state.
       * When updating itself its required input will be available,
       * and it will be able to provide the required output.
       */
      Valid,
      
      /*!
       * \brief The IModelComponent wants to update itself,
       * but is not yet able to perform the actual computation,
       * because it is still waiting for input data from other components.
       */
      WaitingForData,
      
      /*!
       * \brief The IModelComponent is in an HydroCouple::Invalid state.
       * When updating itself not all required input will be available,
       * and/or it will not be able to provide the required output.
       * After the user has modified the connections
       * between the IModelComponent's inputs/outputs and those of
       * other components, the HydroCouple::Validating state can be entered again
       */
      Invalid,
      
      /*!
       * \brief The IModelComponent is preparing itself for the first HydroCouple::IValueSet::getValue() call.
       * This HydroCouple::Preparing state will end in a status change
       * to HydroCouple::Updated or HydroCouple::Failed.
       *
       */
      Preparing,
      
      /*!
       * \brief The IModelComponent is updating itself. It has received
       * all required input data from other components,
       * and is now performing the actual computation.
       * This HydroCouple::Updating state will end in a status change to
       * HydroCouple::Updated, HydroCouple::Done or HydroCouple::Failed.
       */
      Updating,
      
      /*!
        * \brief The IModelComponent has succesfully updated itself.
        */
      Updated,
      /*!
       * \brief The last update process that the IModelComponent performed was the final one.
       * A next call to the HydroCouple::Update method will leave the IModelComponent's internal state unchanged
       */
      Done,
      
      /*!
       * \brief The IModelComponent was requested to perform the actions to be performed before it will either be
       * disposed or re-intialized again.Typical actions would be writing
       * the final result files, close all open files, free memory, etc.
       * When all required actions have been performed, the status switches to HydroCouple::Created when re-initialization is possible.
       * The status switches to HydroCouple::Finished when the IModelComponent is to be disposed
       */
      Finishing,
      
      /*!
       * \brief The IModelComponent has successfully performed its finalization actions.
       * Re-initialization of the IModelComponent instance is not possible and should not be attempted.
       * Instead the instance should be disposed, e.g. through the garbage collection mechanism
       */
      Finished,
      
      /*!
       * \brief The IModelComponent was requested to perform the actions to be perform before it will either
       * be disposed or re-initialized again. Typical actions would be writing the final result files,
       * close all open files, free memory, etc. When all required actions have been performed,
       * the status switches back to Created if the IModelComponent supports being re-initialized.
       * If it cannot be re-initialized, it can be released from memory */
      Failed,
   };
   
   /*!
    * \brief HydroCouple::DimensionType enumeration.
    */
   enum DimensionType
   {
      
      /*!
       * \brief Dimension length is constant for the entire simulation
       */
      Constant,
      
      /*!
       * \brief Dimension length changes at runtime.
       */
      Dynamic
   };
   
   /*!
    * \brief HydroCouple::FundamentalUnitDimension are the fundamental units that can be combined to form all types of units.
    */
   enum FundamentalUnitDimension
   {
      /*!
       * \brief Fundamental dimension for length.
       */
      Length,//!
      
      /*!
       * \brief Fundamental dimension for mass.
       */
      Mass,
      
      /*!
       * \brief Fundamental dimension for time.
       */
      Time,
      
      /*!
       * \brief Fundamental dimension for electric current.
       */
      ElectricCurrent,
      
      /*!
       * \brief Fundamental dimension for temperature.
       */
      Temperature,
      
      /*!
       * \brief Fundamental dimension for amount of substance.
       */
      AmountOfSubstance,
      
      /*!
       * \brief Fundamental dimension for luminous intensity.
       */
      LuminousIntensity,
      
      /*!
       * \brief Fundamental dimension for currency.
       */
      Currency
   };
   
   /*!
    * \brief IPropertyChanged interface is used to emit signal/event when a property of an object changes.
    */
   class IPropertyChanged
   {
         
      public:
         virtual ~IPropertyChanged() {}
         
         /*!
          * \brief IPropertyChanged::propertyChanged() is called to emit signal/event when property of child class changes.
          *
          * \param propertyName is a string representing the name of the property.
          *
          * \param value is a QVariant representing the value of the property.
          *
          */
      signals:
         virtual void propertyChanged(const QString& propertyName, const QVariant& value) = 0;
         
   };
   
   /*!
    * \brief IDescription interface class provides descriptive information on a HydroCouple object.
    *
    * \details An entity that is describable has a caption (title or heading)
    *  and a description. These are not to be used for identification (see IIdentity).
    *
    */
   class IDescription : public virtual IPropertyChanged
   {
         
      public:
         virtual ~IDescription() {}
         
         /*!
          * \brief Gets caption for the entity.
          *
          * \returns QString representing caption for entity.
          */
         virtual QString getCaption() const = 0;
         
         /*!
          * \brief Sets caption for the entity.
          *
          * \param caption is a QString representing the caption for the entity.
          *
          */
         virtual void setCaption(const QString & caption) = 0;
         
         /*!
          * \brief Gets additional descriptive information for the entity.
          *
          * \returns QString description of entity.
          */
         virtual QString getDescription() const = 0;
         
         /*!
          * \brief Gets additional descriptive information for the entity.
          * \param description is a string for describing an entity.
          */
         virtual void setDescription(const QString& decription) = 0;
         
   };
   
   /*!
    * \brief IIdentity interface class defines a method to get the Id of an HydroCouple entity.
    *
    * \details IIdentity extends the IDescription interface class, and therefore has, next to the id, a caption and a description.
    *
    */
   class IIdentity : public virtual IDescription
   {
      public:
         virtual ~IIdentity() {}
         
         /*!
          * \brief Gets a unique identifier for the entity.
          *
          * \details An id must be unique within its context but
          * does not need to be globally unique. For example, the id of an input exchange
          * item must be unique in the list of inputs of a IModelComponent, but a
          * similar Id might be used by an exchange item of another IModelComponent.
          *
          * \returns An id as a String.
          *
          */
         virtual QString id() const = 0;
         
   };
   
   /*!
    * \brief IComponentInfo interface class provides detailed metadata about a component.
    *
    * \details It must not be implemented directly. It must be either be implemented as
    *  an IModelComponent or an Data::IAdaptedOutputFactoryComponent.
    *
    */
   class IComponentInfo : public virtual IIdentity
   {
         
      public:
         virtual ~IComponentInfo() {}
         
         /*!
          * \brief Name of the current Component.
          * \returns QString representing the name of this component.
          */
         virtual QString name() const = 0;
         
         /*!
          * \brief File path to Component library.
          * \returns Path to the library location from which this component was created.
          */
         virtual QString libraryFilePath() const = 0;
         
         /*!
          * \brief Sets file path to Component library.
          * \param FilePath to the libary from which this component was created.
          */
         virtual void setLibraryFilePath(const QString& filePath) = 0;
         
         /*!
          * \brief File path to Component icon.
          * Must be specified relative to the component library.
          *
          * \returns filePath to icon for component.
          */
         virtual QString iconFilePath() const = 0;
         
         /*!
          * \brief Component developer information.
          * \returns Name of vendor the developed this component.
          */
         virtual QString vendor() const = 0;
         
         /*!
          * \brief Publications associated with this component.
          * \returns Citations of publications related to this component.
          */
         virtual QList<QString> publications() const = 0;
         
         /*!
          * \brief Component license info.
          * \returns QString representing the license information. HTML tags can be added to it.
          */
         virtual QString license() const = 0;
         
         /*!
          * \brief Component copyright info.
          * \returns QString representing the copyright information associated with this component.
          */
         virtual QString copyright() const = 0;
         
         /*!
          * \brief Component developer url.
          * \returns QString representing the url for the developer.
          */
         virtual QString url() const = 0;
         
         /*!
          * \brief Component developer email.
          * \returns email as QString.
          */
         virtual QString email() const = 0;
         
         /*!
          * \brief Component version info.
          * \returns QString representing the version of this component.
          */
         virtual QString version() const = 0;
         
         /*!
          * \brief Component category.
          * \returns the category that this component belongs to.
          * Sub-categories can be specified by separating categories using the backslash
          * e.g., Hydrology\\Snowmelt.
          */
         virtual QString category() const = 0;
         
         /*!
          * \brief Checks if license is valid.
          *
          * \details Developer is responsible for implementing this validation based on a license.
          *
          * \param validationMessage associated with the license validation process.
          *
          * \returns true if license is valid otherwise false.
          */
         virtual bool validateLicense(QString& validationMessage) const = 0;
   };
   
   /*!
    * \brief IModelComponentInfo interface inherits from the IComponentInfo interface which
    * provides detailed metadata about an IModelComponent. Additionally, it creates new instances of a component.
    *
    * \details The IModelComponentInfo interface is used to provide metadata
    * on a component and create new instances of a component.
    *
    */
   class IModelComponentInfo : public virtual IComponentInfo
   {
         
      public:
         virtual ~IModelComponentInfo() {}
         
         //!Creates a new IModelComponent instance.
         /*!
          * \returns A new instance of an IModelComponent.
          */
         virtual IModelComponent* createComponentInstance() = 0;
   };
   
   /*!
    * \brief The IComponentStatusChangeEventArgs contains the information that will
    * be passed when the IModelComponent fires the IModelComponent::statusChanged signal.
    * \details Sending exchange item events is optional, so it should not be used as a mechanism to build critical functionality upon.
    */
   class IComponentStatusChangeEventArgs
   {
      public:
         virtual ~IComponentStatusChangeEventArgs() {}
         
         //!Gets the IModelComponent that fired the event.
         /*!
          * \returns The IModelComponent that threw the event.
          */
         virtual IModelComponent* component() const = 0;
         
         //!Gets the IModelComponent's status before the status change.
         /*!
          * \returns The previous ComponentStatus of the component that threw the event.
          */
         virtual ComponentStatus oldStatus() const = 0;
         
         //!Gets the IModelComponent's status after the status change.
         /*!
         * \returns The new ComponentStatus of the component that threw the event.
         */
         virtual ComponentStatus newStatus() const = 0;
         
         //!Gets additional information about the status change.
         virtual QString message() const = 0;
         
         //!A bool indicating whether this event has a progresss monitor.
         /*!
          * \returns True if status has a percent progress otherwise false and the progress bar shows busy.
          */
         virtual bool hasProgressMonitor() const = 0;
         
         //!Number between 0 and 100 indicating the progress made by a component in its simulation.
         /*!
          * \returns A number between 0 and 100 indicating the progress made by a component.
          */
         virtual float percentProgress() const = 0;
   };
   
   /*!
    * \brief IModelComponent interface is the core interface in the HydroCouple standard defining a model component.
    */
   class IModelComponent : public virtual IIdentity
   {
         
      public:
         virtual ~IModelComponent() {}
         
         //!Contains the metadata about this IModelComponent instance.
         /*!
          * \returns An IModelComponentInfo that provides metadata about a component.
          */
         virtual const IModelComponentInfo* componentInfo() const = 0;
         
         //!Parent IModelComponent object from which current component was cloned from.
         /*!
          * \returns The parent IModelComponent from which the current component was created.
          */
         virtual IModelComponent* parent() const = 0;
         
         //!Deep clones itself including cloning its Data::IArgument.
         /*!
          * \returns A deep clone of the current component. Configuration files and output files
          * must be written to a different location than those of the parent.
          */
         virtual IModelComponent* clone() const = 0;
         
         //!A list IModelComponent instances cloned from this IModelComponent instance.
         /*!
          * \returns A list of child components created from the current component.
          */
         virtual QList<IModelComponent*> children() const = 0;
         
         /*!
          * \brief Arguments needed to let the component do its work. An unmodifiable list of
          * (modifiable) arguments must be returned that is to be used to get
          * information about the arguments and to set argument values.
          *
          * \details Validation of changes can be done either when they occur (e.g., using notifications) or
          * when the initialize method is called. Initialize will always be called before any
          * call to the update method of the IModelComponent.
          *
          * \details This property must be available as soon is the IModelComponent instance is created
          * Arguments describes the arguments that can be set before the initialize() method is called.
          *
          * \returns A list of IArguments for instantiating this component.
          */
         virtual QList<HydroCouple::IArgument*> arguments() const = 0;
         
         //!Defines current status of the IModelComponent. See HydroCouple::Componentstatus for the possible values
         /*!
          * \details The first status that a component sets is HydroCouple::Created,
          * as soon after it has been created. In this status,
          * Arguments is the only property that may be accessed.
          *
          * \returns the current status of this component
          */
         virtual ComponentStatus status() const = 0;
         
         //!The list of consumer items for which a component can recieve values.
         /*!
          * \details This property must be accessible after the initialize() method has been
          * invoked and until the validate() method has been invoked. If this property
          * is accessed before the initialize() method has been invoked or after the
          * validate() method has been invoked and the IModelComponent cannot handle
          * this an exception must be thrown.
          *
          * \returns This method basically returns references to IInputs items.
          * There is no guarantee that the list of objects is not altered by other components
          * after it has been returned. It is the responsibility of the IModelComponent
          * to make sure that such possible alterations do not subsequently corrupt the IModelComponent.
          */
         virtual QList<HydroCouple::IInput*> inputs() const = 0;
         
         //!The list of IOutputs for which a component can produce results.
         /*!
          * \details This property must be accessible after the initialize() method has been
          * invoked and until the validate() method has been invoked. If this property
          * is accessed before the initialize() method has been invoked or after the
          * validate() method has been invoked and the IModelComponent cannot handle
          * this an exception must be thrown.
          *
          * \details The list only contains the core IOutputs of the IModelComponent, not
          * the IAdaptedProducerItem derived from each IOutput (etc.). To get a complete
          * list of producers traverse the chain of IAdaptedProducerItem that start with the
          * IOutput returned in the list.
          *
          * \returns This method basically returns references to IOutputs.
          * There is no guarantee that the list of objects is not altered by other components
          * after it has been returned. It is the responsibility of the IModelComponents
          * to make sure that such possible alterations do not subsequently corrupt the IModelComponents.
          */
         virtual QList<HydroCouple::IOutput*> outputs() const = 0;
         
         //! Gets a list of IAdaptedOutputFactories, each allowing to create IAdaptedOutput
         //! item for making outputs fit to inputs in case they do not already do so.
         /*! Factories can be added to and removed from the list so that
          * third-party factories and IAdaptedOutput classes can be introduced.
          * \returns A list of IAdaptedOutputFactories associated with this component.
          */
         virtual QList<HydroCouple::IAdaptedOutputFactory*> adaptedOutputFactories() const = 0;
         
         /*!
          * \brief Initializes the  current IModelComponent
          *
          * \details The initialize() method must be invoked before any other
          * method or property in the IModelComponent interface is invoked or accessed, except
          * for the Arguments property.
          *
          * \details Immediately after the method is invoked, it changes the IModelComponent's status to HydroCouple::Initializing.
          * When the method is executed and an error occurs, the status of the component will change to HydroCouple::Failed,
          * and an exception will be thrown. If the component initializes succesfully, the  status is changed to HydroCouple::Initialized.
          *
          * \details When the initialize() method has been finished and the status is HydroCouple::Initialized,
          * the properties Id, Caption, Description, Inputs, Outputs, have been set,
          * and the method validate() can be called.
          *
          * \details It is only required that the method initialize() can be invoked once.
          * If the initialize() method is invoked more than once and the IModelComponent
          * cannot handle this; an exception must be thrown.
          *
          * \remarks The method will typically populate the component based on the
          * values specified in its arguments, which can be retrieved with
          * arguments. Settings can be used to read input files, allocate memory,
          * and organize input and output exchange items.
          */
         virtual void initialize() = 0;
         
         //!Validates the populated instance of the IModelComponent
         /*!
          * \details This method must be accessible after the initialize() method has been
          * invoked and until the finish() method has been invoked. If this property
          * is accessed before the initialize() method has been invoked or after the
          * Finish method has been invoked and the IModelComponent cannot handle
          * this an exception must be thrown.
          *
          * \details The method will and must be invoked after the various provider/consumer
          * relations between this component's exchange items and the exchange
          * items of other components present in the composition.
          *
          * \details Immediately after the method is invoked, it changes the IModelComponent's
          * status to HydroCouple::Validating.
          *
          * \details When the validate() method has finished, the status of the IModelComponent
          * has changed to either Valid or HydroCouple::Invalid.
          *
          * \returns Returns an array of strings of length 0 if there are no messages at all.
          * If there are messages while the components status is HydroCouple::Valid, the messages are purely informative.
          * If there are messages while the components status is HydroCouple::Componentstatus.
          * HydroCouple::Invalid when at least one of the messages indicates a fatal error.
          */
         virtual QList<QString> validate() = 0;
         
         //!Prepares the IModelComponent for calls to the Update method
         /*!
          * \details Before prepare() is called, the IModelComponent are not required to honor
          * any type of action that retrieves values from the IModelComponent.
          * After prepare() is called, the IModelComponent must be ready for providing values.
          *
          * \details This method must be accessible after the initialize() method has been
          * invoked and until the finish() method has been invoked. If this property
          * is accessed before the initialize() method has been invoked or after the
          * finish() method has been invoked and the IModelComponent cannot handle
          * this an exception must be thrown.
          *
          * \details Immediatly after the method is invoked, it changes the IModelComponent's status to HydroCouple::Preparing.
          *
          * \details When the method has finished, the status of the IModelComponent has changed to either HydroCouple::Updated or HydroCouple::Failed.
          *
          * \details It is only required that the prepare() method can be invoked once.
          * If the prepare() method is invoked more that once and the IModelComponent
          * cannot handle this an exception must be thrown.
          */
         virtual void prepare() = 0;
         
         //! This method is called to let the component update itself, thus reaching its next state.
         /*!
          * \details Immediately after this method is invoked, it changes the component's status() to HydroCouple::Updating.
          *
          * \details The type of actions a component takes during the Update method depends
          * on the type of component. A numerical model that progresses in time will typically
          * compute a time step. A database whould typically look at the consumers of its output items,
          * and perform one or more queries to be able to provide the values that the consumers require.
          * A GIS system would typically re-evaluate the values in a grid coverage, so that its output
          * output items can provide up-to-date values.
          *
          * \details If the Update method is performed succesfully, the component sets its state to
          * HydroCouple::Updated, unless after this update() action the
          * component is at the end of its computation, in which case it will be set its State
          * to HydroCouple::Done.
          * If during the update() method a problem arises, the component sets its state to
          * HydroCouple::Failed, and throws an exception.
          *
          * \param requiredOutputs is an optional parameter lets the caller specify the specific
          * producer items that should be updated. If the length is 0, the component
          * will at least update its producer items that have consumers, or all its output items,
          * depending on the component's implementation.
          */
         virtual void update(const QList<HydroCouple::IOutput*> & requiredOutputs = QList<HydroCouple::IOutput*>()) = 0;
         
         //!The finish() must be invoked as the last of any methods in the IModelComponent interface
         /*!
          * \details This method must be accessible after the prepare() method has been invoked.
          * If this method is invoked before the prepare() method has been invoked an
          * exception must be thrown by the IModelComponent.
          * \details Immediately after the method is invoked, it changes the IModelComponent's status() to HydroCouple::Finishing.
          * Once the finishing is completed, the component changes its status() to
          * HydroCouple::Finished if it can not be restarted,
          * or HydroCouple::Created if it can.
          */
         virtual void finish() = 0;
         
         //! The componentStatusChanged() function must be implemented as a signal and emitted when Status of the component changes.
         /*!
          *See HydroCouple::ComponentStatus for the possible states.
          */
         virtual void componentStatusChanged(const IComponentStatusChangeEventArgs& statusChangedEvent) = 0;
   };
   
   //!IDimension provides the properties of the dimensions of a vairable.
   class IDimension : public virtual IDescription
   {
         
      public:
         virtual ~IDimension() {}
         
         /*!
             * \brief Dimension name.
             * \returns The name of this dimension.
             */
         virtual QString name() const = 0;
         
         //! Gets length of dimension.
         /*!
             */
         virtual int length() const = 0;
         
         //! Gets the dimension length type.
         /*!
             */
         virtual DimensionType lengthType() const = 0;
   };
   
   //! IValueDefinition describes a value returned by the getValues function of IValueSet
   /*!
    * This interface is not meant to be implemented directly.
    * Instead, implement either IQuality or IQuantity or a
    * custom derived vale definition interface.
    */
   class IValueDefinition : public virtual IDescription
   {
      public:
         virtual ~IValueDefinition() {}
         
         /*!
          * \brief The object types of value that will be available
          * and is returned by the GetValues function.IPropertyChanged.
          *
          * \returns the value type associated with this IValueDefinition
          */
         virtual QVariant::Type type() const = 0;
         
         //!The value representing that data is missing.
         virtual QVariant missingValue() const = 0;
         
         //!Gets the default value of the argument
         virtual QVariant defaultValue() const = 0;
         
         /*!
          * \brief Gets list of possible allowed values.
          *
          * \details If for integral types or component specific types all possible values are allowed,
          *invalid qvariant is returned. A list with length 0 indicates that there is indeed
          *a limitation on the possible values, but that currently no values are possible.
          *Effectively this means that the values will not and cannot be set.
          *
          */
         virtual QList<QVariant> possibleValues() const = 0;
   };
   
   /*!
    * \brief The ICategory describes one item of a possible categorization.
    * It is used by the IQuality interface for describing qualitative data.
    *
    * \details For qualitative data the IValueSet exchanged between components
    * contains one of the possible ICategory instances per data element.
    *
    * \details A category defines one "class" within a "set of classes".
    */
   class ICategory : public virtual IDescription
   {
      public:
         virtual ~ICategory() {}
         
         //! Value for this category.
         /*!
          * \details Example
          *"blue" in a "red"/"green"/"blue" set.
          */
         virtual QVariant value() const = 0;
   };
   
   /*!
    * \brief  Qualitative data described items in terms of some quality or categorization that may be 'informal'
    * or may use relatively ill-defined characteristics such as warmth and flavour. However,
    * qualitative data can include well-defined aspects such as gender, nationality or commodity type.
    *
    * \returns An IQuality describes qualitative data, where a value is specified as one category
    * within a number of predefined (possible) categories. These categories can be ordered or not.
    *
    * \details For qualitative data the IComponentItem data exchanged between IComponents contains
    * one of the possible ICategory instances per element in the  IComponentItem involved.
    *
    * \details
    *Examples:
    *    - Colors:
    *       - red
    *       - green
    *       - blue
    *    - Land use:
    *       - nature
    *       - recreation
    *       - industry
    *       - infrastructure
    *    - Rating:
    *       - worse
    *       - same
    *       - better
    */
   class IQuality : public virtual IValueDefinition
   {
      public:
         virtual ~IQuality() {}
         
         //! Returns a list of the possible ICategory allowed for this IQuality If the quality is not ordered the list contains
         //! the ICategory's in an unspecified order. When it is ordered the list contains the ICategory's in the same sequence.
         virtual QList<ICategory*> categories() const = 0;
         
         //! Checks if the IQuality is defined by an ordered set of ICategory or not.
         virtual bool isOrdered() const = 0;
   };
   
   //! Defines the order of dimension in each FundamentalDimension for a unit.
   class IUnitDimensions : public virtual IDescription
   {
      public:
         virtual ~IUnitDimensions() {}
         
         //!Returns the power for the requested dimension.
         /*!
          * \param dimension represents the fundamental unit.
          *
          * \details For a quantity such as flow, which may have the unit m<sup>3</sup>/s,
          * The getPower method must work as follows:
          *
          *  * getPower( FundamentalUnitDimension::AmountOfSubstance )
          * \returns 0
          *  * getPower( FundamentalUnitDimension::Currency )
          * \returns 0
          *  * getPower( FundamentalUnitDimension::ElectricCurrent )
          * \returns 0
          *  * getPower( FundamentalUnitDimension::Length )
          * \returns 3
          *  * getPower( FundamentalUnitDimension::LuminousIntensity )
          * \returns 0
          *  * getPower( FundamentalUnitDimension::Mass )
          * \returns 0
          *  * getPower( FundamentalUnitDimension::Temperature )
          * \returns 0
          *  * getPower( FundamentalUnitDimension::Time )
          * \returns -1
          *
          */
         virtual double getPower(FundamentalUnitDimension dimension) const = 0;
   };
   
   /// IUnit interface, describing the physical unit of a IQuantity.
   class IUnit : public virtual IDescription
   {
      public:
         virtual ~IUnit() {}
         
         //!Fundamental dimensions of the unit
         virtual IUnitDimensions* dimensions() const = 0;
         
         //! Conversion factor to SI ('A' in: SI-value = A * quant-value + B)
         virtual double  conversionFactorToSI() const = 0;
         
         //! OffSet to SI ('B' in: SI-value = A * quant-value + B)
         virtual double offSetToSI() const = 0;
   };
   
   //!IQuantity specifies values as an amount of some unit, usually as a floating point number.
   class IQuantity : public virtual IValueDefinition
   {
      public:
         virtual ~IQuantity() {}
         
         //!Unit of quantity
         virtual IUnit* unit() const = 0;
   };
   
   //!IComponentItem is a fundamental unit of data for a component.
   /*!
    * This interface is not to be implemented directly.
    */
   class IComponentItem : public virtual IIdentity
   {
         
      public:
         virtual ~IComponentItem() {}
         
         /*!
          * \brief Gets the owner IModelComponent of this IComponentItem.For an IOutput component item this is the component
          * responsible for providing the content of the IOutput.
          *
          * \details It is possible for an IComponentItem to have no owner, in this case the method will return nullptr.
          *
          * \return an IModelComponent object that is the parent of this IComponentItem
          */
         virtual IModelComponent* modelComponent() const = 0;
         
         /*!
          * \brief provides the dimensions associated with this IComponentItem.
          *
          * \details This function must return the unique IDimension objects from the IValueSet objects asssociated
          * with this IComponentItem.
          *
          * \return A list of IDimension objects.
          *
          */
         virtual QList<IDimension*> dimensions() const = 0;
   };
   
   //! IValueSet provides a multi-dimensional data with its context.
   class IValueSet : public virtual IDescription
   {
         
      public:
         virtual ~IValueSet() {}
         
         //!IComponentItem associated with this IValueSet
         /*!
          */
         virtual IComponentItem* componentItem() const = 0;
         
         //!Subset of the IDimension objects from the IComponentItem that are related to this this IValueSet
         /*!
          */
         virtual QList<IDimension*> dimensions() const = 0;
         
         //!IDimension s related to this IValueSet
         /*!
          *Returns the length (max index count) of the dimension specified by the
          *given indices. To get the size of the first dimension, use a zero-length
          *integer array as input argument. Length of indices must be a least one
          *smaller than the dimensions().
          * \param dimensionIndexes represents the indexes for the IDimension to retrieve the length.
          */
         virtual int getDimensionLength(const QList<int>& dimensionIndexes) const = 0;
         
         //!Gets value for given dimension indexes
         /*!
          * \param dimensionIndexes for the IDimension to retrieve the length
          */
         virtual QVariant getValue(int dimensionIndexes[]) const = 0;
         
         //!sets value for given dimension indexes
         /*!
          * \param dimensionIndexes for the IDimension to retrieve the length
          * \param value to set
          */
         virtual void setValue(int dimensionIndexes[], const QVariant& value) = 0;
         
         //!IValueDefinition for this IValueSet defines the variable type associated with this object
         /*!
          * \returns the variable definition for this variable. This is either a
          */
         virtual IValueDefinition* valueDefinition() const = 0;
   };
   
   /*!
    * \brief IArgument interface class used to set the arguments for components.
    * They can be complex or simple multi-dimensional datasets
    *
    * \details IArgument is primarily used to set arguments of IModelComponent and IAdaptedOutput
    */
   class IArgument : public virtual IComponentItem
   {
         
      public:
         virtual ~IArgument() {}
         
         /*!
          * \brief Specifies whether the argument is optional or not.
          *
          * \details If the getValue property returns null and isOptional == false,
          * a value has to be set before the argument can be used
          */
         virtual bool isOptional() const = 0;
         
         /*!
          * \brief Defines whether the Values property may be edited.
          *
          * \details This is used to let a IModelComponent or an IAdaptedOutput
          * present the actual value of an argument that can not be changed by the user,
          * but is needed to determine the values of other arguments or is informative in any other way.
          */
         virtual bool isReadOnly() const = 0;
         
         /*!
          * \brief Deep clones this IArgument.
          *
          * \details For example if the argument is a file, it makes a copy of the file with a different file name and reads
          */
         virtual IArgument* clone() const = 0;
         
         /*!
          * \brief IValueSet for this IArgument.
          */
         virtual QList<IValueSet*> valueSets() const = 0;
         
         /*!
          * \brief Variables for this IArgument
          */
         virtual QString toString() const = 0;
         
         //!File type extensions that can be read by this IArgument.
         /*!
          * \returns a list of strings for compatible file extensions.
          */
         virtual QList<QString> fileExtensions() const;
         
         /*!
          * \brief Boolean indicating whether this IArgument can read its values from a file.
          */
         virtual bool canReadFromFile() const = 0;
         
         /*!
          * \brief Boolean indicating whether this IArgument copy its values from a QString.
          */
         virtual bool canReadFromString() const = 0;
         
         /*!
          * \brief Reads values from a QString or a QString representing an input file.
          * \param value is QString representing values or file containing values.
          * \param isFile is a boolean indicating whether the values are in the QString or the file pointed by the QString.
          * \return boolean indicating whether file reading was successful
          */
         virtual bool readValues(const QString& value , bool isFile = false) = 0;
         
         /*!
          * \brief Reads values from another IArgument.
          * \param argument is the IArgument from which to copy values from.
          * \param isFile is a boolean indicating whether the values are in the QString or the file pointed by the QString.
          * \return boolean indicating whether file reading was successful.
          */
         virtual bool readValues(const IArgument* argument) = 0;
   };
   
   /*!
    * \brief The IExchangeItemChangeEventArgs contains the information that will
    * be passed when the IComponentItem fires the componentItemChanged signal.
    *
    * \details Sending exchange item events is optional, so it should
    * not be used as a mechanism to build critical functionality upon.
    *
    */
   class IExchangeItemChangeEventArgs
   {
      public:
         virtual ~IExchangeItemChangeEventArgs() {}
         
         /*!
          * \brief IExchangeItem which fired the signal.
          */
         virtual IExchangeItem* exchangeItem() const = 0;
         
         /*!
          * \brief Gets message associated with the event.
          */
         virtual QString message() const = 0;
         
   };
   
   /*!
    * \brief IExchangeItem the base data item the can be exchanged between components at runtime.
    *
    * \details This interface is not to be implemented directly, any class is to implement either the IInput or IOutput.
    */
   class IExchangeItem : public virtual IComponentItem
   {
         
      public:
         virtual ~IExchangeItem() {}
         
         /*!
          * \brief The componentItemChanged event is fired when
          * the content of an IComponentItem has changed.
          *
          * \param statusChangedEvent provides information about the status change
          */
         virtual void itemChanged(const IExchangeItemChangeEventArgs& statusChangedEvent) = 0;
   };
   
   /*!
    * \brief An output exchange item that can deliver values from an IModelComponent.
    *
    * \details If an output does not provide the data in the way a consumer would like to
    * have it the output can be adapted by an IAdaptedOutput, which can transform
    * the data according to the consumer's wishes. E.g. by performing interpolation in time, spatial aggregation, etc.).
    *
    */
   class IOutput : public virtual IExchangeItem
   {
      public:
         virtual ~IOutput() {}
         
         /*!
          * \brief  Input items that will consume the values, by calling the GetValues() method
          *
          * \details Every input item that will call this method, needs to call the addConsumer()
          * method first. If the input item is not interested any longer in calling the IValueSet::getValue(),
          * it should remove itself by calling the removeConsumer() method. The list is readonly.
          * Add and remove from the list by using addConsumer() and removeConsumer().
          * Please be aware that the "unadulterated" values in the output item, provided by the read only Values property,
          * may be called anyway, even if there are no values available.
          *
          */
         virtual QList<IInput*> consumers() const = 0;
         
         
         /*!
          * \brief Add a consumer to this output item. Every input item that wants to call
          *  the IValueSet::getValue() method, needs to add itself as a consumer first.
          *
          * \details If a consumer is added that can not be handled, or that is incompatible with the already
          *  added consumers, an exception will be thrown.
          *
          * \details The addConsumer() method must and will automatically set the consumer's
          *  provider (see IInput.provider() )
          *
          * \param consumer that has to be added
          *
          */
         virtual void addConsumer(IInput* consumer) = 0;
         
         /*!
          * \brief Remove a consumer.
          *
          * \details If an input item is not interested any longer in calling the IValueSet::getValue() method,
          *  it should remove itself by calling RemoveConsumer.
          *
          * \param consumer that has to be removed
          *
          */
         virtual bool removeConsumer(IInput* consumer) = 0;
         
         /*!
          * \brief The adaptedOutputs that have this current output item as adaptee.
          *
          * \details As soon as the output item's values have been updated, for each adaptedOutput its
          * IAdaptedOutput.refresh() method must be called.
          *
          * \details The list is readonly. Add and remov from the list by using addAdaptedOutput() and removeAdaptedOutput().
          *
          */
         virtual QList<IAdaptedOutput*> adaptedOutputs() const = 0;
         
         /*!
          * \brief Add a IAdaptedOutput to this output item.
          *
          * \details Every adaptedOutput that uses data from this output item,
          * needs to add itself as a consumer first.
          *
          * \details If a adaptedOutput is added that can not be handled, or that is
          * incompatible with the already added adaptedOutputs, an exception will be thrown
          *
          * \param adaptedOutput is consumer that has to be added
          *
          */
         virtual void addAdaptedOutput(IAdaptedOutput* adaptedOutput) = 0;
         
         /*!
          * \brief Removes an IAdaptedOutput.
          *
          * \details If a adaptedOutput is not interested any longer in this output item data,
          * it should remove itself by calling removeConsumer().
          *
          * \param adaptedOutput is a consumer that has to be removed.
          *
          */
         virtual bool removeAdaptedOutput(IAdaptedOutput* adaptedOutput) = 0;
         
         /*!
          *
          * \brief Provides the values matching the value definition specified by the
          * <param name="querySpecifier">. Extensions can overwrite this base version to include
          * more details in the query, e.g. time and space.
          *
          * \details One might expect to be the querySpecifier to be of the type IInput, because every input item that calls
          * the GetValues method needs to add itself as a consumer first.
          *
          * \details However, the IExchangeItem suffices to  specify what is required. Therefore,
          * to have the flexibility to loosen the "always register as consumer" approach, it is chosen to provide
          * an IExchangeItem as an argument.
          *
          */
         virtual void updateValues(IInput* querySpecifier) = 0;
   };
   
   /*!
    * \brief An IAdaptedOutput adds one or more data operations on top of an IOutput.
    *
    * \details It is in itself an IOutput. The IAdaptedOutput extends an output with operations including
    * spatial interpolation, temporal interpolation, unit conversion etc.
    *
    * \details IAdaptedOutput instances are created by means of an IAdaptedOutputFactory.
    *
    * \details The IAdaptedOutput is based on the adaptor design pattern. It adapts an IOutput or another IAdaptedOutput to make it
    * suitable for new use or purpose. The object being adapted is typically called the adaptee. The IAdaptedOutput replaces the DataOperation that was
    * used in OpenMI Standard version 1.x.
    *
    */
   class IAdaptedOutput : public virtual IOutput
   {
         
      public:
         virtual ~IAdaptedOutput() {}
         
         /*!
          *
          * \brief IAdaptedOutputFactory that generated this IAdaptedOutput.
          *
          * \returns IAdaptedOutputFactory parent.
          *
          */
         virtual const IAdaptedOutputFactory* adaptedOutputFactory() const = 0;
         
         /*!
          * \brief  IArgument represents input parameters needed for this IAdaptedOutput.
          *
          * \details An unmodifiable list of the (modifiable) IArguments should be returned that can be used to
          * get information on an IArgument and to modify its values. Validation of changes is done when they occur (e.g. using notifications).
          *
          * \returns Unmodifiable list of IArgument for the adapted output.
          */
         virtual QList<IArgument*> arguments() const = 0;
         
         /*!
          *
          * \brief Sets an IArgument for the argument list of this IAdaptedOutput.Returns true if successful. Takes ownership of the pointer.
          *
          * \param argument to set.
          *
          */
         virtual bool setArgument(IArgument * argument) = 0;
         
         /*!
          * \brief Lets this IAdaptedOutput initialize() itself, based on the current values specified by the arguments.
          *
          * \details Only after initialize() is called the refresh() method might be called.
          *
          * \details A component must invoke the initialize() method of all its
          *  adapted outputs at the end of the component's Prepare phase. In case of stacked adapted outputs,
          *  the adaptee must be initialized first.
          *
          */
         virtual void initialize() = 0;
         
         /*!
          * \brief IOutput that this IAdaptedOutput extracts content from.
          * In the adapter design pattern, it is the item being adapted.
          *
          * \returns an IOutput that is being modified by this IAdaptedOutput.
          *
          */
         virtual IOutput* adaptee() const = 0;
         
         /*!
          * \brief Requests the IAdaptedOutput to refresh itself and perform any necessary calculations.
          *
          * \details This method will be called by the adaptee(), when it has been refreshed/updated.
          * In the implementation of the refresh method the adapted output should update its contents according to the changes in the adaptee.
          *
          * \details After updating itself the IAdaptedOutput must call refresh() on all
          * its IAdaptedOutput children, so the chain of IOutput 's refreshes themselves.
          *
          */
         virtual void refresh() = 0;
   };
   
   /*!
    * \brief IAdaptedOutputFactory is used to create instances of IAdaptedProducerExchangeItems.
    *
    * \details This class can be internal to an IModelComponent by calling
    * IModelComponent::adaptedOutputFactories() or can be generated
    * from an HydroCouple::IAdaptedOutputFactoryComponent.
    *
    */
   class IAdaptedOutputFactory : public virtual IIdentity
   {
      public:
         virtual ~IAdaptedOutputFactory() {}
         
         /*!
          * \brief Get a list of IIdentity objects representing the list
          * of the available IAdaptedOutput that can make the producer match the consumer.
          *
          * \details If the consumer is NULL, the identifiers of all IAdaptedOutputs
          * that can adapt the producer are returned.
          *
          * \param provide is the IOutput to adapt.
          * \param consumer is the IInput to adapt the producer to, can be NULL.
          * \returns A list of identifiers for the available IAdaptedOutputs.
          */
         virtual QList<IIdentity*> getAvailableAdaptedOutputIds(const IOutput* provider, const IInput* consumer) = 0;
         
         /*!
          * \brief Creates a IAdaptedOutput that adapts the producer so that it fits the consumer.
          *
          * \details The adaptedProducerId used must be one of the IHIdentifier instances
          * returned by the createAdaptedProducerItem method. The returned IAdaptedOutputs
          * will already be registered with the producer.
          * \param adaptedProducerId is an identifier of the IAdaptedOutput to create.
          * \param producer IOutput to adapt.
          * \param consumer IInput to adapt the adaptee to, can be NULL.
          * \returns An IAdaptedOutput.
          */
         virtual IAdaptedOutput* createAdaptedOutput(IIdentity* adaptedProviderId, const IOutput* provider, const IInput* consumer) = 0;
   };
   
   /*!
    * \brief IAdaptedOutputFactoryComponentInfo interface class provides
    * information about an IAdaptedOutputFactoryComponent.
    *
    * \details IAdaptedOutputFactoryComponentInfo used to provide
    * metadata on a producer item factory component and create instances of it.
    *
    */
   class IAdaptedOutputFactoryComponentInfo : public virtual IComponentInfo
   {
      public:
         virtual ~IAdaptedOutputFactoryComponentInfo() {}
         
         /*!
          * \brief New IAdaptedOutputFactoryComponent instance.
          */
         virtual IAdaptedOutputFactoryComponent* createComponentInstance() const = 0;
   };
   
   /*!
    * \brief IAdaptedOutputFactoryComponent is an
    * IAdaptedOutputFactory generated from an IAdaptedOutputFactoryComponentInfo.
    *
    */
   class IAdaptedOutputFactoryComponent : public virtual IAdaptedOutputFactory
   {
      public:
         virtual ~IAdaptedOutputFactoryComponent() {}
         
         /*!
          * \brief Contains the metadata about the IModelComponent.
          *
          * \details This information includes the developer, component version number, contact url etc.
          */
         virtual IAdaptedOutputFactoryComponentInfo* componentInfo() const = 0;
   };
   
   /*!
    * \brief An IInput item that can accept values for an IModelComponent.
    */
   class IInput : public virtual IExchangeItem
   {
      public:
         virtual ~IInput() {}
         
         /*!
          * \brief Gets the producer this consumer should get its values from.
          */
         virtual IOutput* getProvider() const = 0;
         
         /*!
          * \brief Sets the producer this consumer should get its values from.
          *
          * \param provider is the IOutput that supplies the data to this IInput.
          */
         virtual void setProvider(IOutput* provider) = 0;
         
         /*!
          * \brief Returns true if this IInput can consume this producer.
          *
          * \param provider is the IOutput that can supply the data to this IInput.
          */
         virtual bool canConsume(const IOutput* provider) const = 0;
   };
}

//!HydroCouple Interface Declarations
Q_DECLARE_INTERFACE(HydroCouple::IPropertyChanged, "HydroCouple::IPropertyChanged/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IDescription, "HydroCouple::IDescription/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IIdentity, "HydroCouple::IIdentity/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IComponentInfo, "HydroCouple::IComponentInfo/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IModelComponentInfo, "HydroCouple::IModelComponentInfo/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IComponentStatusChangeEventArgs, "HydroCouple::IComponentStatusChangeEventArgs/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IModelComponent, "HydroCouple::IModelComponent/1.0")

//!HydroCouple Interface Declarations
Q_DECLARE_INTERFACE(HydroCouple::IDimension, "HydroCouple::IDimension/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IValueDefinition, "HydroCouple::IValueDefinition/1.0")
Q_DECLARE_INTERFACE(HydroCouple::ICategory, "HydroCouple::ICategory/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IQuality, "HydroCouple::IQuality/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IUnitDimensions, "HydroCouple::IUnitDimensions/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IUnit, "HydroCouple::IUnit/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IQuantity, "HydroCouple::IQuality/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IValueSet, "HydroCouple::IValueSet/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IComponentItem, "HydroCouple::IComponentItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IArgument, "HydroCouple::IArgument/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IExchangeItemChangeEventArgs, "HydroCouple::IExchangeItemChangeEventArgs/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IExchangeItem, "HydroCouple::IExchangeItem/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IOutput, "HydroCouple::IOutput/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IAdaptedOutput, "HydroCouple::IAdaptedOutput/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IAdaptedOutputFactory, "HydroCouple::IAdaptedOutputFactory/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IAdaptedOutputFactoryComponentInfo, "HydroCouple::IAdaptedOutputFactoryComponentInfo/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IAdaptedOutputFactoryComponent, "HydroCouple::IAdaptedOutputFactoryComponent/1.0")
Q_DECLARE_INTERFACE(HydroCouple::IInput, "HydroCouple::IInput/1.0")


#endif // HYDROCOUPLE_H
