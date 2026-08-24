"""
HydroCouple core abstract base classes.

Python ABC mirrors of the C++ HydroCouple v2.0.0 interfaces defined in
``hydrocouple.h``. These define the component-based modeling framework
contracts that all HydroCouple-compliant components must implement.

The hierarchy follows the C++ namespace ``HydroCouple`` and preserves the
same inheritance relationships and the two governing principles of the
standard: the ABCs carry no behavior (mirroring the header-only,
no-implementation rule — conveniences live in :mod:`hydrocouple.helpers`),
and nothing is defined here that the C++ headers do not declare.

Data plane: field data moves exclusively through
:meth:`IComponentDataItem.get_values_into` /
:meth:`IComponentDataItem.set_values_from` as NumPy arrays. An ndarray is
the Python-side ``BufferDescriptor``: its data pointer, dtype, shape, and
strides carry exactly the information the C++ descriptor encodes, so the
Cython bridge marshals zero-copy in both directions.

@see: U{HydroCouple C++ API <https://hydrocouple.org/HydroCouple/html/>}
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass, field
from enum import IntEnum
from typing import TYPE_CHECKING, Callable, Optional, Sequence

if TYPE_CHECKING:
    import numpy as np


# ---------------------------------------------------------------------------
# Enums
# ---------------------------------------------------------------------------


class ByteOrder(IntEnum):
    """Byte-order enumeration mirroring C++ ``ByteOrder``.

    - ``BigEndian``    -- Most-significant byte first (network order).
    - ``LittleEndian`` -- Least-significant byte first (x86 native).
    """

    BigEndian = 0
    LittleEndian = 1


class DataKind(IntEnum):
    """Element type of a typed data buffer.

    Mirrors C++ ``HydroCouple::DataKind``. This is the type vocabulary of
    the data-exchange plane; :mod:`hydrocouple.helpers` maps each kind to
    its NumPy dtype.
    """

    Unknown = 0
    Int8 = 1
    UInt8 = 2
    Int16 = 3
    UInt16 = 4
    Int32 = 5
    UInt32 = 6
    Int64 = 7
    UInt64 = 8
    Float32 = 9
    Float64 = 10
    Boolean = 11
    String = 12
    Opaque = 13


class MemorySpace(IntEnum):
    """Where a buffer's bytes physically live.

    Mirrors C++ ``HydroCouple::MemorySpace``. Vendor-neutral by design.
    """

    Host = 0
    HostPinned = 1
    Device = 2
    Unified = 3


class Capability(IntEnum):
    """Optional behavior a component may support.

    Mirrors C++ ``HydroCouple::Capability``. Orchestrators branch on
    :meth:`IModelComponent.capabilities` instead of type-probing chains.
    """

    DeviceBuffers = 0
    PartitionedData = 1
    DistributedExecution = 2
    Checkpointing = 3
    Cloneable = 4
    UserInterface = 5
    Licensing = 6


class ComponentStatus(IntEnum):
    """Lifecycle status of a model component.

    Mirrors C++ ``HydroCouple::IModelComponent::ComponentStatus``.
    Legal transitions are encoded by
    :func:`hydrocouple.helpers.is_valid_component_status_transition`.
    """

    Created = 0
    Initializing = 1
    Initialized = 2
    Validating = 3
    Valid = 4
    WaitingForData = 5
    Invalid = 6
    Preparing = 7
    Updating = 8
    Updated = 9
    Checkpointing = 10
    Done = 11
    Finishing = 12
    Finished = 13
    Failed = 14


class LengthType(IntEnum):
    """Dimension length type mirroring C++ ``IDimension::LengthType``."""

    Static = 0
    Dynamic = 1


class FundamentalUnitDimension(IntEnum):
    """Fundamental unit dimensions mirroring C++
    ``IUnitDimensions::FundamentalUnitDimension``."""

    Length = 0
    Mass = 1
    Time = 2
    ElectricCurrent = 3
    Temperature = 4
    AmountOfSubstance = 5
    LuminousIntensity = 6
    Currency = 7
    Unitless = 8


class DistanceUnitType(IntEnum):
    """Distance unit type mirroring C++ ``IUnit::DistanceUnitType``."""

    Standard = 0
    Geographic = 1
    Unknown = 2


class DistanceUnits(IntEnum):
    """Distance units mirroring C++ ``IUnit::DistanceUnits``."""

    Meters = 0
    Kilometers = 1
    Feet = 2
    NauticalMiles = 3
    Yards = 4
    Miles = 5
    Degrees = 6
    Centimeters = 7
    Millimeters = 8
    Inches = 9
    Unknown = 10


class AreaUnits(IntEnum):
    """Area units mirroring C++ ``IUnit::AreaUnits``."""

    SquareMeters = 0
    SquareKilometers = 1
    SquareFeet = 2
    SquareYards = 3
    SquareMiles = 4
    Hectares = 5
    Acres = 6
    SquareNauticalMiles = 7
    SquareDegrees = 8
    SquareCentimeters = 9
    SquareMillimeters = 10
    SquareInches = 11
    Unknown = 12


class ArgumentInputType(IntEnum):
    """Input representation of an argument value.

    Mirrors C++ ``IArgument::ArgumentInputType``.
    """

    String = 0
    File = 1
    JSON = 2
    YAML = 3
    XML = 4
    URL = 5
    MEMORY_OBJECT = 6


class WorkflowStatus(IntEnum):
    """Workflow lifecycle status mirroring C++
    ``IWorkflowComponent::WorkflowStatus``."""

    Created = 0
    Initializing = 1
    Initialized = 2
    Validating = 3
    Validated = 4
    Preparing = 5
    Prepared = 6
    Updating = 7
    Updated = 8
    Paused = 9
    Done = 10
    Finishing = 11
    Finished = 12
    Failed = 13


# ---------------------------------------------------------------------------
# ErrorEntry
# ---------------------------------------------------------------------------


@dataclass
class ErrorEntry:
    """One diagnostic record in a component's error queue.

    Mirrors C++ ``HydroCouple::ErrorEntry``. The error queue is the
    normative failure channel for distributed and embedded execution, where
    exceptions cannot cross process, C-ABI, or language boundaries.
    """

    class Severity(IntEnum):
        """Severity of an :class:`ErrorEntry`."""

        Information = 0
        Warning = 1
        Error = 2
        Fatal = 3

    severity: "ErrorEntry.Severity" = Severity.Information
    code: int = 0
    source: str = ""
    message: str = ""


# ---------------------------------------------------------------------------
# Signal / slot
# ---------------------------------------------------------------------------


class ISignal(ABC):
    """Signal emitter mirroring C++ ``ISignal<Args...>``.

    Slots are Python callables; the argument signature is documented by
    each concrete signal owner.
    """

    @abstractmethod
    def connect(self, slot: Callable) -> None:
        """Connect a slot to this signal."""
        raise NotImplementedError

    @abstractmethod
    def disconnect(self, slot: Callable) -> None:
        """Disconnect a slot from this signal."""
        raise NotImplementedError

    @abstractmethod
    def block_signals(self, block: bool) -> None:
        """Block or unblock emission of this signal."""
        raise NotImplementedError


class IPropertyChanged(ISignal):
    """Emits a signal when a property of an object changes.

    Mirrors C++ ``IPropertyChanged`` (``ISignal<std::string>``); slots are
    called as ``slot(property_name: str)``.
    """


# ---------------------------------------------------------------------------
# Identity chain
# ---------------------------------------------------------------------------


class IDescription(IPropertyChanged):
    """Descriptive information on a HydroCouple object.

    Mirrors C++ ``IDescription``.
    """

    @property
    @abstractmethod
    def caption(self) -> str:
        """Caption (title or heading) for the entity."""
        raise NotImplementedError

    @caption.setter
    @abstractmethod
    def caption(self, value: str) -> None:
        raise NotImplementedError

    @property
    @abstractmethod
    def description(self) -> str:
        """Additional descriptive information for the entity."""
        raise NotImplementedError

    @description.setter
    @abstractmethod
    def description(self, value: str) -> None:
        raise NotImplementedError


class IIdentity(IDescription):
    """Adds a unique identifier to a describable entity.

    Mirrors C++ ``IIdentity``. The id must be unique within its context
    and must not be empty.
    """

    @property
    @abstractmethod
    def id(self) -> str:
        """Unique identifier for the entity within its context."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Component info
# ---------------------------------------------------------------------------


class IComponentInfo(IIdentity):
    """Factory metadata about a component.

    Mirrors C++ ``IComponentInfo``. Must be implemented as an
    :class:`IModelComponentInfo`, :class:`IAdaptedOutputFactoryComponentInfo`,
    or :class:`IWorkflowComponentInfo`. Licensing lives on the optional
    :class:`ILicensedComponent` side interface.
    """

    @property
    @abstractmethod
    def library_file_path(self) -> str:
        """Path to the library from which this component was created."""
        raise NotImplementedError

    @library_file_path.setter
    @abstractmethod
    def library_file_path(self, value: str) -> None:
        raise NotImplementedError

    @property
    @abstractmethod
    def icon_file_path(self) -> str:
        """Path to the component icon, relative to the component library."""
        raise NotImplementedError

    @property
    @abstractmethod
    def developer(self) -> str:
        """Name of the developer/vendor of this component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def documentation(self) -> list[str]:
        """Citations of publications related to this component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def license(self) -> str:
        """License information for this component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def copyright(self) -> str:
        """Copyright information for this component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def url(self) -> str:
        """Developer URL."""
        raise NotImplementedError

    @property
    @abstractmethod
    def email(self) -> str:
        """Developer email."""
        raise NotImplementedError

    @property
    @abstractmethod
    def version(self) -> str:
        """Component version string."""
        raise NotImplementedError

    @property
    @abstractmethod
    def tags(self) -> set[str]:
        """Categorical tags classifying this component."""
        raise NotImplementedError


class ILicensedComponent(ABC):
    """Optional side interface for components requiring license validation.

    Mirrors C++ ``ILicensedComponent``. Components implementing it
    advertise :attr:`Capability.Licensing`.
    """

    @abstractmethod
    def validate_license(self, license_info: Optional[str] = None) -> tuple[bool, str]:
        """Validate (and optionally register) the component license.

        :param license_info: License information to register, or ``None``
            to check the current license.
        :returns: ``(ok, validation_message)``.
        """
        raise NotImplementedError


class IUIProvider(ABC):
    """Optional side interface for entities with a graphical editor/viewer.

    Mirrors C++ ``IUIProvider``. UI concerns were removed from
    :class:`IModelComponent` and :class:`IComponentDataItem` so the core
    standard stays headless; the owning component advertises
    :attr:`Capability.UserInterface`.
    """

    @property
    @abstractmethod
    def has_editor(self) -> bool:
        """Whether this entity has a UI editor."""
        raise NotImplementedError

    @abstractmethod
    def show_editor(self, opaque_ui_pointer: object = None) -> None:
        """Show the editor for this entity."""
        raise NotImplementedError

    @property
    @abstractmethod
    def has_viewer(self) -> bool:
        """Whether this entity has a UI viewer."""
        raise NotImplementedError

    @abstractmethod
    def show_viewer(self, opaque_ui_pointer: object = None) -> None:
        """Show the viewer for this entity."""
        raise NotImplementedError


class IModelComponentInfo(IComponentInfo):
    """Metadata about an :class:`IModelComponent`; creates instances of it.

    Mirrors C++ ``IModelComponentInfo``.
    """

    @abstractmethod
    def create_component_instance(self) -> "IModelComponent":
        """Create a new :class:`IModelComponent` instance."""
        raise NotImplementedError

    @property
    @abstractmethod
    def adapted_output_factories(self) -> list["IAdaptedOutputFactory"]:
        """Factories for creating adapted outputs for this component."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Model component
# ---------------------------------------------------------------------------


class IModelComponent(IIdentity):
    """The core interface defining a model component.

    Mirrors C++ ``IModelComponent``. Also an
    ``ISignal<shared_ptr<IComponentStatusChangeEventArgs>>``: status-changed
    slots are called as ``slot(event_args: IComponentStatusChangeEventArgs)``.

    Lifecycle: ``initialize() -> validate() -> prepare() -> update()... ->
    finish()``; legal status transitions are encoded by
    :func:`hydrocouple.helpers.is_valid_component_status_transition`.
    """

    @property
    @abstractmethod
    def component_info(self) -> Optional[IModelComponentInfo]:
        """Metadata about this component instance."""
        raise NotImplementedError

    @property
    @abstractmethod
    def status(self) -> ComponentStatus:
        """Current lifecycle status of this component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def arguments(self) -> list["IArgument"]:
        """Arguments needed to let the component do its work.

        Available as soon as the instance is created; the entire persistent
        configuration of a component must be expressible through them.
        """
        raise NotImplementedError

    @property
    @abstractmethod
    def inputs(self) -> list["IInput"]:
        """Consumer items through which this component can receive values."""
        raise NotImplementedError

    @property
    @abstractmethod
    def outputs(self) -> list["IOutput"]:
        """Producer items through which this component provides results."""
        raise NotImplementedError

    @property
    @abstractmethod
    def results(self) -> list["IComponentDataItem"]:
        """The model's output result data items."""
        raise NotImplementedError

    @abstractmethod
    def initialize(self) -> None:
        """Initialize the component from its arguments."""
        raise NotImplementedError

    @abstractmethod
    def validate(self) -> list[str]:
        """Validate the populated instance after connections are made.

        :returns: Messages; with status ``Invalid`` at least one message
            indicates a fatal error.
        """
        raise NotImplementedError

    @abstractmethod
    def prepare(self) -> None:
        """Prepare the component for calls to :meth:`update`."""
        raise NotImplementedError

    @abstractmethod
    def update(self, required_outputs: Optional[Sequence["IOutput"]] = None) -> None:
        """Let the component update itself, reaching its next state."""
        raise NotImplementedError

    @abstractmethod
    def finish(self) -> None:
        """The last method invoked; writes final results, frees resources."""
        raise NotImplementedError

    @property
    @abstractmethod
    def workflow(self) -> Optional["IWorkflowComponent"]:
        """The workflow this component is part of, or ``None``."""
        raise NotImplementedError

    @workflow.setter
    @abstractmethod
    def workflow(self, value: Optional["IWorkflowComponent"]) -> None:
        raise NotImplementedError

    @abstractmethod
    def capabilities(self) -> set[Capability]:
        """The optional capabilities this component supports.

        Components with no optional capabilities return an empty set.
        """
        raise NotImplementedError

    @abstractmethod
    def errors(self, clear_after_read: bool = False) -> list[ErrorEntry]:
        """Drain this component's diagnostic queue.

        Implementations must queue an entry for every Warning-or-worse
        condition and a ``Fatal`` entry whenever status becomes ``Failed``.
        """
        raise NotImplementedError

    @property
    @abstractmethod
    def reference_directory(self) -> str:
        """Directory from which this component's relative paths resolve."""
        raise NotImplementedError

    @reference_directory.setter
    @abstractmethod
    def reference_directory(self, value: str) -> None:
        raise NotImplementedError


class IComponentStatusChangeEventArgs(ABC):
    """Payload of a component status-changed signal.

    Mirrors C++ ``IComponentStatusChangeEventArgs``.
    """

    @property
    @abstractmethod
    def component(self) -> IModelComponent:
        """The component that fired the event."""
        raise NotImplementedError

    @property
    @abstractmethod
    def previous_status(self) -> ComponentStatus:
        """Status before the change."""
        raise NotImplementedError

    @property
    @abstractmethod
    def status(self) -> ComponentStatus:
        """Status after the change."""
        raise NotImplementedError

    @property
    @abstractmethod
    def message(self) -> str:
        """Details about the status change."""
        raise NotImplementedError

    @property
    @abstractmethod
    def has_progress_monitor(self) -> bool:
        """Whether :attr:`percent_progress` is meaningful."""
        raise NotImplementedError

    @property
    @abstractmethod
    def percent_progress(self) -> float:
        """Progress in percent (0-100)."""
        raise NotImplementedError


class ICloneableModelComponent(IModelComponent):
    """A model component supporting deep cloning.

    Mirrors C++ ``ICloneableModelComponent``; the owning component
    advertises :attr:`Capability.Cloneable`.
    """

    @property
    @abstractmethod
    def parent(self) -> Optional["ICloneableModelComponent"]:
        """The component this instance was cloned from."""
        raise NotImplementedError

    @abstractmethod
    def clone(self, clone_optional_arguments: Optional[dict[str, str]] = None
              ) -> "ICloneableModelComponent":
        """Deep clone this component, including its arguments.

        Argument values are string-encoded (numeric values in decimal form).
        """
        raise NotImplementedError

    @property
    @abstractmethod
    def clones(self) -> list["ICloneableModelComponent"]:
        """Components cloned from this instance."""
        raise NotImplementedError


class ICheckpointableModelComponent(IModelComponent):
    """A model component that can save and restore its complete state.

    Mirrors C++ ``ICheckpointableModelComponent``; the owning component
    advertises :attr:`Capability.Checkpointing`. During save/restore the
    status is :attr:`ComponentStatus.Checkpointing`.
    """

    @abstractmethod
    def save_state(self) -> tuple[bool, str, str]:
        """Save the component's complete state.

        :returns: ``(ok, token, message)`` where ``token`` is an opaque
            identifier with which the state can be restored later.
        """
        raise NotImplementedError

    @abstractmethod
    def restore_state(self, token: str) -> tuple[bool, str]:
        """Restore state previously saved by :meth:`save_state`.

        :returns: ``(ok, message)``.
        """
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Value definitions, dimensions, units
# ---------------------------------------------------------------------------


class IValueDefinition(IDescription):
    """Describes the type and properties of values of a data item.

    Mirrors C++ ``IValueDefinition``. Implement :class:`IQuality` or
    :class:`IQuantity`, not this directly.
    """

    @property
    @abstractmethod
    def type(self) -> type:
        """The Python type of the values (mirror of C++ ``type_info``)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def missing_value(self) -> float:
        """Value representing missing data.

        Meaningful for numeric DataKinds only; for String/Opaque kinds the
        value must be ignored.
        """
        raise NotImplementedError

    @property
    @abstractmethod
    def default_value(self) -> float:
        """Default value for this definition (numeric DataKinds only)."""
        raise NotImplementedError


class IDimension(IIdentity):
    """Properties of one dimension of a variable.

    Mirrors C++ ``IDimension``.
    """

    @property
    @abstractmethod
    def length_type(self) -> LengthType:
        """Whether the dimension extent is static or dynamic."""
        raise NotImplementedError


class IQuality(IValueDefinition):
    """Qualitative (categorical) value definition.

    Mirrors C++ ``IQuality``. Data values are indexes into
    :attr:`categories`.
    """

    @property
    @abstractmethod
    def categories(self) -> list[str]:
        """The category labels allowed for this quality.

        Ordered qualities list them in their defined sequence.
        """
        raise NotImplementedError

    @property
    @abstractmethod
    def is_ordered(self) -> bool:
        """Whether the categories form an ordered set."""
        raise NotImplementedError


class IUnitDimensions(IDescription):
    """Powers of the fundamental dimensions of a unit.

    Mirrors C++ ``IUnitDimensions``.
    """

    @abstractmethod
    def power(self, dimension: FundamentalUnitDimension) -> float:
        """The power of the given fundamental dimension (e.g. Length -> 3
        and Time -> -1 for flow in m3/s)."""
        raise NotImplementedError


class IUnit(IDescription):
    """The physical unit of an :class:`IQuantity`.

    Mirrors C++ ``IUnit``.
    """

    @property
    @abstractmethod
    def dimensions(self) -> IUnitDimensions:
        """Fundamental dimensions of the unit."""
        raise NotImplementedError

    @property
    @abstractmethod
    def conversion_factor_to_si(self) -> float:
        """'A' in: SI-value = A * value + B."""
        raise NotImplementedError

    @property
    @abstractmethod
    def offset_to_si(self) -> float:
        """'B' in: SI-value = A * value + B."""
        raise NotImplementedError


class IQuantity(IValueDefinition):
    """Quantitative value definition with a unit.

    Mirrors C++ ``IQuantity``.
    """

    @property
    @abstractmethod
    def unit(self) -> IUnit:
        """Unit of this quantity."""
        raise NotImplementedError

    @property
    @abstractmethod
    def min_value(self) -> float:
        """Minimum allowed value."""
        raise NotImplementedError

    @property
    @abstractmethod
    def max_value(self) -> float:
        """Maximum allowed value."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Component data item — the typed data plane
# ---------------------------------------------------------------------------


class IComponentDataItemValueChanged(ABC):
    """Payload of a data-item value-changed signal.

    Mirrors C++ ``IComponentDataItemValueChanged``.
    """

    @property
    @abstractmethod
    def component_data_item(self) -> "IComponentDataItem":
        """The data item that fired the event."""
        raise NotImplementedError

    @property
    @abstractmethod
    def start(self) -> list[int]:
        """First changed index in each dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def count(self) -> list[int]:
        """Changed extent in each dimension."""
        raise NotImplementedError


class IComponentDataItem(IIdentity):
    """A fundamental unit of typed, multi-dimensional data for a component.

    Mirrors C++ ``IComponentDataItem``. Also an
    ``ISignal<shared_ptr<IComponentDataItemValueChanged>>``; value-changed
    slots are called as ``slot(event_args)``.

    Data access is typed and bulk-oriented: all field data moves through
    :meth:`get_values_into` / :meth:`set_values_from` as hyperslab
    selections copied into/from NumPy arrays whose dtype must correspond to
    :attr:`data_kind` (see :data:`hydrocouple.helpers.DATA_KIND_TO_DTYPE`).
    Dimension semantics (which axis is time, entity, layer, ...) are
    described by :attr:`dimensions` and by the canonical orderings
    documented on each specialization.
    """

    @property
    @abstractmethod
    def model_component(self) -> Optional[IModelComponent]:
        """The owner component of this item, or ``None``."""
        raise NotImplementedError

    @property
    @abstractmethod
    def dimensions(self) -> list[IDimension]:
        """Descriptive metadata for each dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def shape(self) -> tuple[int, ...]:
        """The extent of each dimension, parallel to :attr:`dimensions`."""
        raise NotImplementedError

    @property
    @abstractmethod
    def data_kind(self) -> DataKind:
        """The element type of this item's values."""
        raise NotImplementedError

    @property
    @abstractmethod
    def value_definition(self) -> IValueDefinition:
        """The value definition (an :class:`IQuality` or :class:`IQuantity`)."""
        raise NotImplementedError

    @abstractmethod
    def get_values_into(self, destination: "np.ndarray",
                        start: Sequence[int],
                        count: Sequence[int]) -> tuple[bool, str]:
        """Copy a hyperslab of this item's values into ``destination``.

        The selection is the box ``[start[k], start[k] + count[k])`` in each
        dimension ``k`` of :attr:`shape`. ``destination.dtype`` must
        correspond to :attr:`data_kind` (no implicit conversion) and
        ``destination.size`` must equal the product of ``count``.
        ``destination`` may be non-contiguous (strided views are honored).

        :returns: ``(ok, message)``.
        """
        raise NotImplementedError

    @abstractmethod
    def set_values_from(self, source: "np.ndarray",
                        start: Sequence[int],
                        count: Sequence[int]) -> tuple[bool, str]:
        """Copy values from ``source`` into a hyperslab of this item.

        Selection and compatibility rules are identical to
        :meth:`get_values_into`.

        :returns: ``(ok, message)``.
        """
        raise NotImplementedError


class IIdBasedComponentDataItem(IComponentDataItem):
    """An :class:`IComponentDataItem` indexed by string identifiers.

    Mirrors C++ ``IIdBasedComponentDataItem``. Canonical dimension
    ordering: the identifier dimension is dimension 0 of :attr:`shape`;
    data access uses the inherited hyperslab API with the identifier index
    as ``start[0]``.
    """

    @property
    @abstractmethod
    def identifiers(self) -> list[str]:
        """The identifiers of this item's identifier dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def identifier_dimension(self) -> IDimension:
        """The identifier dimension (dimension 0 of :attr:`shape`)."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Arguments
# ---------------------------------------------------------------------------


class IArgument(IComponentDataItem):
    """Configuration argument of a component or adapted output.

    Mirrors C++ ``IArgument``. Arguments are the normative serialization
    unit: a component's entire persistent configuration must be expressible
    through its arguments, so any driver can round-trip a component without
    knowing its internals.
    """

    @property
    @abstractmethod
    def is_optional(self) -> bool:
        """Whether this argument is optional."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_read_only(self) -> bool:
        """Whether the argument's values may not be edited."""
        raise NotImplementedError

    @abstractmethod
    def __str__(self) -> str:
        """String representation of this argument."""
        raise NotImplementedError

    @abstractmethod
    def save_data(self) -> None:
        """Write data to files associated with this argument, if any."""
        raise NotImplementedError

    @property
    @abstractmethod
    def file_filters(self) -> list[str]:
        """File filters readable by this argument, e.g.
        ``"Configuration Files (*.yaml *.yml *.json)"``."""
        raise NotImplementedError

    @property
    @abstractmethod
    def valid_component_data_item_types(self) -> list[type]:
        """Data item types this argument can be initialized from."""
        raise NotImplementedError

    @abstractmethod
    def is_valid_arg_type(self, arg_type: ArgumentInputType) -> bool:
        """Whether the given input representation is supported."""
        raise NotImplementedError

    @property
    @abstractmethod
    def current_argument_input_type(self) -> ArgumentInputType:
        """How this argument was initialized."""
        raise NotImplementedError

    @abstractmethod
    def initialize(self, value: "str | IComponentDataItem",
                   arg_type: Optional[ArgumentInputType] = None
                   ) -> tuple[bool, str]:
        """Read the argument value from a string representation or an
        equivalent :class:`IComponentDataItem`.

        :returns: ``(ok, message)``.
        """
        raise NotImplementedError

    @abstractmethod
    def serialize(self, arg_type: ArgumentInputType) -> tuple[bool, str, str]:
        """Serialize the current value to the requested representation.

        The write-side counterpart of :meth:`initialize`. For large field
        payloads implementations must not inline bulk data into text
        formats: the serialized form should carry an external binary
        payload reference (URI, DataKind, and shape inline; bulk bytes in a
        sidecar), with inline text arrays only for small payloads.

        :returns: ``(ok, value, message)``.
        """
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Exchange items
# ---------------------------------------------------------------------------


class IExchangeItemChangeEventArgs(ABC):
    """Payload of an exchange-item-changed signal.

    Mirrors C++ ``IExchangeItemChangeEventArgs``.
    """

    @property
    @abstractmethod
    def exchange_item(self) -> "IExchangeItem":
        """The exchange item that fired the signal."""
        raise NotImplementedError

    @property
    @abstractmethod
    def message(self) -> str:
        """Message associated with the event."""
        raise NotImplementedError


class IExchangeItem(IComponentDataItem):
    """Base data item exchangeable between components at runtime.

    Mirrors C++ ``IExchangeItem``; implement :class:`IInput` or
    :class:`IOutput`.
    """


class IOutput(IExchangeItem):
    """An output exchange item that delivers values from a component.

    Mirrors C++ ``IOutput``.
    """

    @property
    @abstractmethod
    def consumers(self) -> list["IInput"]:
        """Inputs that will consume this output's values."""
        raise NotImplementedError

    @abstractmethod
    def add_consumer(self, consumer: "IInput") -> None:
        """Add a consumer to this output."""
        raise NotImplementedError

    @abstractmethod
    def remove_consumer(self, consumer: "IInput") -> bool:
        """Remove a consumer from this output."""
        raise NotImplementedError

    @property
    @abstractmethod
    def adapted_outputs(self) -> list["IAdaptedOutput"]:
        """Adapted outputs that adapt this output."""
        raise NotImplementedError

    @abstractmethod
    def add_adapted_output(self, adapted_output: "IAdaptedOutput") -> None:
        """Add an adapted output to this output."""
        raise NotImplementedError

    @abstractmethod
    def remove_adapted_output(self, adapted_output: "IAdaptedOutput") -> bool:
        """Remove an adapted output from this output."""
        raise NotImplementedError

    @abstractmethod
    def update_values(self, query_specifier: "IInput") -> None:
        """Provide values matching the query specifier's requirements."""
        raise NotImplementedError


class IAdaptedOutput(IOutput):
    """Adds data operations (interpolation, aggregation, unit conversion,
    ...) on top of an adaptee :class:`IOutput`.

    Mirrors C++ ``IAdaptedOutput``.
    """

    @property
    @abstractmethod
    def adapted_output_factory(self) -> "IAdaptedOutputFactory":
        """The factory that generated this adapted output."""
        raise NotImplementedError

    @property
    @abstractmethod
    def arguments(self) -> list[IArgument]:
        """Arguments configuring this adapted output."""
        raise NotImplementedError

    @abstractmethod
    def initialize(self) -> None:
        """Initialize based on the current argument values."""
        raise NotImplementedError

    @property
    @abstractmethod
    def adaptee(self) -> IOutput:
        """The output being adapted."""
        raise NotImplementedError

    @abstractmethod
    def refresh(self) -> None:
        """Refresh after the adaptee has been updated; must cascade to
        child adapted outputs."""
        raise NotImplementedError


class IAdaptedOutputFactory(IIdentity):
    """Creates :class:`IAdaptedOutput` instances.

    Mirrors C++ ``IAdaptedOutputFactory``.
    """

    @abstractmethod
    def get_available_adapted_output_ids(
            self, provider: IOutput,
            consumer: Optional["IInput"] = None) -> list[IIdentity]:
        """Identifiers of adapted outputs that can adapt the provider."""
        raise NotImplementedError

    @abstractmethod
    def create_adapted_output(self, adapted_provider_id: IIdentity,
                              provider: IOutput,
                              consumer: Optional["IInput"] = None
                              ) -> IAdaptedOutput:
        """Create an adapted output that fits the provider to the consumer."""
        raise NotImplementedError


class IAdaptedOutputFactoryComponentInfo(IComponentInfo):
    """Metadata about an :class:`IAdaptedOutputFactoryComponent`.

    Mirrors C++ ``IAdaptedOutputFactoryComponentInfo``.
    """

    @abstractmethod
    def create_component_instance(self) -> "IAdaptedOutputFactoryComponent":
        """Create a new factory component instance."""
        raise NotImplementedError


class IAdaptedOutputFactoryComponent(IAdaptedOutputFactory):
    """An adapted-output factory generated from a component info.

    Mirrors C++ ``IAdaptedOutputFactoryComponent``.
    """

    @property
    @abstractmethod
    def component_info(self) -> IAdaptedOutputFactoryComponentInfo:
        """Metadata about this factory component."""
        raise NotImplementedError


class IInput(IExchangeItem):
    """An input exchange item that accepts values for a component.

    Mirrors C++ ``IInput``.
    """

    @property
    @abstractmethod
    def provider(self) -> Optional[IOutput]:
        """The output this input gets its values from."""
        raise NotImplementedError

    @abstractmethod
    def set_provider(self, provider: Optional[IOutput]) -> bool:
        """Set the provider of this input."""
        raise NotImplementedError

    @abstractmethod
    def can_consume(self, provider: IOutput) -> tuple[bool, str]:
        """Whether this input can consume the given provider.

        :returns: ``(ok, message)``.
        """
        raise NotImplementedError


class IMultiInput(IInput):
    """An input supplied by multiple providers.

    Mirrors C++ ``IMultiInput``.
    """

    @property
    @abstractmethod
    def provider_labels(self) -> list[IIdentity]:
        """Role labels for the providers required by this consumer."""
        raise NotImplementedError

    @abstractmethod
    def is_required_provider(self, provider_label: IIdentity) -> bool:
        """Whether the labeled provider role is required."""
        raise NotImplementedError

    @property
    @abstractmethod
    def providers(self) -> list[IOutput]:
        """The providers supplying data to this input."""
        raise NotImplementedError

    @abstractmethod
    def add_provider(self, provider: IOutput,
                     provider_role_identifier: Optional[IIdentity] = None
                     ) -> bool:
        """Add a provider."""
        raise NotImplementedError

    @abstractmethod
    def remove_provider(self, provider: IOutput) -> bool:
        """Remove a provider."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Workflow
# ---------------------------------------------------------------------------


class IWorkflowComponentInfo(IComponentInfo):
    """Metadata about an :class:`IWorkflowComponent`.

    Mirrors C++ ``IWorkflowComponentInfo``.
    """

    @abstractmethod
    def create_component_instance(self) -> "IWorkflowComponent":
        """Create a new workflow component instance."""
        raise NotImplementedError


class IWorkflowComponent(IIdentity):
    """Orchestrates the execution of a set of coupled model components.

    Mirrors C++ ``IWorkflowComponent``. Also an
    ``ISignal<shared_ptr<IWorkflowComponentStatusChangeEventArgs>>``.
    """

    @property
    @abstractmethod
    def component_info(self) -> Optional[IWorkflowComponentInfo]:
        """Metadata about this workflow component."""
        raise NotImplementedError

    @property
    @abstractmethod
    def model_component_labels(self) -> list[IIdentity]:
        """Role labels of the model components required by this workflow."""
        raise NotImplementedError

    @abstractmethod
    def is_required_model_component(self, label: IIdentity) -> bool:
        """Whether the labeled component role is required."""
        raise NotImplementedError

    @abstractmethod
    def initialize(self) -> None:
        """Initialize the workflow."""
        raise NotImplementedError

    @abstractmethod
    def update(self) -> None:
        """Update the workflow for the current step."""
        raise NotImplementedError

    @abstractmethod
    def finish(self) -> None:
        """Finalize the workflow and release resources."""
        raise NotImplementedError

    @property
    @abstractmethod
    def status(self) -> WorkflowStatus:
        """Current workflow status."""
        raise NotImplementedError

    @property
    @abstractmethod
    def model_components(self) -> list[IModelComponent]:
        """The model components managed by this workflow."""
        raise NotImplementedError

    @abstractmethod
    def add_model_component(self, component: IModelComponent,
                            model_role_identifier: Optional[IIdentity] = None
                            ) -> bool:
        """Add a model component to the workflow."""
        raise NotImplementedError

    @abstractmethod
    def remove_model_component(self, component: IModelComponent) -> bool:
        """Remove a model component from the workflow."""
        raise NotImplementedError


class IWorkflowComponentStatusChangeEventArgs(ABC):
    """Payload of a workflow status-changed signal.

    Mirrors C++ ``IWorkflowComponentStatusChangeEventArgs``.
    """

    @property
    @abstractmethod
    def workflow_component(self) -> IWorkflowComponent:
        """The workflow component that fired the event."""
        raise NotImplementedError

    @property
    @abstractmethod
    def previous_status(self) -> WorkflowStatus:
        """Status before the change."""
        raise NotImplementedError

    @property
    @abstractmethod
    def status(self) -> WorkflowStatus:
        """Status after the change."""
        raise NotImplementedError

    @property
    @abstractmethod
    def message(self) -> str:
        """Details about the status change."""
        raise NotImplementedError

    @property
    @abstractmethod
    def has_progress_monitor(self) -> bool:
        """Whether :attr:`percent_progress` is meaningful."""
        raise NotImplementedError

    @property
    @abstractmethod
    def percent_progress(self) -> float:
        """Progress in percent (0-100)."""
        raise NotImplementedError
