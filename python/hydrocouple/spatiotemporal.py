"""
HydroCouple spatiotemporal abstract base classes.

Python ABC mirrors of the C++ HydroCouple v2.0.0 interfaces defined in
``hydrocouplespatiotemporal.h``: component data items whose values vary in
both time and space, formed by combining the temporal and spatial data
item interfaces. Data access is the inherited hyperslab API; each class
documents its canonical dimension ordering (time is always outermost).
"""

from __future__ import annotations

from abc import abstractmethod

from hydrocouple.spatial import (
    IGeometryComponentDataItem,
    INetworkComponentDataItem,
    IPolyhedralSurfaceComponentDataItem,
    IRasterComponentDataItem,
    IRegularGrid2DComponentDataItem,
    IRegularGrid3DComponentDataItem,
    ITIN,
)
from hydrocouple.temporal import ITimeSeriesComponentDataItem


class ITimeGeometryComponentDataItem(ITimeSeriesComponentDataItem,
                                     IGeometryComponentDataItem):
    """Geometry data varying in time.

    Mirrors C++ ``SpatioTemporal::ITimeGeometryComponentDataItem``.
    Canonical dimension ordering: time is dimension 0, geometry is
    dimension 1; "current time step, all geometries" is a contiguous slab.
    """


class ITimeNetworkComponentDataItem(ITimeSeriesComponentDataItem,
                                    INetworkComponentDataItem):
    """Network data varying in time.

    Mirrors C++ ``SpatioTemporal::ITimeNetworkComponentDataItem``.
    Canonical dimension ordering: time is dimension 0, the entity dimension
    selected by ``network_data_type`` is dimension 1.
    """


class ITimeSeriesPolyhedralSurfaceComponentDataItem(
        ITimeSeriesComponentDataItem, IPolyhedralSurfaceComponentDataItem):
    """Polyhedral-surface data varying in time.

    Mirrors C++
    ``SpatioTemporal::ITimeSeriesPolyhedralSurfaceComponentDataItem``.
    Canonical dimension ordering: time is dimension 0, the entity dimension
    selected by ``mesh_data_type`` is dimension 1.
    """


class ITimeSeriesTINComponentDataItem(
        ITimeSeriesPolyhedralSurfaceComponentDataItem):
    """A time-varying polyhedral-surface data item whose surface is a TIN.

    Mirrors C++ ``SpatioTemporal::ITimeSeriesTINComponentDataItem``.
    """

    @property
    @abstractmethod
    def tin(self) -> ITIN:
        """The associated TIN (C++ ``TIN()``)."""
        raise NotImplementedError


class ITimeSeriesRasterComponentDataItem(ITimeSeriesComponentDataItem,
                                         IRasterComponentDataItem):
    """Raster data varying in time.

    Mirrors C++ ``SpatioTemporal::ITimeSeriesRasterComponentDataItem``.
    Canonical dimension ordering: time 0, band 1, y (row) 2, x (column) 3.
    """


class ITimeRegularGrid2DComponentDataItem(ITimeSeriesComponentDataItem,
                                          IRegularGrid2DComponentDataItem):
    """2D regular-grid data varying in time.

    Mirrors C++ ``SpatioTemporal::ITimeRegularGrid2DComponentDataItem``.
    Canonical dimension ordering: time 0, y-cell 1, x-cell 2; optional cell
    edge and cell vertex dimensions follow.
    """


class ITimeRegularGrid3DComponentDataItem(ITimeSeriesComponentDataItem,
                                          IRegularGrid3DComponentDataItem):
    """3D regular-grid data varying in time.

    Mirrors C++ ``SpatioTemporal::ITimeRegularGrid3DComponentDataItem``.
    Canonical dimension ordering: time 0, z-cell 1, y-cell 2, x-cell 3;
    optional cell face and cell vertex dimensions follow.
    """
