"""
Shared fixtures and helper classes for HydroCouple Python binding tests.

Provides:
- DummyComponent: a minimal v2 IModelComponent implementation
- GridDataItem: a working pure-Python IComponentDataItem over a 2D
  float64 array with correct hyperslab semantics (the reference contract
  implementation, mirroring tests/test_core_types.cpp in the C++ repo)
- ConcreteDateTime / ConcreteTimeSpan: minimal temporal implementations
"""

import numpy as np
import pytest

from hydrocouple.core import (
    Capability,
    ComponentStatus,
    DataKind,
    ErrorEntry,
    IComponentDataItem,
    IModelComponent,
)
from hydrocouple.temporal import IDateTime, ITimeSpan


# ======================================================================
# Minimal concrete Python component for testing
# ======================================================================
class DummyComponent(IModelComponent):
    """Minimal v2 IModelComponent implementation for testing."""

    def __init__(self):
        self._id = "dummy-001"
        self._caption = "Dummy"
        self._description = "A test component"
        self._status = ComponentStatus.Created
        self._ref_dir = "/tmp"
        self._lifecycle = []
        self._errors = []

    # -- ISignal / IPropertyChanged ----------------------------------------
    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    # -- IDescription ------------------------------------------------------
    @property
    def caption(self):
        return self._caption

    @caption.setter
    def caption(self, value):
        self._caption = value

    @property
    def description(self):
        return self._description

    @description.setter
    def description(self, value):
        self._description = value

    # -- IIdentity ---------------------------------------------------------
    @property
    def id(self):
        return self._id

    # -- IModelComponent ---------------------------------------------------
    @property
    def component_info(self):
        return None

    @property
    def status(self):
        return self._status

    @property
    def arguments(self):
        return []

    @property
    def inputs(self):
        return []

    @property
    def outputs(self):
        return []

    @property
    def results(self):
        return []

    def initialize(self):
        self._status = ComponentStatus.Initializing
        self._lifecycle.append("initialize")
        self._status = ComponentStatus.Initialized

    def validate(self):
        self._lifecycle.append("validate")
        return []

    def prepare(self):
        self._status = ComponentStatus.Preparing
        self._lifecycle.append("prepare")
        self._status = ComponentStatus.Updated

    def update(self, required_outputs=None):
        self._status = ComponentStatus.Updating
        self._lifecycle.append("update")
        self._status = ComponentStatus.Updated

    def finish(self):
        self._status = ComponentStatus.Finishing
        self._lifecycle.append("finish")
        self._status = ComponentStatus.Finished

    @property
    def workflow(self):
        return None

    @workflow.setter
    def workflow(self, value):
        pass

    def capabilities(self):
        return set()

    def errors(self, clear_after_read=False):
        entries = list(self._errors)
        if clear_after_read:
            self._errors.clear()
        return entries

    @property
    def reference_directory(self):
        return self._ref_dir

    @reference_directory.setter
    def reference_directory(self, value):
        self._ref_dir = value


# ======================================================================
# Reference data item — pure-Python mirror of the C++ GridDataItem
# contract test (tests/test_core_types.cpp)
# ======================================================================
class GridDataItem(IComponentDataItem):
    """A working IComponentDataItem over a row-major [rows][cols] float64
    array with correct hyperslab semantics."""

    def __init__(self, rows, cols):
        self._rows = rows
        self._cols = cols
        self._data = np.zeros((rows, cols), dtype=np.float64)
        self._id = "grid"

    # -- Signals -----------------------------------------------------------
    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    # -- Identity ----------------------------------------------------------
    @property
    def caption(self):
        return ""

    @caption.setter
    def caption(self, value):
        pass

    @property
    def description(self):
        return ""

    @description.setter
    def description(self, value):
        pass

    @property
    def id(self):
        return self._id

    # -- IComponentDataItem ------------------------------------------------
    @property
    def model_component(self):
        return None

    @property
    def dimensions(self):
        return []

    @property
    def shape(self):
        return (self._rows, self._cols)

    @property
    def data_kind(self):
        return DataKind.Float64

    @property
    def value_definition(self):
        return None

    def _validate(self, arr, start, count):
        from hydrocouple.helpers import data_kind_of
        if data_kind_of(arr.dtype) != DataKind.Float64:
            return "kind mismatch"
        if len(start) != 2 or len(count) != 2:
            return "selection rank must be 2"
        if (start[0] < 0 or start[1] < 0
                or start[0] + count[0] > self._rows
                or start[1] + count[1] > self._cols):
            return "selection out of bounds"
        if arr.size != count[0] * count[1]:
            return "buffer element count mismatch"
        return None

    def get_values_into(self, destination, start, count):
        error = self._validate(destination, start, count)
        if error:
            return False, error
        selection = self._data[start[0]:start[0] + count[0],
                               start[1]:start[1] + count[1]]
        destination.reshape(count[0], count[1])[...] = selection
        return True, ""

    def set_values_from(self, source, start, count):
        error = self._validate(source, start, count)
        if error:
            return False, error
        self._data[start[0]:start[0] + count[0],
                   start[1]:start[1] + count[1]] = source.reshape(
            count[0], count[1])
        return True, ""


# ======================================================================
# Minimal temporal implementations for testing
# ======================================================================
class ConcreteDateTime(IDateTime):
    """Minimal IDateTime for testing conversions."""

    def __init__(self, jd: float):
        self._jd = jd

    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    @property
    def julian_day(self):
        return self._jd

    @property
    def modified_julian_day(self):
        return self._jd - 2400000.5

    @property
    def serial_date(self):
        return self._jd - 2415018.5


class ConcreteTimeSpan(ITimeSpan):
    """Minimal ITimeSpan for testing conversions."""

    def __init__(self, jd: float, dur: float):
        self._jd = jd
        self._dur = dur

    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    @property
    def julian_day(self):
        return self._jd

    @property
    def modified_julian_day(self):
        return self._jd - 2400000.5

    @property
    def serial_date(self):
        return self._jd - 2415018.5

    @property
    def duration(self):
        return self._dur


# ======================================================================
# Fixtures
# ======================================================================
@pytest.fixture
def dummy_component():
    """Return a fresh DummyComponent instance."""
    return DummyComponent()


@pytest.fixture
def grid_item():
    """Return a 4x5 GridDataItem seeded with value r*10 + c."""
    grid = GridDataItem(4, 5)
    for r in range(4):
        for c in range(5):
            grid._data[r, c] = r * 10 + c
    return grid
