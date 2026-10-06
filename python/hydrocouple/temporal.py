"""
HydroCouple temporal abstract base classes.

Python ABC mirrors of the C++ HydroCouple v2.0.0 interfaces defined in
``hydrocoupletemporal.h``: date/time representation, time spans,
time-marching model components, and time-series component data items.
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from enum import IntEnum
from typing import TYPE_CHECKING

from hydrocouple.core import (
    IComponentDataItem,
    IDimension,
    IModelComponent,
    IPropertyChanged,
)

if TYPE_CHECKING:
    import numpy as np


class TimeKind(IntEnum):
    """What a value's time coordinate refers to; mirrors C++
    ``Temporal::TimeKind``."""

    Unknown = 0
    Instantaneous = 1
    IntervalMean = 2
    IntervalMinimum = 3
    IntervalMaximum = 4
    Accumulated = 5


class TimeInterpolation(IntEnum):
    """How a provider produces a value between held instants; mirrors C++
    ``Temporal::TimeInterpolation``."""

    Unknown = 0
    NoInterpolation = 1  # C++ ``None``; not a legal Python identifier
    Previous = 2
    Nearest = 3
    Linear = 4


class TimeExtrapolation(IntEnum):
    """How a provider produces a value outside its held range; mirrors C++
    ``Temporal::TimeExtrapolation``."""

    Unknown = 0
    Refuse = 1
    HoldLast = 2
    Linear = 3


class IDateTime(IPropertyChanged):
    """A date/time based on a Julian day.

    Mirrors C++ ``Temporal::IDateTime``. The normative convention is the
    astronomical Julian day number in the proleptic Gregorian ("standard")
    calendar, UTC; persistence layers writing CF metadata should emit
    ``units = "days since ..."`` with ``calendar = "standard"``.
    """

    @property
    @abstractmethod
    def julian_day(self) -> float:
        """Date and time as a Julian day value."""
        raise NotImplementedError

    @property
    @abstractmethod
    def modified_julian_day(self) -> float:
        """Modified Julian day value."""
        raise NotImplementedError

    @property
    @abstractmethod
    def serial_date(self) -> float:
        """Serial date number."""
        raise NotImplementedError


class ITimeSpan(IDateTime):
    """A time duration anchored at a start date/time.

    Mirrors C++ ``Temporal::ITimeSpan``.
    """

    @property
    @abstractmethod
    def duration(self) -> float:
        """Duration of the timespan in days."""
        raise NotImplementedError

    @property
    @abstractmethod
    def end_julian_day(self) -> float:
        """End of the interval: ``julian_day + duration``."""
        raise NotImplementedError


class ITimeModelComponent(IModelComponent):
    """A model component that advances through time during simulation.

    Mirrors C++ ``Temporal::ITimeModelComponent``.
    """

    @property
    @abstractmethod
    def current_date_time(self) -> IDateTime:
        """Current date and time of the model simulation."""
        raise NotImplementedError

    @property
    @abstractmethod
    def simulation_period(self) -> ITimeSpan:
        """The time horizon of the model."""
        raise NotImplementedError

    @property
    @abstractmethod
    def next_date_time_julian_day(self) -> float:
        """The Julian day the next ``update()`` will advance
        :attr:`current_date_time` to; never earlier than the current time."""
        raise NotImplementedError


class ITimeSeriesComponentDataItem(IComponentDataItem):
    """An :class:`~hydrocouple.core.IComponentDataItem` with a temporal
    dimension.

    Mirrors C++ ``Temporal::ITimeSeriesComponentDataItem``. Canonical
    dimension ordering: time is dimension 0 of :attr:`shape`; any
    additional dimensions follow. Data access uses the inherited
    :meth:`~hydrocouple.core.IComponentDataItem.get_values_into` /
    :meth:`~hydrocouple.core.IComponentDataItem.set_values_from` hyperslab
    API with the time index as ``start[0]``, so "current time step, all
    entities" is a contiguous slab.
    """

    @abstractmethod
    def time(self, time_index: int) -> IDateTime:
        """The :class:`IDateTime` at the given time index (spot queries)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def time_count(self) -> int:
        """The number of times."""
        raise NotImplementedError

    @property
    @abstractmethod
    def times(self) -> "np.ndarray":
        """Bulk access to all time coordinates as Julian day values.

        A float64 array of :attr:`time_count` elements ordered with the
        time dimension; the accessor IO writers and interpolating adapters
        must use.
        """
        raise NotImplementedError

    @property
    @abstractmethod
    def time_span(self) -> ITimeSpan:
        """The time span covered by this data item."""
        raise NotImplementedError

    @property
    @abstractmethod
    def time_dimension(self) -> IDimension:
        """The time dimension (dimension 0 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def time_kind(self) -> TimeKind:
        """What each value's time coordinate refers to."""
        raise NotImplementedError

    @property
    @abstractmethod
    def interval_length(self) -> float:
        """Averaging/accumulation interval in days; 0 for instantaneous."""
        raise NotImplementedError

    @property
    @abstractmethod
    def time_interpolation(self) -> TimeInterpolation:
        """What this item does (output) or accepts (input) between held instants."""
        raise NotImplementedError

    @property
    @abstractmethod
    def time_extrapolation(self) -> TimeExtrapolation:
        """What this item does (output) or accepts (input) outside its held range."""
        raise NotImplementedError


class ITimeIdBasedComponentDataItem(ITimeSeriesComponentDataItem):
    """A time-series data item whose entity dimension is identifier-based.

    Mirrors C++ ``Temporal::ITimeIdBasedComponentDataItem``. Canonical
    dimension ordering: time is dimension 0, the identifier dimension is
    dimension 1 of :attr:`shape`; any additional dimensions follow.
    """

    @property
    @abstractmethod
    def identifiers(self) -> list[str]:
        """The identifiers of the identifier dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def identifier_dimension(self) -> IDimension:
        """The identifier dimension (dimension 1 of :attr:`shape`)."""
        raise NotImplementedError
