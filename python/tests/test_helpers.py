"""
Tests for hydrocouple.helpers: dtype mapping, transition table, typed
conveniences over the hyperslab data plane, and Julian day conversions.
"""

import datetime as _dt

import numpy as np
import pytest

from hydrocouple.core import ComponentStatus, DataKind
from hydrocouple import helpers

from conftest import GridDataItem


class TestDataKindDtypeMapping:
    def test_round_trip_all_numeric_kinds(self):
        for kind, dtype in helpers.DATA_KIND_TO_DTYPE.items():
            assert helpers.data_kind_of(dtype) == kind
            assert helpers.dtype_of(kind) == dtype

    def test_sizes_match_numpy(self):
        assert helpers.data_kind_size(DataKind.Float64) == 8
        assert helpers.data_kind_size(DataKind.Float32) == 4
        assert helpers.data_kind_size(DataKind.Int64) == 8
        assert helpers.data_kind_size(DataKind.Boolean) == 1
        assert helpers.data_kind_size(DataKind.String) == 0
        assert helpers.data_kind_size(DataKind.Opaque) == 0
        assert helpers.data_kind_size(DataKind.Unknown) == 0

    def test_unmapped_dtype_is_opaque(self):
        assert helpers.data_kind_of(np.dtype("datetime64[s]")) == DataKind.Opaque

    def test_non_native_endian_rejected(self):
        swapped = ">f8" if np.little_endian else "<f8"
        assert helpers.data_kind_of(np.dtype(swapped)) == DataKind.Opaque

    def test_dtype_of_raises_for_non_numeric(self):
        with pytest.raises(ValueError):
            helpers.dtype_of(DataKind.String)


class TestTransitionTable:
    CS = ComponentStatus

    def test_happy_path(self):
        f = helpers.is_valid_component_status_transition
        assert f(self.CS.Created, self.CS.Initializing)
        assert f(self.CS.Initializing, self.CS.Initialized)
        assert f(self.CS.Updated, self.CS.Checkpointing)
        assert f(self.CS.Checkpointing, self.CS.Updated)
        assert f(self.CS.Finishing, self.CS.Finished)

    def test_illegal(self):
        f = helpers.is_valid_component_status_transition
        assert not f(self.CS.Created, self.CS.Updating)
        assert not f(self.CS.Finished, self.CS.Created)
        assert not f(self.CS.Done, self.CS.Updating)
        assert not f(self.CS.Checkpointing, self.CS.Updating)

    def test_failure_and_recovery(self):
        f = helpers.is_valid_component_status_transition
        assert f(self.CS.Updating, self.CS.Failed)
        assert f(self.CS.Failed, self.CS.Initializing)
        assert f(self.CS.Failed, self.CS.Finishing)

    def test_table_is_total(self):
        # Every status appears as a source in the table.
        for status in ComponentStatus:
            helpers.is_valid_component_status_transition(
                status, ComponentStatus.Created)


class TestGridDataItemContract:
    """Pure-Python mirror of the C++ GridDataItem contract tests."""

    def test_full_slab_round_trip(self, grid_item):
        ok, values, message = helpers.get_values(grid_item, (0, 0), (4, 5))
        assert ok, message
        assert values.shape == (4, 5)
        assert values[0, 0] == 0.0
        assert values[1, 2] == 12.0
        assert values[3, 4] == 34.0

    def test_interior_hyperslab(self, grid_item):
        ok, values, message = helpers.get_values(grid_item, (1, 2), (2, 3))
        assert ok, message
        np.testing.assert_array_equal(
            values, [[12.0, 13.0, 14.0], [22.0, 23.0, 24.0]])

    def test_scalar_helpers(self, grid_item):
        ok, value, message = helpers.get_value(grid_item, (2, 3))
        assert ok, message
        assert value == 23.0

        ok, message = helpers.set_value(grid_item, 99.5, (2, 3))
        assert ok, message
        ok, value, _ = helpers.get_value(grid_item, (2, 3))
        assert value == 99.5

    def test_set_hyperslab_then_read_back(self, grid_item):
        block = np.array([[100.0, 101.0], [110.0, 111.0]])
        ok, message = helpers.set_values(grid_item, block, (2, 1), (2, 2))
        assert ok, message
        assert grid_item._data[2, 1] == 100.0
        assert grid_item._data[3, 2] == 111.0
        # Neighbors untouched
        assert grid_item._data[2, 0] == 20.0
        assert grid_item._data[3, 3] == 33.0

    def test_kind_mismatch_rejected(self, grid_item):
        wrong = np.zeros(6, dtype=np.float32)
        ok, message = grid_item.get_values_into(wrong, (0, 0), (2, 3))
        assert not ok
        assert message == "kind mismatch"

    def test_out_of_bounds_rejected(self, grid_item):
        out = np.zeros(6, dtype=np.float64)
        ok, message = grid_item.get_values_into(out, (3, 3), (2, 3))
        assert not ok
        assert message == "selection out of bounds"

    def test_raising_wrappers(self, grid_item):
        values = helpers.get_values_or_raise(grid_item, (0, 0), (1, 5))
        assert values.shape == (1, 5)
        with pytest.raises(RuntimeError, match="out of bounds"):
            helpers.get_values_or_raise(grid_item, (9, 9), (1, 1))


class TestJulianDayConversions:
    def test_unix_epoch(self):
        result = helpers.julian_day_to_datetime(2440587.5)
        assert result == _dt.datetime(1970, 1, 1, tzinfo=_dt.timezone.utc)

    def test_j2000(self):
        result = helpers.julian_day_to_datetime(2451545.0)
        expected = _dt.datetime(2000, 1, 1, 12, tzinfo=_dt.timezone.utc)
        assert abs((result - expected).total_seconds()) < 1e-3

    def test_round_trip(self):
        original = _dt.datetime(2024, 6, 15, 8, 30, tzinfo=_dt.timezone.utc)
        jd = helpers.datetime_to_julian_day(original)
        back = helpers.julian_day_to_datetime(jd)
        assert abs((back - original).total_seconds()) < 1e-3

    def test_days_to_timedelta(self):
        assert helpers.days_to_timedelta(7.0) == _dt.timedelta(days=7)
