"""
Tests for spatiotemporal ABCs and Cython wrapper registration (v2).
"""

from hydrocouple.spatial import IGeometryComponentDataItem
from hydrocouple.spatiotemporal import (
    ITimeGeometryComponentDataItem,
    ITimeNetworkComponentDataItem,
    ITimeRegularGrid2DComponentDataItem,
    ITimeRegularGrid3DComponentDataItem,
    ITimeSeriesPolyhedralSurfaceComponentDataItem,
    ITimeSeriesRasterComponentDataItem,
    ITimeSeriesTINComponentDataItem,
)
from hydrocouple.temporal import ITimeSeriesComponentDataItem


class TestSpatioTemporalABCs:
    def test_multiple_inheritance(self):
        assert issubclass(ITimeGeometryComponentDataItem,
                          ITimeSeriesComponentDataItem)
        assert issubclass(ITimeGeometryComponentDataItem,
                          IGeometryComponentDataItem)
        assert issubclass(ITimeSeriesTINComponentDataItem,
                          ITimeSeriesPolyhedralSurfaceComponentDataItem)


class TestSpatioTemporalABCRegistration:
    def test_wrappers_registered(self):
        from _hydrocouple._spatiotemporal import (
            CppTimeGeometryComponentDataItemWrapper,
            CppTimeNetworkComponentDataItemWrapper,
            CppTimeRegularGrid2DComponentDataItemWrapper,
            CppTimeRegularGrid3DComponentDataItemWrapper,
            CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper,
            CppTimeSeriesRasterComponentDataItemWrapper,
            CppTimeSeriesTINComponentDataItemWrapper,
        )

        assert issubclass(CppTimeGeometryComponentDataItemWrapper,
                          ITimeGeometryComponentDataItem)
        assert issubclass(CppTimeNetworkComponentDataItemWrapper,
                          ITimeNetworkComponentDataItem)
        assert issubclass(
            CppTimeSeriesPolyhedralSurfaceComponentDataItemWrapper,
            ITimeSeriesPolyhedralSurfaceComponentDataItem)
        assert issubclass(CppTimeSeriesTINComponentDataItemWrapper,
                          ITimeSeriesTINComponentDataItem)
        assert issubclass(CppTimeSeriesRasterComponentDataItemWrapper,
                          ITimeSeriesRasterComponentDataItem)
        assert issubclass(CppTimeRegularGrid2DComponentDataItemWrapper,
                          ITimeRegularGrid2DComponentDataItem)
        assert issubclass(CppTimeRegularGrid3DComponentDataItemWrapper,
                          ITimeRegularGrid3DComponentDataItem)
