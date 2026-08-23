"""
hydrocouple — Python interface definitions for the HydroCouple component framework.

This package provides abstract base classes (ABCs) that mirror the C++20
HydroCouple v2.0.0 header-only interfaces. Python component developers
subclass these ABCs exactly as C++ developers implement the pure-virtual
interfaces. Field data moves through the typed hyperslab API
(``get_values_into`` / ``set_values_from``) as NumPy arrays; see
:mod:`hydrocouple.helpers` for the DataKind/dtype mapping and convenience
wrappers, and :mod:`hydrocouple.distributed` for the distributed-execution
contracts.
"""

from hydrocouple.core import (
    # Enums
    AreaUnits,
    ArgumentInputType,
    ByteOrder,
    Capability,
    ComponentStatus,
    DataKind,
    DistanceUnits,
    DistanceUnitType,
    FundamentalUnitDimension,
    LengthType,
    MemorySpace,
    WorkflowStatus,
    # Structs
    ErrorEntry,
    # Signal / slot
    IPropertyChanged,
    ISignal,
    # Core interfaces
    IAdaptedOutput,
    IAdaptedOutputFactory,
    IAdaptedOutputFactoryComponent,
    IAdaptedOutputFactoryComponentInfo,
    IArgument,
    ICheckpointableModelComponent,
    ICloneableModelComponent,
    IComponentDataItem,
    IComponentDataItemValueChanged,
    IComponentInfo,
    IComponentStatusChangeEventArgs,
    IDescription,
    IDimension,
    IExchangeItem,
    IExchangeItemChangeEventArgs,
    IIdBasedComponentDataItem,
    IIdentity,
    IInput,
    ILicensedComponent,
    IModelComponent,
    IModelComponentInfo,
    IMultiInput,
    IOutput,
    IQuality,
    IQuantity,
    IUIProvider,
    IUnit,
    IUnitDimensions,
    IValueDefinition,
    IWorkflowComponent,
    IWorkflowComponentInfo,
    IWorkflowComponentStatusChangeEventArgs,
)

# PEP 440 form of the C++ release designation 2.0.0-alpha.1
__version__ = "2.0.0a1"

__all__ = [
    "AreaUnits",
    "ArgumentInputType",
    "ByteOrder",
    "Capability",
    "ComponentStatus",
    "DataKind",
    "DistanceUnits",
    "DistanceUnitType",
    "ErrorEntry",
    "FundamentalUnitDimension",
    "IAdaptedOutput",
    "IAdaptedOutputFactory",
    "IAdaptedOutputFactoryComponent",
    "IAdaptedOutputFactoryComponentInfo",
    "IArgument",
    "ICheckpointableModelComponent",
    "ICloneableModelComponent",
    "IComponentDataItem",
    "IComponentDataItemValueChanged",
    "IComponentInfo",
    "IComponentStatusChangeEventArgs",
    "IDescription",
    "IDimension",
    "IExchangeItem",
    "IExchangeItemChangeEventArgs",
    "IIdBasedComponentDataItem",
    "IIdentity",
    "IInput",
    "ILicensedComponent",
    "IModelComponent",
    "IModelComponentInfo",
    "IMultiInput",
    "IOutput",
    "IPropertyChanged",
    "IQuality",
    "IQuantity",
    "ISignal",
    "IUIProvider",
    "IUnit",
    "IUnitDimensions",
    "IValueDefinition",
    "IWorkflowComponent",
    "IWorkflowComponentInfo",
    "IWorkflowComponentStatusChangeEventArgs",
    "LengthType",
    "MemorySpace",
    "WorkflowStatus",
]
