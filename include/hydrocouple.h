/*!
 * \file hydrocouple.h
 * \author Caleb Buahin <caleb.buahin@gmail.com>
 * \version 2.0.0-alpha.2
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
/*!
 * These interfaces cross shared-library boundaries by design — a component
 * in one image hands its data items to an SDK or host living in another —
 * and dynamic_cast across images only works when a class's type_info is one
 * entity. Projects routinely build with -fvisibility=hidden, which would
 * give every image its own private copy of these typeinfos and make such
 * casts fail silently; forcing default visibility here keeps the RTTI
 * shared regardless of the including project's flags.
 */
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility push(default)
#endif

namespace HydroCouple
{
  //! ABI version for the HydroCouple interface.
  //! 3: ICheckpointableModelComponent gained releaseState() (its vtable grew).
  //! 4: contract-consistency round (2026-09-29): BufferDescriptor gained
  //!    itemSizeBytes/backend/queue; IDimension::role(); IValueDefinition
  //!    gained valueKind() and lost type(); IModelComponent::states();
  //!    IArgument::role(); temporal/spatial vtables changed (see CHANGELOG);
  //!    IAdaptedOutput::states(), IDifferentiableAdaptedOutput::
  //!    differentiableStates() and ICheckpointableAdaptedOutput (stateful
  //!    adapters, 2026-10-02 -- the same, still unreleased, ABI 4).
  constexpr int HYDROCOUPLE_ABI_VERSION = 4;

  /*!
   * \page hc_conventions Normative conventions that apply to every interface
   *
   * **Ownership.** A method that *returns an existing object* — an accessor,
   * a lookup, a parent/child navigation — returns a non-owning raw pointer or
   * reference that stays valid for the lifetime of the object it was obtained
   * from (or until that object's documented invalidation event, e.g. "until
   * the mesh topology changes"). A method that *creates an object* returns
   * `std::unique_ptr` and the caller owns the result. No interface method
   * returns a raw pointer the caller must delete. Containers of pointers
   * (`std::vector<IInput*>`) are snapshots of non-owning observers.
   *
   * **Error channel.** The `errors()` diagnostic queue is the normative
   * failure channel: it is the only one that crosses process, C-ABI and
   * language boundaries. Every method that reports failure does so in one of
   * two shapes, and both feed the queue:
   *  - lifecycle methods (`initialize()`, `validate()`, `prepare()`,
   *    `update()`, `finish()`) set `status()` to `Failed`, queue a
   *    `Severity::Fatal` entry, and *may additionally* throw locally;
   *    `validate()`'s returned messages are also queued (`Error` when the
   *    component is `Invalid`, `Warning` otherwise);
   *  - every `bool` + `message` method queues an `Error` entry whenever it
   *    returns false.
   * A consumer that reads only the queue therefore misses nothing. The one
   * object with no queue of its own is an adapted output (its factory may
   * belong to no component): an adapter's `bool` + `message` methods report
   * through the message alone, and the orchestrator calling them queues it
   * where it belongs.
   *
   * **Toolchain.** STL types (`std::string`, `std::vector`, `std::set`,
   * `std::shared_ptr`, ...) cross the plugin boundary by value. A host and the
   * components it loads must therefore be built with the same compiler,
   * standard library and runtime: the interfaces themselves are a C++ ABI,
   * not a C one. What the standard does define in C is the narrow door a
   * library is loaded through — hydrocouplecomponentabi.h: two `extern "C"`
   * entry points, the first of which returns a toolchain-and-interface stamp
   * a host compares *before* calling anything that crosses a vtable. The stamp
   * turns this paragraph's "must be built with the same toolchain" from a
   * requirement a host trusts into one it checks, and is tied to
   * HYDROCOUPLE_ABI_VERSION by a static_assert. Cross-process and
   * cross-language use goes through a transport (hydrocoupledistributed.h) or
   * the Python bridge, never through a foreign-toolchain vtable. The
   * visibility pragma below is GCC/Clang only; MSVC compares RTTI by name and
   * needs nothing.
   *
   * **Threading.** `status()` on every component and workflow is safe to call
   * from any thread at any time (implementations keep it atomic). All other
   * thread-safety guarantees are stated on the interface that gives them.
   *
   * **Signals.** Classes that inherit two `ISignal<>` instantiations (a
   * component is both an `IPropertyChanged` and a status signal) bring both
   * `connect`/`disconnect`/`blockSignals` overload sets into scope with
   * using-declarations, so callers never need to qualify.
   */

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
   * \details This is the type vocabulary of the data-exchange plane. The numeric and
   * Boolean kinds have a fixed element size (see Helpers::dataKindSize()) and are the
   * kinds a transport or a device can carry.
   *
   * \details String is a **host-only metadata kind**: a String buffer points to an array
   * of std::string objects, whose layout is not stable across toolchains, cannot live in
   * device memory, and cannot be serialized by an ITransport. Items may use it for
   * labels and arguments; a transport or a device-space request presented a String
   * buffer must refuse it with a message rather than attempt to move it.
   *
   * \details Opaque carries implementation-defined bytes whose *meaning* both endpoints
   * agree on out of band. Its *size* is not out of band: an Opaque descriptor must
   * carry BufferDescriptor::itemSizeBytes > 0, so that transports, halo exchangers and
   * helpers can compute byte extents without knowing the meaning.
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
   * \brief DeviceBackend identifies the runtime that owns a device-resident buffer.
   * \details Vendor-neutral in the sense that matters — no vendor *types* appear in the
   * standard — but a device ordinal alone is ambiguous on a node that carries more
   * than one runtime (a CUDA device 0 and a Level Zero device 0 are different memory).
   * The backend and BufferDescriptor::deviceId together name one address space.
   */
  enum class DeviceBackend : uint8_t
  {
    None = 0,  //!< Host memory; no device runtime involved.
    CUDA,      //!< NVIDIA CUDA runtime.
    HIP,       //!< AMD HIP/ROCm runtime.
    SYCL,      //!< SYCL (any implementation).
    LevelZero, //!< Intel oneAPI Level Zero.
    OpenCL,    //!< OpenCL.
    Other      //!< An implementation-defined runtime both endpoints agree on.
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
   * \details Element size. itemSizeBytes is the size of one element. For every kind
   * with a fixed size it must equal Helpers::dataKindSize(kind) (a value of 0 is
   * accepted as shorthand for "the kind's natural size" so that fixed-size buffers can
   * be described without repeating it); for DataKind::Opaque it is mandatory and
   * positive; for DataKind::String it is sizeof(std::string). Every byte-extent
   * computation in the standard uses this field, never the kind alone.
   *
   * \details Device buffers. space, backend and deviceId together name the address
   * space; queue is an opaque, backend-specific stream/queue handle (a cudaStream_t,
   * hipStream_t or sycl::queue*) on which an implementation may enqueue the copy
   * instead of synchronizing the device. A null queue means the default stream and a
   * synchronous transfer. Ownership of the queue stays with the caller. A device
   * buffer's pointer must remain valid and stable between the owning component's
   * prepare() and finish() so that kernels may operate on it directly.
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
    int64_t        itemSizeBytes = 0;      //!< Size of one element; 0 => the kind's natural size (illegal for Opaque).
    MemorySpace    space = MemorySpace::Host; //!< Memory space holding the bytes.
    DeviceBackend  backend = DeviceBackend::None; //!< Runtime owning the device memory when space is Device/Unified.
    int32_t        deviceId = 0;           //!< Device ordinal within the backend when space is Device/Unified.
    void          *queue = nullptr;        //!< Opaque backend stream/queue for asynchronous transfers; nullptr => default stream, synchronous.
  };

  /*!
   * \brief Capability identifies an optional behavior a component may support.
   * \details Orchestrators query IModelComponent::capabilities() to learn which
   * optional *component* interfaces and data-item mixins a component offers, and
   * dynamic_cast only to interfaces the set advertises. The set is the discovery
   * mechanism; the cast is the access mechanism. A component must advertise every
   * capability whose interface it (or any of its data items) implements.
   *
   * \details Adapted outputs are the one exception. An IAdaptedOutput is made by a
   * factory, not owned by the component whose output it adapts, and has no capability
   * set of its own; its optional interfaces (IDifferentiableAdaptedOutput,
   * ICheckpointableAdaptedOutput) are discovered by casting the adapter itself.
   *
   * \details Values at or above VendorBase are reserved for vendor- or
   * project-specific capabilities: a vendor picks values in that range, casts them to
   * Capability, and documents them; the standard never assigns values there.
   */
  enum class Capability : uint32_t
  {
    DeviceBuffers = 0,    //!< Data items can produce/accept Device/Unified BufferDescriptors.
    PartitionedData,      //!< Component exposes IPartitionedComponentDataItem items (see hydrocoupledistributed.h).
    DistributedExecution, //!< Component implements IDistributedModelComponent.
    Checkpointing,        //!< Component implements ICheckpointableModelComponent.
    Cloneable,            //!< Component implements ICloneableModelComponent.
    UserInterface,        //!< Component implements IUIProvider.
    Licensing,            //!< Component implements ILicensedComponent.
    Differentiable,       //!< Component implements IDifferentiableModelComponent.
    LayeredData,          //!< Component exposes data items implementing Spatial::ILayering.
    VendorBase = 2147483648 //!< First value of the vendor-reserved range (0x80000000).
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
       * update() and the first IComponentDataItem::getValuesInto() call.
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
       *
       * \details What a component releases in this state is its *computational*
       * resources — solver state, threads, scratch memory. Its data items may
       * remain readable: a component backed by previously recorded results is
       * Finished from the moment it is initialized, and exists precisely so its
       * results() and outputs() can be read afterwards for analysis and
       * visualization. Consumers must therefore not assume that Finished
       * implies unreadable values; a component that genuinely cannot serve
       * values after finalization should say so through its data items.
       */
      Finished,

      /*!
       * \brief The IModelComponent has encountered an unrecoverable error.
       * Reachable from every status except Finished: a failure can be detected
       * while the component rests (a proxy whose peer dies while it is Updated).
       * Diagnostics describing the failure must be available from IModelComponent::errors().
       * From this state the component may be re-initialized by calling initialize()
       * if it supports re-initialization, or finished and disposed via finish(). */
      Failed,
    };

    // The legal lifecycle transitions form a normative state machine. The
    // non-normative constexpr helper isValidComponentStatusTransition() in
    // hydrocouplehelpers.h encodes the transition table; implementations must not
    // perform transitions that table rejects. In summary:
    //
    //   Created ─► Initializing ─► Initialized ─► Validating ─► Valid ─► Preparing ─► Updated
    //   Updated ⇄ Updating ─► Done            Updating ⇄ WaitingForData
    //   Updated | Done ⇄ Checkpointing         (returns to the state it was entered from;
    //                                           a successful restoreState() lands in Updated)
    //   Initialized | Valid | Invalid | Updated | Done | Failed ─► Finishing ─► Finished | Created
    //   every status except Finished ─► Failed   (a failure can be detected while resting:
    //                                             a proxy's peer can die while it is Updated)
    //   Failed ─► Initializing            Invalid ─► Validating          Valid ─► Validating
    //
    // There is no separate "Prepared" status: Updated immediately after prepare()
    // means "ready to update, nothing computed yet".

    /*!
     * \brief IModelComponent::~IModelComponent destructor
     */
    virtual ~IModelComponent() = default;

    using IPropertyChanged::connect;
    using IPropertyChanged::disconnect;
    using IPropertyChanged::blockSignals;
    using ISignal<const std::shared_ptr<IComponentStatusChangeEventArgs> &>::connect;
    using ISignal<const std::shared_ptr<IComponentStatusChangeEventArgs> &>::disconnect;
    using ISignal<const std::shared_ptr<IComponentStatusChangeEventArgs> &>::blockSignals;

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
     * \details Thread-safety: safe to call from any thread at any time; implementations
     * keep the status atomic. A proxy for a remote component updates it from the
     * peer's notifications on whatever thread services the transport.
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
     * \brief The list of consumer items for which a component can receive values.
     *
     * \details Available from the moment initialize() succeeds until finish() is
     * called: orchestrators wire connections between Initialized and Validating, and
     * providers read their consumers' inputs during every update(). Accessing the
     * list before initialize() is an error (see the error-channel convention).
     * The set of inputs is fixed after initialize(); their *values* change.
     *
     * \returns Non-owning observers of this component's IInput items (see the
     * ownership convention). The vector is a snapshot; the items stay valid until
     * finish().
     */
    [[nodiscard]] virtual std::vector<IInput *> inputs() const = 0;

    /*!
     * \brief The list of IOutputs for which a component can produce results.
     *
     * \details Available from the moment initialize() succeeds until finish() is
     * called, under the same rules as inputs().
     *
     * \details The list only contains the core IOutput items of the IModelComponent, not
     * the IAdaptedOutput items derived from each IOutput. To get a complete
     * list of outputs, traverse the chain of IAdaptedOutput items that start with the
     * IOutput items returned in the list.
     *
     * \returns Non-owning observers of this component's IOutput items; a snapshot,
     * valid until finish().
     */
    [[nodiscard]] virtual std::vector<IOutput *> outputs() const = 0;

    /*!
     * \brief List of the model's output results
     * \returns A list of IComponentDataItem that are the results of the model.
     */
    [[nodiscard]] virtual std::vector<IComponentDataItem *> results() const = 0;

    /*!
     * \brief The data items that make up the state carried from one update() to the next.
     *
     * \details The prognostic variables — what a checkpoint must capture, what a
     * data-assimilation driver perturbs, what an ensemble generator copies, and what
     * IDifferentiableModelComponent::differentiableStates() is a subset of. Diagnostic
     * quantities that update() recomputes from scratch are *not* state and belong in
     * results() or outputs(). A component whose update() does not depend on its past
     * returns an empty vector. Items may appear here and in outputs() at once.
     *
     * \details Available under the same rules as inputs(): from initialize() to
     * finish(); non-owning observers.
     * \returns The state items, in a stable order.
     */
    [[nodiscard]] virtual std::vector<IComponentDataItem *> states() const = 0;

    /*!
     * \brief Initializes the current IModelComponent.
     *
     * \details The initialize() method must be invoked before any other
     * method or property in the IModelComponent interface is invoked or accessed, except
     * for arguments(), status(), capabilities(), errors() and componentInfo().
     *
     * \details Immediately after the method is invoked, it changes the IModelComponent's
     * status to HydroCouple::Initializing. On success the status becomes
     * HydroCouple::Initialized and id(), caption(), description(), inputs(), outputs()
     * and states() are populated. On failure the status becomes HydroCouple::Failed and
     * a Severity::Fatal entry is queued (see the error-channel convention); an
     * exception may additionally be thrown.
     *
     * \details Legal from HydroCouple::Created, HydroCouple::Failed and
     * HydroCouple::Initialized (re-initialization).
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
     * \details Legal from HydroCouple::Initialized, HydroCouple::Valid and
     * HydroCouple::Invalid — i.e. after initialize() and after the provider/consumer
     * relations between this component's exchange items and those of other components
     * in the composition have been established; it may be repeated after connections
     * change.
     *
     * \details Immediately after the method is invoked, it changes the IModelComponent's
     * status to HydroCouple::Validating; when it has finished the status is either
     * HydroCouple::Valid or HydroCouple::Invalid.
     *
     * \returns An empty vector if there are no messages. Messages returned while the
     * status is Valid are informative; while the status is Invalid at least one names
     * a fatal problem. Every returned message is also queued on errors() —
     * Severity::Error when Invalid, Severity::Warning when Valid — so that a proxy
     * forwarding a remote validate() loses nothing.
     */
    [[nodiscard]] virtual std::vector<std::string> validate() = 0;

    /*!
     * \brief Prepares the IModelComponent for calls to the update() method.
     *
     * \details Before prepare() is called, the IModelComponent is not required to honor
     * any request that retrieves values from it. After prepare() has succeeded it must be
     * ready to provide values, and any device-resident buffers it exports must keep a
     * stable address until finish().
     *
     * \details Legal from HydroCouple::Valid only. Immediately after the method is
     * invoked the status becomes HydroCouple::Preparing; on success it becomes
     * HydroCouple::Updated (meaning "ready to update, nothing computed yet" — there is
     * no separate Prepared status), on failure HydroCouple::Failed with a queued
     * Severity::Fatal entry.
     *
     * \details prepare() is invoked at most once per initialize()/finish() cycle.
     */
    virtual void prepare() = 0;

    /*!
     * \brief This method is called to let the component update itself, thus reaching its next state.
     *
     * \details Legal from HydroCouple::Updated. Immediately after this method is invoked,
     * it changes the component's status() to HydroCouple::Updating.
     *
     * \details The type of actions a component takes during update() depends
     * on the type of component. A numerical model that progresses in time will typically
     * compute a time step. A database would typically look at the consumers of its output items,
     * and perform one or more queries to be able to provide the values that the consumers require.
     * A GIS system would typically re-evaluate the values in a grid coverage, so that its
     * output items can provide up-to-date values.
     *
     * \details On success the component sets its status to HydroCouple::Updated, or to
     * HydroCouple::Done when this update was the final one. A component that cannot
     * proceed because a provider has not yet produced the values it needs may set
     * HydroCouple::WaitingForData and return; the orchestrator is then responsible for
     * updating the provider and calling update() again. An orchestrator that finds every
     * component of a cycle WaitingForData must declare the composition deadlocked and
     * fail it — no component retries on its own. On failure the status becomes
     * HydroCouple::Failed with a queued Severity::Fatal entry; an exception may
     * additionally be thrown.
     *
     * \param[in] requiredOutputs is an optional parameter lets the caller specify the specific
     * producer items that should be updated. If the length is 0, the component
     * will at least update its producer items that have consumers, or all its output items,
     * depending on the component's implementation.
     */
    virtual void update(const std::vector<IOutput *> &requiredOutputs = {}) = 0;

    /*!
     * \brief finish() must be invoked as the last of any methods in the IModelComponent interface.
     *
     * \details Legal from HydroCouple::Initialized, HydroCouple::Valid,
     * HydroCouple::Invalid, HydroCouple::Updated, HydroCouple::Done and
     * HydroCouple::Failed, so that a composition can be torn down cleanly from any
     * resting state — including one that never validated or was abandoned before
     * prepare(). A component that has nothing to release in the earlier states simply
     * transitions.
     *
     * \details Immediately after the method is invoked, it changes the IModelComponent's
     * status() to HydroCouple::Finishing. Once finishing is complete, the status becomes
     * HydroCouple::Finished if the component cannot be restarted, or HydroCouple::Created
     * if it can.
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
     * \details This is the discovery mechanism for every optional interface in the
     * standard (see Capability): an orchestrator consults the set and then
     * dynamic_casts only to what is advertised. A component must list every
     * capability it or its data items implement. Components with no optional
     * capabilities return an empty set. Available in every status, including Created.
     * \returns The set of supported Capability values.
     */
    [[nodiscard]] virtual std::set<Capability> capabilities() const = 0;

    /*!
     * \brief Drains this component's diagnostic queue.
     * \details The error queue is the normative failure channel (see the error-channel
     * convention): implementations must queue an ErrorEntry for every Warning-or-worse
     * condition, must queue a Severity::Fatal entry whenever status() transitions to
     * HydroCouple::Failed, must queue every message validate() returns, and must queue
     * a Severity::Error entry whenever a bool-returning method returns false.
     * Exceptions may additionally be thrown locally but do not replace the queue,
     * because they cannot cross process, C-ABI, or language boundaries.
     * Available in every status, including Created.
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
     * \returns Non-owning pointer to the parent, or nullptr for an original. A clone
     * may outlive its parent; implementations must then return nullptr rather than a
     * dangling pointer.
     */
    [[nodiscard]] virtual ICloneableModelComponent *parent() const = 0;

    /*!
     * \brief Deep clones itself including cloning its IArgument instances.
     * \param[in] clone_optional_arguments are optional arguments that can be passed to the clone method. These arguments are used to
     * pass additional information to the clone method. The arguments are specific to the component being cloned;
     * values are string-encoded (numeric values in decimal form).
     * \returns A deep clone of the current component, owned by the caller (see the
     * ownership convention). Configuration files and output files
     * must be written to a different location than those of the parent. Cloning can only occur after the parent component has been
     * initialized successfully. Cloned components must also be initialized. Returns
     * nullptr, with a Severity::Error entry queued, when cloning is not possible.
     */
    [[nodiscard]] virtual std::unique_ptr<ICloneableModelComponent> clone(const std::unordered_map<std::string, std::string> &clone_optional_arguments = std::unordered_map<std::string, std::string>()) = 0;

    /*!
     * \brief The ICloneableModelComponent instances cloned from this instance that are still alive.
     * \details Non-owning observers: a clone registers with its parent on creation and
     * deregisters in its destructor, so the vector never holds a dangling pointer.
     * \returns A snapshot of the live clones.
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
   *
   * \details Lifecycle. Both saveState() and restoreState() are legal when status()
   * is HydroCouple::Updated or HydroCouple::Done. saveState() returns the component
   * to the status it was entered from. A successful restoreState() always lands in
   * HydroCouple::Updated — even from Done — because the restored state is, in general,
   * not the final one. A failed call lands in HydroCouple::Failed. A restart in a fresh
   * process therefore runs initialize(), validate(), prepare() and then restoreState():
   * the checkpoint replaces the prepared state, it does not replace preparation.
   *
   * \details What a checkpoint captures is at least every item in
   * IModelComponent::states() plus whatever private solver state update() depends on.
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
     * Legal when status() is HydroCouple::Updated or HydroCouple::Done; on return the
     * status is what it was before the call (or Failed).
     * \param[out] token is an opaque identifier for the saved state.
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool saveState(std::string &token, std::string &message) = 0;

    /*!
     * \brief Restores state previously saved by saveState().
     * \details Legal when status() is HydroCouple::Updated or HydroCouple::Done; on
     * success the status is HydroCouple::Updated and the component behaves as if it
     * had computed its way to the checkpointed simulation state.
     * \param[in] token is the opaque identifier returned by saveState().
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool restoreState(const std::string &token, std::string &message) = 0;

    /*!
     * \brief Releases a state saved by saveState() that will not be restored.
     * \details Whatever saveState() set aside for \p token -- a file under
     * referenceDirectory(), a buffer, a remote object -- may be reclaimed. An
     * orchestrator that keeps checkpoints for replay (reverse-mode
     * differentiation among them) saves far more states than it ever restores
     * and calls this for each one it drops, so a component whose token names
     * storage must honour it or leak that storage for the length of the run.
     * A component whose token IS the state has nothing to free and returns
     * true. After this call the token is dead: restoring or releasing it again
     * is an error the component may refuse.
     * \param[in] token is an opaque identifier returned by saveState().
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool releaseState(const std::string &token, std::string &message) = 0;
  };

  /*!
   * \brief DifferentialRole says which side of a component's step a derivative buffer
   * belongs to.
   * \details A step maps (StateBefore, Input, Argument) to (StateAfter, Output). The
   * same state item appears on both sides, which is why the role is carried explicitly
   * rather than inferred from the item's type.
   */
  enum class DifferentialRole : uint8_t
  {
    Input = 0,   //!< An IInput of the component, as consumed by the step.
    Argument,    //!< An IArgument of the component (a parameter of the step).
    Output,      //!< An IOutput of the component, as produced by the step.
    StateBefore, //!< A state item's value at the start of the step.
    StateAfter   //!< The same state item's value at the end of the step.
  };

  /*!
   * \brief DifferentialEntry pairs one data item, in one role, with a buffer holding its
   * tangent (forward mode) or cotangent (reverse mode).
   * \details A plain aggregate, like BufferDescriptor, so that a C-ABI shim can carry it.
   * The buffer always spans the item's whole index space: value.shape equals the
   * item's shape() and value.kind equals its dataKind(), which must be Float32 or
   * Float64. The caller owns the memory; the memory-space rules are those of the data
   * plane (a component that cannot service a space returns false with a message).
   */
  struct DifferentialEntry
  {
    const IComponentDataItem *item = nullptr; //!< The item this derivative belongs to.
    DifferentialRole role = DifferentialRole::Input; //!< Which side of the step.
    BufferDescriptor value;                   //!< The tangent or cotangent buffer.
  };

  /*!
   * \brief DifferentialSet is a non-owning view of DifferentialEntry records.
   */
  using DifferentialSet = std::span<const DifferentialEntry>;

  /*!
   * \brief IDifferentiableModelComponent is a model component that can report the
   * derivative of its most recent step.
   *
   * \details The contract, not the method. How a component obtains its derivative --
   * a hand-written adjoint, an operator-overloading tool, source transformation, or a
   * machine-learning framework's autograd -- is the component's business; the interface
   * asks only for the products below, so that derivatives compose across components
   * written in different languages, across shared-library and process boundaries, and
   * under any orchestrator (a C++ engine, or a framework's own autograd driving the
   * components from Python).
   *
   * \details The step. update() maps the state the component held before it, the values
   * of its inputs and the values of its arguments to the state it holds after it and the
   * values of its outputs. The items that make up the state are listed by
   * differentiableStates(); they are ordinary data items (the data plane reads them), so
   * a state cotangent has a shape and a kind like any other buffer and can be carried
   * from one step to the previous one by whoever is composing the derivative. That is
   * what lets a gradient flow through time inside a component.
   *
   * \details The linearization point is the most recent update(). vjp() and jvp()
   * describe that step and do not change the component's state. To differentiate an
   * earlier step, the orchestrator restores the state before it (e.g. through
   * ICheckpointableModelComponent), re-supplies that step's inputs and arguments, and
   * calls update() again.
   *
   * \details Buffers. Results are written, not accumulated: every buffer in `results`
   * is overwritten in full. A seed that is absent is zero. An entry naming an item the
   * component does not list as differentiable, or in a role that item does not have, is
   * refused with a message.
   *
   * \details Declared through Capability::Differentiable.
   */
  class IDifferentiableModelComponent : public virtual IModelComponent
  {
  public:
    /*!
     * \brief ~IDifferentiableModelComponent destructor.
     */
    virtual ~IDifferentiableModelComponent() = default;

    /*!
     * \brief The inputs a derivative reaches. Inputs not listed are treated as constant.
     */
    [[nodiscard]] virtual std::vector<IInput *> differentiableInputs() const = 0;

    /*!
     * \brief The arguments (parameters) a derivative reaches.
     */
    [[nodiscard]] virtual std::vector<IArgument *> differentiableArguments() const = 0;

    /*!
     * \brief The outputs whose derivative the component can report.
     */
    [[nodiscard]] virtual std::vector<IOutput *> differentiableOutputs() const = 0;

    /*!
     * \brief The data items that make up the state carried from one step to the next.
     * \details Empty for a component whose step does not depend on its past.
     */
    [[nodiscard]] virtual std::vector<IComponentDataItem *> differentiableStates() const = 0;

    /*!
     * \brief Vector-Jacobian product of the most recent step (reverse mode).
     * \param[in] seeds holds cotangents of Output and StateAfter entries.
     * \param[in] results holds the buffers to overwrite with cotangents of Input,
     * Argument and StateBefore entries.
     * \param[out] message optionally receives a failure description.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool vjp(DifferentialSet seeds, DifferentialSet results,
                                   std::string *message = nullptr) = 0;

    /*!
     * \brief Jacobian-vector product of the most recent step (forward mode).
     * \param[in] seeds holds tangents of Input, Argument and StateBefore entries.
     * \param[in] results holds the buffers to overwrite with tangents of Output and
     * StateAfter entries.
     * \param[out] message optionally receives a failure description.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool jvp(DifferentialSet seeds, DifferentialSet results,
                                   std::string *message = nullptr) = 0;
  };

  /*!
   * \brief How a value behaves when it is regridded or aggregated.
   *
   * Two components rarely share a mesh, so almost every exchange is resampled —
   * and the right way to resample depends on what the number *is*. A temperature
   * handed from a coarse cell to four fine ones is copied; a mass is divided; a
   * flux is scaled by area. Every IValueDefinition therefore declares its kind, so
   * that an adapter never has to be told out of band — being told out of band is
   * how two sides end up disagreeing about what a number means.
   */
  enum class ValueKind : uint8_t
  {
    Unknown = 0,   //!< Not declared. Adapters must not guess; they should refuse or ask.
    Intensive,     //!< Independent of extent — temperature, concentration, elevation. Averages.
    Extensive,     //!< Proportional to extent — mass, volume, heat content. Sums.
    Flux,          //!< Per unit area and time. Integrates over the area it crosses.
    Density        //!< Per unit volume. Integrates over the volume it fills.
  };

  /*!
   * \brief IValueDefinition describes the type and properties of values
   * held by an IComponentDataItem.
   *
   * \details This interface is not meant to be implemented directly.
   * Instead, implement either IQuality or IQuantity or a
   * custom derived value definition interface. The element type of the values is
   * reported by the owning item's IComponentDataItem::dataKind(), not here.
   */
  class IValueDefinition : public virtual IDescription
  {
  public:
    /*!
     * \brief ~IValueDefinition destructor
     */
    virtual ~IValueDefinition() = default;

    /*!
     * \brief How values of this definition behave under regridding and aggregation.
     * \details Returning ValueKind::Unknown is legal and means an adapter that needs
     * the distinction must refuse the exchange rather than assume.
     */
    [[nodiscard]] virtual ValueKind valueKind() const = 0;

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
   * \brief IDimension provides the properties of one axis of a data item's index space.
   *
   * \details Every specialization of IComponentDataItem documents a canonical dimension
   * ordering ("time is dimension 0, the entity dimension is dimension 1, ..."). That
   * prose is for readers; role() is the same information for programs. A generic
   * consumer — a NetCDF writer, a partitioner, an autograd driver — asks each
   * dimension for its role instead of knowing which specialization it holds.
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
     * \brief What an axis of the index space means.
     */
    enum class DimensionRole : uint8_t
    {
      Unknown = 0,  //!< Not declared.
      Time,         //!< Time steps (Temporal::ITimeSeriesComponentDataItem::times()).
      Entity,       //!< Spatial entities: geometries, mesh nodes/edges/faces, network nodes/edges, or the identifiers of an id-based item.
      Layer,        //!< Vertical layers (Spatial::ILayering), top first.
      Band,         //!< Raster bands.
      Row,          //!< Raster rows / structured-grid y index.
      Column,       //!< Raster columns / structured-grid x index.
      Depth,        //!< Structured-grid z index (IRegularGrid3D).
      Component,    //!< Vector or tensor components (Spatial::SpatialDataType).
      Realization,  //!< Ensemble members.
      Other         //!< Declared but none of the above.
    };

    /*!
     * \brief ~IDimension destructor
     */
    virtual ~IDimension() = default;

    /*!
     * \brief Gets the length type of the dimension.
     */
    [[nodiscard]] virtual LengthType lengthType() const = 0;

    /*!
     * \brief What this axis means.
     * \details Must agree with the canonical ordering documented by the owning item's
     * specialization; an item with two axes of the same role (a matrix of
     * entity × entity, say) reports DimensionRole::Other for the second.
     */
    [[nodiscard]] virtual DimensionRole role() const = 0;
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
     * into this vector, so an item whose valueDefinition() is an IQuality must report
     * an integer DataKind (Int8..UInt64); valueKind() is ValueKind::Intensive
     * (categories do not sum) unless the implementation declares Unknown.
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
       * \brief Fundamental dimension for unitless quantities (all powers zero).
       * \details Kept as a named member so that a unit can say it is dimensionless
       * explicitly; it is not a basis dimension and power(Unitless) is always 0.
       */
      Unitless,

      /*!
       * \brief Fundamental dimension for plane angle (radian).
       * \details Dimensionally pure in SI but carried as a basis so that angular
       * quantities (degrees, radians, geographic coordinates) do not silently
       * convert to unitless numbers.
       */
      PlaneAngle,
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
    [[nodiscard]] virtual double power(HydroCouple::IUnitDimensions::FundamentalUnitDimension dimension) const = 0;
  };

  /*!
   * \brief IUnit interface, describing the physical unit of a IQuantity.
   */
  class IUnit : public virtual IDescription
  {
  public:
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
       * \brief Degrees (angular; only meaningful for geographic reference systems).
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

    using IPropertyChanged::connect;
    using IPropertyChanged::disconnect;
    using IPropertyChanged::blockSignals;
    using ISignal<const std::shared_ptr<IComponentDataItemValueChanged> &>::connect;
    using ISignal<const std::shared_ptr<IComponentDataItemValueChanged> &>::disconnect;
    using ISignal<const std::shared_ptr<IComponentDataItemValueChanged> &>::blockSignals;

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
     * \brief The axes of this item's index space, in storage order.
     *
     * \details Parallel to shape(). Each IDimension::role() names what its axis means,
     * and the sequence must match the canonical ordering documented by this item's
     * specialization. Non-owning observers valid for the lifetime of the item.
     *
     * \return A list of IDimension objects.
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
     * conversion), destination.itemSizeBytes must be 0 or the kind's size (and, for
     * Opaque, must match what this item stores), and destination's element count must
     * equal the product of count. Items that cannot service the destination's memory
     * space return false with a message (host-only items accept MemorySpace::Host;
     * device support is advertised component-wide via Capability::DeviceBuffers). A
     * non-null destination.queue lets a device-capable item enqueue the copy on that
     * stream and return before it completes; the caller then synchronizes the stream.
     * When destination is C-contiguous and the selection is contiguous in this item's
     * storage, implementations should degenerate to memcpy. A false return also
     * queues a Severity::Error entry on the owning component (error-channel convention).
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
     * \brief What kind of thing an argument is to the model that owns it.
     * \details A calibration driver perturbs Parameters and leaves Configuration alone;
     * a data-assimilation driver perturbs InitialConditions; a scenario generator swaps
     * Forcings. Without this distinction each of those tools invents its own tag.
     */
    enum class ArgumentRole : uint8_t
    {
      Configuration = 0, //!< How the model runs: paths, options, solver settings, output selection.
      Parameter,         //!< A coefficient of the process equations (roughness, conductivity, rate constants).
      InitialCondition,  //!< The state at the start of the simulation.
      Forcing,           //!< A prescribed time-varying input that is not supplied through an IInput (a boundary time series read from file).
      Geometry           //!< The spatial discretization itself (a mesh, a network, a raster grid).
    };

    /*!
     * \brief IArgument::~IArgument is a virtual destructor.
     */
    virtual ~IArgument() = default;

    /*!
     * \brief What this argument is to the model.
     */
    [[nodiscard]] virtual ArgumentRole role() const = 0;

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
     * \brief Whether this argument can be initialized from, and serialized to, the given representation.
     * \param argType is the representation to query.
     * \returns True if initialize(value, argType, ...) and serialize(argType, ...) are supported.
     */
    [[nodiscard]] virtual bool isValidArgType(ArgumentInputType argType) const = 0;

    /*!
     * \brief Gets the current input type used for this argument.
     * \return The ArgumentInputType indicating how this argument was initialized.
     */
    [[nodiscard]] virtual ArgumentInputType currentArgumentInputType() const = 0;

    /*!
     * \brief Reads values from a string in the given representation (a JSON/YAML/XML
     * document, a file path, a URL, ...).
     * \param[in] value is the string to read; for File and URL it names the source.
     * \param[in] argType is the representation of value.
     * \param[out] message describes the failure when the return value is false.
     * \return True on success.
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
     * \details This is the single entry point for wiring a connection. The output
     * checks consumer->canConsume(this, message); on success it records the consumer
     * and calls consumer->setProvider(this) so both ends agree; on failure it queues a
     * Severity::Error entry on its owning component and may throw. Orchestrators call
     * addConsumer()/removeConsumer() only, never IInput::setProvider() directly.
     *
     * \param[in] consumer that has to be added (non-owning; the consumer's component owns it)
     *
     */
    virtual void addConsumer(IInput *consumer) = 0;

    /*!
     * \brief Remove a consumer.
     *
     * \details If an input item is not interested any longer in calling the
     *  IOutput::updateValues() method, it should remove itself by calling removeConsumer().
     *  On success the output calls consumer->setProvider(nullptr).
     *
     * \param[in] consumer that has to be removed
     * \returns True if the consumer was registered and has been removed.
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
     * \details Registration is non-owning: whoever holds the std::unique_ptr returned
     * by IAdaptedOutputFactory::createAdaptedOutput() owns the adapter, and an
     * adapter's destructor must call adaptee()->removeAdaptedOutput(this) so that
     * this list never dangles.
     *
     * \details If a adaptedOutput is added that can not be handled, or that is
     * incompatible with the already added adaptedOutputs, a Severity::Error entry is
     * queued and an exception may be thrown.
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
     * \brief Brings this output's values up to what the querySpecifier requires.
     *
     * \details The query is the input itself. What it asks for is read from the
     * input's own metadata, by the specialization the two sides share:
     *  - **time**: if the input is a Temporal::ITimeSeriesComponentDataItem, its
     *    times() are the instants the consumer needs values for, and its
     *    timeInterpolation()/timeExtrapolation() say what the provider may do to
     *    produce them; the provider (or the adapted output between them) advances
     *    its owning component through update() until it can serve the last of them,
     *    or sets WaitingForData when it cannot yet;
     *  - **space**: if the input is a spatial item, its geometry/mesh/grid is the
     *    target the values must be resolved on; a mismatch with this output's
     *    geometry is an adapted output's job, and an unadapted mismatch fails
     *    canConsume() at wiring time rather than here;
     *  - **values**: the input's valueDefinition() (unit, valueKind()) is what the
     *    values must be expressed in.
     * A base IInput with none of these carries no query beyond "your current values".
     *
     * \details Synchronous: on return the values the consumer asked for are readable
     * through this output's getValuesInto(). Asynchronous exchange exists only at the
     * distributed layer (hydrocoupledistributed.h).
     *
     * \param[in] querySpecifier The IInput specifying the required values. Must be a
     * registered consumer of this output.
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

    /*!
     * \brief The data items that make up the state carried from one refresh() to the next.
     *
     * \details The adapter's counterpart of IModelComponent::states(). Most adapters
     * (a unit conversion, a regridding) compute their values from the adaptee's
     * current values and their arguments alone, and return an empty vector. An
     * adapter whose values also depend on its previous refreshes -- under-relaxation,
     * interpolation over a recorded history -- lists the items holding that past, so
     * an orchestrator can tell it apart: a composition containing it can be returned
     * to an earlier step only if the adapter can be returned too (see
     * ICheckpointableAdaptedOutput), and IDifferentiableAdaptedOutput::differentiableStates()
     * is a subset of it. An item may be the adapter itself, when its own previous
     * values are the state.
     *
     * \details Available from initialize(); non-owning observers.
     * \returns The state items, in a stable order.
     */
    [[nodiscard]] virtual std::vector<IComponentDataItem *> states() const = 0;
  };

  /*!
   * \brief IDifferentiableAdaptedOutput is an adapted output that can report the
   * derivative of its most recent refresh().
   *
   * \details The adapter's counterpart of IDifferentiableModelComponent, so that a
   * derivative can cross an adapted connection (a unit conversion, a regridding, a
   * rescaling) on its way from a consumer back to a provider. An adapter maps its
   * adaptee's values, its own arguments and -- when it has one -- the state it held
   * before the refresh to its own values and the state it holds after it. In a
   * DifferentialEntry:
   *  - the adapter itself appears in role Output;
   *  - its adaptee() appears in role Input (the adapter's input side);
   *  - its differentiableArguments() appear in role Argument;
   *  - its differentiableStates() appear in roles StateBefore and StateAfter.
   *
   * \details Buffers, seeds and results follow IDifferentiableModelComponent exactly:
   * whole-item buffers, results written rather than accumulated, absent seeds zero,
   * and the linearization point is the most recent refresh(). vjp() and jvp() do not
   * change the adapter's values or its state.
   *
   * \details State. An adapter whose values depend on its previous refreshes
   * (under-relaxation, say) lists the items carrying that dependence in
   * differentiableStates(), exactly as a component does, so a state cotangent can be
   * carried from one refresh to the previous one by whoever composes the derivative.
   * A state item keeps its shape across a refresh; an adapter that must change it
   * (its adaptee changed shape, say) starts its state afresh, and that refresh's
   * derivative reaches nothing before it. An orchestrator that must
   * differentiate more than one refresh restores the earlier ones through
   * ICheckpointableAdaptedOutput, which a stateful differentiable adapter should
   * therefore also implement. An adapter whose state the derivative cannot follow
   * (one whose state changes shape as it grows) must not implement this interface,
   * and an orchestrator treats a connection through it as non-differentiable
   * rather than guess.
   */
  class IDifferentiableAdaptedOutput : public virtual IAdaptedOutput
  {
  public:
    /*!
     * \brief ~IDifferentiableAdaptedOutput destructor.
     */
    virtual ~IDifferentiableAdaptedOutput() = default;

    /*!
     * \brief The arguments of this adapter a derivative reaches (may be empty).
     */
    [[nodiscard]] virtual std::vector<IArgument *> differentiableArguments() const = 0;

    /*!
     * \brief The data items that carry state from one refresh() to the next and that
     * the derivative follows; a subset of states().
     * \details Empty for an adapter whose values depend only on its adaptee's current
     * values and its arguments (the common case).
     */
    [[nodiscard]] virtual std::vector<IComponentDataItem *> differentiableStates() const = 0;

    /*!
     * \brief Vector-Jacobian product of the most recent refresh().
     * \param[in] seeds holds cotangents of this adapter (role Output) and of its
     * states after the refresh (role StateAfter).
     * \param[in] results holds buffers to overwrite with cotangents of the adaptee
     * (role Input), of arguments (role Argument) and of states before the refresh
     * (role StateBefore).
     * \param[out] message optionally receives a failure description.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool vjp(DifferentialSet seeds, DifferentialSet results,
                                   std::string *message = nullptr) = 0;

    /*!
     * \brief Jacobian-vector product of the most recent refresh().
     * \param[in] seeds holds tangents of the adaptee (role Input), of arguments
     * (role Argument) and of states before the refresh (role StateBefore).
     * \param[in] results holds buffers to overwrite with this adapter's tangent
     * (role Output) and the tangents of its states after the refresh (role
     * StateAfter).
     * \param[out] message optionally receives a failure description.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool jvp(DifferentialSet seeds, DifferentialSet results,
                                   std::string *message = nullptr) = 0;
  };

  /*!
   * \brief ICheckpointableAdaptedOutput is an adapted output that can save and restore
   * the state it carries between refreshes.
   *
   * \details The adapter's counterpart of ICheckpointableModelComponent. An
   * orchestrator that returns a composition to an earlier step -- to replay it for
   * reverse-mode differentiation, to rewind a training loop, to restart a run --
   * restores every component, and every adapter whose states() is not empty, to the
   * same moment. An adapter with state that cannot be restored makes that impossible,
   * and the orchestrator must refuse rather than replay through it.
   *
   * \details Semantics are those of ICheckpointableModelComponent: saveState() returns
   * an opaque token, possibly usable by a different process; restoreState() makes the
   * adapter behave as if it had refreshed its way to the checkpointed state (it does
   * not refresh its children, which an orchestrator restores in their own right);
   * releaseState() frees whatever the token names, after which the token is dead.
   * A restore is not a refresh: an adapter that is also an IDifferentiableAdaptedOutput
   * has nothing to differentiate until its next refresh().
   * Adapters have no status, so all three are legal at any time after initialize().
   * What a checkpoint captures is at least every item in states() plus whatever
   * private state refresh() depends on.
   */
  class ICheckpointableAdaptedOutput : public virtual IAdaptedOutput
  {
  public:
    /*!
     * \brief ~ICheckpointableAdaptedOutput destructor.
     */
    virtual ~ICheckpointableAdaptedOutput() = default;

    /*!
     * \brief Saves the adapter's complete state.
     * \param[out] token is an opaque identifier for the saved state.
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool saveState(std::string &token, std::string &message) = 0;

    /*!
     * \brief Restores state previously saved by saveState().
     * \details On success the adapter's values and state are those it held when the
     * token was saved.
     * \param[in] token is the opaque identifier returned by saveState().
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool restoreState(const std::string &token, std::string &message) = 0;

    /*!
     * \brief Releases a state saved by saveState() that will not be restored.
     * \details An adapter whose token IS the state has nothing to free and returns
     * true. After this call the token is dead: restoring or releasing it again is an
     * error the adapter may refuse.
     * \param[in] token is an opaque identifier returned by saveState().
     * \param[out] message describes the failure when the return value is false.
     * \returns True on success.
     */
    [[nodiscard]] virtual bool releaseState(const std::string &token, std::string &message) = 0;
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
     * is already registered with the provider (IOutput::addAdaptedOutput()); the caller
     * owns it and must keep it alive for as long as the connection exists, and the
     * adapter deregisters itself from its adaptee when destroyed (see
     * IOutput::addAdaptedOutput()). Returns nullptr, with a Severity::Error entry
     * queued on the factory's component, when the id is unknown or the adaptation is
     * impossible.
     * \param adaptedProviderId is an identifier of the IAdaptedOutput to create.
     * \param provider IOutput to adapt.
     * \param consumer IInput to adapt the adaptee to.
     * \returns An IAdaptedOutput owned by the caller, or nullptr.
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
     * \details The caller owns the returned instance, mirroring
     * IModelComponentInfo::createComponentInstance(); a host must destroy it
     * before unloading the library that produced it.
     */
    [[nodiscard]] virtual std::unique_ptr<IAdaptedOutputFactoryComponent> createComponentInstance() = 0;
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
     * \brief Records the producer this consumer gets its values from.
     *
     * \details Called by IOutput::addConsumer() (with the output) and
     * IOutput::removeConsumer() (with nullptr) — the output is the single entry point
     * for wiring, and orchestrators must not call this directly. An implementation
     * may still refuse (returning false) if it holds a different provider; the output
     * then treats the wiring as failed.
     *
     * \param provider is the IOutput that supplies the data to this IInput, or nullptr.
     * \returns True if the provider was recorded.
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

    //! The two-argument overload from IInput stays visible beside the role-qualified one below.
    using IInput::canConsume;

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
     * \brief Creates a new IWorkflowComponent instance.
     * \returns A new instance owned by the caller, mirroring
     * IModelComponentInfo::createComponentInstance(); a host must destroy it before
     * unloading the library that produced it.
     */
    [[nodiscard]] virtual std::unique_ptr<IWorkflowComponent> createComponentInstance() = 0;
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
      //! The workflow component is validating the composition (cross-component links, required roles).
      Validating,
      //! The workflow component found the composition valid and ready to prepare.
      Validated,
      //! The workflow component is preparing the composition for computation (e.g., driving IModelComponent::prepare(), building execution schedules, negotiating exchange patterns).
      Preparing,
      //! The workflow component has prepared the composition and can begin updating.
      Prepared,
      //! The workflow component is performing an update step.
      Updating,
      //! The workflow component has successfully updated.
      Updated,
      //! The workflow component is paused at a synchronization point (see requestPause()) and can be resumed.
      Paused,
      //! The workflow component has completed all update steps.
      Done,
      //! The workflow component is finalizing and releasing resources.
      Finishing,
      //! The workflow component has finished and cannot be restarted.
      Finished,
      //! The workflow component encountered an error.
      Failed
    };

    // The legal workflow transitions are encoded in the non-normative helper
    // isValidWorkflowStatusTransition() in hydrocouplehelpers.h:
    //
    //   Created ─► Initializing ─► Initialized ─► Validating ─► Validated ─► Preparing ─► Prepared
    //   Prepared | Updated ─► Updating ─► Updated | Paused | Done      Paused ─► Updated (resume)
    //   Initialized | Validated | Prepared | Updated | Paused | Done | Failed ─► Finishing ─► Finished
    //   every status except Finished ─► Failed                          Failed ─► Initializing

    /*!
     * \brief ~IWorkflowComponent destructor for IWorkflowComponent class.
     */
    virtual ~IWorkflowComponent() = default;

    using IPropertyChanged::connect;
    using IPropertyChanged::disconnect;
    using IPropertyChanged::blockSignals;
    using ISignal<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>::connect;
    using ISignal<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>::disconnect;
    using ISignal<const std::shared_ptr<IWorkflowComponentStatusChangeEventArgs> &>::blockSignals;

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
     * \brief Validates the composition as a whole before computation begins.
     *
     * \details Where IModelComponent::validate() checks one component in isolation,
     * the workflow validates the coupled composition: that every required role is
     * filled (see isRequiredModelComponent()), that connected exchange items are
     * compatible, and that any feedback loops in the connection graph are ones the
     * workflow's execution strategy can resolve. Must only be called after
     * initialize() and after the exchange connections have been established.
     * Transitions status() through WorkflowStatus::Validating to
     * WorkflowStatus::Validated on success or WorkflowStatus::Failed otherwise.
     *
     * \return An empty vector when the composition is valid; otherwise one
     * human-readable description per problem found.
     */
    [[nodiscard]] virtual std::vector<std::string> validate() = 0;

    /*!
     * \brief Prepares the composition for computation.
     *
     * \details The workflow drives IModelComponent::prepare() on its managed
     * components and performs its own pre-computation work, such as deriving an
     * execution schedule from the connection graph or negotiating distributed
     * exchange patterns. Must only be called after validate() has succeeded.
     * Transitions status() through WorkflowStatus::Preparing to
     * WorkflowStatus::Prepared on success or WorkflowStatus::Failed otherwise.
     */
    virtual void prepare() = 0;

    /*!
     * \brief Performs one orchestration step.
     *
     * \details One call advances the composition to the workflow's next
     * synchronization point; what constitutes that point is defined by the
     * concrete workflow's execution strategy (e.g., servicing one request-reply
     * chain from a trigger input, or advancing every component to the next
     * common synchronization interval). Callers repeat update() until status()
     * becomes WorkflowStatus::Done, WorkflowStatus::Paused, or
     * WorkflowStatus::Failed. Pause and stop requests are honored at
     * synchronization points (see requestPause() and requestStop()).
     */
    virtual void update() = 0;

    /*!
     * \brief Finalizes the workflow component and releases resources.
     */
    virtual void finish() = 0;

    /*!
     * \brief Requests a cooperative stop of the workflow.
     *
     * \details The workflow completes the in-flight orchestration step, then
     * transitions to WorkflowStatus::Done instead of starting another step, so
     * that finish() can produce a consistent final state. Thread-safe: may be
     * called from any thread while update() runs on another (it only sets a flag);
     * it is not async-signal-safe, so a POSIX signal handler should set its own flag
     * and let the driving thread call this. Takes effect at the next
     * synchronization point.
     */
    virtual void requestStop() = 0;

    /*!
     * \brief Requests a cooperative pause of the workflow.
     *
     * \details The workflow completes the in-flight orchestration step, then
     * transitions to WorkflowStatus::Paused instead of starting another step.
     * A paused composition is at a consistent synchronization point — the
     * natural moment to checkpoint components that implement
     * ICheckpointableModelComponent. Resume with resume(). Same threading
     * contract as requestStop().
     */
    virtual void requestPause() = 0;

    /*!
     * \brief Resumes a paused workflow.
     *
     * \details Transitions status() from WorkflowStatus::Paused back to
     * WorkflowStatus::Updated so that update() may be called again. Has no
     * effect when the workflow is not paused.
     */
    virtual void resume() = 0;

    /*!
     * \brief Drains this workflow's diagnostic queue.
     *
     * \details Mirrors IModelComponent::errors(): the workflow must queue an
     * ErrorEntry for every Warning-or-worse condition it detects — including
     * failures surfaced by its managed components — and must queue a
     * Severity::Fatal entry whenever it transitions to WorkflowStatus::Failed.
     *
     * \param clearAfterRead when true, the queue is emptied after being read.
     * \return The queued diagnostic records in the order they were recorded.
     */
    [[nodiscard]] virtual std::vector<ErrorEntry> errors(bool clearAfterRead = false) = 0;

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
     * \param message is an optional out parameter that receives a description of why the
     * component could not be added (e.g., unknown role, duplicate component) when returning false.
     * \return True if the component was added successfully, otherwise false.
     */
    [[nodiscard]] virtual bool addModelComponent(IModelComponent *component, const IIdentity *modelRoleIdentifier = nullptr, std::string *message = nullptr) = 0;

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

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility pop
#endif

#endif // HYDROCOUPLE_H

