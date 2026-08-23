# distutils: language = c++
# cython: language_level = 3
"""
Cython wrapper classes for the C++ v2.0.0 spatial interfaces.

Bulk accessors (mesh views, grid coordinate arrays) are exposed as
zero-copy, read-only NumPy views over the C++ spans; they remain valid
until the underlying mesh or grid changes.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport int64_t, uint8_t

cimport _hydrocouple._core as cpp
from _hydrocouple._core cimport CppDimensionWrapper
cimport _hydrocouple._spatial as sp

import numpy as np
cimport numpy as cnp

cnp.import_array()


# ---------------------------------------------------------------------------
# span -> read-only ndarray helpers (zero-copy)
# ---------------------------------------------------------------------------

cdef object _f64_view(sp.span_const_double span):
    cdef size_t n = span.size()
    if n == 0:
        return np.empty(0, dtype=np.float64)
    cdef double[::1] mv = <double[:n]>(<double*>span.data())
    arr = np.asarray(mv)
    arr.flags.writeable = False
    return arr


cdef object _i64_view(sp.span_const_int64 span):
    cdef size_t n = span.size()
    if n == 0:
        return np.empty(0, dtype=np.int64)
    cdef int64_t[::1] mv = <int64_t[:n]>(<int64_t*>span.data())
    arr = np.asarray(mv)
    arr.flags.writeable = False
    return arr


cdef object _u8_view(sp.span_const_uint8 span):
    cdef size_t n = span.size()
    if n == 0:
        return np.empty(0, dtype=np.uint8)
    cdef uint8_t[::1] mv = <uint8_t[:n]>(<uint8_t*>span.data())
    arr = np.asarray(mv)
    arr.flags.writeable = False
    return arr


# ---------------------------------------------------------------------------
# SRS / envelope
# ---------------------------------------------------------------------------

cdef class CppSpatialReferenceSystemWrapper:
    """Wrapper around a C++ ``Spatial::ISpatialReferenceSystem`` pointer."""

    cdef sp.ISpatialReferenceSystem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppSpatialReferenceSystemWrapper wrap(
            sp.ISpatialReferenceSystem* ptr):
        cdef CppSpatialReferenceSystemWrapper obj = (
            CppSpatialReferenceSystemWrapper.__new__(
                CppSpatialReferenceSystemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def auth_srid(self) -> int:
        """The authority-specific spatial reference id (e.g. EPSG code)."""
        return self._ptr.authSRID()

    @property
    def auth_name(self) -> str:
        """The authority name (e.g. ``"EPSG"``)."""
        return self._ptr.authName().decode("utf-8")

    @property
    def sr_text(self) -> str:
        """Well-known text representation of the SRS."""
        return self._ptr.srText().decode("utf-8")


cdef class CppEnvelopeWrapper:
    """Wrapper around a C++ ``Spatial::IEnvelope`` pointer."""

    cdef sp.IEnvelope* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppEnvelopeWrapper wrap(sp.IEnvelope* ptr):
        cdef CppEnvelopeWrapper obj = CppEnvelopeWrapper.__new__(
            CppEnvelopeWrapper)
        obj._ptr = ptr
        return obj

    @property
    def min_x(self) -> float:
        """Minimum x."""
        return self._ptr.minX()

    @property
    def max_x(self) -> float:
        """Maximum x."""
        return self._ptr.maxX()

    @property
    def min_y(self) -> float:
        """Minimum y."""
        return self._ptr.minY()

    @property
    def max_y(self) -> float:
        """Maximum y."""
        return self._ptr.maxY()

    @property
    def min_z(self) -> float:
        """Minimum z."""
        return self._ptr.minZ()

    @property
    def max_z(self) -> float:
        """Maximum z."""
        return self._ptr.maxZ()


# ---------------------------------------------------------------------------
# Geometry wrappers
# ---------------------------------------------------------------------------

cdef class CppGeometryWrapper:
    """Wrapper around a C++ ``Spatial::IGeometry`` pointer."""

    cdef sp.IGeometry* _gptr

    def __cinit__(self):
        self._gptr = NULL

    @staticmethod
    cdef CppGeometryWrapper wrap(sp.IGeometry* ptr):
        cdef CppGeometryWrapper obj = CppGeometryWrapper.__new__(
            CppGeometryWrapper)
        obj._gptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Id of the geometry."""
        return self._gptr.id().decode("utf-8")

    @property
    def index(self) -> int:
        """Index of the geometry within a collection."""
        return self._gptr.index()

    @property
    def dimension(self) -> int:
        """Topological dimension."""
        return self._gptr.dimension()

    @property
    def coordinate_dimension(self) -> int:
        """Dimension of the coordinates."""
        return self._gptr.coordinateDimension()

    @property
    def geometry_type(self):
        """The instantiable OGC subtype."""
        from hydrocouple.spatial import GeometryType
        return GeometryType(<int>self._gptr.geometryType())

    @property
    def spatial_reference_system(self):
        """The SRS of this geometry."""
        return CppSpatialReferenceSystemWrapper.wrap(
            self._gptr.spatialReferenceSystem())

    @property
    def envelope(self):
        """The bounding envelope."""
        return CppEnvelopeWrapper.wrap(self._gptr.envelope())

    def get_wkt(self) -> str:
        """Well-known text representation."""
        return self._gptr.getWKT().decode("utf-8")

    @property
    def is_empty(self) -> bool:
        """Whether this is the empty geometry."""
        return self._gptr.isEmpty()

    @property
    def is_3d(self) -> bool:
        """Whether this geometry has z coordinates."""
        return self._gptr.is3D()

    @property
    def is_measured(self) -> bool:
        """Whether this geometry has m values."""
        return self._gptr.isMeasured()


cdef class CppPointWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::IPoint`` pointer."""

    # Typed pointer: IGeometry is a virtual base, so downcasting from
    # _gptr is not possible.
    cdef sp.IPoint* _pptr

    @staticmethod
    cdef CppPointWrapper wrap_point(sp.IPoint* ptr):
        cdef CppPointWrapper obj = CppPointWrapper.__new__(CppPointWrapper)
        obj._gptr = <sp.IGeometry*>ptr
        obj._pptr = ptr
        return obj

    @property
    def x(self) -> float:
        """x coordinate."""
        return self._pptr.x()

    @property
    def y(self) -> float:
        """y coordinate."""
        return self._pptr.y()

    @property
    def z(self) -> float:
        """z coordinate."""
        return self._pptr.z()

    @property
    def m(self) -> float:
        """m value."""
        return self._pptr.m()


cdef class CppVertexWrapper(CppPointWrapper):
    """Wrapper around a C++ ``Spatial::IVertex`` pointer."""

    cdef sp.IVertex* _vptr

    @staticmethod
    cdef CppVertexWrapper wrap_vertex(sp.IVertex* ptr):
        cdef CppVertexWrapper obj = CppVertexWrapper.__new__(CppVertexWrapper)
        obj._gptr = <sp.IGeometry*>ptr
        obj._pptr = <sp.IPoint*>ptr
        obj._vptr = ptr
        return obj

    @property
    def edge(self):
        """An arbitrary outgoing edge of this vertex, or ``None``."""
        cdef sp.IEdge* e = self._vptr.edge()
        if e == NULL:
            return None
        return CppEdgeWrapper.wrap(e)


cdef class CppLineStringWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::ILineString`` pointer."""

    cdef sp.ILineString* _lptr

    @staticmethod
    cdef CppLineStringWrapper wrap_linestring(sp.ILineString* ptr):
        cdef CppLineStringWrapper obj = CppLineStringWrapper.__new__(
            CppLineStringWrapper)
        obj._gptr = <sp.IGeometry*>ptr
        obj._lptr = ptr
        return obj

    @property
    def length(self) -> float:
        """Curve length."""
        return self._lptr.length()

    @property
    def point_count(self) -> int:
        """Number of points."""
        return self._lptr.pointCount()

    def point(self, int index):
        """The point at the given index."""
        return CppPointWrapper.wrap_point(self._lptr.point(index))

    @property
    def is_closed(self) -> bool:
        """Whether start and end points coincide."""
        return self._lptr.isClosed()


cdef class CppPolygonWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::IPolygon`` pointer."""

    cdef sp.IPolygon* _polyptr

    @staticmethod
    cdef CppPolygonWrapper wrap_polygon(sp.IPolygon* ptr):
        cdef CppPolygonWrapper obj = CppPolygonWrapper.__new__(
            CppPolygonWrapper)
        obj._gptr = <sp.IGeometry*>ptr
        obj._polyptr = ptr
        return obj

    @property
    def area(self) -> float:
        """Surface area."""
        return self._polyptr.area()

    @property
    def exterior_ring(self):
        """The exterior boundary ring."""
        return CppLineStringWrapper.wrap_linestring(
            self._polyptr.exteriorRing())

    @property
    def interior_ring_count(self) -> int:
        """Number of interior rings."""
        return self._polyptr.interiorRingCount()


cdef class CppTriangleWrapper(CppPolygonWrapper):
    """Wrapper around a C++ ``Spatial::ITriangle`` pointer."""

    cdef sp.ITriangle* _triptr

    @staticmethod
    cdef CppTriangleWrapper wrap_triangle(sp.ITriangle* ptr):
        cdef CppTriangleWrapper obj = CppTriangleWrapper.__new__(
            CppTriangleWrapper)
        obj._gptr = <sp.IGeometry*>ptr
        obj._polyptr = <sp.IPolygon*>ptr
        obj._triptr = ptr
        return obj

    def vertex(self, int index):
        """The vertex at the given index (0-2)."""
        return CppVertexWrapper.wrap_vertex(self._triptr.vertex(index))


cdef class CppEdgeWrapper:
    """Wrapper around a C++ ``Spatial::IEdge`` pointer (quad-edge)."""

    cdef sp.IEdge* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppEdgeWrapper wrap(sp.IEdge* ptr):
        cdef CppEdgeWrapper obj = CppEdgeWrapper.__new__(CppEdgeWrapper)
        obj._ptr = ptr
        return obj

    @property
    def index(self) -> int:
        """Unique edge index."""
        return self._ptr.index()

    @property
    def orig(self):
        """Origin vertex, or ``None``."""
        cdef sp.IVertex* v = self._ptr.orig()
        if v == NULL:
            return None
        return CppVertexWrapper.wrap_vertex(v)

    @property
    def dest(self):
        """Destination vertex, or ``None``."""
        cdef sp.IVertex* v = self._ptr.dest()
        if v == NULL:
            return None
        return CppVertexWrapper.wrap_vertex(v)

    @property
    def sym(self):
        """The edge from dest to orig."""
        return CppEdgeWrapper.wrap(self._ptr.sym())

    @property
    def orig_next(self):
        """Next ccw edge around the origin."""
        return CppEdgeWrapper.wrap(self._ptr.origNext())

    @property
    def dest_next(self):
        """Next ccw edge around the destination."""
        return CppEdgeWrapper.wrap(self._ptr.destNext())


# ---------------------------------------------------------------------------
# Bulk mesh view
# ---------------------------------------------------------------------------

cdef class CppMeshViewWrapper:
    """Wrapper around a C++ ``Spatial::IMeshView`` pointer.

    All array properties are zero-copy, read-only NumPy views over the
    C++ spans; they remain valid until the mesh changes.
    """

    cdef const sp.IMeshView* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppMeshViewWrapper wrap(const sp.IMeshView* ptr):
        cdef CppMeshViewWrapper obj = CppMeshViewWrapper.__new__(
            CppMeshViewWrapper)
        obj._ptr = ptr
        return obj

    @property
    def node_count(self) -> int:
        """Number of nodes."""
        return self._ptr.nodeCount()

    @property
    def edge_count(self) -> int:
        """Number of edges."""
        return self._ptr.edgeCount()

    @property
    def face_count(self) -> int:
        """Number of faces; 0 for a pure network."""
        return self._ptr.faceCount()

    @property
    def node_x(self):
        """x coordinates of all nodes (float64, read-only, zero-copy)."""
        return _f64_view(self._ptr.nodeX())

    @property
    def node_y(self):
        """y coordinates of all nodes (float64, read-only, zero-copy)."""
        return _f64_view(self._ptr.nodeY())

    @property
    def node_z(self):
        """z coordinates of all nodes; empty for a 2D mesh."""
        return _f64_view(self._ptr.nodeZ())

    @property
    def face_node_offsets(self):
        """CSR row offsets into :attr:`face_nodes` (int64)."""
        return _i64_view(self._ptr.faceNodeOffsets())

    @property
    def face_nodes(self):
        """Concatenated node indexes of all faces (int64)."""
        return _i64_view(self._ptr.faceNodes())

    @property
    def edge_nodes(self):
        """Node index pairs of all edges (int64, ``2 * edge_count``)."""
        return _i64_view(self._ptr.edgeNodes())


# ---------------------------------------------------------------------------
# Network / polyhedral surface / TIN
# ---------------------------------------------------------------------------

cdef class CppNetworkWrapper:
    """Wrapper around a C++ ``Spatial::INetwork`` pointer."""

    cdef sp.INetwork* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppNetworkWrapper wrap(sp.INetwork* ptr):
        cdef CppNetworkWrapper obj = CppNetworkWrapper.__new__(
            CppNetworkWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this network."""
        return (<cpp.IIdentity*>self._ptr).id().decode("utf-8")

    @property
    def edge_count(self) -> int:
        """Number of edges."""
        return self._ptr.edgeCount()

    def edge(self, index: int):
        """The edge at the given index."""
        return CppEdgeWrapper.wrap(self._ptr.edge(<int64_t>index))

    @property
    def vertex_count(self) -> int:
        """Number of vertices."""
        return self._ptr.vertexCount()

    def vertex(self, index: int):
        """The vertex at the given index."""
        return CppVertexWrapper.wrap_vertex(self._ptr.vertex(<int64_t>index))

    @property
    def mesh_view(self):
        """Bulk structure-of-arrays view of this network."""
        return CppMeshViewWrapper.wrap(self._ptr.meshView())


cdef class CppPolyhedralSurfaceWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::IPolyhedralSurface`` pointer."""

    cdef sp.IPolyhedralSurface* _sptr

    @staticmethod
    cdef CppPolyhedralSurfaceWrapper wrap_surface(sp.IPolyhedralSurface* ptr):
        cdef CppPolyhedralSurfaceWrapper obj = (
            CppPolyhedralSurfaceWrapper.__new__(CppPolyhedralSurfaceWrapper))
        obj._gptr = <sp.IGeometry*>ptr
        obj._sptr = ptr
        return obj

    @property
    def patch_count(self) -> int:
        """Number of polygon patches."""
        return self._sptr.patchCount()

    def patch(self, index: int):
        """The patch at the given index."""
        return CppPolygonWrapper.wrap_polygon(
            self._sptr.patch(<int64_t>index))

    @property
    def vertex_count(self) -> int:
        """Number of vertices."""
        return self._sptr.vertexCount()

    def vertex(self, index: int):
        """The vertex at the given index."""
        return CppVertexWrapper.wrap_vertex(
            self._sptr.vertex(<int64_t>index))

    @property
    def is_closed(self) -> bool:
        """Whether the surface bounds a solid."""
        return self._sptr.isClosed()

    @property
    def mesh_view(self):
        """Bulk structure-of-arrays view of this surface."""
        return CppMeshViewWrapper.wrap(self._sptr.meshView())


cdef class CppTINWrapper(CppPolyhedralSurfaceWrapper):
    """Wrapper around a C++ ``Spatial::ITIN`` pointer."""

    cdef sp.ITIN* _tinptr

    @staticmethod
    cdef CppTINWrapper wrap_tin(sp.ITIN* ptr):
        cdef CppTINWrapper obj = CppTINWrapper.__new__(CppTINWrapper)
        obj._gptr = <sp.IGeometry*>ptr
        obj._sptr = <sp.IPolyhedralSurface*>ptr
        obj._tinptr = ptr
        return obj

    def triangle(self, index: int):
        """The triangle at the given index."""
        return CppTriangleWrapper.wrap_triangle(
            self._tinptr.triangle(<int64_t>index))


# ---------------------------------------------------------------------------
# Raster
# ---------------------------------------------------------------------------

cdef class CppRasterWrapper:
    """Wrapper around a C++ ``Spatial::IRaster`` pointer."""

    cdef sp.IRaster* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppRasterWrapper wrap(sp.IRaster* ptr):
        cdef CppRasterWrapper obj = CppRasterWrapper.__new__(CppRasterWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this raster."""
        return (<cpp.IIdentity*>self._ptr).id().decode("utf-8")

    @property
    def x_size(self) -> int:
        """Number of columns."""
        return self._ptr.xSize()

    @property
    def y_size(self) -> int:
        """Number of rows."""
        return self._ptr.ySize()

    @property
    def raster_band_count(self) -> int:
        """Number of bands."""
        return self._ptr.rasterBandCount()

    def get_raster_band(self, int band_index):
        """The band at the given index."""
        return CppRasterBandWrapper.wrap(
            self._ptr.getRasterBand(band_index))


cdef class CppRasterBandWrapper:
    """Wrapper around a C++ ``Spatial::IRasterBand`` pointer."""

    cdef sp.IRasterBand* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppRasterBandWrapper wrap(sp.IRasterBand* ptr):
        cdef CppRasterBandWrapper obj = CppRasterBandWrapper.__new__(
            CppRasterBandWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this band."""
        return (<cpp.IIdentity*>self._ptr).id().decode("utf-8")

    @property
    def x_size(self) -> int:
        """Number of columns."""
        return self._ptr.xSize()

    @property
    def y_size(self) -> int:
        """Number of rows."""
        return self._ptr.ySize()

    @property
    def no_data(self) -> float:
        """The no-data sentinel value."""
        return self._ptr.noData()


# ---------------------------------------------------------------------------
# Regular grids
# ---------------------------------------------------------------------------

cdef class CppRegularGrid2DWrapper:
    """Wrapper around a C++ ``Spatial::IRegularGrid2D`` pointer."""

    cdef sp.IRegularGrid2D* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppRegularGrid2DWrapper wrap(sp.IRegularGrid2D* ptr):
        cdef CppRegularGrid2DWrapper obj = CppRegularGrid2DWrapper.__new__(
            CppRegularGrid2DWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this grid."""
        return (<cpp.IIdentity*>self._ptr).id().decode("utf-8")

    @property
    def grid_type(self):
        """The type of regular grid."""
        from hydrocouple.spatial import RegularGridType
        return RegularGridType(<int>self._ptr.gridType())

    @property
    def num_x_nodes(self) -> int:
        """Number of nodes in the x direction."""
        return self._ptr.numXNodes()

    @property
    def num_y_nodes(self) -> int:
        """Number of nodes in the y direction."""
        return self._ptr.numYNodes()

    def x_node_location(self, int x_node_index, int y_node_index) -> float:
        """x coordinate of a node (spot queries)."""
        return self._ptr.xNodeLocation(x_node_index, y_node_index)

    def y_node_location(self, int x_node_index, int y_node_index) -> float:
        """y coordinate of a node (spot queries)."""
        return self._ptr.yNodeLocation(x_node_index, y_node_index)

    @property
    def node_xs(self):
        """Bulk x coordinates, row-major ``[y][x]`` (zero-copy)."""
        return _f64_view(self._ptr.nodeXs()).reshape(
            self._ptr.numYNodes(), self._ptr.numXNodes())

    @property
    def node_ys(self):
        """Bulk y coordinates, row-major ``[y][x]`` (zero-copy)."""
        return _f64_view(self._ptr.nodeYs()).reshape(
            self._ptr.numYNodes(), self._ptr.numXNodes())

    def is_active(self, int x_cell_index, int y_cell_index) -> bool:
        """Whether a cell is active (spot queries)."""
        return self._ptr.isActive(x_cell_index, y_cell_index)

    @property
    def active_cells(self):
        """Bulk activity mask of all cells, row-major ``[y][x]`` (uint8)."""
        return _u8_view(self._ptr.activeCells()).reshape(
            self._ptr.numYNodes() - 1, self._ptr.numXNodes() - 1)


cdef class CppRegularGrid3DWrapper:
    """Wrapper around a C++ ``Spatial::IRegularGrid3D`` pointer."""

    cdef sp.IRegularGrid3D* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppRegularGrid3DWrapper wrap(sp.IRegularGrid3D* ptr):
        cdef CppRegularGrid3DWrapper obj = CppRegularGrid3DWrapper.__new__(
            CppRegularGrid3DWrapper)
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this grid."""
        return (<cpp.IIdentity*>self._ptr).id().decode("utf-8")

    @property
    def grid_type(self):
        """The type of regular grid."""
        from hydrocouple.spatial import RegularGridType
        return RegularGridType(<int>self._ptr.gridType())

    @property
    def num_x_nodes(self) -> int:
        """Number of nodes in the x direction."""
        return self._ptr.numXNodes()

    @property
    def num_y_nodes(self) -> int:
        """Number of nodes in the y direction."""
        return self._ptr.numYNodes()

    @property
    def num_z_nodes(self) -> int:
        """Number of nodes in the z direction."""
        return self._ptr.numZNodes()

    @property
    def node_zs(self):
        """Bulk z coordinates, row-major ``[z][y][x]`` (zero-copy)."""
        return _f64_view(self._ptr.nodeZs()).reshape(
            self._ptr.numZNodes(), self._ptr.numYNodes(),
            self._ptr.numXNodes())

    def is_active(self, int x_cell_index, int y_cell_index,
                  int z_cell_index) -> bool:
        """Whether a cell is active (spot queries)."""
        return self._ptr.isActive(x_cell_index, y_cell_index, z_cell_index)


# ---------------------------------------------------------------------------
# Spatial component data items
# ---------------------------------------------------------------------------

cdef class CppGeometryComponentDataItemWrapper:
    """Wrapper around a C++ ``Spatial::IGeometryComponentDataItem``.

    Canonical dimension ordering: geometry is dimension 0 of ``shape``.
    """

    cdef sp.IGeometryComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppGeometryComponentDataItemWrapper wrap(
            sp.IGeometryComponentDataItem* ptr):
        cdef CppGeometryComponentDataItemWrapper obj = (
            CppGeometryComponentDataItemWrapper.__new__(
                CppGeometryComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def geometry_count(self) -> int:
        """Number of associated geometries."""
        return self._ptr.geometryCount()

    def geometry(self, index: int):
        """The geometry at the given index."""
        return CppGeometryWrapper.wrap(self._ptr.geometry(<int64_t>index))

    @property
    def geometry_dimension(self):
        """The geometry dimension (dimension 0 of ``shape``)."""
        return CppDimensionWrapper.wrap(self._ptr.geometryDimension())

    @property
    def envelope(self):
        """Envelope bounding all associated geometries."""
        return CppEnvelopeWrapper.wrap(self._ptr.envelope())


cdef class CppNetworkComponentDataItemWrapper:
    """Wrapper around a C++ ``Spatial::INetworkComponentDataItem``."""

    cdef sp.INetworkComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppNetworkComponentDataItemWrapper wrap(
            sp.INetworkComponentDataItem* ptr):
        cdef CppNetworkComponentDataItemWrapper obj = (
            CppNetworkComponentDataItemWrapper.__new__(
                CppNetworkComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def network(self):
        """The associated network."""
        return CppNetworkWrapper.wrap(self._ptr.network())

    @property
    def edge_dimension(self):
        """The network edge dimension."""
        return CppDimensionWrapper.wrap(self._ptr.edgeDimension())

    @property
    def vertex_dimension(self):
        """The network vertex dimension."""
        return CppDimensionWrapper.wrap(self._ptr.vertexDimension())


cdef class CppPolyhedralSurfaceComponentDataItemWrapper:
    """Wrapper around a C++ ``Spatial::IPolyhedralSurfaceComponentDataItem``."""

    cdef sp.IPolyhedralSurfaceComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppPolyhedralSurfaceComponentDataItemWrapper wrap(
            sp.IPolyhedralSurfaceComponentDataItem* ptr):
        cdef CppPolyhedralSurfaceComponentDataItemWrapper obj = (
            CppPolyhedralSurfaceComponentDataItemWrapper.__new__(
                CppPolyhedralSurfaceComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def polyhedral_surface(self):
        """The associated polyhedral surface."""
        return CppPolyhedralSurfaceWrapper.wrap_surface(
            self._ptr.polyhedralSurface())

    @property
    def patch_dimension(self):
        """The surface patch dimension."""
        return CppDimensionWrapper.wrap(self._ptr.patchDimension())


cdef class CppTINComponentDataItemWrapper(
        CppPolyhedralSurfaceComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::ITINComponentDataItem``."""

    cdef sp.ITINComponentDataItem* _tin_item_ptr

    @staticmethod
    cdef CppTINComponentDataItemWrapper wrap_tin_item(
            sp.ITINComponentDataItem* ptr):
        cdef CppTINComponentDataItemWrapper obj = (
            CppTINComponentDataItemWrapper.__new__(
                CppTINComponentDataItemWrapper))
        obj._ptr = <sp.IPolyhedralSurfaceComponentDataItem*>ptr
        obj._tin_item_ptr = ptr
        return obj

    @property
    def tin(self):
        """The associated TIN."""
        return CppTINWrapper.wrap_tin(self._tin_item_ptr.TIN())


cdef class CppRasterComponentDataItemWrapper:
    """Wrapper around a C++ ``Spatial::IRasterComponentDataItem``.

    Canonical dimension ordering: band 0, y (row) 1, x (column) 2.
    """

    cdef sp.IRasterComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppRasterComponentDataItemWrapper wrap(
            sp.IRasterComponentDataItem* ptr):
        cdef CppRasterComponentDataItemWrapper obj = (
            CppRasterComponentDataItemWrapper.__new__(
                CppRasterComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def raster(self):
        """The associated raster."""
        return CppRasterWrapper.wrap(self._ptr.raster())

    @property
    def band_dimension(self):
        """The band dimension (dimension 0 of ``shape``)."""
        return CppDimensionWrapper.wrap(self._ptr.bandDimension())


cdef class CppRegularGrid2DComponentDataItemWrapper:
    """Wrapper around a C++ ``Spatial::IRegularGrid2DComponentDataItem``.

    Canonical dimension ordering: y-cell 0, x-cell 1.
    """

    cdef sp.IRegularGrid2DComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppRegularGrid2DComponentDataItemWrapper wrap(
            sp.IRegularGrid2DComponentDataItem* ptr):
        cdef CppRegularGrid2DComponentDataItemWrapper obj = (
            CppRegularGrid2DComponentDataItemWrapper.__new__(
                CppRegularGrid2DComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def grid(self):
        """The associated grid."""
        return CppRegularGrid2DWrapper.wrap(self._ptr.grid())


cdef class CppRegularGrid3DComponentDataItemWrapper:
    """Wrapper around a C++ ``Spatial::IRegularGrid3DComponentDataItem``.

    Canonical dimension ordering: z-cell 0, y-cell 1, x-cell 2.
    """

    cdef sp.IRegularGrid3DComponentDataItem* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @staticmethod
    cdef CppRegularGrid3DComponentDataItemWrapper wrap(
            sp.IRegularGrid3DComponentDataItem* ptr):
        cdef CppRegularGrid3DComponentDataItemWrapper obj = (
            CppRegularGrid3DComponentDataItemWrapper.__new__(
                CppRegularGrid3DComponentDataItemWrapper))
        obj._ptr = ptr
        return obj

    @property
    def id(self) -> str:
        """Unique identifier for this data item."""
        return (<cpp.IComponentDataItem*>self._ptr).id().decode("utf-8")

    @property
    def grid(self):
        """The associated grid."""
        return CppRegularGrid3DWrapper.wrap(self._ptr.grid())


def _register_abc_subclasses():
    """Register wrappers with the spatial ABCs."""
    from hydrocouple.spatial import (
        IEdge,
        IEnvelope,
        IGeometry,
        IGeometryComponentDataItem,
        ILineString,
        IMeshView,
        INetwork,
        INetworkComponentDataItem,
        IPoint,
        IPolygon,
        IPolyhedralSurface,
        IPolyhedralSurfaceComponentDataItem,
        IRaster,
        IRasterBand,
        IRasterComponentDataItem,
        IRegularGrid2D,
        IRegularGrid2DComponentDataItem,
        IRegularGrid3D,
        IRegularGrid3DComponentDataItem,
        ISpatialReferenceSystem,
        ITIN,
        ITINComponentDataItem,
        ITriangle,
        IVertex,
    )

    ISpatialReferenceSystem.register(CppSpatialReferenceSystemWrapper)
    IEnvelope.register(CppEnvelopeWrapper)
    IGeometry.register(CppGeometryWrapper)
    IPoint.register(CppPointWrapper)
    IVertex.register(CppVertexWrapper)
    ILineString.register(CppLineStringWrapper)
    IPolygon.register(CppPolygonWrapper)
    ITriangle.register(CppTriangleWrapper)
    IEdge.register(CppEdgeWrapper)
    IMeshView.register(CppMeshViewWrapper)
    INetwork.register(CppNetworkWrapper)
    IPolyhedralSurface.register(CppPolyhedralSurfaceWrapper)
    ITIN.register(CppTINWrapper)
    IRaster.register(CppRasterWrapper)
    IRasterBand.register(CppRasterBandWrapper)
    IRegularGrid2D.register(CppRegularGrid2DWrapper)
    IRegularGrid3D.register(CppRegularGrid3DWrapper)
    IGeometryComponentDataItem.register(CppGeometryComponentDataItemWrapper)
    INetworkComponentDataItem.register(CppNetworkComponentDataItemWrapper)
    IPolyhedralSurfaceComponentDataItem.register(
        CppPolyhedralSurfaceComponentDataItemWrapper)
    ITINComponentDataItem.register(CppTINComponentDataItemWrapper)
    IRasterComponentDataItem.register(CppRasterComponentDataItemWrapper)
    IRegularGrid2DComponentDataItem.register(
        CppRegularGrid2DComponentDataItemWrapper)
    IRegularGrid3DComponentDataItem.register(
        CppRegularGrid3DComponentDataItemWrapper)


_register_abc_subclasses()
