/*!
 * \file hydrocouple.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0
 * \description
 * This header file contains the core interface definitions for the
 * HydroCouple component-based modeling definitions.
 * \license
 * This file and its associated files, and libraries are free software.
 * You can redistribute it and/or modify it under the terms of the
 * Lesser GNU Lesser General Public License as published by the Free Software Foundation;
 * either version 3 of the License, or (at your option) any later version.
 * This file and its associated files is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
 * \copyright Copyright 2014-2024, Caleb Buahin, All rights reserved.
 * \date 2014-2024
 * \pre
 * \bug
 * \warning
 * \todo
 */

#ifndef HYDROCOUPLE_H
#define HYDROCOUPLE_H

#include <variant>
#include <string>
#include <any>
#include <functional>
#include <list>
#include <set>
#include <memory>

using namespace std;

/*!
 * \brief The ByteOrder enum of serialized data
 */
enum ByteOrder
{
  BigEndian = 0,
  LittleEndian = 1
};

/*!
 * \brief HydroCouple namespace contains the core interface specifications
 * for the HydroCouple component-based modeling framework interface specification.
 */
namespace HydroCouple
{
  //! Forward declarations
  template <typename... Args>
  class ISignal;

  class IModelComponent;
  class IAdaptedOutputFactory;
  class IArgument;
  class IInput;
  class IOutput;
  class IExchangeItem;
  class IAdaptedOutput;
  class IAdaptedOutputFactory;
  class IAdaptedOutputFactoryComponent;
  class IUnit;
  class IComponentStatusChangeEventArgs;
  class IWorkflowComponent;
  class IWorkflowComponentStatusChangeEventArgs;

  /*!
   * \brief hydrocouple_variant is a variant type that can be used to store values of different types.
   */
  typedef variant<
      bool,
      char,
      short,
      int,
      long,
      unsigned char,
      unsigned short,
      unsigned int,
      unsigned long,
      float,
      double,
      long double,
      string>
      hydrocouple_variant;

  /*!
   * \brief ISlot interface class must be implemented by classes that want to listen to signals.
   * \details ISlot is a template class that can be used to listen to signals with any number of arguments.
   * \tparam Args are the arguments that will be passed by the signal.
   * \sa ISignal
   */
  template <typename... Args>
  class ISlot
  {
  public:
    /*!
     * \brief ISlot::~ISlot is a virtual destructor.
     */
    virtual ~ISlot() = default;

    /*!
     * \brief operator() is the function call operator that is called when a signal is emitted.
     * \param sender is the object that emitted the signal.
     * \param args are the arguments passed by the signal.
     */
    virtual void operator()(const ISignal<Args...> &sender, Args... args) = 0;
  };

  /*!
   * \brief ISignal interface class is used to emit signals/events to listeners.
   * \tparam Args are the arguments that will be passed by the signal.
   * \sa ISlot
   * \sa IPropertyChanged
   */
  template <typename... Args>
  class ISignal
  {
  public:
    /*!
     * \brief ISignal::~ISignal is a virtual destructor.
     */
    virtual ~ISignal() = default;

    /*!
     * \brief connect is used to connect a slot to the signal.
     * \param slot is the slot that will listen to the signal.
     */
    virtual void connect(const shared_ptr<ISlot<Args...>> &slot) = 0;

    /*!
     * \brief disconnect is used to disconnect a slot from the signal.
     * \param slot is the slot that will be disconnected from the signal.
     */
    virtual void disconnect(const shared_ptr<ISlot<Args...>> &slot) = 0;

  protected:
    /*!
     * \brief emit is used to emit the signal.
     * \param args are the arguments that will be passed by the signal.
     */
    virtual void emit(Args... args) = 0;
  };

  /*!
   * \brief IPropertyChanged interface is used to emit signal/event when a
   * property of an object changes.
   */
  class IPropertyChanged : public virtual ISignal<std::string>
  {

  public:
    /*!
     * \brief IPropertyChanged::~IPropertyChanged is a virtual destructor.
     */
    virtual ~IPropertyChanged() = default;
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
    /*!
     * \brief IDescription::~IDescription is a virtual destructor.
     */
    virtual ~IDescription() = default;

    /*!
     * \brief Gets caption for the entity.
     * \returns string representing caption for entity.
     * \sa setCaption()
     */
    virtual string caption() const = 0;

    /*!
     * \brief Sets caption for the entity.
     * \param caption is a string representing the caption for the entity.
     * \sa caption()
     */
    virtual void setCaption(const string &caption) = 0;

    /*!
     * \brief Gets additional descriptive information for the entity.
     * \returns string description of entity.
     * \sa setDescription()
     */
    virtual string description() const = 0;

    /*!
     * \brief Gets additional descriptive information for the entity.
     * \param description is a string for describing an entity.
     * \sa description()
     */
    virtual void setDescription(const string &description) = 0;
  };

  /*!
   * \brief IIdentity interface class defines a method to get the Id of an HydroCouple entity.
   * \details IIdentity extends the IDescription interface class, and therefore has, next to the id, a caption and a description.
   */
  class IIdentity : public virtual IDescription
  {
  public:
    /*!
     * \brief IIdentity::~IIdentity is a virtual destructor.
     */
    virtual ~IIdentity() = default;

    /*!
     * \brief Gets a unique identifier for the entity.
     *
     * \details An id must be unique within its context but
     * does not need to be globally unique. For example, the id of an input exchange
     * item must be unique in the list of inputs of a IModelComponent, but a
     * similar Id might be used by an exchange item of another IModelComponent.
     *
     * \returns An id as a string. The id must be unique within its context. It must not be empty.
     */
    virtual string id() const = 0;
  };

  /*!
   * \brief IComponentInfo interface class is a factory that provides detailed metadata
   * about a component and creates new instances of a component.
   *
   * \details It must not be implemented directly. It must be either be implemented as
   *  an IModelComponent or an Data::IAdaptedOutputFactoryComponent.
   *
   */
  class IComponentInfo : public virtual IIdentity
  {

  public:
    /*!
     * \brief IComponentInfo::~IComponentInfo is a virtual destructor.
     */
    virtual ~IComponentInfo() = default;

    /*!
     * \brief File path to Component library.
     * \returns Path to the library location from which this component was created.
     * \sa setLibraryFilePath()
     */
    virtual string libraryFilePath() const = 0;

    /*!
     * \brief Sets file path to Component library.
     * \param filePath to the libary from which this component was created.
     * \sa libraryFilePath()
     */
    virtual void setLibraryFilePath(const string &filePath) = 0;

    /*!
     * \brief File path to Component icon.
     * Must be specified relative to the component library.
     *
     * \returns filePath to icon for component.
     */
    virtual string iconFilePath() const = 0;

    /*!
     * \brief Component developer information.
     * \returns Name of vendor the developed this component.
     */
    virtual string vendor() const = 0;

    /*!
     * \brief Documentation associated with this component.
     * \returns Citations of publication related to this component.
     */
    virtual list<string> documentation() const = 0;

    /*!
     * \brief Component license info.
     * \returns string representing the license information. HTML tags can be added to it.
     */
    virtual string license() const = 0;

    /*!
     * \brief Component copyright info.
     * \returns string representing the copyright information associated with this component.
     */
    virtual string copyright() const = 0;

    /*!
     * \brief Component developer url.
     * \returns string representing the url for the developer.
     */
    virtual string url() const = 0;

    /*!
     * \brief Component developer email.
     * \returns email as string.
     */
    virtual string email() const = 0;

    /*!
     * \brief Component version info.
     * \returns string representing the version of this component.
     */
    virtual string version() const = 0;

    /*!
     * \brief tags used to classify this component.
     * \returns the categorical tags that can be used to classify components.
     * e.g., Hydrology, Groundwater, Finite Volume, Finite difference.
     */
    virtual set<string> tags() const = 0;

    /*!
     * \brief Checks if license is valid and persists license information.
     * \details Developer is responsible for implementing this validation based on a license.
     * \param licenseInfo license information to use to register this component.
     * \param validationMessage A validation message associated with the license validation process.
     * \returns true if license is valid otherwise false.
     */
    virtual bool validateLicense(const string &licenseInfo, string &validationMessage) = 0;

    /*!
     * \brief validateLicense Checks if component is licensed and returns.
     * \param validationMessage A validation message associated with the license validation process.
     * \return true if component is licensed otherwise false.
     */
    virtual bool validateLicense(string &validationMessage) = 0;
  };

  /*!
   * \brief IModelComponentInfo interface inherits from the IComponentInfo interface which
   * provides detailed metadata about an IModelComponent. Additionally, it creates new instances of a component.
   *
   * \details The IModelComponentInfo interface is used to provide metadata
   * on a component and create new instances of a component.
   */
  class IModelComponentInfo : public virtual IComponentInfo
  {

  public:
    /*!
     * \brief IModelComponentInfo::~IModelComponentInfo is a virtual destructor.
     */
    virtual ~IModelComponentInfo() = default;

    /*!
     * \brief Creates a new IModelComponent instance.
     * \returns A new instance of an IModelComponent.
     */
    virtual IModelComponent *createComponentInstance() = 0;

    /*!
     * \brief Gets a list of IAdaptedOutputFactories, each allowing
     * to create IAdaptedOutput item for making outputs fit to
     * inputs in case they do not already do so.
     *
     * \details Factories can be added to and removed from the list so that
     * third-party factories and IAdaptedOutput classes can be introduced.
     *
     * \returns A list of IAdaptedOutputFactories associated with this component.
     */
    virtual vector<IAdaptedOutputFactory *> adaptedOutputFactories() const = 0;
  };

  /*!
   * \brief IModelComponent interface is the core interface in the HydroCouple standard defining a model component.
   */
  class IModelComponent : public virtual IIdentity, public virtual ISignal<const shared_ptr<IComponentStatusChangeEventArgs> &>
  {

  public:
    /*!
     * \brief HydroCouple::ComponentStatus is an enumerator that describes the status of
     * a component over the course of its lifetime.
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
       * \brief The IModelComponent has successfully initialized itself by calling
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
       * \brief The IModelComponent has successfully updated itself.
       */
      Updated,

      /*!
       * \brief The last update process that the IModelComponent performed was the final one.
       * A next call to the HydroCouple::Update method will leave the IModelComponent's internal state unchanged
       */
      Done,

      /*!
       * \brief The IModelComponent was requested to perform the actions to be performed before it will either be
       * disposed or re-initialized again.Typical actions would be writing
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
     * \brief IModelComponent::~IModelComponent destructor
     */
    virtual ~IModelComponent() = default;

    /*!
     * \brief delete IModelComponent copy assignment operator.
     */
    virtual IModelComponent &operator=(const IModelComponent&) = delete;

    /*!
     * \brief Contains the metadata about this IModelComponent instance.
     * \returns An IModelComponentInfo that provides metadata about a component.
     */
    virtual IModelComponentInfo *componentInfo() const = 0;

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
    virtual vector<IArgument *> arguments() const = 0;

    /*!
     * \brief Defines current status of the IModelComponent.
     * See HydroCouple::Componentstatus for the possible values.
     * \details The first status that a component sets is HydroCouple::Created,
     * as soon after it has been created. In this status,
     * Arguments is the only property that may be accessed.
     * \returns the current status of this component
     */
    virtual ComponentStatus status() const = 0;

    /*!
     * \brief The list of consumer items for which a component can recieve values.
     *
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
    virtual vector<IInput *> inputs() const = 0;

    /*!
     * \brief The list of IOutputs for which a component can produce results.
     *
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
    virtual vector<IOutput *> outputs() const = 0;

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
     * \details The initialize() method can be invoked as long as a component is either HydroCouple::Created
     * HydroCouple::Failed, or HydroCouple::Initialized
     *
     * \remarks The method will typically populate the component based on the
     * values specified in its arguments, which can be retrieved with
     * arguments. Settings can be used to read input files, allocate memory,
     * and organize input and output exchange items.
     */
    virtual void initialize() = 0;

    /*!
     * \brief Validates the populated instance of the IModelComponent.
     *
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
    virtual vector<string> validate() = 0;

    /*!
     * \brief Prepares the IModelComponent for calls to the Update method.
     *
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
     * \details Immediately after the method is invoked, it changes the IModelComponent's status to HydroCouple::Preparing.
     *
     * \details When the method has finished, the status of the IModelComponent has changed to either HydroCouple::Updated or HydroCouple::Failed.
     *
     * \details It is only required that the prepare() method can be invoked once.
     * If the prepare() method is invoked more that once and the IModelComponent
     * cannot handle this an exception must be thrown.
     */
    virtual void prepare() = 0;

    /*!
     * \brief This method is called to let the component update itself, thus reaching its next state.
     *
     * \details Immediately after this method is invoked, it changes the component's status() to HydroCouple::Updating.
     *
     * \details The type of actions a component takes during the Update method depends
     * on the type of component. A numerical model that progresses in time will typically
     * compute a time step. A database would typically look at the consumers of its output items,
     * and perform one or more queries to be able to provide the values that the consumers require.
     * A GIS system would typically re-evaluate the values in a grid coverage, so that its output
     * output items can provide up-to-date values.
     *
     * \details If the Update method is performed successfully, the component sets its state to
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
    virtual void update(const vector<IOutput *> &requiredOutputs = vector<IOutput *>()) = 0;

    /*!
     * \brief The finish() must be invoked as the last of any methods in the IModelComponent interface.
     *
     * \details This method must be accessible after the prepare() method has been invoked.
     * If this method is invoked before the prepare() method has been invoked an
     * exception must be thrown by the IModelComponent.
     *
     * \details Immediately after the method is invoked, it changes the IModelComponent's status() to HydroCouple::Finishing.
     * Once the finishing is completed, the component changes its status() to
     * HydroCouple::Finished if it can not be restarted,
     * or HydroCouple::Created if it can.
     */
    virtual void finish() = 0;

    /*!
     * \brief workflow is the workflow that this component is part of.
     * \return
     */
    virtual const IWorkflowComponent *workflow() const = 0;

    /*!
     * \brief setWorkflow
     * \param workflow is the workflow that this component is part of.
     */
    virtual void setWorkflow(const IWorkflowComponent *workflow) = 0;

    /*!
     * \brief mpiNumProcesses
     * \return Returns the number of MPI processes allocated to this component.
     */
    virtual int mpiNumOfProcesses() const = 0;

    /*!
     * \brief mpiProcess is the MPI process/rank of this component.
     * \return Returns the integer identifier of the MPI process/rank of this component.
     */
    virtual int mpiProcessRank() const = 0;

    /*!
     * \brief mpiSetProcess
     * \param processRank
     */
    virtual void mpiSetProcessRank(int processRank) = 0;

    /*!
     * \brief mpiAllocatedProcesses are the set of MPI processes/ranks allocated to this component.
     * \return
     */
    virtual set<int> mpiAllocatedProcesses() const = 0;

    /*!
     * \brief mpiAllocateResources allocates the specified MPI processes/ranks to this component.
     * \param mpiProcessesToAllocate are the list of MPI processes/ranks to allocate to this component.
     * \details This method must be accessible after the initialize() method has been invoked.
     * If this method is invoked before the initialize() method has been invoked an exception must be thrown.
     *
     * This method must preferably be called on a processor with rank 0.
     */
    virtual void mpiAllocateProcesses(const set<int> &mpiProcessesToAllocate) = 0;

    /*!
     * \brief mpiClearAllocatedProcesses
     */
    virtual void mpiClearAllocatedProcesses() = 0;

    /*!
     * \brief referenceDirectory
     * \details All relative file paths specified that are associated with this component instance are referenced
     * from this directory. Typically, this will be the directory for the project file for the current composition. Values
     * are typically set from the Composition GUI and can be referenced internally for saving files and writing
     * arguments for the component.
     * \return
     */
    virtual string referenceDirectory() const = 0;

    /*!
     * \brief setReferenceDirectory
     * \details sets the reference directory for this component instance.
     * \param referenceDirectory
     */
    virtual void setReferenceDirectory(const string &referenceDirectory) = 0;
  };

  /*!
   * \brief The IProxyModelComponent class is a class for a remote IModelComponent's proxy
   */
  class IProxyModelComponent : public virtual IModelComponent
  {

  public:
    /*!
     * \brief ~IProxyModelComponent
     */
    virtual ~IProxyModelComponent() = default;

    /*!
     * \brief parentMpiProcessRank
     * \return An integer representing the MPI process rank of its parent model
     */
    virtual int parentMpiProcessRank() const = 0;

    /*!
     * \brief parentProcessAddress
     * \return A string representing the address of its parent model
     */
    virtual string parentProcessAddress() const = 0;

    /*!
     * \brief parentId
     * \return A string representing the unique identifier of its parent model
     */
    virtual string parentId() const = 0;
  };

  /*!
   * \brief The IComponentStatusChangeEventArgs contains the information that will
   * be passed when the IModelComponent fires a signal.
   * \details Sending exchange item events is optional, so it should not be used as a mechanism to build critical functionality upon.
   */
  class IComponentStatusChangeEventArgs
  {
  public:
    /*!
     * \brief ~IComponentStatusChangeEventArgs destructor
     */
    virtual ~IComponentStatusChangeEventArgs() = default;

    /*!
     * \brief Gets the IModelComponent that fired the event.
     * \returns The IModelComponent that threw the event.
     */
    virtual IModelComponent *component() const = 0;

    /*!
     * \brief Gets the IModelComponent's status before the status change.
     * \returns The previous ComponentStatus of the component that threw the event.
     */
    virtual IModelComponent::ComponentStatus previousStatus() const = 0;

    /*!
     * \brief Gets the IModelComponent's status after the status change.
     * \returns The new ComponentStatus of the component that threw the event.
     */
    virtual IModelComponent::ComponentStatus status() const = 0;

    /*!
     * \brief Gets additional information about the status change.
     */
    virtual string message() const = 0;

    /*!
     * \brief A bool indicating whether this event has a progresss monitor.
     * \returns True if status has a percent progress otherwise false and the progress bar shows busy.
     */
    virtual bool hasProgressMonitor() const = 0;

    /*!
     * \brief Number between 0 and 100 indicating the progress made by a component in its simulation.
     * \returns A number between 0 and 100 indicating the progress made by a component.
     */
    virtual float percentProgress() const = 0;
  };

  /*!
   * \brief The ICloneableModelComponent class
   */
  class ICloneableModelComponent : public virtual IModelComponent
  {
  public:
    /*!
     * \brief ~ICloneableModelComponent destructor
     */
    virtual ~ICloneableModelComponent() = default;

    /*!
     * \brief Parent ICloneableModelComponent object from which current component was cloned from.
     * \returns The parent ICloneableModelComponent from which the current component was created.
     */
    virtual ICloneableModelComponent *parent() const = 0;

    /*!
     * \brief Deep clones itself including cloning its Data::IArgument.
     * \param clone_optional_arguments are optional arguments that can be passed to the clone method. These arguments are used to
     * pass additional information to the clone method. The arguments are specific to the component being cloned.
     * \returns A deep clone of the current component. Configuration files and output files
     * must be written to a different location than those of the parent. Cloning can only occur after the parent component has been
     * initialized successfully. Cloned components must also be initialized.
     */
    virtual ICloneableModelComponent *clone(const unordered_map<string, hydrocouple_variant> &clone_optional_arguments = unordered_map<string, hydrocouple_variant>()) = 0;

    /*!
     * \brief A vector ICloneableModelComponent instances cloned from this IModelComponent instance.
     * \returns A vector of child components created from the current component.
     */
    virtual vector<ICloneableModelComponent *> clones() const = 0;
  };

  /*!
   * \brief IValueDefinition describes a value returned by the getValues() function of IValueSet.
   *
   * \details This interface is not meant to be implemented directly.
   * Instead, implement either IQuality or IQuantity or a
   * custom derived vale definition interface.
   */
  class IValueDefinition : public virtual IDescription
  {
  public:
    /*!
     * \brief ~IValueDefinition destructor
     */
    virtual ~IValueDefinition() = default;

    /*!
     * \brief The object types of value that will be available
     * and is returned by the GetValues function.IPropertyChanged.
     *
     * \returns the value type associated with this IValueDefinition
     */
    virtual type_info type() const = 0;

    //! The value representing that data is missing.
    virtual hydrocouple_variant missingValue() const = 0;

    //! Gets the default value of the argument.
    virtual hydrocouple_variant defaultValue() const = 0;
  };

  /*!
   * \brief IDimension provides the properties of the dimensions of a variable.
   */
  class IDimension : public virtual IIdentity
  {
  public:
    /*!
     * \brief ~IDimension destructor
     */
    virtual ~IDimension() = default;
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
   *   - Colors:
   *      - red
   *      - green
   *      - blue
   *   - Land use:
   *      - nature
   *      - recreation
   *      - industry3
   *      - infrastructure
   *   - Rating:
   *      - worse
   *      - same
   *      - better
   */
  class IQuality : public virtual IValueDefinition
  {
  public:
    /*!
     * \brief IQuality::~IQuality is a virtual destructor.
     */
    virtual ~IQuality() = default;

    /*!
     * \returns A list of the possible ICategory allowed for this IQuality
     * If the quality is not ordered the list contains the ICategory's in an unspecified order.
     * When it is ordered the list contains the ICategory's in the same sequence.
     */
    virtual set<hydrocouple_variant> categories() const = 0;

    /*!
     * \brief Checks if the IQuality is defined by an ordered set of ICategory or not.
     */
    virtual bool isOrdered() const = 0;
  };

  /*!
   * \brief Defines the order of dimension in each FundamentalDimension for a unit.
   */
  class IUnitDimensions : public virtual IDescription
  {
  public:
    /*!
     * \brief HydroCouple::FundamentalUnitDimension are the fundamental units that can be combined to form all types of units.
     */
    enum FundamentalUnitDimension
    {
      /*!
       * \brief Fundamental dimension for length.
       */
      Length,

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
      Currency,

      /*!
       * \brief Fundamental dimension for unitless quantities.
       */
      Unitless,
    };

    /*!
     * \brief IUnitDimensions::~IUnitDimensions is a virtual destructor.
     */
    virtual ~IUnitDimensions() = default;

    /*!
     * \brief Returns the power for the requested dimension.
     *
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
    virtual double getPower(HydroCouple::IUnitDimensions::FundamentalUnitDimension dimension) = 0;
  };

  /*!
   * \brief IUnit interface, describing the physical unit of a IQuantity.
   */
  class IUnit : public virtual IDescription
  {
  public:
    /*!
     * \brief HydroCouple::DistanceUnitType are the types of units that can be used to measure distance.
     */
    enum class DistanceUnitType
    {
      Standard,
      Geographic,
      Unknown
    };

    /*!
     * \brief HydroCouple::DistanceUnits are the types of units that can be used to measure distance.
     */
    enum class DistanceUnits
    {
      Meters,
      Kilometers,
      Feet,
      NauticalMiles,
      Yards,
      Miles,
      Degrees,
      Centimeters,
      Millimeters,
      Inches,
      Unknown
    };

    /*!
     * \brief HydroCouple::AreaUnits are the types of units that can be used to measure area.
     */
    enum AreaUnits
    {
      SquareMeters,
      SquareKilometers,
      SquareFeet,
      SquareYards,
      SquareMiles,
      Hectares,
      Acres,
      SquareNauticalMiles,
      SquareDegrees,
      SquareCentimeters,
      SquareMillimeters,
      SquareInches,
      Unknown
    };

    /*!
     * \brief IUnit::~IUnit is a virtual destructor.
     */
    virtual ~IUnit() = default;

    /*!
     * \brief Fundamental dimensions of the unit.
     */
    virtual IUnitDimensions *dimensions() const = 0;

    /*!
     * \brief Conversion factor to SI ('A' in: SI-value = A * quant-value + B)
     */
    virtual double conversionFactorToSI() const = 0;

    /*!
     * \brief OffSet to SI ('B' in: SI-value = A * quant-value + B).
     */
    virtual double offsetToSI() const = 0;
  };

  /*!
   * \brief IQuantity specifies values as an amount
   * of some unit, usually as a floating point number.
   */
  class IQuantity : public virtual IValueDefinition
  {
  public:
    virtual ~IQuantity() = default;

    /*!
     * \brief Unit of quantity.
     */
    virtual IUnit *unit() const = 0;

    /*!
     * \brief minValue
     * \return
     */
    virtual hydrocouple_variant minValue() const = 0;

    /*!
     * \brief maxValue
     * \return
     */
    virtual hydrocouple_variant maxValue() const = 0;
  };

  /*!
   * \brief IComponentItem is a fundamental unit of data for a component.
   *
   * \details This interface is not to be implemented directly. Input and output data must be 1D arrays indexed
   * using dim1 + dim2*size1 + dim3*size1*size2 + dim4*size1*size2*size3 + ...
   */
  class IComponentDataItem : public virtual IIdentity
  {
  public:
    /*!
     * \brief IComponentDataItem::~IComponentDataItem is a virtual destructor.
     */
    virtual ~IComponentDataItem() = default;

    /*!
     * \brief Gets the owner IModelComponent of this IComponentItem.
     * For an IOutput component item this is the component
     * responsible for providing the content of the IOutput.
     *
     * \details It is possible for an IComponentItem to have no owner, in this case the method will return nullptr.
     *
     * \return an IModelComponent object that is the parent of this IComponentItem
     */
    virtual IModelComponent *modelComponent() const = 0;

    /*!
     * \brief provides purely descriptive information of the dimensions associated with this IComponentItem
     *
     * \return A list of IDimension objects.
     *
     */
    virtual vector<IDimension *> dimensions() const = 0;

    /*!
     * \brief dimensionLength  returns the length of the dimension specified by the
     * given dimension indexes. To get the size of the first dimension, use a null
     * integer array as input argument. Length of indices must be a least one
     * smaller than the numDimensions()
     * \param dimensionIndexes array of indexes of the dimensions to get the length of. Its size must be
     * less than the number of dimensions.
     * \return length of the last dimension corresponding to the dimensionIndexes provided.
     */
    virtual int dimensionLength(const vector<int> &dimensionIndexes) const = 0;

    /*!
     * \brief IValueDefinition for this IValueSet defines the variable type associated with this object.
     * \returns the variable definition for this variable. This is either a
     */
    virtual IValueDefinition *valueDefinition() const = 0;

    /*!
     * \brief Gets a multi-dimensional array of value for given dimension indexes.
     * IndexArray = x + y * InSizeX + z * InSizeX * InSizeY etc;
     * \param dimensionIndexes are the indexes for the data to be obtained.
     * \param data Pointer to pre-allocated location where data is to be saved.
     */
    virtual void getValue(const vector<int> &dimensionIndexes, void *data) const = 0;

    /*!
     * \brief Sets a multi-dimensional array of values for given dimension indexes.
     * \param dimensionIndexes are the indexes for where data is to be written.
     * \param data is the pointer to the input data to be set.
     */
    virtual void setValue(const vector<int> &dimensionIndexes, const void *data) = 0;

    /*!
     * \brief hasEditor indicates whether this IComponentItem has a UI editor.
     * \return A boolean indicating whether this IComponentItem has an editor.
     */
    virtual bool hasEditor() const = 0;

    /*!
     * \brief showEditor shows the editor for this IComponentItem.
     * \param opaqueUIPointer Is an opaque pointer to the UI object that is used to show the editor.
     */
    virtual void showEditor(void* opaqueUIPointer = nullptr) = 0;

    /*!
     * \brief hasViewer indicates whether this IComponentItem has a UI viewer.
     * \return  A boolean indicating whether this IComponentItem has a viewer.
     */
    virtual bool hasViewer() const = 0;

    /*!
     * \brief showViewer shows the viewer for this IComponentItem.
      * \param opaqueUIPointer Is an opaque pointer to the UI object that is used to show the viewer.
     */
    virtual void showViewer(void* opaqueUIPointer = nullptr) = 0;
  };

  /*!
   * \brief IArgument interface class used to set the arguments for components.
   * They can be complex or simple multi-dimensional datasets
   *
   * \details IArgument is primarily used to set arguments of IModelComponent and IAdaptedOutput
   */
  class IArgument : public virtual IComponentDataItem
  {

  public:
    /*!
     * \brief Enumeration indicating the type of input for the argument.
     */
    enum ArgumentInputType
    {
      /*!
       * \brief Enumeration indicating that the argument was read from string.
       */
      String,

      /*!
       * \brief Enumeration indicating that the argument was read from a file.
       */
      File,

      /*
       * \brief Enumeration indicating that the argument was read from a URL.
       */
      JSON,

      /*!
       * \brief Enumeration indicating that the argument was read from a file.
       */
      XML,

      /*!
       * \brief Enumeration indicating that the argument was read from a URL.
       */
      URL,

      /*!
       * \brief Enumeration indicating that the argument was read from a memory object.
       */
      MEMORY_OBJECT
    };

    /*!
     * \brief IArgument::~IArgument is a virtual destructor.
     */
    virtual ~IArgument() = default;

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
     * \brief String/XML representation for this IArgument
     */
    virtual string toString() const = 0;

    /*!
     * \brief Writes data to files associated with this argument if they exist.
     * \return
     */
    virtual void saveData() = 0;

    /*!
     * \brief File type extensions that can be read by this IArgument.
     * \details  File extensions must be specified using the Qt format e.g. "Images (*.png *.xpm *.jpg) "
     * \returns a list of strings for compatible file extensions.
     */
    virtual list<string> fileFilters() const = 0;

    /*!
     * \brief Valid IComponentDataItem instance types that can be read by this argument.
    */
    virtual list<type_info> validComponentDataItemTypes() const = 0;

    /*!
     * \brief Boolean indicating whether this IArgument copy its values from a string.
     */
    virtual bool isValidArgType(ArgumentInputType argType) const = 0;

    /*!
     * \brief argumentIOType
     * \return
     */
    virtual ArgumentInputType currentArgumentInputType() const = 0;

    /*!
     * \brief Reads values from a JSON string.
     * \param value is a string representing values in JSON format.
     * \param argType is the type of input to be read.
     * \param message message returned from file read operation.
     * \return boolean indicating whether file/string reading was successful
     */
    virtual bool initialize(const string &value, ArgumentInputType argType, string &message) = 0;

    /*!
     * \brief Reads values from an equivalent IComponentDataItem. IComponentDataItem has been used instead of IArgument
     * so that outputs from one model can be used as initialization arguments for another.
     * \param componentDataItem is the IArgument from which to copy values from.
     * \param message message returned from file read operation.
     * \return boolean indicating whether file reading was successful.
     */
    virtual bool initialize(const IComponentDataItem *componentDataItem, string &message) = 0;
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
    /*!
     * \brief Standard destructor.
     */
    virtual ~IExchangeItemChangeEventArgs() = default;

    /*!
     * \brief IExchangeItem which fired the signal.
     */
    virtual IExchangeItem *exchangeItem() const = 0;

    /*!
     * \brief Gets message associated with the event.
     */
    virtual string message() const = 0;
  };

  /*!
   * \brief IExchangeItem the base data item the can be exchanged between components at runtime.
   *
   * \details This interface is not to be implemented directly, any class is to implement either the IInput or IOutput.
   */
  class IExchangeItem : public virtual IComponentDataItem
  {

  public:
    /*!
     * \brief IExchangeItem::~IExchangeItem is a virtual destructor.
     */
    virtual ~IExchangeItem() = default;

    /*!
     * \brief The componentItemChanged event is fired when
     * the content of an IComponentItem has changed.
     *
     * \param statusChangedEvent provides information about the status change
     */
    virtual void registerExchangeItemStatusChangedListener(const function<void(const shared_ptr<IExchangeItemChangeEventArgs> &)> &exchangeItemChangedEventListener) = 0;

    /*!
     * \brief The deRegisterExchangeItemStatusChangedListener() method deregisters a listener that is
     * called when the status of the component changes.
     *
     * \details See HydroCouple::ComponentStatus for the possible states.
     */
    virtual void deRegisterExchangeItemStatusChangedListener(const function<void(const shared_ptr<IExchangeItemChangeEventArgs> &)> &exchangeItemChangedEventListener) = 0;
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
    /*!
     * \brief IOutput::~IOutput is a virtual destructor.
     */
    virtual ~IOutput() = default;

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
    virtual vector<IInput *> consumers() const = 0;

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
    virtual void addConsumer(IInput *consumer) = 0;

    /*!
     * \brief Remove a consumer.
     *
     * \details If an input item is not interested any longer in calling the IValueSet::getValue() method,
     *  it should remove itself by calling RemoveConsumer.
     *
     * \param consumer that has to be removed
     *
     */
    virtual bool removeConsumer(IInput *consumer) = 0;

    /*!
     * \brief The adaptedOutputs that have this current output item as adaptee.
     *
     * \details As soon as the output item's values have been updated, for each adaptedOutput its
     * IAdaptedOutput.refresh() method must be called.
     *
     * \details The list is readonly. Add and remove from the list by using addAdaptedOutput() and removeAdaptedOutput().
     *
     */
    virtual vector<IAdaptedOutput *> adaptedOutputs() const = 0;

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
    virtual void addAdaptedOutput(IAdaptedOutput *adaptedOutput) = 0;

    /*!
     * \brief Removes an IAdaptedOutput.
     *
     * \details If a adaptedOutput is not interested any longer in this output item data,
     * it should remove itself by calling removeConsumer().
     *
     * \param adaptedOutput is a consumer that has to be removed.
     *
     */
    virtual bool removeAdaptedOutput(IAdaptedOutput *adaptedOutput) = 0;

    /*!
     * \brief Provides the values matching the value definition specified by the
     * <param name="querySpecifier"></param>. Extensions can overwrite this base version to include
     * more details in the query, e.g. time and space.
     *
     * \details One might expect to be the querySpecifier to be of the type IInput, because every input item that calls
     * the getValues method needs to add itself as a consumer first.
     *
     * \details However, the IExchangeItem suffices to  specify what is required. Therefore,
     * to have the flexibility to loosen the "always register as consumer" approach, it is chosen to provide
     * an IExchangeItem as an argument.
     *
     * \param querySpecifier
     */
    virtual void updateValues(IInput *querySpecifier) = 0;
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
    /*!
     * \brief IAdaptedOutput::~IAdaptedOutput is a virtual destructor.
     */
    virtual ~IAdaptedOutput() = default;

    /*!
     *
     * \brief IAdaptedOutputFactory that generated this IAdaptedOutput.
     *
     * \returns IAdaptedOutputFactory parent.
     *
     */
    virtual IAdaptedOutputFactory *adaptedOutputFactory() const = 0;

    /*!
     * \brief  IArgument represents input parameters needed for this IAdaptedOutput.
     *
     * \details An unmodifiable vector of the (modifiable) IArguments should be returned that can be used to
     * get information on an IArgument and to modify its values. Validation of changes is done when they occur (e.g. using notifications).
     *
     * \returns Unmodifiable list of IArgument for the adapted output.
     */
    virtual vector<IArgument *> arguments() const = 0;

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
    virtual IOutput *adaptee() const = 0;

    /*!
     * \brief Requests the IAdaptedOutput to refresh itself and perform any necessary calculations.
     *
     * \details This method will be called by the adaptee(), when it has been refreshed/updated.
     * In the implementation of the refresh method the adapted output should update its contents
     * according to the cha
     * \details After updating itself the IAdaptedOutput must call refresh() on all
     * its IAdaptedOutput children, nges in the adaptee.
     *so the chain of IOutput 's refreshes themselves.
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
    /*!
     * \brief IAdaptedOutputFactory::~IAdaptedOutputFactory is a virtual destructor.
     */
    virtual ~IAdaptedOutputFactory() = default;

    /*!
     * \brief Get a vector of IIdentity objects representing the vector of IAdaptedOutput instances that can be created by this factory.
     * of the available IAdaptedOutput that can make the producer match the consumer.
     *
     * \details If the consumer is NULL, the identifiers of all IAdaptedOutputs
     * that can adapt the producer are returned.
     *
     * \param provider is the IOutput to adapt.
     * \param consumer is the IInput to adapt the producer to, can be NULL.
     * \returns A vector of identifiers for the available IAdaptedOutputs.
     */
    virtual vector<IIdentity *> getAvailableAdaptedOutputIds(const IOutput *provider, const IInput *consumer = nullptr) = 0;

    /*!
     * \brief Creates a IAdaptedOutput that adapts the producer so that it fits the consumer.
     *
     * \details The adaptedProviderId used must be one of the IIdentifier instances
     * returned by the createAdaptedProducerItem method. The returned IAdaptedOutputs
     * will already be registered with the producer.
     * \param adaptedProviderId is an identifier of the IAdaptedOutput to create.
     * \param provider IOutput to adapt.
     * \param consumer IInput to adapt the adaptee to.
     * \returns An IAdaptedOutput.
     */
    virtual IAdaptedOutput *createAdaptedOutput(IIdentity *adaptedProviderId, IOutput *provider, IInput *consumer = nullptr) = 0;
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
    /*!
     * \brief IAdaptedOutputFactoryComponentInfo::~IAdaptedOutputFactoryComponentInfo is a virtual destructor.
     */
    virtual ~IAdaptedOutputFactoryComponentInfo() = default;

    /*!
     * \brief New IAdaptedOutputFactoryComponent instance.
     */
    virtual IAdaptedOutputFactoryComponent *createComponentInstance() = 0;
  };

  /*!
   * \brief IAdaptedOutputFactoryComponent is an
   * IAdaptedOutputFactory generated from an IAdaptedOutputFactoryComponentInfo.
   *
   */
  class IAdaptedOutputFactoryComponent : public virtual IAdaptedOutputFactory
  {
  public:
    /*!
     * \brief IAdaptedOutputFactoryComponent::~IAdaptedOutputFactoryComponent is a virtual destructor.
     */
    virtual ~IAdaptedOutputFactoryComponent() = default;

    /*!
     * \brief Contains the metadata about the IModelComponent.
     *
     * \details This information includes the developer, component version number, contact url etc.
     */
    virtual IAdaptedOutputFactoryComponentInfo *componentInfo() const = 0;
  };

  /*!
   * \brief An IInput item that can accept values for an IModelComponent.
   */
  class IInput : public virtual IExchangeItem
  {
  public:
    /*!
     * \brief IInput::~IInput is a virtual destructor.
     */
    virtual ~IInput() = default;

    /*!
     * \brief Gets the producer this consumer should get its values from.
     */
    virtual IOutput *provider() const = 0;

    /*!
     * \brief Sets the producer this consumer should get its values from.
     *
     * \param provider is the IOutput that supplies the data to this IInput.
     */
    virtual bool setProvider(IOutput *provider) = 0;

    /*!
     * \brief Returns true if this IInput can consume this producer.
     *
     * \param provider is the IOutput that can supply the data to this IInput.
     * \param message The error message from the canConsume function.
     */
    virtual bool canConsume(IOutput *provider, string &message) const = 0;
  };

  /*!
   * \brief The IMultiInput class is an IInput class that has multiple outputs supplying data to it.
   */
  class IMultiInput : public virtual IInput
  {
  public:
    /*!
     * \brief IMultiInput::~IMultiInput is a virtual destructor.
     */
    virtual ~IMultiInput() = default;

    /*!
    * \return vector of identifiers for the provides that a required by this consumer if any.
    */
    virtual vector<IIdentity*> providerLabels() const = 0;

    /*!
     * \brief isRequiredProvider checks if the provider is required by the consumer.
     * \param providerLabel is the IIdentity label specifying where to add the provider.
     * \return boolean indicating whether the provider is required by the consumer.
     */
    virtual bool isRequiredProvider(const IIdentity* providerLabel) const = 0;

    /*!
     * \brief providers
     * \return
     */
    virtual vector<IOutput *> providers() const = 0;

    /*!
     * \brief canConsume checks if the provider can supply data to this consumer.
     * \param provider is the IOutput that can supply the data to this IInput.
     * \param message is the error message from the canConsume function.
     * \param providerRoleIdentifier is the IIdentity label specifying where to add the provider.
     * \return boolean indicating whether the provider can supply data to this consumer.
     */
    virtual void canConsume(IOutput *provider, string &message, const IIdentity* providerRoleIdentifier = nullptr) const = 0;

    /*!
     * \brief addProvider adds a provider to the list of providers.
     * \param provider is the IOutput to add to the list of providers.
     * \param id is the IIdentity label specifying where to add the provider.
     */
    virtual bool addProvider(IOutput *provider, const IIdentity* providerRoleIdentifier = nullptr) = 0;

    /*!
     * \brief removeProvider
     * \param provider
     */
    virtual bool removeProvider(IOutput *provider) = 0;
  };

  /*!
   * \brief The IIdBasedComponentItem class is an idbased IComponentItem
   */
  class IIdBasedComponentDataItem : public virtual IComponentDataItem
  {
    using IComponentDataItem::getValue;
    using IComponentDataItem::setValue;

  public:
    /*!
     * \brief IIdBasedComponentItem::~IIdBasedComponentItem is a virtual destructor.
     */
    virtual ~IIdBasedComponentDataItem() = default;

    /*!
     * \brief identifiers
     * \return
     */
    virtual vector<string> identifiers() const = 0;

    /*!
     * \brief idDimensions
     * \return
     */
    virtual IDimension *identifierDimension() const = 0;

    /*!
     * \brief Gets a multi-dimensional array of values for given
     *  id dimension index and size for a hyperslab.
     * \param idIndex is the id dimension index from where to obtain the requested data.
     * \param data is pre-allocated memory where the data will be written.
     */
    virtual void getValue(int idIndex, void *data) const = 0;

    /*!
     * \brief Gets a multi-dimensional array of values for given id dimension index and size for a hyperslab.
     * \param idIndex is the id dimension index from where to obtain the requested data.
     * \param stride is the size for hyperslab from which to copy data.
     * \param data is a 1d array where data is to be written. Must be allocated beforehand with the correct data type.
     */
    virtual void getValues(int idIndex, int stride, void *data) const = 0;

    /*!
     * \brief Sets a multi-dimensional array of values for given time
     *  dimension index and size for a hyperslab.
     * \param idIndex is the id dimension index where data is to be written.
     * \param data is the pre-allocated location where data is to be set.
     */
    virtual void setValue(int idIndex, const void *data) = 0;

    /*!
     * \brief Sets a multi-dimensional array of values for given
     * id dimension index and size for a hyperslab.
     * \param idIndex is the id dimension index where data is to be written.
     * \param stride is the size for hyperslab where data is to be written.
     * \param data is the input 1d array to be written.
     */
    virtual void setValues(int idIndex, int stride, const void *data) = 0;
  };

  /*!
   * \brief The IDataExchangeWorkflowComponentInfo class
   */
  class IWorkflowComponentInfo : public virtual IComponentInfo
  {

  public:
    /*!
     * \brief ~IWorkflowComponentInfo
     */
    virtual ~IWorkflowComponentInfo() = default;

    /*!
     * \brief Creates a new IModelComponent instance.
     * \returns A new instance of an IModelComponent.
     */
    virtual IWorkflowComponent *createComponentInstance() = 0;
  };

  /*!
   * \brief The IDataExchangeWorkflowComponent class
   */
  class IWorkflowComponent : public virtual IIdentity, public virtual ISignal<const shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>
  {

  public:
    /*!
     * \brief The WorkflowStatus enum
     */
    enum WorkflowStatus
    {
      Created,
      Initializing,
      Initialized,
      Updating,
      Updated,
      Done,
      Finishing,
      Finished,
      Failed
    };

    /*!
     * \brief ~IWorkflowComponent destructor for IWorkflowComponent class.
     */
    virtual ~IWorkflowComponent() = default;

    /*!
     * \brief componentInfo
     * \return
     */
    virtual IWorkflowComponentInfo *componentInfo() const = 0;

    /*!
     * \brief requiredModelComponentIdentifiers returns the vector of IModelComponent identifiers that are required by this component.
     * \return A vector of IModelComponent identifiers that are required by this component.
     */
    virtual vector<IIdentity *> modelComponentLabels() const = 0;

    /*!
     * \brief isRequiredModelComponent checks if the model component is required by this component.
     * \param modelComponentLabel is the IIdentity label specifying the model component.
     * \return boolean indicating whether the model component is required by this component.
     */
    virtual bool isRequiredModelComponent(const IIdentity* modelComponentLabel) const = 0;

    /*!
     * \brief initialize
     */
    virtual void initialize() = 0;

    /*!
     * \brief update
     */
    virtual void update() = 0;

    /*!
     * \brief finish
     */
    virtual void finish() = 0;

    /*!
     * \brief status
     * \return
     */
    virtual WorkflowStatus status() const = 0;

    /*!
     * \brief modelComponents
     * \return
     */
    virtual vector<IModelComponent *> modelComponents() const = 0;

    /*!
     * \brief addModelComponent Adds model component instance to workflow
     * \param component is the IModelComponent to add to the workflow.
     * \param modelRoleIdentifier is the IIdentity of the role of the model component. If null, the component is added as a standalone component.
     * in which case the workflow likely does not require ordered or specific components for its operation.
     * \return True if the component was added successfully, otherwise false.
     */
    virtual bool addModelComponent(IModelComponent *component, const IIdentity* modelRoleIdentifier = nullptr) = 0;

    /*!
     * \brief removeModelComponent Removes model component instance from workflow 
     * \param component is the IModelComponent to remove from the workflow.
     * \return True if the component was removed successfully, otherwise false.
     */
    virtual bool removeModelComponent(IModelComponent *component) = 0;

  };

  /*!
   * \brief The IWorkflowComponentStatusChangeEventArgs contains the information that will
   * be passed when the IWorkflowComponent fires a signal.
   */
  class IWorkflowComponentStatusChangeEventArgs
  {
  public:
    /*!
     * \brief ~IComponentStatusChangeEventArgs destructor
     */
    virtual ~IWorkflowComponentStatusChangeEventArgs() = default;

    /*!
     * \brief Gets the IModelComponent that fired the event.
     * \returns The IModelComponent that threw the event.
     */
    virtual IWorkflowComponent *workflowComponent() const = 0;

    /*!
     * \brief Gets the IWorkflowComponent's status before the status change.
     * \returns The previous ComponentStatus of the component that threw the event.
     */
    virtual IWorkflowComponent::WorkflowStatus previousStatus() const = 0;

    /*!
     * \brief Gets the IWorkflowComponent's status after the status change.
     * \returns The new ComponentStatus of the component that threw the event.
     */
    virtual IWorkflowComponent::WorkflowStatus status() const = 0;

    /*!
     * \brief Gets additional information about the status change.
     */
    virtual string message() const = 0;

    /*!
     * \brief A bool indicating whether this event has a progresss monitor.
     * \returns True if status has a percent progress otherwise false and the progress bar shows busy.
     */
    virtual bool hasProgressMonitor() const = 0;

    /*!
     * \brief Number between 0 and 100 indicating the progress made by a component in its simulation.
     * \returns A number between 0 and 100 indicating the progress made by a component.
     */
    virtual float percentProgress() const = 0;
  };

}

#endif // HYDROCOUPLE_H
