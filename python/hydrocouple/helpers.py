"""
Non-normative convenience helpers for the HydroCouple Python bindings.

Mirrors ``hydrocouplehelpers.h`` — the single sanctioned exception to the
standard's no-implementation rule. Nothing here is required to implement or
consume the standard; it may be ignored entirely.

The :data:`DATA_KIND_TO_DTYPE` / :data:`DTYPE_TO_DATA_KIND` maps are the
Python spelling of the C++ ``DataKindOf<T>`` trait: they tie the standard's
:class:`~hydrocouple.core.DataKind` vocabulary to NumPy dtypes, which are
the Python-side ``BufferDescriptor``.
"""

from __future__ import annotations

import datetime as _dt
from typing import TYPE_CHECKING, Optional, Sequence

import numpy as np

from hydrocouple.core import ComponentStatus, DataKind

if TYPE_CHECKING:
    from hydrocouple.core import IComponentDataItem

__all__ = [
    "DATA_KIND_TO_DTYPE",
    "DTYPE_TO_DATA_KIND",
    "JULIAN_DAY_UNIX_EPOCH",
    "data_kind_size",
    "data_kind_of",
    "dtype_of",
    "is_valid_component_status_transition",
    "get_value",
    "set_value",
    "get_values",
    "set_values",
    "get_values_or_raise",
    "set_values_or_raise",
    "julian_day_to_datetime",
    "datetime_to_julian_day",
    "days_to_timedelta",
]


# ---------------------------------------------------------------------------
# DataKind <-> NumPy dtype mapping (the Python DataKindOf<T>)
# ---------------------------------------------------------------------------

DATA_KIND_TO_DTYPE: dict[DataKind, np.dtype] = {
    DataKind.Int8: np.dtype(np.int8),
    DataKind.UInt8: np.dtype(np.uint8),
    DataKind.Int16: np.dtype(np.int16),
    DataKind.UInt16: np.dtype(np.uint16),
    DataKind.Int32: np.dtype(np.int32),
    DataKind.UInt32: np.dtype(np.uint32),
    DataKind.Int64: np.dtype(np.int64),
    DataKind.UInt64: np.dtype(np.uint64),
    DataKind.Float32: np.dtype(np.float32),
    DataKind.Float64: np.dtype(np.float64),
    DataKind.Boolean: np.dtype(np.bool_),
}
"""Native-endian NumPy dtype for each numeric :class:`DataKind`.

``String``, ``Opaque``, and ``Unknown`` have no dtype mapping.
"""

DTYPE_TO_DATA_KIND: dict[np.dtype, DataKind] = {
    dtype: kind for kind, dtype in DATA_KIND_TO_DTYPE.items()
}
"""Inverse of :data:`DATA_KIND_TO_DTYPE`."""


def data_kind_size(kind: DataKind) -> int:
    """Size in bytes of one element of the given kind.

    Mirrors ``Helpers::dataKindSize``; 0 for String, Opaque, and Unknown.
    """
    dtype = DATA_KIND_TO_DTYPE.get(kind)
    return int(dtype.itemsize) if dtype is not None else 0


def data_kind_of(dtype: "np.dtype | type") -> DataKind:
    """The :class:`DataKind` corresponding to a NumPy dtype.

    Mirrors ``Helpers::DataKindOf``; unmapped dtypes are
    :attr:`DataKind.Opaque`. Byte-swapped (non-native-endian) dtypes are
    deliberately unmapped: the wire and memory formats of the standard are
    native-endian.
    """
    dt = np.dtype(dtype)
    if dt.byteorder not in ("=", "|") and dt.byteorder != _NATIVE_ORDER:
        return DataKind.Opaque
    return DTYPE_TO_DATA_KIND.get(dt.newbyteorder("="), DataKind.Opaque)


_NATIVE_ORDER = "<" if np.little_endian else ">"


def dtype_of(kind: DataKind) -> np.dtype:
    """The NumPy dtype for a numeric :class:`DataKind`.

    :raises ValueError: for String, Opaque, and Unknown.
    """
    dtype = DATA_KIND_TO_DTYPE.get(kind)
    if dtype is None:
        raise ValueError(f"DataKind.{kind.name} has no NumPy dtype mapping")
    return dtype


# ---------------------------------------------------------------------------
# Component lifecycle transition table
# ---------------------------------------------------------------------------

_CS = ComponentStatus

_VALID_TRANSITIONS: dict[ComponentStatus, frozenset[ComponentStatus]] = {
    _CS.Created: frozenset({_CS.Initializing}),
    _CS.Initializing: frozenset({_CS.Initialized, _CS.Failed}),
    _CS.Initialized: frozenset({_CS.Validating, _CS.Initializing}),
    _CS.Validating: frozenset({_CS.Valid, _CS.Invalid}),
    _CS.Valid: frozenset({_CS.Preparing, _CS.Validating}),
    _CS.Invalid: frozenset({_CS.Validating}),
    _CS.Preparing: frozenset({_CS.Updated, _CS.Failed}),
    _CS.Updating: frozenset({_CS.Updated, _CS.Done, _CS.WaitingForData, _CS.Failed}),
    _CS.WaitingForData: frozenset({_CS.Updating, _CS.Failed}),
    _CS.Updated: frozenset({_CS.Updating, _CS.Checkpointing, _CS.Finishing}),
    _CS.Checkpointing: frozenset({_CS.Updated, _CS.Failed}),
    _CS.Done: frozenset({_CS.Finishing}),
    _CS.Finishing: frozenset({_CS.Finished, _CS.Created}),
    _CS.Finished: frozenset(),
    _CS.Failed: frozenset({_CS.Initializing, _CS.Finishing}),
}


def is_valid_component_status_transition(from_status: ComponentStatus,
                                         to_status: ComponentStatus) -> bool:
    """Whether a component status transition is legal.

    Mirrors ``Helpers::isValidComponentStatusTransition`` — the normative
    statement of the component lifecycle state machine. Implementations
    must not perform transitions for which this returns ``False``.
    """
    return to_status in _VALID_TRANSITIONS[from_status]


# ---------------------------------------------------------------------------
# Typed convenience wrappers over the hyperslab data plane
# ---------------------------------------------------------------------------


def get_value(item: "IComponentDataItem", index: Sequence[int]):
    """Read the single element at ``index``; returns ``(ok, value, message)``.

    Mirrors ``Helpers::getValue``.
    """
    dest = np.empty(1, dtype=dtype_of(item.data_kind))
    ok, message = item.get_values_into(dest, list(index), [1] * len(index))
    return ok, dest[0] if ok else None, message


def set_value(item: "IComponentDataItem", value, index: Sequence[int]) -> tuple[bool, str]:
    """Write the single element at ``index``; returns ``(ok, message)``.

    Mirrors ``Helpers::setValue``.
    """
    src = np.asarray([value], dtype=dtype_of(item.data_kind))
    return item.set_values_from(src, list(index), [1] * len(index))


def get_values(item: "IComponentDataItem", start: Sequence[int],
               count: Sequence[int],
               out: Optional[np.ndarray] = None) -> tuple[bool, np.ndarray, str]:
    """Read a hyperslab into a C-ordered array of shape ``count``.

    Mirrors ``Helpers::getValues``; allocates ``out`` when not supplied.
    :returns: ``(ok, values, message)``.
    """
    if out is None:
        out = np.empty(tuple(count), dtype=dtype_of(item.data_kind))
    ok, message = item.get_values_into(out, start, count)
    return ok, out, message


def set_values(item: "IComponentDataItem", values: np.ndarray,
               start: Sequence[int], count: Sequence[int]) -> tuple[bool, str]:
    """Write a hyperslab from ``values``; returns ``(ok, message)``.

    Mirrors ``Helpers::setValues``.
    """
    return item.set_values_from(values, start, count)


def get_values_or_raise(item: "IComponentDataItem", start: Sequence[int],
                        count: Sequence[int],
                        out: Optional[np.ndarray] = None) -> np.ndarray:
    """Raising wrapper over :func:`get_values`.

    :raises RuntimeError: with the item's failure message on error.
    """
    ok, values, message = get_values(item, start, count, out)
    if not ok:
        raise RuntimeError(f"get_values_into failed on '{item.id}': {message}")
    return values


def set_values_or_raise(item: "IComponentDataItem", values: np.ndarray,
                        start: Sequence[int], count: Sequence[int]) -> None:
    """Raising wrapper over :func:`set_values`.

    :raises RuntimeError: with the item's failure message on error.
    """
    ok, message = set_values(item, values, start, count)
    if not ok:
        raise RuntimeError(f"set_values_from failed on '{item.id}': {message}")


# ---------------------------------------------------------------------------
# Julian day conversions (standard calendar, UTC — the IDateTime convention)
# ---------------------------------------------------------------------------

JULIAN_DAY_UNIX_EPOCH = 2440587.5
"""The Julian day of the Unix epoch (1970-01-01 00:00:00 UTC)."""


def julian_day_to_datetime(julian_day: float) -> _dt.datetime:
    """Convert a Julian day value to a timezone-aware UTC datetime."""
    seconds = (julian_day - JULIAN_DAY_UNIX_EPOCH) * 86400.0
    return _dt.datetime.fromtimestamp(seconds, tz=_dt.timezone.utc)


def datetime_to_julian_day(value: _dt.datetime) -> float:
    """Convert a datetime (naive values are taken as UTC) to a Julian day."""
    if value.tzinfo is None:
        value = value.replace(tzinfo=_dt.timezone.utc)
    return JULIAN_DAY_UNIX_EPOCH + value.timestamp() / 86400.0


def days_to_timedelta(days: float) -> _dt.timedelta:
    """Convert a duration in days (e.g. ITimeSpan.duration) to a timedelta."""
    return _dt.timedelta(days=days)
