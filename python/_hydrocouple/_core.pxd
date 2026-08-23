# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ v2.0.0 interfaces in ``hydrocouple.h``.
"""

from libcpp cimport bool as bint
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.set cimport set as cppset
from libcpp.unordered_map cimport unordered_map
from cpython.ref cimport PyObject
from libc.stdint cimport (
    int8_t, int16_t, int32_t, int64_t,
    uint8_t, uint16_t, uint32_t, uint64_t,
)

# ---------------------------------------------------------------------------
# std::span<const int64_t> — Cython has no built-in binding
# ---------------------------------------------------------------------------
cdef extern from "<span>" namespace "std":
    cdef cppclass const_int64_span "std::span<const int64_t>":
        const_int64_span()
        const_int64_span(const int64_t* ptr, size_t count)
        const int64_t* data() const
        size_t size() const
        bint empty() const

# ---------------------------------------------------------------------------
# std::type_info
# ---------------------------------------------------------------------------
cdef extern from "<typeinfo>" namespace "std":
    cdef cppclass type_info:
        const char* name() const

cdef extern from "hydrocouple.h" namespace "HydroCouple":

    # ------------------------------------------------------------------
    # Top-level enums
    # ------------------------------------------------------------------
    cdef enum class ByteOrder(uint8_t):
        BigEndian
        LittleEndian

    cdef enum class DataKind(uint8_t):
        Unknown
        Int8
        UInt8
        Int16
        UInt16
        Int32
        UInt32
        Int64
        UInt64
        Float32
        Float64
        Boolean
        String
        Opaque

    cdef enum class MemorySpace(uint8_t):
        Host
        HostPinned
        Device
        Unified

    cdef enum class Capability(uint32_t):
        DeviceBuffers
        PartitionedData
        DistributedExecution
        Checkpointing
        Cloneable
        UserInterface
        Licensing

    # ------------------------------------------------------------------
    # BufferDescriptor — plain aggregate
    # ------------------------------------------------------------------
    cdef cppclass BufferDescriptor:
        BufferDescriptor()
        void* data
        DataKind kind
        int32_t rank
        const int64_t* shape
        const int64_t* stridesBytes
        MemorySpace space
        int32_t deviceId

    # ------------------------------------------------------------------
    # ErrorEntry
    # ------------------------------------------------------------------
    cdef enum class ErrorEntry_Severity "HydroCouple::ErrorEntry::Severity" (uint8_t):
        Information
        Warning
        Error
        Fatal

    cdef cppclass ErrorEntry:
        ErrorEntry()
        ErrorEntry_Severity severity
        int32_t code
        string source
        string message

    # ------------------------------------------------------------------
    # Forward declarations
    # ------------------------------------------------------------------
    cdef cppclass IModelComponent
    cdef cppclass IInput
    cdef cppclass IOutput
    cdef cppclass IAdaptedOutput
    cdef cppclass IAdaptedOutputFactory
    cdef cppclass IAdaptedOutputFactoryComponent
    cdef cppclass IComponentDataItem
    cdef cppclass IComponentDataItemValueChanged
    cdef cppclass IComponentStatusChangeEventArgs
    cdef cppclass IWorkflowComponent
    cdef cppclass IWorkflowComponentStatusChangeEventArgs
    cdef cppclass IArgument
    cdef cppclass IExchangeItem

    # ------------------------------------------------------------------
    # IPropertyChanged  (flattened ISignal<string> template)
    # ------------------------------------------------------------------
    cdef cppclass IPropertyChanged:
        pass

    # ------------------------------------------------------------------
    # IDescription / IIdentity
    # ------------------------------------------------------------------
    cdef cppclass IDescription(IPropertyChanged):
        const string& caption() const
        void setCaption(const string& caption)
        const string& description() const
        void setDescription(const string& description)

    cdef cppclass IIdentity(IDescription):
        const string& id() const

    # ------------------------------------------------------------------
    # IComponentInfo (licensing moved to ILicensedComponent side interface)
    # ------------------------------------------------------------------
    cdef cppclass IComponentInfo(IIdentity):
        string libraryFilePath() const
        void setLibraryFilePath(const string& filePath)
        string iconFilePath() const
        string developer() const
        vector[string] documentation() const
        string license() const
        string copyright() const
        string url() const
        string email() const
        string version() const
        cppset[string] tags() const

    cdef cppclass IModelComponentInfo(IComponentInfo):
        unique_ptr[IModelComponent] createComponentInstance()
        vector[IAdaptedOutputFactory*] adaptedOutputFactories() const

    # ------------------------------------------------------------------
    # IModelComponent — nested enum + methods
    # ------------------------------------------------------------------
    cdef enum class IModelComponent_ComponentStatus "HydroCouple::IModelComponent::ComponentStatus":
        Created
        Initializing
        Initialized
        Validating
        Valid
        WaitingForData
        Invalid
        Preparing
        Updating
        Updated
        Checkpointing
        Done
        Finishing
        Finished
        Failed

    cdef cppclass IModelComponent(IIdentity):
        IModelComponentInfo* componentInfo() const
        IModelComponent_ComponentStatus status() const
        vector[IArgument*] arguments() const
        vector[IInput*] inputs() const
        vector[IOutput*] outputs() const
        vector[IComponentDataItem*] results() const
        void initialize() except +
        vector[string] validate() except +
        void prepare() except +
        void update(const vector[IOutput*]& requiredOutputs) except + nogil
        void finish() except +
        const IWorkflowComponent* workflow() const
        void setWorkflow(const IWorkflowComponent* workflow)
        cppset[Capability] capabilities() const
        vector[ErrorEntry] errors(bint clearAfterRead)
        string referenceDirectory() const
        void setReferenceDirectory(const string& referenceDirectory)

    # ------------------------------------------------------------------
    # IComponentStatusChangeEventArgs
    # ------------------------------------------------------------------
    cdef cppclass IComponentStatusChangeEventArgs:
        IModelComponent* component() const
        IModelComponent_ComponentStatus previousStatus() const
        IModelComponent_ComponentStatus status() const
        string message() const
        bint hasProgressMonitor() const
        float percentProgress() const

    # ------------------------------------------------------------------
    # ICloneableModelComponent / ICheckpointableModelComponent
    # ------------------------------------------------------------------
    cdef cppclass ICloneableModelComponent(IModelComponent):
        ICloneableModelComponent* parent() const
        ICloneableModelComponent* clone(
            const unordered_map[string, string]& args)
        vector[ICloneableModelComponent*] clones() const

    cdef cppclass ICheckpointableModelComponent(IModelComponent):
        bint saveState(string& token, string& message)
        bint restoreState(const string& token, string& message)

    # ------------------------------------------------------------------
    # IValueDefinition
    # ------------------------------------------------------------------
    cdef cppclass IValueDefinition(IDescription):
        const type_info& type() const
        double missingValue() const
        double defaultValue() const

    # ------------------------------------------------------------------
    # IDimension — nested enum
    # ------------------------------------------------------------------
    cdef enum class IDimension_LengthType "HydroCouple::IDimension::LengthType":
        Static
        Dynamic

    cdef cppclass IDimension(IIdentity):
        IDimension_LengthType lengthType() const

    # ------------------------------------------------------------------
    # IQuality / IUnitDimensions / IUnit / IQuantity
    # ------------------------------------------------------------------
    cdef cppclass IQuality(IValueDefinition):
        vector[string] categories() const
        bint isOrdered() const

    cdef enum class IUnitDimensions_FundamentalUnitDimension \
            "HydroCouple::IUnitDimensions::FundamentalUnitDimension":
        Length
        Mass
        Time
        ElectricCurrent
        Temperature
        AmountOfSubstance
        LuminousIntensity
        Currency
        Unitless

    cdef cppclass IUnitDimensions(IDescription):
        double power(IUnitDimensions_FundamentalUnitDimension dimension)

    cdef enum class IUnit_DistanceUnitType "HydroCouple::IUnit::DistanceUnitType":
        Standard
        Geographic
        Unknown

    cdef enum class IUnit_DistanceUnits "HydroCouple::IUnit::DistanceUnits":
        Meters
        Kilometers
        Feet
        NauticalMiles
        Yards
        Miles
        Degrees
        Centimeters
        Millimeters
        Inches
        Unknown

    cdef enum class IUnit_AreaUnits "HydroCouple::IUnit::AreaUnits":
        SquareMeters
        SquareKilometers
        SquareFeet
        SquareYards
        SquareMiles
        Hectares
        Acres
        SquareNauticalMiles
        SquareDegrees
        SquareCentimeters
        SquareMillimeters
        SquareInches
        Unknown

    cdef cppclass IUnit(IDescription):
        IUnitDimensions* dimensions() const
        double conversionFactorToSI() const
        double offsetToSI() const

    cdef cppclass IQuantity(IValueDefinition):
        IUnit* unit() const
        double minValue() const
        double maxValue() const

    # ------------------------------------------------------------------
    # IComponentDataItemValueChanged
    # ------------------------------------------------------------------
    cdef cppclass IComponentDataItemValueChanged:
        IComponentDataItem* componentDataItem() const
        vector[int64_t] start() const
        vector[int64_t] count() const

    # ------------------------------------------------------------------
    # IComponentDataItem — the typed hyperslab data plane
    # ------------------------------------------------------------------
    cdef cppclass IComponentDataItem(IIdentity):
        IModelComponent* modelComponent() const
        vector[IDimension*] dimensions() const
        vector[int64_t] shape() const
        DataKind dataKind() const
        IValueDefinition* valueDefinition() const
        bint getValuesInto(const BufferDescriptor& destination,
                           const_int64_span start,
                           const_int64_span count,
                           string* message) except + nogil
        bint setValuesFrom(const BufferDescriptor& source,
                           const_int64_span start,
                           const_int64_span count,
                           string* message) except + nogil

    # ------------------------------------------------------------------
    # IArgument — nested enum
    # ------------------------------------------------------------------
    cdef enum class IArgument_ArgumentInputType \
            "HydroCouple::IArgument::ArgumentInputType":
        String
        File
        JSON
        YAML
        XML
        URL
        MEMORY_OBJECT

    cdef cppclass IArgument(IComponentDataItem):
        bint isOptional() const
        bint isReadOnly() const
        string toString() const
        void saveData()
        vector[string] fileFilters() const
        bint isValidArgType(IArgument_ArgumentInputType argType) const
        IArgument_ArgumentInputType currentArgumentInputType() const
        bint initialize(const string& value,
                        IArgument_ArgumentInputType argType,
                        string& message)
        bint serialize(IArgument_ArgumentInputType argType,
                       string& value,
                       string& message) const

    # ------------------------------------------------------------------
    # IExchangeItemChangeEventArgs / IExchangeItem
    # ------------------------------------------------------------------
    cdef cppclass IExchangeItemChangeEventArgs:
        IExchangeItem* exchangeItem() const
        string message() const

    cdef cppclass IExchangeItem(IComponentDataItem):
        pass

    # ------------------------------------------------------------------
    # IOutput / IAdaptedOutput / factories
    # ------------------------------------------------------------------
    cdef cppclass IOutput(IExchangeItem):
        vector[IInput*] consumers() const
        void addConsumer(IInput* consumer)
        bint removeConsumer(IInput* consumer)
        vector[IAdaptedOutput*] adaptedOutputs() const
        void addAdaptedOutput(IAdaptedOutput* adaptedOutput)
        bint removeAdaptedOutput(IAdaptedOutput* adaptedOutput)
        void updateValues(const IInput* querySpecifier) except + nogil

    cdef cppclass IAdaptedOutput(IOutput):
        IAdaptedOutputFactory* adaptedOutputFactory() const
        vector[IArgument*] arguments() const
        void initialize() except +
        IOutput* adaptee() const
        void refresh() except +

    cdef cppclass IAdaptedOutputFactory(IIdentity):
        vector[IIdentity*] getAvailableAdaptedOutputIds(
            const IOutput* provider, const IInput* consumer)
        unique_ptr[IAdaptedOutput] createAdaptedOutput(
            IIdentity* adaptedProviderId,
            IOutput* provider, IInput* consumer)

    cdef cppclass IAdaptedOutputFactoryComponentInfo(IComponentInfo):
        IAdaptedOutputFactoryComponent* createComponentInstance()

    cdef cppclass IAdaptedOutputFactoryComponent(IAdaptedOutputFactory):
        IAdaptedOutputFactoryComponentInfo* componentInfo() const

    # ------------------------------------------------------------------
    # IInput / IMultiInput
    # ------------------------------------------------------------------
    cdef cppclass IInput(IExchangeItem):
        IOutput* provider() const
        bint setProvider(IOutput* provider)
        bint canConsume(IOutput* provider, string& message) const

    cdef cppclass IMultiInput(IInput):
        vector[IIdentity*] providerLabels() const
        bint isRequiredProvider(const IIdentity* providerLabel) const
        vector[IOutput*] providers() const
        bint addProvider(IOutput* provider,
                         const IIdentity* providerRoleIdentifier)
        bint removeProvider(IOutput* provider)

    # ------------------------------------------------------------------
    # IIdBasedComponentDataItem
    # ------------------------------------------------------------------
    cdef cppclass IIdBasedComponentDataItem(IComponentDataItem):
        vector[string] identifiers() const
        IDimension* identifierDimension() const

    # ------------------------------------------------------------------
    # IWorkflowComponent — nested enum
    # ------------------------------------------------------------------
    cdef enum class IWorkflowComponent_WorkflowStatus \
            "HydroCouple::IWorkflowComponent::WorkflowStatus":
        Created
        Initializing
        Initialized
        Updating
        Updated
        Done
        Finishing
        Finished
        Failed

    cdef cppclass IWorkflowComponentInfo(IComponentInfo):
        IWorkflowComponent* createComponentInstance()

    cdef cppclass IWorkflowComponent(IIdentity):
        IWorkflowComponentInfo* componentInfo() const
        vector[IIdentity*] modelComponentLabels() const
        bint isRequiredModelComponent(const IIdentity* label) const
        void initialize() except +
        void update() except + nogil
        void finish() except +
        IWorkflowComponent_WorkflowStatus status() const
        vector[IModelComponent*] modelComponents() const
        bint addModelComponent(IModelComponent* component,
                               const IIdentity* modelRoleIdentifier)
        bint removeModelComponent(IModelComponent* component)

    cdef cppclass IWorkflowComponentStatusChangeEventArgs:
        IWorkflowComponent* workflowComponent() const
        IWorkflowComponent_WorkflowStatus previousStatus() const
        IWorkflowComponent_WorkflowStatus status() const
        string message() const
        bint hasProgressMonitor() const
        float percentProgress() const


# ---------------------------------------------------------------------------
# PyModelComponentBridge — reverse bridge (Python -> C++)
# ---------------------------------------------------------------------------
cdef extern from "py_component_bridge.h" namespace "HydroCouple::Python":
    cdef cppclass PyModelComponentBridge:
        PyModelComponentBridge(PyObject* obj)
        PyObject* pyObject() const
        void emitPropertyChanged(const string& propertyName)
        void emitStatusChanged(
            const shared_ptr[IComponentStatusChangeEventArgs]& args)

    PyModelComponentBridge* make_py_component_bridge(PyObject* pyobj)


# ---------------------------------------------------------------------------
# Shared cdef classes (implemented in _core.pyx, cimportable by the
# temporal/spatial/spatiotemporal extension modules)
# ---------------------------------------------------------------------------
cdef class CppDimensionWrapper:
    cdef IDimension* _ptr

    @staticmethod
    cdef CppDimensionWrapper wrap(IDimension* ptr)


cdef class PyComponentBridge:
    cdef PyModelComponentBridge* _bridge

    cdef IModelComponent* ptr(self)
