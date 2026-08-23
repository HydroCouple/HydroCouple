"""
HydroCouple geospatial abstract base classes.

Python ABC mirrors of the C++ HydroCouple v2.0.0 interfaces defined in
``hydrocouplespatial.h``: OGC Simple Features geometry types, spatial
reference systems, mesh/network structures with bulk structure-of-arrays
views, rasters, regular grids, and the spatial component data items.

Bulk accessors (:class:`IMeshView`, grid coordinate arrays) return NumPy
arrays and are the accessors partitioners, interpolating adapters, IO
writers, and device staging must use; per-entity object accessors are a
convenience for spot queries and editing.
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from enum import IntEnum
from typing import TYPE_CHECKING, Optional

from hydrocouple.core import (
    DistanceUnits,
    IComponentDataItem,
    IDimension,
    IIdentity,
)

if TYPE_CHECKING:
    import numpy as np


# ---------------------------------------------------------------------------
# Enums
# ---------------------------------------------------------------------------


class MeshDataObjectType(IntEnum):
    """Part of a mesh's geometry that data corresponds to.

    Mirrors C++ ``Spatial::MeshDataObjectType``.
    """

    Cell = 0
    Vertex = 1
    Edge = 2
    Face = 3


class NetworkDataObjectType(IntEnum):
    """Part of a network that data corresponds to.

    Mirrors C++ ``Spatial::NetworkDataObjectType``.
    """

    Node = 0
    Edge = 1


class SpatialDataType(IntEnum):
    """Structure of per-entity values.

    Mirrors C++ ``Spatial::SpatialDataType``.
    """

    Scalar = 0
    MultiScalar = 1
    Vector = 2
    Tensor = 3


class RegularGridType(IntEnum):
    """Type of a regular grid.

    Mirrors C++ ``Spatial::RegularGridType``.
    """

    Cartesian = 0
    Rectilinear = 1
    Curvilinear = 2


class GeometryType(IntEnum):
    """OGC geometry type codes mirroring C++ ``IGeometry::GeometryType``."""

    Geometry = 0
    Point = 1
    LineString = 2
    Polygon = 3
    MultiPoint = 4
    MultiLineString = 5
    MultiPolygon = 6
    GeometryCollection = 7
    CircularString = 8
    CompoundCurve = 9
    CurvePolygon = 10
    MultiCurve = 11
    MultiSurface = 12
    Curve = 13
    Surface = 14
    PolyhedralSurface = 15
    TIN = 16
    Triangle = 17

    GeometryZ = 1000
    PointZ = 1001
    LineStringZ = 1002
    PolygonZ = 1003
    MultiPointZ = 1004
    MultiLineStringZ = 1005
    MultiPolygonZ = 1006
    GeometryCollectionZ = 1007
    CircularStringZ = 1008
    CompoundCurveZ = 1009
    CurvePolygonZ = 1010
    MultiCurveZ = 1011
    MultiSurfaceZ = 1012
    CurveZ = 1013
    SurfaceZ = 1014
    PolyhedralSurfaceZ = 1015
    TINZ = 1016
    TriangleZ = 1017

    GeometryM = 2000
    PointM = 2001
    LineStringM = 2002
    PolygonM = 2003
    MultiPointM = 2004
    MultiLineStringM = 2005
    MultiPolygonM = 2006
    GeometryCollectionM = 2007
    CircularStringM = 2008
    CompoundCurveM = 2009
    CurvePolygonM = 2010
    MultiCurveM = 2011
    MultiSurfaceM = 2012
    CurveM = 2013
    SurfaceM = 2014
    PolyhedralSurfaceM = 2015
    TINM = 2016
    TriangleM = 2017

    GeometryZM = 3000
    PointZM = 3001
    LineStringZM = 3002
    PolygonZM = 3003
    MultiPointZM = 3004
    MultiLineStringZM = 3005
    MultiPolygonZM = 3006
    GeometryCollectionZM = 3007
    CircularStringZM = 3008
    CompoundCurveZM = 3009
    CurvePolygonZM = 3010
    MultiCurveZM = 3011
    MultiSurfaceZM = 3012
    CurveZM = 3013
    SurfaceZM = 3014
    PolyhedralSurfaceZM = 3015
    TINZM = 3016
    TriangleZM = 3017


class RasterDataType(IntEnum):
    """Raster band element type mirroring C++ ``IRaster::RasterDataType``."""

    Unknown = 0
    Byte = 1
    UInt16 = 2
    Int16 = 3
    UInt32 = 4
    Int32 = 5
    Float32 = 6
    Float64 = 7
    CInt16 = 8
    CInt32 = 9
    CFloat32 = 10
    CFloat64 = 11
    ARGB32 = 12
    ARGB32_Premultiplied = 13


# ---------------------------------------------------------------------------
# Spatial reference system and envelope
# ---------------------------------------------------------------------------


class ISpatialReferenceSystem(ABC):
    """Spatial reference system of a geometric object.

    Mirrors C++ ``Spatial::ISpatialReferenceSystem``.
    """

    @property
    @abstractmethod
    def auth_srid(self) -> int:
        """The authority-specific spatial reference id (e.g. EPSG code)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def auth_name(self) -> str:
        """The authority name (e.g. ``"EPSG"``)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def sr_text(self) -> str:
        """Well-known text representation of the SRS."""
        raise NotImplementedError

    @property
    @abstractmethod
    def distance_units(self) -> DistanceUnits:
        """The measurement distance units of the SRS."""
        raise NotImplementedError


class IEnvelope(ABC):
    """Axis-aligned bounding box.

    Mirrors C++ ``Spatial::IEnvelope``.
    """

    @property
    @abstractmethod
    def min_x(self) -> float:
        """Minimum x."""
        raise NotImplementedError

    @property
    @abstractmethod
    def max_x(self) -> float:
        """Maximum x."""
        raise NotImplementedError

    @property
    @abstractmethod
    def min_y(self) -> float:
        """Minimum y."""
        raise NotImplementedError

    @property
    @abstractmethod
    def max_y(self) -> float:
        """Maximum y."""
        raise NotImplementedError

    @property
    @abstractmethod
    def min_z(self) -> float:
        """Minimum z."""
        raise NotImplementedError

    @property
    @abstractmethod
    def max_z(self) -> float:
        """Maximum z."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Geometry hierarchy (OGC Simple Features)
# ---------------------------------------------------------------------------


class IGeometry(ABC):
    """Base OGC geometry.

    Mirrors C++ ``Spatial::IGeometry``.
    """

    @property
    @abstractmethod
    def id(self) -> str:
        """Id of the geometry."""
        raise NotImplementedError

    @property
    @abstractmethod
    def index(self) -> int:
        """Index of the geometry within a collection."""
        raise NotImplementedError

    @property
    @abstractmethod
    def dimension(self) -> int:
        """Topological dimension: 0 points, 1 lines, 2 surfaces."""
        raise NotImplementedError

    @property
    @abstractmethod
    def coordinate_dimension(self) -> int:
        """Dimension of the coordinates (2 or 3; 0 for an empty point)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def geometry_type(self) -> GeometryType:
        """The instantiable OGC subtype of this geometry."""
        raise NotImplementedError

    @property
    @abstractmethod
    def spatial_reference_system(self) -> ISpatialReferenceSystem:
        """The SRS of this geometry."""
        raise NotImplementedError

    @property
    @abstractmethod
    def envelope(self) -> IEnvelope:
        """The bounding envelope of this geometry."""
        raise NotImplementedError

    @abstractmethod
    def get_wkt(self) -> str:
        """Well-known text representation."""
        raise NotImplementedError

    @abstractmethod
    def get_wkb(self) -> bytes:
        """Well-known binary representation."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_empty(self) -> bool:
        """Whether this geometry is the empty geometry."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_simple(self) -> bool:
        """Whether this geometry has no anomalous points."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_3d(self) -> bool:
        """Whether this geometry has z coordinates."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_measured(self) -> bool:
        """Whether this geometry has m values."""
        raise NotImplementedError

    @property
    @abstractmethod
    def boundary(self) -> "IGeometry":
        """The closure of the combinatorial boundary."""
        raise NotImplementedError

    # -- Spatial predicates -------------------------------------------------

    @abstractmethod
    def equals(self, geom: "IGeometry") -> bool:
        """Spatial equality."""
        raise NotImplementedError

    @abstractmethod
    def disjoint(self, geom: "IGeometry") -> bool:
        """Spatially disjoint."""
        raise NotImplementedError

    @abstractmethod
    def intersects(self, geom: "IGeometry") -> bool:
        """Spatially intersects."""
        raise NotImplementedError

    @abstractmethod
    def touches(self, geom: "IGeometry") -> bool:
        """Spatially touches."""
        raise NotImplementedError

    @abstractmethod
    def crosses(self, geom: "IGeometry") -> bool:
        """Spatially crosses."""
        raise NotImplementedError

    @abstractmethod
    def within(self, geom: "IGeometry") -> bool:
        """Spatially within."""
        raise NotImplementedError

    @abstractmethod
    def contains(self, geom: "IGeometry") -> bool:
        """Spatially contains."""
        raise NotImplementedError

    @abstractmethod
    def overlaps(self, geom: "IGeometry") -> bool:
        """Spatially overlaps."""
        raise NotImplementedError

    @abstractmethod
    def relate(self, geom: "IGeometry") -> bool:
        """DE-9IM relation test."""
        raise NotImplementedError

    # -- Measures and constructive operations --------------------------------

    @abstractmethod
    def locate_along(self, value: float) -> "IGeometry":
        """The m-locate-along geometry."""
        raise NotImplementedError

    @abstractmethod
    def locate_between(self, m_start: float, m_end: float) -> "IGeometry":
        """The m-locate-between geometry."""
        raise NotImplementedError

    @abstractmethod
    def distance(self, geom: "IGeometry") -> float:
        """Shortest distance to another geometry."""
        raise NotImplementedError

    @abstractmethod
    def buffer(self, buffer_distance: float) -> "IGeometry":
        """All points within the given distance of this geometry."""
        raise NotImplementedError

    @abstractmethod
    def convex_hull(self) -> "IGeometry":
        """The convex hull."""
        raise NotImplementedError

    @abstractmethod
    def intersection(self, geom: "IGeometry") -> "IGeometry":
        """Point-set intersection."""
        raise NotImplementedError

    @abstractmethod
    def union(self, geom: "IGeometry") -> "IGeometry":
        """Point-set union (C++ ``unionG``)."""
        raise NotImplementedError

    @abstractmethod
    def difference(self, geom: "IGeometry") -> "IGeometry":
        """Point-set difference."""
        raise NotImplementedError

    @abstractmethod
    def symmetric_difference(self, geom: "IGeometry") -> "IGeometry":
        """Point-set symmetric difference."""
        raise NotImplementedError


class IGeometryCollection(IGeometry):
    """A collection of geometries. Mirrors C++ ``IGeometryCollection``."""

    @property
    @abstractmethod
    def geometry_count(self) -> int:
        """Number of geometries in the collection."""
        raise NotImplementedError

    @abstractmethod
    def geometry(self, index: int) -> IGeometry:
        """The geometry at the given index."""
        raise NotImplementedError


class IPoint(IGeometry):
    """A 0-dimensional point. Mirrors C++ ``IPoint``."""

    @property
    @abstractmethod
    def x(self) -> float:
        """x coordinate."""
        raise NotImplementedError

    @property
    @abstractmethod
    def y(self) -> float:
        """y coordinate."""
        raise NotImplementedError

    @property
    @abstractmethod
    def z(self) -> float:
        """z coordinate (when :attr:`IGeometry.is_3d`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def m(self) -> float:
        """m value (when :attr:`IGeometry.is_measured`)."""
        raise NotImplementedError


class IMultiPoint(IGeometryCollection):
    """A collection of points. Mirrors C++ ``IMultiPoint``."""

    @abstractmethod
    def point(self, index: int) -> IPoint:
        """The point at the given index."""
        raise NotImplementedError


class IVertex(IPoint):
    """A point participating in topology. Mirrors C++ ``IVertex``."""

    @property
    @abstractmethod
    def vertex_index(self) -> int:
        """Unique vertex index (C++ ``index()``; renamed to avoid clashing
        with :attr:`IGeometry.index`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def edge(self) -> "IEdge":
        """An arbitrary outgoing edge of this vertex."""
        raise NotImplementedError


class ICurve(IGeometry):
    """A 1-dimensional geometry. Mirrors C++ ``ICurve``."""

    @property
    @abstractmethod
    def length(self) -> float:
        """Curve length."""
        raise NotImplementedError

    @property
    @abstractmethod
    def start_point(self) -> IPoint:
        """First point of the curve."""
        raise NotImplementedError

    @property
    @abstractmethod
    def end_point(self) -> IPoint:
        """Last point of the curve."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_closed(self) -> bool:
        """Whether start and end points coincide."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_ring(self) -> bool:
        """Whether the curve is closed and simple."""
        raise NotImplementedError


class IMultiCurve(IGeometryCollection):
    """A collection of curves. Mirrors C++ ``IMultiCurve``."""

    @property
    @abstractmethod
    def is_closed(self) -> bool:
        """Whether every member curve is closed."""
        raise NotImplementedError

    @property
    @abstractmethod
    def length(self) -> float:
        """Total length of the member curves."""
        raise NotImplementedError


class ILineString(ICurve):
    """A curve with linear interpolation between points.

    Mirrors C++ ``ILineString``.
    """

    @property
    @abstractmethod
    def point_count(self) -> int:
        """Number of points in the line string."""
        raise NotImplementedError

    @abstractmethod
    def point(self, index: int) -> IPoint:
        """The point at the given index."""
        raise NotImplementedError


class IMultiLineString(IMultiCurve):
    """A collection of line strings. Mirrors C++ ``IMultiLineString``."""

    @abstractmethod
    def line_string(self, index: int) -> ILineString:
        """The line string at the given index."""
        raise NotImplementedError


class ILine(ILineString):
    """A line string with exactly two points. Mirrors C++ ``ILine``."""


class ILinearRing(ILineString):
    """A closed, simple line string. Mirrors C++ ``ILinearRing``."""


class IEdge(ABC):
    """A quad-edge topology edge.

    Mirrors C++ ``IEdge``. Per-entity topology navigation is a convenience
    for spot queries and editing; bulk consumers must use
    :class:`IMeshView`.
    """

    @property
    @abstractmethod
    def index(self) -> int:
        """Unique edge index."""
        raise NotImplementedError

    @property
    @abstractmethod
    def orig(self) -> Optional[IVertex]:
        """Origin vertex, or ``None`` if unknown."""
        raise NotImplementedError

    @property
    @abstractmethod
    def dest(self) -> Optional[IVertex]:
        """Destination vertex, or ``None`` if unknown."""
        raise NotImplementedError

    @property
    @abstractmethod
    def left(self) -> Optional["IPolygon"]:
        """Left face, or ``None`` if unknown."""
        raise NotImplementedError

    @property
    @abstractmethod
    def right(self) -> Optional["IPolygon"]:
        """Right face, or ``None`` if unknown."""
        raise NotImplementedError

    @property
    @abstractmethod
    def face(self) -> Optional["IPolygon"]:
        """Target face if dual, else ``None``."""
        raise NotImplementedError

    @property
    @abstractmethod
    def rot(self) -> "IEdge":
        """Dual edge, right-to-left."""
        raise NotImplementedError

    @property
    @abstractmethod
    def inv_rot(self) -> "IEdge":
        """Dual edge, left-to-right."""
        raise NotImplementedError

    @property
    @abstractmethod
    def sym(self) -> "IEdge":
        """The edge from dest to orig."""
        raise NotImplementedError

    @property
    @abstractmethod
    def orig_next(self) -> "IEdge":
        """Next ccw edge around the origin."""
        raise NotImplementedError

    @property
    @abstractmethod
    def orig_prev(self) -> "IEdge":
        """Next cw edge around the origin."""
        raise NotImplementedError

    @property
    @abstractmethod
    def dest_next(self) -> "IEdge":
        """Next ccw edge around the destination."""
        raise NotImplementedError

    @property
    @abstractmethod
    def dest_prev(self) -> "IEdge":
        """Next cw edge around the destination."""
        raise NotImplementedError

    @property
    @abstractmethod
    def left_next(self) -> "IEdge":
        """Ccw edge around the left face after this edge."""
        raise NotImplementedError

    @property
    @abstractmethod
    def left_prev(self) -> "IEdge":
        """Ccw edge around the left face before this edge."""
        raise NotImplementedError

    @property
    @abstractmethod
    def right_next(self) -> "IEdge":
        """Ccw edge around the right face after this edge."""
        raise NotImplementedError

    @property
    @abstractmethod
    def right_prev(self) -> "IEdge":
        """Ccw edge around the right face before this edge."""
        raise NotImplementedError


class ISurface(IGeometry):
    """A 2-dimensional geometry. Mirrors C++ ``ISurface``."""

    @property
    @abstractmethod
    def area(self) -> float:
        """Surface area."""
        raise NotImplementedError

    @property
    @abstractmethod
    def centroid(self) -> IPoint:
        """Mathematical centroid (not necessarily on the surface)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def point_on_surface(self) -> IPoint:
        """A point guaranteed to be on the surface."""
        raise NotImplementedError

    @property
    @abstractmethod
    def boundary_multi_curve(self) -> IMultiCurve:
        """The boundary curves of the surface."""
        raise NotImplementedError


class IMultiSurface(IGeometryCollection):
    """A collection of surfaces. Mirrors C++ ``IMultiSurface``."""

    @property
    @abstractmethod
    def area(self) -> float:
        """Total area of the member surfaces."""
        raise NotImplementedError

    @property
    @abstractmethod
    def centroid(self) -> IPoint:
        """Mathematical centroid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def point_on_surface(self) -> IPoint:
        """A point guaranteed to be on one of the member surfaces."""
        raise NotImplementedError


class IPolygon(ISurface):
    """A planar surface with an exterior ring and interior rings.

    Mirrors C++ ``IPolygon``.
    """

    @property
    @abstractmethod
    def exterior_ring(self) -> ILineString:
        """The exterior boundary ring."""
        raise NotImplementedError

    @property
    @abstractmethod
    def interior_ring_count(self) -> int:
        """Number of interior rings (holes)."""
        raise NotImplementedError

    @abstractmethod
    def interior_ring(self, index: int) -> ILineString:
        """The interior ring at the given index."""
        raise NotImplementedError

    @property
    @abstractmethod
    def edge(self) -> Optional[IEdge]:
        """An arbitrary boundary edge, when the polygon participates in
        topology."""
        raise NotImplementedError

    @property
    @abstractmethod
    def polyhedral_surface(self) -> Optional["IPolyhedralSurface"]:
        """The polyhedral surface this polygon is a patch of, if any."""
        raise NotImplementedError


class IMultiPolygon(IMultiSurface):
    """A collection of polygons. Mirrors C++ ``IMultiPolygon``."""

    @abstractmethod
    def polygon(self, index: int) -> IPolygon:
        """The polygon at the given index."""
        raise NotImplementedError


class ITriangle(IPolygon):
    """A triangular polygon. Mirrors C++ ``ITriangle``."""

    @property
    @abstractmethod
    def vertex1(self) -> IVertex:
        """First vertex."""
        raise NotImplementedError

    @property
    @abstractmethod
    def vertex2(self) -> IVertex:
        """Second vertex."""
        raise NotImplementedError

    @property
    @abstractmethod
    def vertex3(self) -> IVertex:
        """Third vertex."""
        raise NotImplementedError

    @abstractmethod
    def vertex(self, index: int) -> IVertex:
        """The vertex at the given index (0-2)."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Bulk mesh view
# ---------------------------------------------------------------------------


class IMeshView(ABC):
    """Bulk, structure-of-arrays view of an unstructured mesh or network.

    Mirrors C++ ``Spatial::IMeshView``: flat coordinate arrays plus CSR
    (compressed sparse row) connectivity, deliberately congruent with the
    UGRID conventions so persistence, message packing, and device staging
    can consume the view without transformation. Arrays remain valid until
    the underlying mesh topology or geometry changes.
    """

    @property
    @abstractmethod
    def node_count(self) -> int:
        """Number of nodes (vertices)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def edge_count(self) -> int:
        """Number of edges."""
        raise NotImplementedError

    @property
    @abstractmethod
    def face_count(self) -> int:
        """Number of faces (patches/cells); 0 for a pure network."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_x(self) -> "np.ndarray":
        """x coordinates of all nodes (float64, :attr:`node_count`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_y(self) -> "np.ndarray":
        """y coordinates of all nodes (float64, :attr:`node_count`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_z(self) -> "np.ndarray":
        """z coordinates of all nodes; empty for a 2D mesh."""
        raise NotImplementedError

    @property
    @abstractmethod
    def face_node_offsets(self) -> "np.ndarray":
        """CSR row offsets into :attr:`face_nodes` (int64,
        :attr:`face_count` + 1 elements; empty for a pure network)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def face_nodes(self) -> "np.ndarray":
        """Concatenated node indexes of all faces, ccw per face (int64)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def edge_nodes(self) -> "np.ndarray":
        """Node index pairs of all edges (int64, ``2 * edge_count``):
        edge ``e`` connects ``edge_nodes[2*e]`` and ``edge_nodes[2*e+1]``."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Network / polyhedral surface / TIN
# ---------------------------------------------------------------------------


class INetwork(IIdentity):
    """A graph of connected vertices and edges.

    Mirrors C++ ``Spatial::INetwork``.
    """

    @property
    @abstractmethod
    def edge_count(self) -> int:
        """Number of edges in the network."""
        raise NotImplementedError

    @abstractmethod
    def edge(self, index: int) -> IEdge:
        """The edge at the given index (spot queries)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def vertex_count(self) -> int:
        """Number of vertices in the network."""
        raise NotImplementedError

    @abstractmethod
    def vertex(self, index: int) -> IVertex:
        """The vertex at the given index (spot queries)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def mesh_view(self) -> IMeshView:
        """Bulk structure-of-arrays view of this network."""
        raise NotImplementedError


class IPolyhedralSurface(ISurface):
    """A contiguous collection of polygon patches stitched along shared
    boundary edges.

    Mirrors C++ ``Spatial::IPolyhedralSurface``.
    """

    @property
    @abstractmethod
    def patch_count(self) -> int:
        """Number of polygon patches."""
        raise NotImplementedError

    @abstractmethod
    def patch(self, index: int) -> IPolygon:
        """The patch at the given index (spot queries)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def vertex_count(self) -> int:
        """Number of vertices."""
        raise NotImplementedError

    @abstractmethod
    def vertex(self, index: int) -> IVertex:
        """The vertex at the given index (spot queries)."""
        raise NotImplementedError

    @abstractmethod
    def bounding_polygons(self, polygon: IPolygon) -> IMultiPolygon:
        """The polygons bounding the given polygon."""
        raise NotImplementedError

    @property
    @abstractmethod
    def is_closed(self) -> bool:
        """Whether the surface is closed and therefore bounds a solid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def mesh_view(self) -> IMeshView:
        """Bulk structure-of-arrays view of this surface."""
        raise NotImplementedError


class ITIN(IPolyhedralSurface):
    """A triangulated irregular network of :class:`ITriangle` patches.

    Mirrors C++ ``Spatial::ITIN``.
    """

    @abstractmethod
    def triangle(self, index: int) -> ITriangle:
        """The triangle at the given index."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Raster
# ---------------------------------------------------------------------------


class IRaster(IIdentity):
    """A raster spatial feature.

    Mirrors C++ ``Spatial::IRaster``.
    """

    @property
    @abstractmethod
    def x_size(self) -> int:
        """Number of columns."""
        raise NotImplementedError

    @property
    @abstractmethod
    def y_size(self) -> int:
        """Number of rows."""
        raise NotImplementedError

    @property
    @abstractmethod
    def raster_band_count(self) -> int:
        """Number of bands."""
        raise NotImplementedError

    @abstractmethod
    def add_raster_band(self, data_type: RasterDataType) -> None:
        """Add a band of the given element type."""
        raise NotImplementedError

    @property
    @abstractmethod
    def spatial_reference_system(self) -> ISpatialReferenceSystem:
        """The SRS of this raster."""
        raise NotImplementedError

    @abstractmethod
    def geo_transformation(self) -> "np.ndarray":
        """The six-element affine geotransform (float64)."""
        raise NotImplementedError

    @abstractmethod
    def get_raster_band(self, band_index: int) -> "IRasterBand":
        """The band at the given index."""
        raise NotImplementedError


class IRasterBand(IIdentity):
    """One band of an :class:`IRaster`.

    Mirrors C++ ``Spatial::IRasterBand``. Block read/write is the
    storage-level accessor; the canonical exchange path is the raster
    component data item's hyperslab API.
    """

    @property
    @abstractmethod
    def x_size(self) -> int:
        """Number of columns."""
        raise NotImplementedError

    @property
    @abstractmethod
    def y_size(self) -> int:
        """Number of rows."""
        raise NotImplementedError

    @property
    @abstractmethod
    def raster(self) -> IRaster:
        """The owning raster."""
        raise NotImplementedError

    @property
    @abstractmethod
    def data_type(self) -> RasterDataType:
        """Element type of this band."""
        raise NotImplementedError

    @abstractmethod
    def read(self, x_offset: int, y_offset: int,
             x_size: int, y_size: int) -> "np.ndarray":
        """Read a block as a ``[y_size, x_size]`` array."""
        raise NotImplementedError

    @abstractmethod
    def write(self, x_offset: int, y_offset: int, image: "np.ndarray") -> None:
        """Write a ``[y_size, x_size]`` block."""
        raise NotImplementedError

    @property
    @abstractmethod
    def no_data(self) -> float:
        """The no-data sentinel value."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Regular grids
# ---------------------------------------------------------------------------


class IRegularGrid2D(IIdentity):
    """A two-dimensional structured grid of nodes and cells.

    Mirrors C++ ``Spatial::IRegularGrid2D``.
    """

    @property
    @abstractmethod
    def spatial_reference_system(self) -> ISpatialReferenceSystem:
        """The SRS of this grid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def grid_type(self) -> RegularGridType:
        """The type of regular grid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def num_x_nodes(self) -> int:
        """Number of nodes in the x direction."""
        raise NotImplementedError

    @property
    @abstractmethod
    def num_y_nodes(self) -> int:
        """Number of nodes in the y direction."""
        raise NotImplementedError

    @abstractmethod
    def x_node_location(self, x_node_index: int, y_node_index: int) -> float:
        """x coordinate of a node (spot queries; bulk consumers use
        :attr:`node_xs`)."""
        raise NotImplementedError

    @abstractmethod
    def y_node_location(self, x_node_index: int, y_node_index: int) -> float:
        """y coordinate of a node (spot queries; bulk consumers use
        :attr:`node_ys`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_xs(self) -> "np.ndarray":
        """Bulk x coordinates of all nodes, row-major ``[y][x]`` (float64)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_ys(self) -> "np.ndarray":
        """Bulk y coordinates of all nodes, row-major ``[y][x]`` (float64)."""
        raise NotImplementedError

    @abstractmethod
    def is_active(self, x_cell_index: int, y_cell_index: int) -> bool:
        """Whether a cell is active (spot queries; bulk consumers use
        :attr:`active_cells`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def active_cells(self) -> "np.ndarray":
        """Bulk activity mask of all cells, row-major ``[y][x]``; nonzero
        means active (uint8)."""
        raise NotImplementedError


class IRegularGrid3D(IIdentity):
    """A three-dimensional structured grid of nodes and cells.

    Mirrors C++ ``Spatial::IRegularGrid3D``.
    """

    @property
    @abstractmethod
    def spatial_reference_system(self) -> ISpatialReferenceSystem:
        """The SRS of this grid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def grid_type(self) -> RegularGridType:
        """The type of regular grid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def num_x_nodes(self) -> int:
        """Number of nodes in the x direction."""
        raise NotImplementedError

    @property
    @abstractmethod
    def num_y_nodes(self) -> int:
        """Number of nodes in the y direction."""
        raise NotImplementedError

    @property
    @abstractmethod
    def num_z_nodes(self) -> int:
        """Number of nodes in the z direction."""
        raise NotImplementedError

    @abstractmethod
    def x_node_location(self, x_node_index: int, y_node_index: int) -> float:
        """x coordinate of a node (spot queries)."""
        raise NotImplementedError

    @abstractmethod
    def y_node_location(self, x_node_index: int, y_node_index: int) -> float:
        """y coordinate of a node (spot queries)."""
        raise NotImplementedError

    @abstractmethod
    def z_node_location(self, x_node_index: int, y_node_index: int,
                        z_node_index: int) -> float:
        """z coordinate of a node (spot queries)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_xs(self) -> "np.ndarray":
        """Bulk x coordinates of a horizontal layer, row-major ``[y][x]``."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_ys(self) -> "np.ndarray":
        """Bulk y coordinates of a horizontal layer, row-major ``[y][x]``."""
        raise NotImplementedError

    @property
    @abstractmethod
    def node_zs(self) -> "np.ndarray":
        """Bulk z coordinates of all nodes, row-major ``[z][y][x]``."""
        raise NotImplementedError

    @abstractmethod
    def is_active(self, x_cell_index: int, y_cell_index: int,
                  z_cell_index: int) -> bool:
        """Whether a cell is active (spot queries)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def active_cells(self) -> "np.ndarray":
        """Bulk activity mask of all cells, row-major ``[z][y][x]``
        (uint8)."""
        raise NotImplementedError


# ---------------------------------------------------------------------------
# Spatial component data items (metadata + canonical dimension orderings;
# data access is the inherited hyperslab API)
# ---------------------------------------------------------------------------


class IGeometryComponentDataItem(IComponentDataItem):
    """Data associated with a collection of geometries.

    Mirrors C++ ``Spatial::IGeometryComponentDataItem``. Canonical
    dimension ordering: the geometry dimension is dimension 0 of
    :attr:`shape`; any additional dimensions follow.
    """

    @property
    @abstractmethod
    def geometry_type(self) -> GeometryType:
        """The type of the associated geometries."""
        raise NotImplementedError

    @property
    @abstractmethod
    def geometry_count(self) -> int:
        """Number of associated geometries."""
        raise NotImplementedError

    @abstractmethod
    def geometry(self, geometry_index: int) -> IGeometry:
        """The geometry at the given index."""
        raise NotImplementedError

    @property
    @abstractmethod
    def geometry_dimension(self) -> IDimension:
        """The geometry dimension (dimension 0 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def envelope(self) -> IEnvelope:
        """Envelope bounding all associated geometries."""
        raise NotImplementedError


class INetworkComponentDataItem(IComponentDataItem):
    """Data associated with the edges and/or vertices of a network.

    Mirrors C++ ``Spatial::INetworkComponentDataItem``. Canonical dimension
    ordering: the entity dimension selected by :attr:`network_data_type`
    (edge or vertex) is dimension 0 of :attr:`shape`.
    """

    @property
    @abstractmethod
    def network(self) -> INetwork:
        """The associated network."""
        raise NotImplementedError

    @property
    @abstractmethod
    def network_data_object_type(self) -> NetworkDataObjectType:
        """The kind of network object the values describe."""
        raise NotImplementedError

    @property
    @abstractmethod
    def network_data_type(self) -> SpatialDataType:
        """The mesh entity the values are attached to."""
        raise NotImplementedError

    @property
    @abstractmethod
    def edge_dimension(self) -> IDimension:
        """The network edge dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def vertex_dimension(self) -> IDimension:
        """The network vertex dimension."""
        raise NotImplementedError


class IPolyhedralSurfaceComponentDataItem(IComponentDataItem):
    """Data associated with the patches, edges, or vertices of a
    polyhedral surface.

    Mirrors C++ ``Spatial::IPolyhedralSurfaceComponentDataItem``. Canonical
    dimension ordering: the entity dimension selected by
    :attr:`mesh_data_type` is dimension 0 of :attr:`shape`.
    """

    @property
    @abstractmethod
    def mesh_data_object_type(self) -> MeshDataObjectType:
        """The kind of mesh object the values describe."""
        raise NotImplementedError

    @property
    @abstractmethod
    def mesh_data_type(self) -> SpatialDataType:
        """The mesh entity the values are attached to."""
        raise NotImplementedError

    @property
    @abstractmethod
    def polyhedral_surface(self) -> IPolyhedralSurface:
        """The associated polyhedral surface."""
        raise NotImplementedError

    @property
    @abstractmethod
    def patch_dimension(self) -> IDimension:
        """The surface patch dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def edge_dimension(self) -> IDimension:
        """The surface edge dimension."""
        raise NotImplementedError

    @property
    @abstractmethod
    def vertex_dimension(self) -> IDimension:
        """The surface vertex dimension."""
        raise NotImplementedError


class ITINComponentDataItem(IPolyhedralSurfaceComponentDataItem):
    """A polyhedral-surface data item whose surface is a TIN.

    Mirrors C++ ``Spatial::ITINComponentDataItem``.
    """

    @property
    @abstractmethod
    def tin(self) -> ITIN:
        """The associated TIN (C++ ``TIN()``)."""
        raise NotImplementedError


class IRasterComponentDataItem(IComponentDataItem):
    """Data associated with a raster.

    Mirrors C++ ``Spatial::IRasterComponentDataItem``. Canonical dimension
    ordering: band is dimension 0, y (row) is dimension 1, x (column) is
    dimension 2 of :attr:`shape`.
    """

    @property
    @abstractmethod
    def raster(self) -> IRaster:
        """The associated raster."""
        raise NotImplementedError

    @property
    @abstractmethod
    def x_dimension(self) -> IDimension:
        """The column dimension (dimension 2 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def y_dimension(self) -> IDimension:
        """The row dimension (dimension 1 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def band_dimension(self) -> IDimension:
        """The band dimension (dimension 0 of :attr:`shape`)."""
        raise NotImplementedError


class IRegularGrid2DComponentDataItem(IComponentDataItem):
    """Data associated with the cells of a 2D regular grid.

    Mirrors C++ ``Spatial::IRegularGrid2DComponentDataItem``. Canonical
    dimension ordering: y-cell is dimension 0, x-cell is dimension 1 of
    :attr:`shape`; optional cell edge and cell vertex dimensions follow.
    """

    @property
    @abstractmethod
    def grid(self) -> IRegularGrid2D:
        """The associated grid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def mesh_data_object_type(self) -> MeshDataObjectType:
        """The kind of mesh object the values describe."""
        raise NotImplementedError

    @property
    @abstractmethod
    def x_cell_dimension(self) -> IDimension:
        """The x-cell dimension (dimension 1 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def y_cell_dimension(self) -> IDimension:
        """The y-cell dimension (dimension 0 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def cell_edge_dimension(self) -> IDimension:
        """The per-cell edge dimension, when values attach to cell edges."""
        raise NotImplementedError

    @property
    @abstractmethod
    def cell_vertex_dimension(self) -> IDimension:
        """The per-cell vertex dimension, when values attach to cell
        vertices."""
        raise NotImplementedError


class IRegularGrid3DComponentDataItem(IComponentDataItem):
    """Data associated with the cells of a 3D regular grid.

    Mirrors C++ ``Spatial::IRegularGrid3DComponentDataItem``. Canonical
    dimension ordering: z-cell is dimension 0, y-cell is dimension 1,
    x-cell is dimension 2 of :attr:`shape`; optional cell face and cell
    vertex dimensions follow.
    """

    @property
    @abstractmethod
    def grid(self) -> IRegularGrid3D:
        """The associated grid."""
        raise NotImplementedError

    @property
    @abstractmethod
    def mesh_data_object_type(self) -> MeshDataObjectType:
        """The kind of mesh object the values describe."""
        raise NotImplementedError

    @property
    @abstractmethod
    def x_cell_dimension(self) -> IDimension:
        """The x-cell dimension (dimension 2 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def y_cell_dimension(self) -> IDimension:
        """The y-cell dimension (dimension 1 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def z_cell_dimension(self) -> IDimension:
        """The z-cell dimension (dimension 0 of :attr:`shape`)."""
        raise NotImplementedError

    @property
    @abstractmethod
    def cell_face_dimension(self) -> IDimension:
        """The per-cell face dimension, when values attach to cell faces."""
        raise NotImplementedError

    @property
    @abstractmethod
    def cell_vertex_dimension(self) -> IDimension:
        """The per-cell vertex dimension, when values attach to cell
        vertices."""
        raise NotImplementedError
