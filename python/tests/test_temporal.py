"""
Tests for temporal ABCs and Cython temporal wrapper registration (v2).
"""

from hydrocouple.temporal import (
    IDateTime,
    ITimeIdBasedComponentDataItem,
    ITimeModelComponent,
    ITimeSeriesComponentDataItem,
    ITimeSpan,
)

from conftest import ConcreteDateTime, ConcreteTimeSpan


class TestTemporalABCs:
    def test_concrete_datetime_is_idatetime(self):
        dt_obj = ConcreteDateTime(2440587.5)
        assert isinstance(dt_obj, IDateTime)
        assert dt_obj.julian_day == 2440587.5
        assert dt_obj.modified_julian_day == 2440587.5 - 2400000.5

    def test_concrete_timespan_is_itimespan(self):
        ts = ConcreteTimeSpan(2440587.5, 7.0)
        assert isinstance(ts, ITimeSpan)
        assert isinstance(ts, IDateTime)
        assert ts.duration == 7.0

    def test_time_series_item_declares_bulk_times(self):
        # The v2 contract: bulk `times` plus per-index `time` spot queries.
        assert hasattr(ITimeSeriesComponentDataItem, "times")
        assert hasattr(ITimeSeriesComponentDataItem, "time")
        assert hasattr(ITimeSeriesComponentDataItem, "time_count")
        assert hasattr(ITimeSeriesComponentDataItem, "time_dimension")

    def test_id_based_item_contract(self):
        assert hasattr(ITimeIdBasedComponentDataItem, "identifiers")
        assert hasattr(ITimeIdBasedComponentDataItem, "identifier_dimension")


class TestTemporalABCRegistration:
    def test_wrappers_registered(self):
        from _hydrocouple._temporal import (
            CppDateTimeWrapper,
            CppTimeIdBasedComponentDataItemWrapper,
            CppTimeModelComponentWrapper,
            CppTimeSeriesComponentDataItemWrapper,
            CppTimeSpanWrapper,
        )

        assert issubclass(CppDateTimeWrapper, IDateTime)
        assert issubclass(CppTimeSpanWrapper, ITimeSpan)
        assert issubclass(CppTimeModelComponentWrapper, ITimeModelComponent)
        assert issubclass(CppTimeSeriesComponentDataItemWrapper,
                          ITimeSeriesComponentDataItem)
        assert issubclass(CppTimeIdBasedComponentDataItemWrapper,
                          ITimeIdBasedComponentDataItem)

    def test_wrapper_methods_available(self):
        from _hydrocouple._temporal import (
            CppDateTimeWrapper,
            CppTimeSeriesComponentDataItemWrapper,
            CppTimeSpanWrapper,
        )

        assert hasattr(CppDateTimeWrapper, "julian_day")
        assert hasattr(CppTimeSpanWrapper, "duration")
        assert hasattr(CppTimeSeriesComponentDataItemWrapper, "times")
        assert hasattr(CppTimeSeriesComponentDataItemWrapper, "time_count")
