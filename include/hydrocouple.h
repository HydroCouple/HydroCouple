/*!
 * \file hydrocouple.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0
 * \brief Core interface definitions for the HydroCouple component-based modeling framework.
 * \details This header file contains the core interface definitions for the
 * HydroCouple component-based modeling framework. It defines the fundamental
 * abstractions for model components, exchange items, signals/slots, value
 * definitions, dimensions, arguments, and workflow management.
 * \license
 * This file and its associated files and libraries are free software.
 * You can redistribute them and/or modify them under the terms of the
 * MIT License. They are distributed in the hope that they will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the MIT License for details.
 * \copyright Copyright 2014-2026, Caleb Buahin, All rights reserved.
 * \date 2014-2026
 */

#ifndef HYDROCOUPLE_H
#define HYDROCOUPLE_H

#include <string>
#include <array>
#include <vector>
#include <span>
#include <cstdint>
#include <functional>
#include <list>
#include <set>
#include <memory>
#include <typeinfo>
#include <unordered_map>


/*!
 * \brief The ByteOrder enum class indicates the byte order of serialized data.
 */
enum class ByteOrder : uint8_t
{

  /*!
   * \brief BigEndian serialized data byte order (most significant byte first).
   */
  BigEndian = 0,

  /*!
   * \brief LittleEndian serialized data byte order (least significant byte first).
   */
  LittleEndian = 1
};

/*!
 * \brief HydroCouple namespace contains the core interface specifications
 * for the HydroCouple component-based modeling framework interface specification.
 */
namespace HydroCouple
{
  //! ABI version for the HydroCouple interface.
  constexpr int HYDROCOUPLE_ABI_VERSION = 2;

  //! Forward declarations
  template <typename... Args>
  class ISignal;
  class IComponentDataItem;
  class IModelComponent;
  class IAdaptedOutputFactory;
  class IArgument;
  class IInput;
  class IOutput;
  class IExchangeItem;
  class IAdaptedOutput;
  class IAdaptedOutputFactoryComponent;
  class IUnit;
  class IComponentDataItemValueChanged;
  class IComponentStatusChangeEventArgs;
  class IWorkflowComponent;
  class IWorkflowComponentStatusChangeEventArgs;

  /*!
   * \brief DataKind identifies the element type of a typed data buffer.
   * \details This is the type vocabulary of the data-exchange plane. String and Opaque
   * are host-only kinds: String buffers point to arrays of std::string, Opaque buffers
   * carry implementation-defined bytes whose meaning both endpoints must agree on.
   */
  enum class DataKind : uint8_t
  {
    Unknown = 0, //!< No/unknown element type.
    Int8,        //!< int8_t
    UInt8,       //!< uint8_t
    Int16,       //!< int16_t
    UInt16,      //!< uint16_t
    Int32,       //!< int32_t
    UInt32,      //!< uint32_t
    Int64,       //!< int64_t
    UInt64,      //!< uint64_t
    Float32,     //!< float
    Float64,     //!< double
    Boolean,     //!< bool (stored one element per byte)
    String,      //!< std::string (host memory only)
    Opaque       //!< Implementation-defined bytes (element size must be agreed out of band).
  };

  /*!
   * \brief MemorySpace identifies where a buffer's bytes physically live.
   * \details Vendor-neutral by design: no CUDA/HIP/SYCL types appear in this standard.
   * The pairing of MemorySpace and device id is resolved to a concrete runtime by the
   * implementing SDK's execution backend.
   */
  enum class MemorySpace : uint8_t
  {
    Host = 0,   //!< Ordinary pageable host memory.
    HostPinned, //!< Page-locked host memory (fast staging to/from devices).
    Device,     //!< Accelerator-resident memory addressed by (backend, deviceId).
    Unified     //!< Unified/managed memory accessible from host and device.
  };

  /*!
   * \brief BufferDescriptor describes a typed, possibly strided, possibly device-resident
   * multi-dimensional array. It is the sole currency of field data exchange.
   *
   * \details The layout model follows the DLPack/NumPy buffer protocol: element
   * (i0, i1, ..., i[rank-1]) lives at data + sum(ik * stridesBytes[k]). A null
   * stridesBytes means C-contiguous row-major layout. The descriptor does not own
   * its memory or its shape/stride arrays; the caller guarantees they outlive the call.
   * Dimension *semantics* (which axis is time, entity, layer, ...) are supplied by the
   * owning IComponentDataItem's dimensions() metadata, not by this struct.
   *
   * \details This is a plain aggregate: the interface standard carries no executable
   * code. Non-normative helper functions for descriptors (element counts, contiguity
   * checks, byte offsets, factories) are provided separately in hydrocouplehelpers.h.
   */
  struct BufferDescriptor
  {
    void          *data = nullptr;         //!< Base address of element (0, 0, ..., 0).
    DataKind       kind = DataKind::Unknown; //!< Element type.
    int32_t        rank = 0;               //!< Number of dimensions; 0 denotes a scalar.
    const int64_t *shape = nullptr;        //!< Extent per dimension; length == rank.
    const int64_t *stridesBytes = nullptr; //!< Byte step per dimension; nullptr => C-contiguous.
    MemorySpace    space = MemorySpace::Host; //!< Memory space holding the bytes.
    int32_t        deviceId = 0;           //!< Device ordinal when space is Device/Unified.
  };

  /*!
   * \brief Capability identifies an optional behavior a component may support.
   * \details Orchestrators query IModelComponent::capabilities() and branch on the
   * result instead of chains of dynamic_cast probes.
   */
  enum class Capability : uint32_t
  {
    DeviceBuffers = 0,    //!< Data items can produce/accept Device/Unified BufferDescriptors.
    PartitionedData,      //!< Component exposes partitioned data items (see hydrocoupledistributed.h).
    DistributedExecution, //!< Component implements IDistributedModelComponent.
    Checkpointing,        //!< Component implements ICheckpointableModelComponent.
    Cloneable,            //!< Component implements ICloneableModelComponent.
    UserInterface,        //!< Component implements IUIProvider.
    Licensing             //!< Component implements ILicensedComponent.
  };

  /*!
   * \brief ErrorEntry is one diagnostic record in a component's error queue.
   * \details The error queue is the normative failure channel for distributed and
   * embedded execution, where exceptions cannot cross process, C-ABI, or language
   * boundaries. Exceptions remain a local convenience.
   */
  struct ErrorEntry
  {
    /*!
     * \brief Severity of an ErrorEntry.
     */
    enum class Severity : uint8_t
    {
      Information = 0, //!< Informational message.
      Warning,         //!< Recoverable anomaly.
      Error,           //!< Operation failed; component may continue.
      Fatal            //!< Component is in the Failed state.
    };

    Severity    severity = Severity::Information; //!< Severity of this record.
    int32_t     code = 0;                         //!< Implementation-defined error code.
    std::string source;                           //!< Id of the originating entity.
    std::string message;                          //!< Human-readable description.
  };

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
     * \param[in] sender is the object that emitted the signal.
     * \param[in] args are the arguments passed by the signal.
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
     * \param[in] slot is the slot that will listen to the signal.
     */
    virtual void connect(const std::shared_ptr<ISlot<Args...>> &slot) = 0;

    /*!
     * \brief disconnect is used to disconnect a slot from the signal.
     * \param[in] slot is the slot that will be disconnected from the signal.
     */
    virtual void disconnect(const std::shared_ptr<ISlot<Args...>> &slot) = 0;

    /*!
     * \brief blockSignals is used to block signals from being emitted.
     * \param[in] block is a boolean value that is used to specify if signals should be blocked or not.
     */
    virtual void blockSignals(bool block) = 0;

  protected:
    /*!
     * \brief emit is used to emit the signal.
     * \param[in] args are the arguments that will be passed by the signal.
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
    [[nodiscard]] virtual const std::string &caption() const = 0;

    /*!
     * \brief Sets caption for the entity.
     * \param[in] caption is a string representing the caption for the entity.
     * \sa caption()
     */
    virtual void setCaption(const std::string &caption) = 0;

    /*!
     * \brief Gets additional descriptive information for the entity.
     * \returns string description of entity.
     * \sa setDescription()
     */
    [[nodiscard]] virtual const std::string &description() const = 0;

    /*!
     * \brief Gets additional descriptive information for the entity.
     * \param[in] description is a string for describing an entity.
     * \sa description()
     */
    virtual void setDescription(const std::string &description) = 0;
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
    [[nodiscard]] virtual const std::string &id() const = 0;
  };

  /*!
   * \brief IComponentInfo interface class is a factory that provides detailed metadata
   * about a component and creates new instances of a component.
   *
   * \details It must not be implemented directly. It must be implemented as an
   * IModelComponentInfo, an IAdaptedOutputFactoryComponentInfo, or an IWorkflowComponentInfo.
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
    [[nodiscard]] virtual std::string libraryFilePath() const = 0;

    /*!
     * \brief Sets file path to Component library.
     * \param filePath to the library from which this component was created.
     * \sa libraryFilePath()
     */
    virtual void setLibraryFilePath(const std::string &filePath) = 0;

    /*!
     * \brief File path to Component icon.
     * Must be specified relative to the component library.
     *
     * \returns filePath to icon for component.
     */
    [[nodiscard]] virtual std::string iconFilePath() const = 0;

    /*!
     * \brief Component developer information.
     * \returns Name of developer/vendor the developed this component.
     */
    [[nodiscard]] virtual std::string developer() const = 0;

    /*!
     * \brief Documentation associated with this component.
     * \returns Citations of publication related to this component.
     */
    [[nodiscard]] virtual std::vector<std::string> documentation() const = 0;

    /*!
     * \brief Component license info.
     * \returns string representing the license information. HTML tags can be added to it.
     */
    [[nodiscard]] virtual std::string license() const = 0;

    /*!
     * \brief Component copyright info.
     * \returns string representing the copyright information associated with this component.
     */
    [[nodiscard]] virtual std::string copyright() const = 0;

    /*!
     * \brief Component developer url.
     * \returns string representing the url for the developer.
     */
    [[nodiscard]] virtual std::string url() const = 0;

    /*!
     * \brief Component developer email.
     * \returns email as string.
     */
    [[nodiscard]] virtual std::string email() const = 0;

    /*!
     * \brief Component version info.
     * \returns string representing the version of this component.
     */
    [[nodiscard]] virtual std::string version() const = 0;

    /*!
     * \brief tags used to classify this component.
     * \returns the categorical tags that can be used to classify components.
     * e.g., Hydrology, Groundwater, Finite Volume, Finite difference.
     */
    [[nodiscard]] virtual std::set<std::string> tags() const = 0;
  };

  /*!
   * \brief ILicensedComponent is an optional side interface for components that
   * require license validation.
   * \details Licensing was removed from IComponentInfo so that headless HPC and cloud
   * builds need not implement licensing stubs. Components that require licensing
   * implement this interface and advertise Capability::Licensing.
   */
  class ILicensedComponent
  {
  public:
    /*!
     * \brief ILicensedComponent::~ILicensedComponent is a virtual destructor.
     */
    virtual ~ILicensedComponent() = default;

    /*!
     * \brief Checks if license is valid and persists license information.
     * \details Developer is responsible for implementing this validation based on a license.
     * \param[in] licenseInfo license information to use to register this component.
     * \param[out] validationMessage A validation message associated with the license validation process.
     * \returns true if license is valid otherwise false.
     */
    [[nodiscard]] virtual bool validateLicense(const std::string &licenseInfo, std::string &validationMessage) = 0;

    /*!
     * \brief validateLicense Checks if component is licensed and returns.
     * \param[out] validationMessage A validation message associated with the license validation process.
     * \return true if component is licensed otherwise false.
     */
    [[nodiscard]] virtual bool validateLicense(std::string &validationMessage) = 0;
  };

  /*!
   * \brief IUIProvider is an optional side interface for entities that can present
   * a graphical editor and/or viewer.
   * \details UI concerns were removed from IModelComponent and IComponentDataItem so
   * that the core standard stays headless. Entities with UI support implement this
   * interface and their component advertises Capability::UserInterface.
   */
  class IUIProvider
  {
  public:
    /*!
     * \brief IUIProvider::~IUIProvider is a virtual destructor.
     */
    virtual ~IUIProvider() = default;

    /*!
     * \brief hasEditor indicates whether this entity has a UI editor.
     * \return A boolean indicating whether this entity has an editor.
     */
    [[nodiscard]] virtual bool hasEditor() const = 0;

    /*!
     * \brief showEditor shows the editor for this entity.
     * \param[in] opaqueUIPointer Is an opaque pointer to the UI object that is used to show the editor if it is available otherwise nullptr.
     */
    virtual void showEditor(void *opaqueUIPointer = nullptr) = 0;

    /*!
     * \brief hasViewer indicates whether this entity has a UI viewer.
     * \return  A boolean indicating whether this entity has a viewer.
     */
    [[nodiscard]] virtual bool hasViewer() const = 0;

    /*!
     * \brief showViewer shows the viewer for this entity.
     * \param[in] opaqueUIPointer Is an opaque pointer to the UI object that is used to show the viewer if it is available otherwise nullptr.
     */
    virtual void showViewer(void *opaqueUIPointer = nullptr) = 0;
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
    [[nodiscard]] virtual std::unique_ptr<IModelComponent> createComponentInstance() = 0;

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
    [[nodiscard]] virtual std::vector<IAdaptedOutputFactory *> adaptedOutputFactories() const = 0;
  };

  /*!
   * \brief IModelComponent interface is the core interface in the HydroCouple standard defining a model component.
   */
  class IModelComponent : public virtual IIdentity, public virtual ISignal<const std::shared_ptr<IComponentStatusChangeEventArgs> &>
  {

  public:
    /*!
     * \brief HydroCouple::ComponentStatus is an enumerator that describes the status of
     * a component over the course of its lifetime.
     */
    enum class ComponentStatus
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
       * \brief The IModelComponent is preparing itself for the first
       * IComponentDataItem::getValue() call.
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
       * \brief The IModelComponent is saving or restoring a checkpoint of its state
       * (see ICheckpointableModelComponent). This status will end in a status change
       * to HydroCouple::Updated or HydroCouple::Failed.
       */
      Checkpointing,

      /*!
       * \brief The last update process that the IModelComponent performed was the final one.
       * A next call to the IModelComponent::update() method will leave the IModelComponent's internal state unchanged.
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
       * \brief The IModelComponent has encountered an unrecoverable error.
       * Diagnostics describing the failure must be available from IModelComponent::errors().
       * From this state the component may be re-initialized by calling initialize()
       * if it supports re-initialization, or finished and disposed via finish(). */
      Failed,
    };

    // The legal lifecycle transitions form a normative state machine. The
    // non-normative constexpr helper isValidComponentStatusTransition() in
    // hydrocouplehelpers.h encodes the transition table; implementations must not
    // perform transitions that table rejects.

    /*!
     * \brief IModelComponent::~IModelComponent destructor
     */
    virtual ~IModelComponent() = default;

    /*!
     * \brief Contains the metadata about this IModelComponent instance.
     * \returns An IModelComponentInfo that provides metadata about a component.
     */
    [[nodiscard]] virtual IModelComponentInfo *componentInfo() const = 0;

    /*!
     * \brief Defines current status of the IModelComponent.
     * See IModelComponent::ComponentStatus for the possible values.
     * \details The first status that a component sets is ComponentStatus::Created,
     * as soon after it has been created. In this status,
     * arguments() is the only property that may be accessed.
     * \returns The current ComponentStatus of this component.
     */
    [[nodiscard]] virtual ComponentStatus status() const = 0;

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
    [[nodiscard]] virtual std::vector<IArgument *> arguments() const = 0;

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
    [[nodiscard]] virtual std::vector<IInput *> inputs() const = 0;

    /*!
     * \brief The list of IOutputs for which a component can produce results.
     *
     * \details This property must be accessible after the initialize() method has been
     * invoked and until the validate() method has been invoked. If this property
     * is accessed before the initialize() method has been invoked or after the
     * validate() method has been invoked and the IModelComponent cannot handle
     * this an exception must be thrown.
     *
     * \details The list only contains the core IOutput items of the IModelComponent, not
     * the IAdaptedOutput items derived from each IOutput. To get a complete
     * list of outputs, traverse the chain of IAdaptedOutput items that start with the
     * IOutput items returned in the list.
     *
     * \returns This method basically returns references to IOutputs.
     * There is no guarantee that the list of objects is not altered by other components
     * after it has been returned. It is the responsibility of the IModelComponents
     * to make sure that such possible alterations do not subsequently corrupt the IModelComponents.
     */
    [[nodiscard]] virtual std::vector<IOutput *> outputs() const = 0;

    /*!
     * \brief List of the model's output results
     * \returns A list of IComponentDataItem that are the results of the model.
     */
    [[nodiscard]] virtual std::vector<IComponentDataItem *> results() const = 0;

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
     * If there are messages while the component's status is ComponentStatus::Valid, the messages are purely informative.
     * If there are messages while the component's status is ComponentStatus::Invalid,
     * at least one of the messages indicates a fatal error.
     */
    [[nodiscard]] virtual std::vector<std::string> validate() = 0;

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
     * \param[in] requiredOutputs is an optional parameter lets the caller specify the specific
     * producer items that should be updated. If the length is 0, the component
     * will at least update its producer items that have consumers, or all its output items,
     * depending on the component's implementation.
     */
    virtual void update(const std::vector<IOutput *> &requiredOutputs = {}) = 0;

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
     * \brief Gets the workflow that this component is part of.
     * \return The IWorkflowComponent that this component belongs to, or nullptr if not part of a workflow.
     */
    [[nodiscard]] virtual const IWorkflowComponent *workflow() const = 0;

    /*!
     * \brief Sets the workflow that this component is part of.
     * \param[in] workflow is the workflow that this component is part of.
     */
    virtual void setWorkflow(const IWorkflowComponent *workflow) = 0;

    /*!
     * \brief The set of optional capabilities this component supports.
     * \details Orchestrators must branch on this set rather than probing with
     * dynamic_cast chains. Components with no optional capabilities return an empty set.
     * \returns The set of supported Capability values.
     */
    [[nodiscard]] virtual std::set<Capability> capabilities() const = 0;

    /*!
     * \brief Drains this component's diagnostic queue.
     * \details The error queue is the normative failure channel: implementations must
     * queue an ErrorEntry for every Warning-or-worse condition, and must queue a
     * Severity::Fatal entry whenever status() transitions to HydroCouple::Failed.
     * Exceptions may additionally be thrown locally but do not replace the queue,
     * because they cannot cross process, C-ABI, or language boundaries.
     * \param[in] clearAfterRead indicates whether the queue is cleared after being read.
     * \returns The queued diagnostics in insertion order.
     */
    [[nodiscard]] virtual std::vector<ErrorEntry> errors(bool clearAfterRead = false) = 0;

    /*!
     * \brief Gets the reference directory for this component instance.
     * \details All relative file paths specified that are associated with this component instance are referenced
     * from this directory. Typically, this will be the directory for the project file for the current composition. Values
     * are typically set from the Composition GUI and can be referenced internally for saving files and writing
     * arguments for the component.
     * \return The reference directory path as a string.
     */
    [[nodiscard]] virtual std::string referenceDirectory() const = 0;

    /*!
     * \brief setReferenceDirectory Sets the reference directory for this component instance.
     * \param[in] referenceDirectory path to the reference directory.
     */
    virtual void setReferenceDirectory(const std::string &referenceDirectory) = 0;
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
    [[nodiscard]] virtual IModelComponent *component() const = 0;

    /*!
     * \brief Gets the IModelComponent's status before the status change.
     * \returns The previous ComponentStatus of the component that threw the event.
     */
    [[nodiscard]] virtual IModelComponent::ComponentStatus previousStatus() const = 0;

    /*!
     * \brief Gets the IModelComponent's status after the status change.
     * \returns The new ComponentStatus of the component that threw the event.
     */
    [[nodiscard]] virtual IModelComponent::ComponentStatus status() const = 0;

    /*!
     * \brief Gets additional information about the status change.
     * \returns A message string with details about the status change.
     */
    [[nodiscard]] virtual std::string message() const = 0;

    /*!
     * \brief Indicates whether this event has a progress monitor.
     * \returns True if status has a percent progress, otherwise false and the progress bar shows busy.
     */
    [[nodiscard]] virtual bool hasProgressMonitor() const = 0;

    /*!
     * \brief Number between 0 and 100 indicating the progress made by a component in its simulation.
     * \returns A number between 0 and 100 indicating the progress made by a component.
     */
    [[nodiscard]] virtual float percentProgress() const = 0;
  };

  /*!
   * \brief The ICloneableModelComponent class is an IModelComponent that supports
   * deep cloning of itself and its configuration.
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
    [[nodiscard]] virtual ICloneableModelComponent *parent() const = 0;

    /*!
     * \brief Deep clones itself including cloning its IArgument instances.
     * \param[in] clone_optional_arguments are optional arguments that can be passed to the clone method. These arguments are used to
     * pass additional information to the clone method. The arguments are specific to the component being cloned;
     * values are string-encoded (numeric values in decimal form).
     * \returns A deep clone of the current component. Configuration files and output files
     * must be written to a different location than those of the parent. Cloning can only occur after the parent component has been
     * initialized successfully. Cloned components must also be initialized.
     */
    [[nodiscard]] virtual ICloneableModelComponent *clone(const std::unordered_map<std::string, std::string> &clone_optional_arguments = std::unordered_map<std::string, std::string>()) = 0;

    /*!
     * \brief A vector ICloneableModelComponent instances cloned from this IModelComponent instance.
     * \returns A vector of child components created from the current component.
     */
    [[nodiscard]] virtual std::vector<ICloneableModelComponent *> clones() const = 0;
  };

  /*!
   * \brief ICheckpointableModelComponent is an IModelComponent that can save and
   * restore its complete simulation state.
   * \details Checkpoint/restart is required for preemptible cloud execution and
   * walltime-limited HPC runs. During saveState()/restoreState() the component's
   * status is HydroCouple::Checkpointing. Components supporting this interface
   * advertise Capability::Checkpointing.
   */
  class ICheckpointableModelComponent : public virtual IModelComponent
  {
  public:
    /*!
     * \brief ~ICheckpointableModelComponent destructor.
     */
    virtual ~ICheckpointableModelComponent() = default;

    /*!
     * \brief Saves the component's complete state.
     * \details The component persists its state to storage of its choosing (typically
     * under referenceDirectory()) and returns an opaque token with which the state can
     * be restored later, possibly by a different process on a different machine.
     * Callable only when status() is HydroCouple::Updated or HydroCouple::Done.
     * \param[out] token is an opaque identifier for the saved state.
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool saveState(std::string &token, std::string &message) = 0;

    /*!
     * \brief Restores state previously saved by saveState().
     * \details Callable after initialize(); on success the component behaves as if it
     * had computed its way to the checkpointed simulation state.
     * \param[in] token is the opaque identifier returned by saveState().
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool restoreState(const std::string &token, std::string &message) = 0;
  };

  /*!
   * \brief IValueDefinition describes the type and properties of values
   * returned by IComponentDataItem::getValue() and related methods.
   *
   * \details This interface is not meant to be implemented directly.
   * Instead, implement either IQuality or IQuantity or a
   * custom derived value definition interface.
   */
  class IValueDefinition : public virtual IDescription
  {
  public:
    /*!
     * \brief ~IValueDefinition destructor
     */
    virtual ~IValueDefinition() = default;

    /*!
     * \brief Gets the object types of value that will be available
     * and is returned by the GetValues function.
     *
     * \returns A const reference to the type_info associated with this IValueDefinition.
     */
    [[nodiscard]] virtual const std::type_info &type() const = 0;

    /*!
     * \brief The value representing that data is missing.
     * \details Meaningful for numeric DataKinds only; for String and Opaque kinds the
     * returned value must be ignored (missing entries are represented by empty values).
     */
    [[nodiscard]] virtual double missingValue() const = 0;

    /*!
     * \brief Gets the default value for this value definition.
     * \details Meaningful for numeric DataKinds only; for String and Opaque kinds the
     * returned value must be ignored.
     */
    [[nodiscard]] virtual double defaultValue() const = 0;
  };

  /*!
   * \brief IDimension provides the properties of the dimensions of a variable.
   */
  class IDimension : public virtual IIdentity
  {
  public:
    /*!
     * \brief IDimension::LengthType dimension length type
     */
    enum class LengthType
    {
      /*!
       * \brief Static length type
       */
      Static = 0,

      /*!
       * \brief Dynamic length type
       */
      Dynamic = 1
    };

    /*!
     * \brief ~IDimension destructor
     */
    virtual ~IDimension() = default;

    /*!
     * \brief Gets the length type of the dimension.
     */
    [[nodiscard]] virtual LengthType lengthType() const = 0;
  };

  /*!
   * \brief IQuality describes qualitative data, where a value is specified as one category
   * within a number of predefined (possible) categories. These categories can be ordered or not.
   *
   * \details Qualitative data describes items in terms of some quality or categorization that may be 'informal'
   * or may use relatively ill-defined characteristics such as warmth and flavour. However,
   * qualitative data can include well-defined aspects such as gender, nationality or commodity type.
   *
   * \details For qualitative data, the IComponentDataItem data exchanged between components contains
   * one of the possible category instances per element in the IComponentDataItem involved.
   *
   * \details Examples:
   *   - Colors: red, green, blue
   *   - Land use: nature, recreation, industry, infrastructure
   *   - Rating: worse, same, better
   */
  class IQuality : public virtual IValueDefinition
  {
  public:
    /*!
     * \brief IQuality::~IQuality is a virtual destructor.
     */
    virtual ~IQuality() = default;

    /*!
     * \brief The possible category labels allowed for this IQuality.
     * \details If the quality is not ordered the vector contains the categories in an
     * unspecified order. When it is ordered the vector contains the categories in
     * their defined sequence. Category values stored in the data item are indexes
     * into this vector.
     * \returns The category labels.
     */
    [[nodiscard]] virtual std::vector<std::string> categories() const = 0;

    /*!
     * \brief Checks if the IQuality is defined by an ordered set of ICategory or not.
     */
    [[nodiscard]] virtual bool isOrdered() const = 0;
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
    enum class FundamentalUnitDimension
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
     * \param[in] dimension represents the fundamental unit.
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
     */
    [[nodiscard]] virtual double power(HydroCouple::IUnitDimensions::FundamentalUnitDimension dimension) = 0;
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

      /*!
       * \brief Meters
       */
      Meters,

      /*!
       * \brief Kilometers
       */
      Kilometers,

      /*!
       * \brief Feet
       */
      Feet,

      /*!
       * \brief NauticalMiles
       */
      NauticalMiles,

      /*!
       * \brief Yards
       */
      Yards,

      /*!
       * \brief Miles
       */
      Miles,

      /*!
       * \brief Degrees
       */
      Degrees,

      /*!
       * \brief Centimeters
       */
      Centimeters,

      /*!
       * \brief Millimeters
       */
      Millimeters,

      /*!
       * \brief Inches
       */
      Inches,

      /*!
       * \brief Unknown
       */
      Unknown
    };

    /*!
     * \brief HydroCouple::AreaUnits are the types of units that can be used to measure area.
     */
    enum class AreaUnits
    {

      /*!
       * \brief SquareMeters
       */
      SquareMeters,

      /*!
       * \brief SquareKilometers
       */
      SquareKilometers,

      /*!
       * \brief Square Feet
       */
      SquareFeet,

      /*!
       * \brief Square Yards
       */
      SquareYards,

      /*!
       * \brief Square Miles
       */
      SquareMiles,

      /*!
       * \brief Hectares
       */
      Hectares,

      /*!
       * \brief Acres
       */
      Acres,

      /*!
       * \brief Square Nautical Miles
       */
      SquareNauticalMiles,

      /*!
       * \brief Square Degrees
       */
      SquareDegrees,

      /*!
       * \brief Square Centimeters
       */
      SquareCentimeters,

      /*!
       * \brief Square Millimeters
       */
      SquareMillimeters,

      /*!
       * \brief Square Inches
       */
      SquareInches,

      /*!
       * \brief Unknown
       */
      Unknown
    };

    /*!
     * \brief IUnit::~IUnit is a virtual destructor.
     */
    virtual ~IUnit() = default;

    /*!
     * \brief Fundamental dimensions of the unit.
     */
    [[nodiscard]] virtual IUnitDimensions *dimensions() const = 0;

    /*!
     * \brief Conversion factor to SI ('A' in: SI-value = A * quant-value + B)
     */
    [[nodiscard]] virtual double conversionFactorToSI() const = 0;

    /*!
     * \brief OffSet to SI ('B' in: SI-value = A * quant-value + B).
     */
    [[nodiscard]] virtual double offsetToSI() const = 0;
  };

  /*!
   * \brief IQuantity specifies values as an amount
   * of some unit, usually as a floating point number.
   */
  class IQuantity : public virtual IValueDefinition
  {
  public:
    /*!
     * \brief IQuantity::~IQuantity is a virtual destructor.
     */
    virtual ~IQuantity() = default;

    /*!
     * \brief Unit of quantity.
     */
    [[nodiscard]] virtual IUnit *unit() const = 0;

    /*!
     * \brief Gets the minimum allowed value for this quantity.
     * \return The minimum value.
     */
    [[nodiscard]] virtual double minValue() const = 0;

    /*!
     * \brief Gets the maximum allowed value for this quantity.
     * \return The maximum value.
     */
    [[nodiscard]] virtual double maxValue() const = 0;
  };

  /*!
   * \brief IComponentDataItem is a fundamental unit of data for a component.
   *
   * \details This interface is not to be implemented directly; implement one of its
   * specializations (IExchangeItem, IArgument, or a domain data item).
   *
   * \details Data access is typed and bulk-oriented: all field data moves through
   * getValuesInto()/setValuesFrom() as BufferDescriptor hyperslabs. The item's full
   * index space is given by shape(); dimension semantics (which axis is time, entity,
   * layer, ...) are described by dimensions() and by the canonical orderings defined
   * in each specialization's documentation. A hyperslab is selected by a start index
   * and a count per dimension, exactly as in HDF5 hyperslab selections. Scalar and
   * span-based convenience helpers are provided as non-virtual templates over the
   * same bulk path.
   *
   * \details Threading contract: after the owning component reaches
   * HydroCouple::ComponentStatus::Initialized, metadata getters are safe for
   * concurrent reads; getValuesInto() calls are safe concurrently with each other but
   * not with setValuesFrom() or with the owning component's update(); lifecycle
   * methods require external synchronization.
   *
   * \sa IExchangeItem, IArgument, IIdBasedComponentDataItem
   */
  class IComponentDataItem : public virtual IIdentity, public virtual ISignal<const std::shared_ptr<IComponentDataItemValueChanged> &>
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
    [[nodiscard]] virtual IModelComponent *modelComponent() const = 0;

    /*!
     * \brief provides purely descriptive information of the dimensions associated with this IComponentItem
     *
     * \return A list of IDimension objects.
     *
     */
    [[nodiscard]] virtual std::vector<IDimension *> dimensions() const = 0;

    /*!
     * \brief The extent of each dimension of this data item's index space.
     * \details The returned vector is parallel to dimensions(). Dynamic dimensions
     * report their current extent.
     * \returns The extent per dimension.
     */
    [[nodiscard]] virtual std::vector<int64_t> shape() const = 0;

    /*!
     * \brief The element type of this data item's values.
     * \returns The DataKind of the stored values.
     */
    [[nodiscard]] virtual DataKind dataKind() const = 0;

    /*!
     * \brief IValueDefinition for this IComponentDataItem defines the variable type associated with this object.
     * \returns The IValueDefinition for this data item (either an IQuality or IQuantity).
     */
    [[nodiscard]] virtual IValueDefinition *valueDefinition() const = 0;

    /*!
     * \brief Copies a hyperslab of this item's values into a caller-described buffer.
     * \details The selection is the box [start[k], start[k] + count[k]) in each
     * dimension k of shape(). destination.kind must equal dataKind() (no implicit
     * conversion) and destination's element count must equal the product of count.
     * Items that cannot service the destination's memory space return false with a
     * message (host-only items accept MemorySpace::Host; device support is advertised
     * component-wide via Capability::DeviceBuffers). When destination is C-contiguous
     * and the selection is contiguous in this item's storage, implementations should
     * degenerate to memcpy.
     * \param[in] destination describes the buffer receiving the values.
     * \param[in] start is the first index of the selection in each dimension; its length must equal the rank of shape().
     * \param[in] count is the selection extent in each dimension; its length must equal the rank of shape().
     * \param[out] message optionally receives a failure description.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool getValuesInto(
        const BufferDescriptor &destination,
        std::span<const int64_t> start,
        std::span<const int64_t> count,
        std::string *message = nullptr) const = 0;

    /*!
     * \brief Copies values from a caller-described buffer into a hyperslab of this item.
     * \details Selection and compatibility rules are identical to getValuesInto().
     * \param[in] source describes the buffer providing the values.
     * \param[in] start is the first index of the selection in each dimension; its length must equal the rank of shape().
     * \param[in] count is the selection extent in each dimension; its length must equal the rank of shape().
     * \param[out] message optionally receives a failure description.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool setValuesFrom(
        const BufferDescriptor &source,
        std::span<const int64_t> start,
        std::span<const int64_t> count,
        std::string *message = nullptr) = 0;

    // Typed convenience wrappers over getValuesInto()/setValuesFrom() (scalar reads,
    // flat-span hyperslab reads/writes) are provided as non-normative free function
    // templates in hydrocouplehelpers.h; the interface itself carries no executable code.
  };

  /*!
   * \brief IComponentDataItemValueChanged interface class used to notify when the values of a IComponentDataItem change.
   */
  class IComponentDataItemValueChanged
  {
  public:
    /*!
     * \brief IComponentDataItemValueChanged::~IComponentDataItemValueChanged is a virtual destructor.
     */
    virtual ~IComponentDataItemValueChanged() = default;

    /*!
     * \brief Gets the IComponentDataItem that fired the event.
     * \returns The IComponentDataItem that threw the event.
     */
    [[nodiscard]] virtual IComponentDataItem *componentDataItem() const = 0;

    /*!
     * \brief Gets the start index of the hyperslab that changed.
     * \returns A vector containing the first changed index in each dimension.
     */
    [[nodiscard]] virtual std::vector<int64_t> start() const = 0;

    /*!
     * \brief Gets the extent of the hyperslab that changed.
     * \returns A vector containing the changed extent in each dimension.
     */
    [[nodiscard]] virtual std::vector<int64_t> count() const = 0;
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
    enum class ArgumentInputType
    {
      /*!
       * \brief Enumeration indicating that the argument was read from string.
       */
      String,

      /*!
       * \brief Enumeration indicating that the argument was read from a file.
       */
      File,

      /*!
       * \brief Enumeration indicating that the argument input is in JSON format.
       */
      JSON,

      /*!
       * \brief Enumeration indicating that the argument input is in YAML format.
       */
      YAML,

      /*!
       * \brief Enumeration indicating that the argument input is in XML format.
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
    [[nodiscard]] virtual bool isOptional() const = 0;

    /*!
     * \brief Defines whether the Values property may be edited.
     *
     * \details This is used to let a IModelComponent or an IAdaptedOutput
     * present the actual value of an argument that can not be changed by the user,
     * but is needed to determine the values of other arguments or is informative in any other way.
     */
    [[nodiscard]] virtual bool isReadOnly() const = 0;

    /*!
     * \brief String/XML representation for this IArgument
     */
    [[nodiscard]] virtual std::string toString() const = 0;

    /*!
     * \brief Writes data to files associated with this argument if they exist.
     */
    virtual void saveData() = 0;

    /*!
     * \brief File type extensions that can be read by this IArgument.
     * \details File filters are specified as a description followed by glob patterns in
     * parentheses, e.g. "Configuration Files (*.yaml *.yml *.json)".
     * \returns a list of strings for compatible file extensions.
     */
    [[nodiscard]] virtual std::vector<std::string> fileFilters() const = 0;

    /*!
     * \brief Gets the valid IComponentDataItem instance types that can be read by this argument.
     * \returns A list of pointers to type_info objects representing the valid component data item types.
     */
    [[nodiscard]] virtual std::vector<const std::type_info *> validComponentDataItemTypes() const = 0;

    /*!
     * \brief Boolean indicating whether this IArgument copy its values from a string.
     * \param argType is the type of input to be read.
     * \returns True if the argument is read from a string otherwise false.
     */
    [[nodiscard]] virtual bool isValidArgType(ArgumentInputType argType) const = 0;

    /*!
     * \brief Gets the current input type used for this argument.
     * \return The ArgumentInputType indicating how this argument was initialized.
     */
    [[nodiscard]] virtual ArgumentInputType currentArgumentInputType() const = 0;

    /*!
     * \brief Reads values from a JSON string.
     * \param[in] value is a string representing values in JSON format.
     * \param[in] argType is the type of input to be read.
     * \param[out] message message returned from file read operation.
     * \return boolean indicating whether file/string reading was successful
     */
    [[nodiscard]] virtual bool initialize(const std::string &value, ArgumentInputType argType, std::string &message) = 0;

    /*!
     * \brief Reads values from an equivalent IComponentDataItem. IComponentDataItem has been used instead of IArgument
     * so that outputs from one model can be used as initialization arguments for another.
     * \param[in] componentDataItem is the IArgument from which to copy values from.
     * \param[out] message message returned from file read operation.
     * \return boolean indicating whether file reading was successful.
     */
    [[nodiscard]] virtual bool initialize(const IComponentDataItem &componentDataItem, std::string &message) = 0;

    /*!
     * \brief Serializes this argument's current value to the requested representation.
     * \details This is the write-side counterpart of initialize() and makes IArgument the
     * normative serialization unit: a component's entire persistent configuration must be
     * expressible through its arguments, so any driver can round-trip a component without
     * knowing its internals. For large field payloads (meshes, time series, initial
     * conditions) implementations must not inline bulk data into text formats; the
     * serialized form should carry an external binary payload reference (URI, DataKind,
     * and shape inline; bulk bytes in a sidecar produced through the BufferDescriptor
     * path), with inline text arrays used only for small payloads.
     * \param[in] argType is the requested representation (e.g., JSON or YAML).
     * \param[out] value receives the serialized representation.
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success; false if argType is unsupported (see isValidArgType()) or serialization failed.
     */
    [[nodiscard]] virtual bool serialize(ArgumentInputType argType, std::string &value, std::string &message) const = 0;
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
    [[nodiscard]] virtual IExchangeItem *exchangeItem() const = 0;

    /*!
     * \brief Gets message associated with the event.
     */
    [[nodiscard]] virtual std::string message() const = 0;
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
     * \brief  Input items that will consume the values by calling the IOutput::updateValues() method.
     *
     * \details Every input item that will call this method needs to call the addConsumer()
     * method first. If the input item is not interested any longer in calling
     * updateValues(), it should remove itself by calling the removeConsumer() method.
     * The list is readonly. Add and remove from the list by using addConsumer() and removeConsumer().
     * Please be aware that the "unadulterated" values in the output item, provided by the read only Values property,
     * may be called anyway, even if there are no values available.
     *
     */
    [[nodiscard]] virtual std::vector<IInput *> consumers() const = 0;

    /*!
     * \brief Add a consumer to this output item. Every input item that wants to call
     *  the IOutput::updateValues() method needs to add itself as a consumer first.
     *
     * \details If a consumer is added that can not be handled, or that is incompatible with the already
     *  added consumers, an exception will be thrown.
     *
     * \details The addConsumer() method must and will automatically set the consumer's
     *  provider (see IInput::provider())
     *
     * \param[in] consumer that has to be added
     *
     */
    virtual void addConsumer(IInput *consumer) = 0;

    /*!
     * \brief Remove a consumer.
     *
     * \details If an input item is not interested any longer in calling the
     *  IOutput::updateValues() method, it should remove itself by calling removeConsumer().
     *
     * \param[in] consumer that has to be removed
     *
     */
    [[nodiscard]] virtual bool removeConsumer(IInput *consumer) = 0;

    /*!
     * \brief The adaptedOutputs that have this current output item as adaptee.
     *
     * \details As soon as the output item's values have been updated, for each adaptedOutput its
     * IAdaptedOutput.refresh() method must be called.
     *
     * \details The list is readonly. Add and remove from the list by using addAdaptedOutput() and removeAdaptedOutput().
     *
     */
    [[nodiscard]] virtual std::vector<IAdaptedOutput *> adaptedOutputs() const = 0;

    /*!
     * \brief Add a IAdaptedOutput to this output item.
     *
     * \details Every adaptedOutput that uses data from this output item,
     * needs to add itself as a consumer first.
     *
     * \details If a adaptedOutput is added that can not be handled, or that is
     * incompatible with the already added adaptedOutputs, an exception will be thrown
     *
     * \param[in] adaptedOutput is consumer that has to be added
     *
     */
    virtual void addAdaptedOutput(IAdaptedOutput *adaptedOutput) = 0;

    /*!
     * \brief Removes an IAdaptedOutput.
     *
     * \details If a adaptedOutput is not interested any longer in this output item data,
     * it should remove itself by calling removeConsumer().
     *
     * \param[in] adaptedOutput is a consumer that has to be removed.
     *
     */
    [[nodiscard]] virtual bool removeAdaptedOutput(IAdaptedOutput *adaptedOutput) = 0;

    /*!
     * \brief Provides the values matching the value definition specified by the
     * querySpecifier. Extensions can overwrite this base version to include
     * more details in the query, e.g. time and space.
     *
     * \details One might expect the querySpecifier to be of the type IInput, because every input item that calls
     * the updateValues() method needs to add itself as a consumer first.
     *
     * \details However, the IExchangeItem suffices to specify what is required. Therefore,
     * to have the flexibility to loosen the "always register as consumer" approach, it is chosen to provide
     * an IExchangeItem as an argument.
     *
     * \param[in] querySpecifier The IInput specifying the required values.
     */
    virtual void updateValues(const IInput *querySpecifier) = 0;
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
   * suitable for new use or purpose. The object being adapted is typically called the adaptee.
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
    [[nodiscard]] virtual IAdaptedOutputFactory *adaptedOutputFactory() const = 0;

    /*!
     * \brief  IArgument represents input parameters needed for this IAdaptedOutput.
     *
     * \details An unmodifiable vector of the (modifiable) IArguments should be returned that can be used to
     * get information on an IArgument and to modify its values. Validation of changes is done when they occur (e.g. using notifications).
     *
     * \returns Unmodifiable list of IArgument for the adapted output.
     */
    [[nodiscard]] virtual std::vector<IArgument *> arguments() const = 0;

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
    [[nodiscard]] virtual IOutput *adaptee() const = 0;

    /*!
     * \brief Requests the IAdaptedOutput to refresh itself and perform any necessary calculations.
     *
     * \details This method will be called by the adaptee() when it has been refreshed/updated.
     * In the implementation of the refresh method, the adapted output should update its contents
     * according to the changes in the adaptee.
     *
     * \details After updating itself, the IAdaptedOutput must call refresh() on all
     * its IAdaptedOutput children, so the chain of IOutput items refreshes themselves.
     *
     */
    virtual void refresh() = 0;
  };

  /*!
   * \brief IAdaptedOutputFactory is used to create instances of IAdaptedOutput.
   *
   * \details This class can be internal to an IModelComponent by calling
   * IModelComponentInfo::adaptedOutputFactories() or can be generated
   * from an IAdaptedOutputFactoryComponent.
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
    [[nodiscard]] virtual std::vector<IIdentity *> getAvailableAdaptedOutputIds(const IOutput *provider, const IInput *consumer = nullptr) = 0;

    /*!
     * \brief Creates a IAdaptedOutput that adapts the producer so that it fits the consumer.
     *
     * \details The adaptedProviderId used must be one of the IIdentity instances
     * returned by the getAvailableAdaptedOutputIds() method. The returned IAdaptedOutput
     * will already be registered with the provider.
     * \param adaptedProviderId is an identifier of the IAdaptedOutput to create.
     * \param provider IOutput to adapt.
     * \param consumer IInput to adapt the adaptee to.
     * \returns An IAdaptedOutput.
     */
    [[nodiscard]] virtual std::unique_ptr<IAdaptedOutput> createAdaptedOutput(IIdentity *adaptedProviderId, IOutput *provider, IInput *consumer = nullptr) = 0;
  };

  /*!
   * \brief IAdaptedOutputFactoryComponentInfo interface class provides
   * information about an IAdaptedOutputFactoryComponent.
   *
   * \details IAdaptedOutputFactoryComponentInfo is used to provide
   * metadata on an adapted output factory component and create instances of it.
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
    [[nodiscard]] virtual IAdaptedOutputFactoryComponent *createComponentInstance() = 0;
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
     * \brief Contains the metadata about this IAdaptedOutputFactoryComponent.
     *
     * \details This information includes the developer, component version number, contact URL etc.
     */
    [[nodiscard]] virtual IAdaptedOutputFactoryComponentInfo *componentInfo() const = 0;
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
    [[nodiscard]] virtual IOutput *provider() const = 0;

    /*!
     * \brief Sets the producer this consumer should get its values from.
     *
     * \param provider is the IOutput that supplies the data to this IInput.
     */
    [[nodiscard]] virtual bool setProvider(IOutput *provider) = 0;

    /*!
     * \brief Returns true if this IInput can consume this producer.
     *
     * \param provider is the IOutput that can supply the data to this IInput.
     * \param message The error message from the canConsume function.
     */
    [[nodiscard]] virtual bool canConsume(IOutput *provider, std::string &message) const = 0;
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
    [[nodiscard]] virtual std::vector<IIdentity *> providerLabels() const = 0;

    /*!
     * \brief isRequiredProvider checks if the provider is required by the consumer.
     * \param providerLabel is the IIdentity label specifying where to add the provider.
     * \return boolean indicating whether the provider is required by the consumer.
     */
    [[nodiscard]] virtual bool isRequiredProvider(const IIdentity *providerLabel) const = 0;

    /*!
     * \brief Gets the list of providers supplying data to this multi-input.
     * \return A vector of IOutput pointers representing the providers.
     */
    [[nodiscard]] virtual std::vector<IOutput *> providers() const = 0;

    /*!
     * \brief canConsume checks if the provider can supply data to this consumer.
     * \param[in] provider is the IOutput that can supply the data to this IInput.
     * \param[out] message is the error message from the canConsume function.
     * \param[in] providerRoleIdentifier is the IIdentity label specifying where to add the provider.
     * \return boolean indicating whether the provider can supply data to this consumer.
     */
    [[nodiscard]] virtual bool canConsume(IOutput *provider, std::string &message, const IIdentity *providerRoleIdentifier = nullptr) const = 0;

    /*!
     * \brief addProvider adds a provider to the list of providers.
     * \param provider is the IOutput to add to the list of providers.
     * \param id is the IIdentity label specifying where to add the provider.
     */
    [[nodiscard]] virtual bool addProvider(IOutput *provider, const IIdentity *providerRoleIdentifier = nullptr) = 0;

    /*!
     * \brief Removes a provider from the list of providers.
     * \param provider is the IOutput to remove from the list of providers.
     * \return True if the provider was removed successfully, otherwise false.
     */
    [[nodiscard]] virtual bool removeProvider(IOutput *provider) = 0;
  };

  /*!
   * \brief IIdBasedComponentDataItem is an IComponentDataItem whose data is indexed by string identifiers.
   *
   * \details It provides get/set methods that accept an id index in addition to the
   * standard dimension indexes from IComponentDataItem.
   */
  class IIdBasedComponentDataItem : public virtual IComponentDataItem
  {

  public:
    /*!
     * \brief IIdBasedComponentItem::~IIdBasedComponentItem is a virtual destructor.
     */
    virtual ~IIdBasedComponentDataItem() = default;

    /*!
     * \brief Gets the identifiers associated with this id-based component data item.
     * \return A vector of strings representing the identifiers.
     */
    [[nodiscard]] virtual std::vector<std::string> identifiers() const = 0;

    /*!
     * \brief identifierDimension returns the identifier dimension of the id based component item.
     * \details Canonical dimension ordering: the identifier dimension is dimension 0 of
     * shape(); any additional dimensions follow. Data access uses the inherited
     * getValuesInto()/setValuesFrom() hyperslab API with the identifier index as start[0].
     * \return The id dimension of the id based component item.
     */
    [[nodiscard]] virtual IDimension *identifierDimension() const = 0;
  };

  /*!
   * \brief IWorkflowComponentInfo provides metadata about an IWorkflowComponent
   * and creates new instances of it.
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
    [[nodiscard]] virtual IWorkflowComponent *createComponentInstance() = 0;
  };

  /*!
   * \brief IWorkflowComponent manages the execution workflow for a set of coupled IModelComponent instances.
   *
   * \details A workflow component orchestrates the initialization, execution, and
   * finalization of multiple model components that form a coupled simulation.
   */
  class IWorkflowComponent : public virtual IIdentity, public virtual ISignal<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>
  {

  public:
    /*!
     * \brief The WorkflowStatus enum describes the status of
     * a workflow component over the course of its lifetime.
     */
    enum class WorkflowStatus
    {
      //! The workflow component has just been created.
      Created,
      //! The workflow component is initializing itself.
      Initializing,
      //! The workflow component has successfully initialized.
      Initialized,
      //! The workflow component is performing an update step.
      Updating,
      //! The workflow component has successfully updated.
      Updated,
      //! The workflow component has completed all update steps.
      Done,
      //! The workflow component is finalizing and releasing resources.
      Finishing,
      //! The workflow component has finished and cannot be restarted.
      Finished,
      //! The workflow component encountered an error.
      Failed
    };

    /*!
     * \brief ~IWorkflowComponent destructor for IWorkflowComponent class.
     */
    virtual ~IWorkflowComponent() = default;

    /*!
     * \brief Gets the metadata information about this workflow component.
     * \return A pointer to the IWorkflowComponentInfo for this component.
     */
    [[nodiscard]] virtual IWorkflowComponentInfo *componentInfo() const = 0;

    /*!
     * \brief requiredModelComponentIdentifiers returns the vector of IModelComponent identifiers that are required by this component.
     * \return A vector of IModelComponent identifiers that are required by this component.
     */
    [[nodiscard]] virtual std::vector<IIdentity *> modelComponentLabels() const = 0;

    /*!
     * \brief isRequiredModelComponent checks if the model component is required by this component.
     * \param modelComponentLabel is the IIdentity label specifying the model component.
     * \return boolean indicating whether the model component is required by this component.
     */
    [[nodiscard]] virtual bool isRequiredModelComponent(const IIdentity *modelComponentLabel) const = 0;

    /*!
     * \brief Initializes the workflow component.
     */
    virtual void initialize() = 0;

    /*!
     * \brief Updates the workflow component for the current time step.
     */
    virtual void update() = 0;

    /*!
     * \brief Finalizes the workflow component and releases resources.
     */
    virtual void finish() = 0;

    /*!
     * \brief Gets the current status of the workflow component.
     * \return The current WorkflowStatus of this component.
     */
    [[nodiscard]] virtual WorkflowStatus status() const = 0;

    /*!
     * \brief Gets the model components managed by this workflow.
     * \return A vector of IModelComponent pointers managed by this workflow.
     */
    [[nodiscard]] virtual std::vector<IModelComponent *> modelComponents() const = 0;

    /*!
     * \brief addModelComponent Adds model component instance to workflow
     * \param component is the IModelComponent to add to the workflow.
     * \param modelRoleIdentifier is the IIdentity of the role of the model component. If null, the component is added as a standalone component.
     * in which case the workflow likely does not require ordered or specific components for its operation.
     * \return True if the component was added successfully, otherwise false.
     */
    [[nodiscard]] virtual bool addModelComponent(IModelComponent *component, const IIdentity *modelRoleIdentifier = nullptr) = 0;

    /*!
     * \brief removeModelComponent Removes model component instance from workflow
     * \param component is the IModelComponent to remove from the workflow.
     * \return True if the component was removed successfully, otherwise false.
     */
    [[nodiscard]] virtual bool removeModelComponent(IModelComponent *component) = 0;
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
    [[nodiscard]] virtual IWorkflowComponent *workflowComponent() const = 0;

    /*!
     * \brief Gets the IWorkflowComponent's status before the status change.
     * \returns The previous ComponentStatus of the component that threw the event.
     */
    [[nodiscard]] virtual IWorkflowComponent::WorkflowStatus previousStatus() const = 0;

    /*!
     * \brief Gets the IWorkflowComponent's status after the status change.
     * \returns The new ComponentStatus of the component that threw the event.
     */
    [[nodiscard]] virtual IWorkflowComponent::WorkflowStatus status() const = 0;

    /*!
     * \brief Gets additional information about the status change.
     * \returns A message string with details about the workflow status change.
     */
    [[nodiscard]] virtual std::string message() const = 0;

    /*!
     * \brief Indicates whether this event has a progress monitor.
     * \returns True if status has a percent progress, otherwise false and the progress bar shows busy.
     */
    [[nodiscard]] virtual bool hasProgressMonitor() const = 0;

    /*!
     * \brief Number between 0 and 100 indicating the progress made by the workflow component.
     * \returns A number between 0 and 100 indicating the progress made by the workflow component.
     */
    [[nodiscard]] virtual float percentProgress() const = 0;
  };

}

#endif // HYDROCOUPLE_H
