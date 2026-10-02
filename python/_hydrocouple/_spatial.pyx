# distutils: language = c++
# cython: language_level = 3
"""
Cython wrapper classes for the C++ v2.0.0 spatial interfaces.

Bulk accessors (mesh views, grid coordinate arrays, layer elevations) are
exposed as zero-copy, read-only NumPy views over the C++ spans; they remain
valid until the underlying mesh, grid or coordinate changes.

``as_spatial(item)`` and ``as_layered(item)`` give the spatial view of what
the core bindings hand out for a loaded component's items. Geometries that a
C++ operation creates (``buffer``, ``union``, ``centroid``, ...) are owned by
the wrapper returned for them; parts reached through a geometry (a ring, a
vertex) keep the geometry they belong to alive.
"""

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.memory cimport unique_ptr
from libc.stdint cimport int64_t, uint8_t, uint64_t, uintptr_t

cimport _hydrocouple._core as cpp
from _hydrocouple._core cimport (
    CppComponentDataItemWrapper,
    CppIdentityWrapper,
    bind_data_item,
    bind_identity,
    fill_host_descriptor,
    owned_by,
    wrap_dimension,
)
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


cdef object _distance_units(cpp.IUnit_DistanceUnits units):
    from hydrocouple.core import DistanceUnits
    return DistanceUnits(<int>units)


# ---------------------------------------------------------------------------
# SRS / envelope
# ---------------------------------------------------------------------------

cdef class CppSpatialReferenceSystemWrapper:
    """Wrapper around a C++ ``Spatial::ISpatialReferenceSystem`` pointer,
    horizontal and vertical."""

    cdef sp.ISpatialReferenceSystem* _ptr
    cdef object _owner

    def __cinit__(self):
        self._ptr = NULL

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

    @property
    def distance_units(self):
        """Units of horizontal distance."""
        return _distance_units(self._ptr.distanceUnits())

    @property
    def vertical_auth_name(self) -> str:
        """Authority of the vertical datum (e.g. ``"EPSG"``); empty when
        none is declared."""
        return self._ptr.verticalAuthName().decode("utf-8")

    @property
    def vertical_auth_srid(self) -> int:
        """Code of the vertical datum (e.g. 5703, NAVD88); 0 when none is
        declared."""
        return self._ptr.verticalAuthSRID()

    @property
    def vertical_sr_text(self) -> str:
        """Well-known text of the vertical datum."""
        return self._ptr.verticalSrText().decode("utf-8")

    @property
    def vertical_distance_units(self):
        """Units of elevations and depths."""
        return _distance_units(self._ptr.verticalDistanceUnits())


cdef object _wrap_srs(sp.ISpatialReferenceSystem* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppSpatialReferenceSystemWrapper obj = (
        CppSpatialReferenceSystemWrapper.__new__(
            CppSpatialReferenceSystemWrapper))
    obj._ptr = ptr
    obj._owner = owner
    return obj


cdef class CppEnvelopeWrapper:
    """Wrapper around a C++ ``Spatial::IEnvelope`` pointer."""

    cdef sp.IEnvelope* _ptr
    cdef object _owner

    def __cinit__(self):
        self._ptr = NULL

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


cdef object _wrap_envelope(sp.IEnvelope* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppEnvelopeWrapper obj = CppEnvelopeWrapper.__new__(
        CppEnvelopeWrapper)
    obj._ptr = ptr
    obj._owner = owner
    return obj


# ---------------------------------------------------------------------------
# Geometry wrappers
# ---------------------------------------------------------------------------

cdef const sp.IGeometry* _geometry_pointer(object geometry) except NULL:
    if not isinstance(geometry, CppGeometryWrapper):
        raise TypeError(f"expected a C++ geometry wrapper, got "
                        f"{type(geometry).__name__}")
    cdef sp.IGeometry* ptr = (<CppGeometryWrapper>geometry)._gptr
    if ptr == NULL:
        raise ValueError("geometry wrapper holds a null pointer")
    return ptr


cdef object _owned_geometry(sp.IGeometry* made):
    """Wrap a geometry a C++ operation created; the wrapper owns it."""
    return _wrap_geometry(made, None, True)


cdef class CppGeometryWrapper:
    """Wrapper around a C++ ``Spatial::IGeometry`` pointer.

    Two wrappers are equal when they wrap the same C++ geometry.
    """

    cdef sp.IGeometry* _gptr
    cdef unique_ptr[sp.IGeometry] _owned
    cdef object _owner

    def __cinit__(self):
        self._gptr = NULL

    def __dealloc__(self):
        self._owned.reset()

    def __eq__(self, other):
        if not isinstance(other, CppGeometryWrapper):
            return NotImplemented
        return self._gptr == (<CppGeometryWrapper>other)._gptr

    def __hash__(self):
        return hash(<uintptr_t>self._gptr)

    @property
    def id(self) -> str:
        """Id of the geometry."""
        return self._gptr.id().decode("utf-8")

    @property
    def index(self) -> int:
        """Index of the geometry within its collection."""
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
        """The SRS of this geometry, or ``None``."""
        return _wrap_srs(self._gptr.spatialReferenceSystem(), self)

    @property
    def envelope(self):
        """The bounding envelope, or ``None``."""
        return _wrap_envelope(self._gptr.envelope(), self)

    def get_wkt(self) -> str:
        """Well-known text representation."""
        return self._gptr.getWKT().decode("utf-8")

    def get_wkb(self) -> bytes:
        """Well-known binary representation."""
        cdef vector[unsigned char] wkb = self._gptr.getWKB()
        if wkb.size() == 0:
            return b""
        return (<char*>wkb.data())[:wkb.size()]

    @property
    def is_empty(self) -> bool:
        """Whether this is the empty geometry."""
        return self._gptr.isEmpty()

    @property
    def is_simple(self) -> bool:
        """Whether the geometry has no anomalous points (OGC simplicity)."""
        return self._gptr.isSimple()

    @property
    def is_3d(self) -> bool:
        """Whether this geometry has z coordinates."""
        return self._gptr.is3D()

    @property
    def is_measured(self) -> bool:
        """Whether this geometry has m values."""
        return self._gptr.isMeasured()

    @property
    def boundary(self):
        """The combinatorial boundary, as a new geometry."""
        return _owned_geometry(self._gptr.boundary().release())

    def equals(self, geom) -> bool:
        """Spatially equal to ``geom``."""
        return self._gptr.equals(_geometry_pointer(geom)[0])

    def disjoint(self, geom) -> bool:
        """No point in common with ``geom``."""
        return self._gptr.disjoint(_geometry_pointer(geom)[0])

    def intersects(self, geom) -> bool:
        """At least one point in common with ``geom``."""
        return self._gptr.intersects(_geometry_pointer(geom)[0])

    def touches(self, geom) -> bool:
        """Touches ``geom`` (boundaries meet, interiors do not)."""
        return self._gptr.touches(_geometry_pointer(geom)[0])

    def crosses(self, geom) -> bool:
        """Crosses ``geom``."""
        return self._gptr.crosses(_geometry_pointer(geom)[0])

    def within(self, geom) -> bool:
        """Within ``geom``."""
        return self._gptr.within(_geometry_pointer(geom)[0])

    def contains(self, geom) -> bool:
        """Contains ``geom``."""
        return self._gptr.contains(_geometry_pointer(geom)[0])

    def overlaps(self, geom) -> bool:
        """Overlaps ``geom``."""
        return self._gptr.overlaps(_geometry_pointer(geom)[0])

    def relate(self, geom, str intersection_pattern_matrix) -> bool:
        """Whether the DE-9IM relation to ``geom`` matches the pattern."""
        return self._gptr.relate(_geometry_pointer(geom)[0],
                                 intersection_pattern_matrix.encode("utf-8"))

    def locate_along(self, double value):
        """The part of the geometry at measure ``value``, as a new geometry."""
        return _owned_geometry(self._gptr.locateAlong(value).release())

    def locate_between(self, double m_start, double m_end):
        """The part between two measures, as a new geometry."""
        return _owned_geometry(
            self._gptr.locateBetween(m_start, m_end).release())

    def distance(self, geom) -> float:
        """Shortest distance to ``geom``."""
        return self._gptr.distance(_geometry_pointer(geom)[0])

    def buffer(self, double buffer_distance):
        """All points within ``buffer_distance``, as a new geometry."""
        return _owned_geometry(self._gptr.buffer(buffer_distance).release())

    def convex_hull(self):
        """The convex hull, as a new geometry."""
        return _owned_geometry(self._gptr.convexHull().release())

    def intersection(self, geom):
        """The point-set intersection with ``geom``, as a new geometry."""
        return _owned_geometry(
            self._gptr.intersection(_geometry_pointer(geom)[0]).release())

    def union(self, geom):
        """The point-set union with ``geom``, as a new geometry."""
        return _owned_geometry(
            self._gptr.unionG(_geometry_pointer(geom)[0]).release())

    def difference(self, geom):
        """The point-set difference with ``geom``, as a new geometry."""
        return _owned_geometry(
            self._gptr.difference(_geometry_pointer(geom)[0]).release())

    def symmetric_difference(self, geom):
        """The symmetric difference with ``geom``, as a new geometry."""
        return _owned_geometry(
            self._gptr.symmetricDifference(_geometry_pointer(geom)[0]).release())

    def __repr__(self):
        return f"<{type(self).__name__} {self.geometry_type.name}>"


cdef class CppPointWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::IPoint`` pointer."""

    cdef sp.IPoint* _pptr

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

    @property
    def vertex_index(self) -> int:
        """The vertex's index in its network or surface."""
        return self._gptr.index()

    @property
    def edge(self):
        """An outgoing edge of this vertex, or ``None``."""
        return _wrap_edge(self._vptr.edge(), self)


cdef class CppLineStringWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::ILineString`` pointer."""

    cdef sp.ILineString* _lptr
    cdef sp.ICurve* _cptr

    @property
    def length(self) -> float:
        """Curve length."""
        return self._cptr.length()

    @property
    def start_point(self):
        """The first point."""
        return _wrap_geometry(<sp.IGeometry*>self._cptr.startPoint(), self,
                              False)

    @property
    def end_point(self):
        """The last point."""
        return _wrap_geometry(<sp.IGeometry*>self._cptr.endPoint(), self, False)

    @property
    def is_closed(self) -> bool:
        """Whether start and end points coincide."""
        return self._cptr.isClosed()

    @property
    def is_ring(self) -> bool:
        """Closed and simple."""
        return self._cptr.isRing()

    @property
    def point_count(self) -> int:
        """Number of points."""
        return self._lptr.pointCount()

    def point(self, int64_t index):
        """The point at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._lptr.point(index), self,
                              False)


cdef class CppSurfaceWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::ISurface`` pointer."""

    cdef sp.ISurface* _surfptr

    @property
    def area(self) -> float:
        """Surface area."""
        return self._surfptr.area()

    @property
    def centroid(self):
        """The mathematical centroid, as a new point."""
        return _owned_geometry(<sp.IGeometry*>self._surfptr.centroid().release())

    @property
    def point_on_surface(self):
        """A point guaranteed to lie on the surface, as a new point."""
        return _owned_geometry(
            <sp.IGeometry*>self._surfptr.pointOnSurface().release())

    @property
    def boundary_multi_curve(self):
        """The boundary as a multi-curve, a new geometry."""
        return _owned_geometry(
            <sp.IGeometry*>self._surfptr.boundaryMultiCurve().release())


cdef class CppPolygonWrapper(CppSurfaceWrapper):
    """Wrapper around a C++ ``Spatial::IPolygon`` pointer."""

    cdef sp.IPolygon* _polyptr

    @property
    def exterior_ring(self):
        """The exterior boundary ring."""
        return _wrap_geometry(<sp.IGeometry*>self._polyptr.exteriorRing(),
                              self, False)

    @property
    def interior_ring_count(self) -> int:
        """Number of interior rings."""
        return self._polyptr.interiorRingCount()

    def interior_ring(self, int64_t index):
        """The interior ring at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._polyptr.interiorRing(index),
                              self, False)

    @property
    def edge(self):
        """An edge on the polygon's boundary (quad-edge topology), or
        ``None``."""
        return _wrap_edge(self._polyptr.edge(), self)

    @property
    def polyhedral_surface(self):
        """The surface this polygon is a patch of, or ``None``."""
        return _wrap_geometry(
            <sp.IGeometry*>self._polyptr.polyhedralSurface(), self, False)


cdef class CppTriangleWrapper(CppPolygonWrapper):
    """Wrapper around a C++ ``Spatial::ITriangle`` pointer."""

    cdef sp.ITriangle* _triptr

    @property
    def vertex1(self):
        """First vertex."""
        return _wrap_geometry(<sp.IGeometry*>self._triptr.vertex1(), self,
                              False)

    @property
    def vertex2(self):
        """Second vertex."""
        return _wrap_geometry(<sp.IGeometry*>self._triptr.vertex2(), self,
                              False)

    @property
    def vertex3(self):
        """Third vertex."""
        return _wrap_geometry(<sp.IGeometry*>self._triptr.vertex3(), self,
                              False)

    def vertex(self, int64_t index):
        """The vertex at the given index (0-2)."""
        return _wrap_geometry(<sp.IGeometry*>self._triptr.vertex(index), self,
                              False)


cdef class CppPolyhedralSurfaceWrapper(CppSurfaceWrapper):
    """Wrapper around a C++ ``Spatial::IPolyhedralSurface`` pointer."""

    cdef sp.IPolyhedralSurface* _sptr

    @property
    def patch_count(self) -> int:
        """Number of polygon patches."""
        return self._sptr.patchCount()

    def patch(self, int64_t index):
        """The patch at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._sptr.patch(index), self,
                              False)

    @property
    def vertex_count(self) -> int:
        """Number of vertices."""
        return self._sptr.vertexCount()

    def vertex(self, int64_t index):
        """The vertex at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._sptr.vertex(index), self,
                              False)

    def bounding_polygons(self, polygon):
        """The patches bounding ``polygon``, as a new multi-polygon."""
        if not isinstance(polygon, CppPolygonWrapper):
            raise TypeError("polygon must be a C++ polygon wrapper")
        return _owned_geometry(<sp.IGeometry*>self._sptr.boundingPolygons(
            (<CppPolygonWrapper>polygon)._polyptr).release())

    @property
    def is_closed(self) -> bool:
        """Whether the surface bounds a solid."""
        return self._sptr.isClosed()

    @property
    def mesh_view(self):
        """Bulk structure-of-arrays view of this surface."""
        return _wrap_mesh_view(self._sptr.meshView(), self)


cdef class CppTINWrapper(CppPolyhedralSurfaceWrapper):
    """Wrapper around a C++ ``Spatial::ITIN`` pointer."""

    cdef sp.ITIN* _tinptr

    def triangle(self, int64_t index):
        """The triangle at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._tinptr.triangle(index),
                              self, False)


cdef class CppGeometryCollectionWrapper(CppGeometryWrapper):
    """Wrapper around a C++ ``Spatial::IGeometryCollection`` pointer."""

    cdef sp.IGeometryCollection* _collptr

    @property
    def geometry_count(self) -> int:
        """Number of member geometries."""
        return self._collptr.geometryCount()

    def geometry(self, int64_t index):
        """The member geometry at the given index."""
        return _wrap_geometry(self._collptr.geometry(index), self, False)


cdef class CppMultiPointWrapper(CppGeometryCollectionWrapper):
    """Wrapper around a C++ ``Spatial::IMultiPoint`` pointer."""

    cdef sp.IMultiPoint* _mpptr

    def point(self, int64_t index):
        """The point at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._mpptr.point(index), self,
                              False)


cdef class CppMultiCurveWrapper(CppGeometryCollectionWrapper):
    """Wrapper around a C++ ``Spatial::IMultiCurve`` pointer."""

    cdef sp.IMultiCurve* _mcptr

    @property
    def is_closed(self) -> bool:
        """Whether every member curve is closed."""
        return self._mcptr.isClosed()

    @property
    def length(self) -> float:
        """Total length."""
        return self._mcptr.length()


cdef class CppMultiLineStringWrapper(CppMultiCurveWrapper):
    """Wrapper around a C++ ``Spatial::IMultiLineString`` pointer."""

    cdef sp.IMultiLineString* _mlptr

    def line_string(self, int64_t index):
        """The line string at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._mlptr.lineString(index),
                              self, False)


cdef class CppMultiSurfaceWrapper(CppGeometryCollectionWrapper):
    """Wrapper around a C++ ``Spatial::IMultiSurface`` pointer."""

    cdef sp.IMultiSurface* _msptr

    @property
    def area(self) -> float:
        """Total area."""
        return self._msptr.area()

    @property
    def centroid(self):
        """The centroid, as a new point."""
        return _owned_geometry(<sp.IGeometry*>self._msptr.centroid().release())

    @property
    def point_on_surface(self):
        """A point on one of the surfaces, as a new point."""
        return _owned_geometry(
            <sp.IGeometry*>self._msptr.pointOnSurface().release())


cdef class CppMultiPolygonWrapper(CppMultiSurfaceWrapper):
    """Wrapper around a C++ ``Spatial::IMultiPolygon`` pointer."""

    cdef sp.IMultiPolygon* _mpolyptr

    def polygon(self, int64_t index):
        """The polygon at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._mpolyptr.polygon(index),
                              self, False)


cdef void _bind_geometry(CppGeometryWrapper obj, sp.IGeometry* g):
    """Set every typed pointer the wrapper's class has. dynamic_cast, not a
    static chain: IGeometry is a virtual base throughout."""
    obj._gptr = g
    if isinstance(obj, CppPointWrapper):
        (<CppPointWrapper>obj)._pptr = sp.asPoint(g)
    if isinstance(obj, CppVertexWrapper):
        (<CppVertexWrapper>obj)._vptr = sp.asVertex(g)
    if isinstance(obj, CppLineStringWrapper):
        (<CppLineStringWrapper>obj)._lptr = sp.asLineString(g)
        (<CppLineStringWrapper>obj)._cptr = <sp.ICurve*>sp.asLineString(g)
    if isinstance(obj, CppSurfaceWrapper):
        (<CppSurfaceWrapper>obj)._surfptr = sp.asSurface(g)
    if isinstance(obj, CppPolygonWrapper):
        (<CppPolygonWrapper>obj)._polyptr = sp.asPolygon(g)
    if isinstance(obj, CppTriangleWrapper):
        (<CppTriangleWrapper>obj)._triptr = sp.asTriangle(g)
    if isinstance(obj, CppPolyhedralSurfaceWrapper):
        (<CppPolyhedralSurfaceWrapper>obj)._sptr = sp.asPolyhedralSurface(g)
    if isinstance(obj, CppTINWrapper):
        (<CppTINWrapper>obj)._tinptr = sp.asTIN(g)
    if isinstance(obj, CppGeometryCollectionWrapper):
        (<CppGeometryCollectionWrapper>obj)._collptr = sp.asGeometryCollection(g)
    if isinstance(obj, CppMultiPointWrapper):
        (<CppMultiPointWrapper>obj)._mpptr = sp.asMultiPoint(g)
    if isinstance(obj, CppMultiCurveWrapper):
        (<CppMultiCurveWrapper>obj)._mcptr = sp.asMultiCurve(g)
    if isinstance(obj, CppMultiLineStringWrapper):
        (<CppMultiLineStringWrapper>obj)._mlptr = sp.asMultiLineString(g)
    if isinstance(obj, CppMultiSurfaceWrapper):
        (<CppMultiSurfaceWrapper>obj)._msptr = sp.asMultiSurface(g)
    if isinstance(obj, CppMultiPolygonWrapper):
        (<CppMultiPolygonWrapper>obj)._mpolyptr = sp.asMultiPolygon(g)


cdef object _geometry_class(sp.IGeometry* g):
    """The most specific wrapper class for a geometry."""
    if sp.asVertex(g) != NULL:
        return CppVertexWrapper
    if sp.asPoint(g) != NULL:
        return CppPointWrapper
    if sp.asTriangle(g) != NULL:
        return CppTriangleWrapper
    if sp.asPolygon(g) != NULL:
        return CppPolygonWrapper
    if sp.asLineString(g) != NULL:
        return CppLineStringWrapper
    if sp.asTIN(g) != NULL:
        return CppTINWrapper
    if sp.asPolyhedralSurface(g) != NULL:
        return CppPolyhedralSurfaceWrapper
    if sp.asSurface(g) != NULL:
        return CppSurfaceWrapper
    if sp.asMultiPolygon(g) != NULL:
        return CppMultiPolygonWrapper
    if sp.asMultiSurface(g) != NULL:
        return CppMultiSurfaceWrapper
    if sp.asMultiLineString(g) != NULL:
        return CppMultiLineStringWrapper
    if sp.asMultiCurve(g) != NULL:
        return CppMultiCurveWrapper
    if sp.asMultiPoint(g) != NULL:
        return CppMultiPointWrapper
    if sp.asGeometryCollection(g) != NULL:
        return CppGeometryCollectionWrapper
    return CppGeometryWrapper


cdef object _wrap_geometry(sp.IGeometry* g, object owner, bint owned):
    """The most specific wrapper for ``g``, or ``None`` for NULL.

    ``owned``: the wrapper takes ownership (a geometry an operation made).
    ``owner``: what to keep alive while the wrapper lives.
    """
    if g == NULL:
        return None
    cls = _geometry_class(g)
    cdef CppGeometryWrapper obj = cls.__new__(cls)
    _bind_geometry(obj, g)
    obj._owner = owner
    if owned:
        obj._owned.reset(g)
    return obj


# ---------------------------------------------------------------------------
# Quad-edge topology
# ---------------------------------------------------------------------------

cdef class CppEdgeWrapper:
    """Wrapper around a C++ ``Spatial::IEdge`` pointer (quad-edge)."""

    cdef sp.IEdge* _ptr
    cdef object _owner

    def __cinit__(self):
        self._ptr = NULL

    def __eq__(self, other):
        if not isinstance(other, CppEdgeWrapper):
            return NotImplemented
        return self._ptr == (<CppEdgeWrapper>other)._ptr

    def __hash__(self):
        return hash(<uintptr_t>self._ptr)

    @property
    def index(self) -> int:
        """Unique edge index."""
        return self._ptr.index()

    @property
    def orig(self):
        """Origin vertex, or ``None``."""
        return _wrap_geometry(<sp.IGeometry*>self._ptr.orig(), self._owner,
                              False)

    @property
    def dest(self):
        """Destination vertex, or ``None``."""
        return _wrap_geometry(<sp.IGeometry*>self._ptr.dest(), self._owner,
                              False)

    @property
    def left(self):
        """The face on the left, or ``None``."""
        return _wrap_geometry(<sp.IGeometry*>self._ptr.left(), self._owner,
                              False)

    @property
    def right(self):
        """The face on the right, or ``None``."""
        return _wrap_geometry(<sp.IGeometry*>self._ptr.right(), self._owner,
                              False)

    @property
    def face(self):
        """The face this edge belongs to, or ``None``."""
        return _wrap_geometry(<sp.IGeometry*>self._ptr.face(), self._owner,
                              False)

    @property
    def rot(self):
        """The dual edge, rotated 90 degrees counter-clockwise."""
        return _wrap_edge(self._ptr.rot(), self._owner)

    @property
    def inv_rot(self):
        """The dual edge, rotated 90 degrees clockwise."""
        return _wrap_edge(self._ptr.invRot(), self._owner)

    @property
    def sym(self):
        """The edge from dest to orig."""
        return _wrap_edge(self._ptr.sym(), self._owner)

    @property
    def orig_next(self):
        """Next counter-clockwise edge around the origin."""
        return _wrap_edge(self._ptr.origNext(), self._owner)

    @property
    def orig_prev(self):
        """Next clockwise edge around the origin."""
        return _wrap_edge(self._ptr.origPrev(), self._owner)

    @property
    def dest_next(self):
        """Next counter-clockwise edge around the destination."""
        return _wrap_edge(self._ptr.destNext(), self._owner)

    @property
    def dest_prev(self):
        """Next clockwise edge around the destination."""
        return _wrap_edge(self._ptr.destPrev(), self._owner)

    @property
    def left_next(self):
        """Next counter-clockwise edge around the left face."""
        return _wrap_edge(self._ptr.leftNext(), self._owner)

    @property
    def left_prev(self):
        """Next clockwise edge around the left face."""
        return _wrap_edge(self._ptr.leftPrev(), self._owner)

    @property
    def right_next(self):
        """Next counter-clockwise edge around the right face."""
        return _wrap_edge(self._ptr.rightNext(), self._owner)

    @property
    def right_prev(self):
        """Next clockwise edge around the right face."""
        return _wrap_edge(self._ptr.rightPrev(), self._owner)


cdef object _wrap_edge(sp.IEdge* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppEdgeWrapper obj = CppEdgeWrapper.__new__(CppEdgeWrapper)
    obj._ptr = ptr
    obj._owner = owner
    return obj


# ---------------------------------------------------------------------------
# Bulk mesh view
# ---------------------------------------------------------------------------

cdef class CppMeshViewWrapper:
    """Wrapper around a C++ ``Spatial::IMeshView`` pointer.

    All array properties are zero-copy, read-only NumPy views over the
    C++ spans; they remain valid until the mesh changes.
    """

    cdef const sp.IMeshView* _ptr
    cdef object _owner

    def __cinit__(self):
        self._ptr = NULL

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

    @property
    def face_edge_offsets(self):
        """CSR row offsets into :attr:`face_edges` (int64)."""
        return _i64_view(self._ptr.faceEdgeOffsets())

    @property
    def face_edges(self):
        """Concatenated edge indexes of all faces (int64)."""
        return _i64_view(self._ptr.faceEdges())

    @property
    def edge_faces(self):
        """Left/right face of every edge (int64, ``2 * edge_count``); -1 marks the outside."""
        return _i64_view(self._ptr.edgeFaces())

    @property
    def face_x(self):
        """x of every face's representative point; empty if the producer holds none."""
        return _f64_view(self._ptr.faceX())

    @property
    def face_y(self):
        """y of every face's representative point; empty if the producer holds none."""
        return _f64_view(self._ptr.faceY())

    @property
    def face_areas(self):
        """Plan area of every face; empty if the producer holds none."""
        return _f64_view(self._ptr.faceAreas())

    @property
    def edge_lengths(self):
        """Length of every edge; empty if the producer holds none."""
        return _f64_view(self._ptr.edgeLengths())

    @property
    def edge_normal_x(self):
        """x of every edge's unit normal (left face to right face); empty if not held."""
        return _f64_view(self._ptr.edgeNormalX())

    @property
    def edge_normal_y(self):
        """y of every edge's unit normal; empty if not held."""
        return _f64_view(self._ptr.edgeNormalY())


cdef object _wrap_mesh_view(const sp.IMeshView* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppMeshViewWrapper obj = CppMeshViewWrapper.__new__(
        CppMeshViewWrapper)
    obj._ptr = ptr
    obj._owner = owner
    return obj


# ---------------------------------------------------------------------------
# Network
# ---------------------------------------------------------------------------

cdef class CppNetworkWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``Spatial::INetwork`` pointer."""

    cdef sp.INetwork* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @property
    def edge_count(self) -> int:
        """Number of edges."""
        return self._ptr.edgeCount()

    def edge(self, int64_t index):
        """The edge at the given index."""
        return _wrap_edge(self._ptr.edge(index), self)

    @property
    def vertex_count(self) -> int:
        """Number of vertices."""
        return self._ptr.vertexCount()

    def vertex(self, int64_t index):
        """The vertex at the given index."""
        return _wrap_geometry(<sp.IGeometry*>self._ptr.vertex(index), self,
                              False)

    @property
    def mesh_view(self):
        """Bulk structure-of-arrays view of this network."""
        return _wrap_mesh_view(self._ptr.meshView(), self)


cdef object _wrap_network(sp.INetwork* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppNetworkWrapper obj = CppNetworkWrapper.__new__(CppNetworkWrapper)
    obj._ptr = ptr
    bind_identity(obj, <cpp.IIdentity*>ptr)
    return owned_by(obj, owner)


# ---------------------------------------------------------------------------
# Raster
# ---------------------------------------------------------------------------

cdef class CppRasterWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``Spatial::IRaster`` pointer."""

    cdef sp.IRaster* _ptr

    def __cinit__(self):
        self._ptr = NULL

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

    def add_raster_band(self, data_type):
        """Append a band of the given ``RasterDataType``."""
        self._ptr.addRasterBand(<sp.IRaster_RasterDataType><int>int(data_type))

    @property
    def spatial_reference_system(self):
        """The raster's SRS, or ``None``."""
        return _wrap_srs(self._ptr.spatialReferenceSystem(), self)

    def geo_transformation(self):
        """The six-element affine geotransform (GDAL order) as float64."""
        out = np.zeros(6, dtype=np.float64)
        cdef double[::1] view = out
        self._ptr.geoTransformation(&view[0])
        return out

    def get_raster_band(self, int64_t band_index):
        """The band at the given index."""
        return _wrap_band(self._ptr.getRasterBand(band_index), self)


cdef class CppRasterBandWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``Spatial::IRasterBand`` pointer.

    ``read``/``write`` lend the caller's ndarray to C++ as a host
    descriptor -- no copy.
    """

    cdef sp.IRasterBand* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @property
    def x_size(self) -> int:
        """Number of columns."""
        return self._ptr.xSize()

    @property
    def y_size(self) -> int:
        """Number of rows."""
        return self._ptr.ySize()

    @property
    def raster(self):
        """The raster this band belongs to."""
        cdef sp.IRaster* raster = self._ptr.raster()
        if raster == NULL:
            return None
        cdef CppRasterWrapper obj = CppRasterWrapper.__new__(CppRasterWrapper)
        obj._ptr = raster
        bind_identity(obj, <cpp.IIdentity*>raster)
        return owned_by(obj, self)

    @property
    def data_type(self):
        """The band's ``RasterDataType``."""
        from hydrocouple.spatial import RasterDataType
        return RasterDataType(<int>self._ptr.dataType())

    def read(self, int64_t x_offset, int64_t y_offset, int64_t x_size,
             int64_t y_size, destination):
        """Read a window into ``destination`` (a writable ndarray).

        :returns: ``(ok, message)``.
        """
        cdef cpp.BufferDescriptor d
        cdef vector[int64_t] shape_buf, strides_buf
        fill_host_descriptor(destination, &d, &shape_buf, &strides_buf, True)
        cdef string msg
        cdef bint ok = self._ptr.read(x_offset, y_offset, x_size, y_size, d,
                                      &msg)
        return bool(ok), msg.decode("utf-8")

    def write(self, int64_t x_offset, int64_t y_offset, int64_t x_size,
              int64_t y_size, source):
        """Write a window from ``source`` (an ndarray).

        :returns: ``(ok, message)``.
        """
        array = np.asarray(source)
        cdef cpp.BufferDescriptor d
        cdef vector[int64_t] shape_buf, strides_buf
        fill_host_descriptor(array, &d, &shape_buf, &strides_buf, False)
        cdef string msg
        cdef bint ok = self._ptr.write(x_offset, y_offset, x_size, y_size, d,
                                       &msg)
        return bool(ok), msg.decode("utf-8")

    @property
    def no_data(self) -> float:
        """The no-data sentinel value."""
        return self._ptr.noData()


cdef object _wrap_band(sp.IRasterBand* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppRasterBandWrapper obj = CppRasterBandWrapper.__new__(
        CppRasterBandWrapper)
    obj._ptr = ptr
    bind_identity(obj, <cpp.IIdentity*>ptr)
    return owned_by(obj, owner)


cdef object _wrap_raster(sp.IRaster* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppRasterWrapper obj = CppRasterWrapper.__new__(CppRasterWrapper)
    obj._ptr = ptr
    bind_identity(obj, <cpp.IIdentity*>ptr)
    return owned_by(obj, owner)


# ---------------------------------------------------------------------------
# Regular grids
# ---------------------------------------------------------------------------

cdef class CppRegularGrid2DWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``Spatial::IRegularGrid2D`` pointer."""

    cdef sp.IRegularGrid2D* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @property
    def spatial_reference_system(self):
        """The grid's SRS, or ``None``."""
        return _wrap_srs(self._ptr.spatialReferenceSystem(), self)

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

    def x_node_location(self, int64_t x_node_index,
                        int64_t y_node_index) -> float:
        """x coordinate of a node (spot queries)."""
        return self._ptr.xNodeLocation(x_node_index, y_node_index)

    def y_node_location(self, int64_t x_node_index,
                        int64_t y_node_index) -> float:
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

    def is_active(self, int64_t x_cell_index, int64_t y_cell_index) -> bool:
        """Whether a cell is active (spot queries)."""
        return self._ptr.isActive(x_cell_index, y_cell_index)

    @property
    def active_cells(self):
        """Bulk activity mask of all cells, row-major ``[y][x]`` (uint8)."""
        return _u8_view(self._ptr.activeCells()).reshape(
            self._ptr.numYNodes() - 1, self._ptr.numXNodes() - 1)


cdef class CppRegularGrid3DWrapper(CppIdentityWrapper):
    """Wrapper around a C++ ``Spatial::IRegularGrid3D`` pointer."""

    cdef sp.IRegularGrid3D* _ptr

    def __cinit__(self):
        self._ptr = NULL

    @property
    def spatial_reference_system(self):
        """The grid's SRS, or ``None``."""
        return _wrap_srs(self._ptr.spatialReferenceSystem(), self)

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

    def x_node_location(self, int64_t x_node_index,
                        int64_t y_node_index) -> float:
        """x coordinate of a node column (spot queries)."""
        return self._ptr.xNodeLocation(x_node_index, y_node_index)

    def y_node_location(self, int64_t x_node_index,
                        int64_t y_node_index) -> float:
        """y coordinate of a node column (spot queries)."""
        return self._ptr.yNodeLocation(x_node_index, y_node_index)

    def z_node_location(self, int64_t x_node_index, int64_t y_node_index,
                        int64_t z_node_index) -> float:
        """z coordinate of a node (spot queries)."""
        return self._ptr.zNodeLocation(x_node_index, y_node_index,
                                       z_node_index)

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

    @property
    def node_zs(self):
        """Bulk z coordinates, row-major ``[z][y][x]`` (zero-copy)."""
        return _f64_view(self._ptr.nodeZs()).reshape(
            self._ptr.numZNodes(), self._ptr.numYNodes(),
            self._ptr.numXNodes())

    def is_active(self, int64_t x_cell_index, int64_t y_cell_index,
                  int64_t z_cell_index) -> bool:
        """Whether a cell is active (spot queries)."""
        return self._ptr.isActive(x_cell_index, y_cell_index, z_cell_index)

    @property
    def active_cells(self):
        """Bulk activity mask of all cells, row-major ``[z][y][x]``
        (uint8)."""
        return _u8_view(self._ptr.activeCells()).reshape(
            self._ptr.numZNodes() - 1, self._ptr.numYNodes() - 1,
            self._ptr.numXNodes() - 1)


cdef object _wrap_grid2d(sp.IRegularGrid2D* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppRegularGrid2DWrapper obj = CppRegularGrid2DWrapper.__new__(
        CppRegularGrid2DWrapper)
    obj._ptr = ptr
    bind_identity(obj, <cpp.IIdentity*>ptr)
    return owned_by(obj, owner)


cdef object _wrap_grid3d(sp.IRegularGrid3D* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppRegularGrid3DWrapper obj = CppRegularGrid3DWrapper.__new__(
        CppRegularGrid3DWrapper)
    obj._ptr = ptr
    bind_identity(obj, <cpp.IIdentity*>ptr)
    return owned_by(obj, owner)


# ---------------------------------------------------------------------------
# Vertical structure: coordinates, layering, cross-sections
# ---------------------------------------------------------------------------

cdef class CppVerticalCoordinateWrapper:
    """Wrapper around a C++ ``Spatial::IVerticalCoordinate`` pointer.

    :attr:`interface_elevations` is a zero-copy, read-only NumPy view of
    shape ``(column_count, layer_count + 1)`` over the producer's own
    profile: valid until :attr:`geometry_epoch` changes, and it sees the
    producer's updates in place for as long as the producer keeps the same
    storage. Copy it to keep a profile beyond that.
    """

    cdef sp.IVerticalCoordinate* _ptr
    cdef object _owner

    def __cinit__(self):
        self._ptr = NULL

    @property
    def kind(self):
        """The coordinate family."""
        from hydrocouple.spatial import VerticalCoordinateKind
        return VerticalCoordinateKind(<int>self._ptr.kind())

    @property
    def layer_count(self) -> int:
        """Number of layers in a column."""
        return self._ptr.layerCount()

    @property
    def column_count(self) -> int:
        """Number of columns (plan entities)."""
        return self._ptr.columnCount()

    @property
    def is_time_varying(self) -> bool:
        """Whether interface elevations change from step to step."""
        return self._ptr.isTimeVarying()

    @property
    def geometry_epoch(self) -> int:
        """Counter incremented every time the elevations change."""
        return self._ptr.geometryEpoch()

    def interface_elevation(self, int64_t cell_index,
                            int64_t interface_index) -> float:
        """Elevation of one interface in one cell (0 = top)."""
        return self._ptr.interfaceElevation(cell_index, interface_index)

    @property
    def interface_elevations(self):
        """All interface elevations, ``(column_count, layer_count + 1)``,
        float64, read-only, zero-copy."""
        cdef int64_t columns = self._ptr.columnCount()
        cdef int64_t interfaces = self._ptr.layerCount() + 1
        cdef sp.span_const_double span = self._ptr.interfaceElevations()
        if <int64_t>span.size() != columns * interfaces:
            raise ValueError(
                f"interfaceElevations() returned {span.size()} values; "
                f"columnCount() * (layerCount() + 1) is "
                f"{columns * interfaces}")
        return _f64_view(span).reshape(columns, interfaces)


cdef object _wrap_vertical(sp.IVerticalCoordinate* ptr, object owner):
    if ptr == NULL:
        return None
    cdef CppVerticalCoordinateWrapper obj = (
        CppVerticalCoordinateWrapper.__new__(CppVerticalCoordinateWrapper))
    obj._ptr = ptr
    obj._owner = owner
    return obj


cdef object _output_span(object array, Py_ssize_t n, str name,
                         sp.span_double* span):
    """Point *span at a caller-owned output array, or leave it empty for None.

    Outputs are filled in place, so a copy would silently drop the results:
    an array that is not already writable, C-contiguous float64 of the right
    length is refused rather than converted.
    """
    if array is None:
        return None
    if not isinstance(array, np.ndarray):
        raise TypeError(f"{name} must be a numpy.ndarray or None")
    if array.dtype != np.float64 or array.ndim != 1 or array.shape[0] != n:
        raise ValueError(
            f"{name} must be a 1-D float64 array of len(stages) = {n}; "
            f"got {array.dtype} {array.shape}")
    if not array.flags.c_contiguous or not array.flags.writeable:
        raise ValueError(f"{name} must be C-contiguous and writable; it is "
                         f"filled in place")
    cdef double[::1] view = array
    if n > 0:
        span[0] = sp.span_double(&view[0], <size_t>n)
    return view


cdef class CppCrossSectionWrapper:
    """Wrapper around a C++ ``Spatial::ICrossSection`` pointer.

    :meth:`evaluate` hands the caller's NumPy arrays to C++ as spans: the
    stages are read and the outputs written without a copy, with the GIL
    released for the call.
    """

    cdef sp.ICrossSection* _ptr
    cdef object _owner

    def __cinit__(self):
        self._ptr = NULL

    @property
    def kind(self):
        """How the producer holds this shape."""
        from hydrocouple.spatial import CrossSectionKind
        return CrossSectionKind(<int>self._ptr.kind())

    @property
    def invert_elevation(self) -> float:
        """Lowest elevation in the section."""
        return self._ptr.invertElevation()

    def top_width(self, double stage) -> float:
        """Width of the water surface at ``stage``."""
        return self._ptr.topWidth(stage)

    def storage_area(self, double stage) -> float:
        """Total wetted area at ``stage``."""
        return self._ptr.storageArea(stage)

    def flow_area(self, double stage) -> float:
        """The conveying part of the wetted area at ``stage``."""
        return self._ptr.flowArea(stage)

    def wetted_perimeter(self, double stage) -> float:
        """Wetted perimeter of the conveying area at ``stage``."""
        return self._ptr.wettedPerimeter(stage)

    def evaluate(self, stages, top_widths=None, storage_areas=None,
                 flow_areas=None, wetted_perimeters=None):
        """Evaluate at many stages; each output is ``None`` (not wanted) or
        a writable float64 array of ``len(stages)`` filled in place."""
        stage_array = np.ascontiguousarray(stages, dtype=np.float64)
        if stage_array.ndim != 1:
            raise ValueError("stages must be one-dimensional")
        cdef const double[::1] stage_view = stage_array
        cdef Py_ssize_t n = stage_view.shape[0]
        cdef sp.span_const_double stage_span
        cdef sp.span_double tw, sa, fa, wp
        # The views are held until the call returns.
        keep = (_output_span(top_widths, n, "top_widths", &tw),
                _output_span(storage_areas, n, "storage_areas", &sa),
                _output_span(flow_areas, n, "flow_areas", &fa),
                _output_span(wetted_perimeters, n, "wetted_perimeters", &wp))
        if n == 0:
            return None
        stage_span = sp.span_const_double(&stage_view[0], <size_t>n)
        cdef const sp.ICrossSection* section = self._ptr
        with nogil:
            section.evaluate(stage_span, tw, sa, fa, wp)
        del keep
        return None

    @property
    def station_count(self) -> int:
        """Number of survey points, or 0 when the producer holds none."""
        return self._ptr.stationCount()

    def stations(self):
        """``(stations, elevations)``, left bank to right bank."""
        cdef int64_t n = self._ptr.stationCount()
        stations = np.empty(max(n, 0), dtype=np.float64)
        elevations = np.empty(max(n, 0), dtype=np.float64)
        cdef double[::1] s_view = stations
        cdef double[::1] e_view = elevations
        if n > 0:
            self._ptr.stations(&s_view[0], &e_view[0])
        return stations, elevations


cdef class CppLayeringWrapper:
    """Wrapper around a C++ ``Spatial::ILayering`` -- the view of a layered
    item for a consumer indifferent to its plan geometry."""

    cdef sp.ILayering* _ptr
    cdef object _owner

    def __cinit__(self):
        self._ptr = NULL

    @property
    def layer_dimension(self):
        """The layer dimension (role ``DimensionRole.Layer``)."""
        return owned_by(wrap_dimension(self._ptr.layerDimension()), self)

    @property
    def vertical_coordinate(self):
        """Where the layers are."""
        return _wrap_vertical(self._ptr.verticalCoordinate(), self)


# ---------------------------------------------------------------------------
# Spatial component data items
# ---------------------------------------------------------------------------
#
# Each derives from the core data-item wrapper (identity, value definition,
# dimensions, the typed data plane, signals) and keeps a typed pointer to its
# own interface. _bind_item_view sets every typed pointer the class has.

cdef class CppGeometryComponentDataItemWrapper(CppComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::IGeometryComponentDataItem``.

    Canonical dimension ordering: geometry is dimension 0 of ``shape``.
    """

    cdef sp.IGeometryComponentDataItem* _geom_item

    @property
    def geometry_type(self):
        """The type of the associated geometries."""
        from hydrocouple.spatial import GeometryType
        return GeometryType(<int>self._geom_item.geometryType())

    @property
    def geometry_count(self) -> int:
        """Number of associated geometries."""
        return self._geom_item.geometryCount()

    def geometry(self, int64_t geometry_index):
        """The geometry at the given index."""
        return _wrap_geometry(self._geom_item.geometry(geometry_index), self,
                              False)

    @property
    def geometry_dimension(self):
        """The geometry dimension (dimension 0 of ``shape``)."""
        return owned_by(wrap_dimension(self._geom_item.geometryDimension()),
                        self)

    @property
    def envelope(self):
        """Envelope bounding all associated geometries."""
        return _wrap_envelope(self._geom_item.envelope(), self)


cdef class CppNetworkComponentDataItemWrapper(CppComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::INetworkComponentDataItem``."""

    cdef sp.INetworkComponentDataItem* _net_item

    @property
    def network(self):
        """The associated network."""
        return _wrap_network(self._net_item.network(), self)

    @property
    def location(self):
        """Which network entity (Node or Edge) the values are attached to."""
        from hydrocouple.spatial import MeshLocation
        return MeshLocation(<int>self._net_item.location())

    @property
    def network_data_type(self):
        """Scalar, MultiScalar, Vector or Tensor."""
        from hydrocouple.spatial import SpatialDataType
        return SpatialDataType(<int>self._net_item.networkDataType())

    @property
    def vector_basis(self):
        """The frame of vector/tensor values."""
        from hydrocouple.spatial import VectorBasis
        return VectorBasis(<int>self._net_item.vectorBasis())

    @property
    def entity_dimension(self):
        """The entity dimension (dimension 0 of shape)."""
        return owned_by(wrap_dimension(self._net_item.entityDimension()), self)


cdef class CppPolyhedralSurfaceComponentDataItemWrapper(
        CppComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::IPolyhedralSurfaceComponentDataItem``."""

    cdef sp.IPolyhedralSurfaceComponentDataItem* _surf_item

    @property
    def polyhedral_surface(self):
        """The associated polyhedral surface."""
        return _wrap_geometry(
            <sp.IGeometry*>self._surf_item.polyhedralSurface(), self, False)

    @property
    def location(self):
        """Which mesh entity (Node, Edge, Face or Volume) the values are attached to."""
        from hydrocouple.spatial import MeshLocation
        return MeshLocation(<int>self._surf_item.location())

    @property
    def mesh_data_type(self):
        """Scalar, MultiScalar, Vector or Tensor."""
        from hydrocouple.spatial import SpatialDataType
        return SpatialDataType(<int>self._surf_item.meshDataType())

    @property
    def vector_basis(self):
        """The frame of vector/tensor values."""
        from hydrocouple.spatial import VectorBasis
        return VectorBasis(<int>self._surf_item.vectorBasis())

    @property
    def entity_dimension(self):
        """The entity dimension (dimension 0 of shape)."""
        return owned_by(wrap_dimension(self._surf_item.entityDimension()),
                        self)


cdef class CppTINComponentDataItemWrapper(
        CppPolyhedralSurfaceComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::ITINComponentDataItem``."""

    cdef sp.ITINComponentDataItem* _tin_item

    @property
    def tin(self):
        """The associated TIN."""
        return _wrap_geometry(<sp.IGeometry*>self._tin_item.TIN(), self, False)


cdef class CppRasterComponentDataItemWrapper(CppComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::IRasterComponentDataItem``.

    Canonical dimension ordering: band 0, y (row) 1, x (column) 2.
    """

    cdef sp.IRasterComponentDataItem* _raster_item

    @property
    def raster(self):
        """The associated raster."""
        return _wrap_raster(self._raster_item.raster(), self)

    @property
    def x_dimension(self):
        """The column dimension (dimension 2 of ``shape``)."""
        return owned_by(wrap_dimension(self._raster_item.xDimension()), self)

    @property
    def y_dimension(self):
        """The row dimension (dimension 1 of ``shape``)."""
        return owned_by(wrap_dimension(self._raster_item.yDimension()), self)

    @property
    def band_dimension(self):
        """The band dimension (dimension 0 of ``shape``)."""
        return owned_by(wrap_dimension(self._raster_item.bandDimension()),
                        self)


cdef class CppRegularGrid2DComponentDataItemWrapper(
        CppComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::IRegularGrid2DComponentDataItem``."""

    cdef sp.IRegularGrid2DComponentDataItem* _grid2d_item

    @property
    def grid(self):
        """The associated grid."""
        return _wrap_grid2d(self._grid2d_item.grid(), self)

    @property
    def location(self):
        """Which grid entity the values are attached to."""
        from hydrocouple.spatial import MeshLocation
        return MeshLocation(<int>self._grid2d_item.location())

    @property
    def x_cell_dimension(self):
        """The x-cell dimension (dimension 1 of ``shape``)."""
        return owned_by(wrap_dimension(self._grid2d_item.xCellDimension()),
                        self)

    @property
    def y_cell_dimension(self):
        """The y-cell dimension (dimension 0 of ``shape``)."""
        return owned_by(wrap_dimension(self._grid2d_item.yCellDimension()),
                        self)

    @property
    def cell_edge_dimension(self):
        """The per-cell edge dimension, when values attach to cell edges."""
        return owned_by(wrap_dimension(self._grid2d_item.cellEdgeDimension()),
                        self)

    @property
    def cell_vertex_dimension(self):
        """The per-cell vertex dimension, when values attach to vertices."""
        return owned_by(
            wrap_dimension(self._grid2d_item.cellVertexDimension()), self)


cdef class CppRegularGrid3DComponentDataItemWrapper(
        CppComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::IRegularGrid3DComponentDataItem``."""

    cdef sp.IRegularGrid3DComponentDataItem* _grid3d_item

    @property
    def grid(self):
        """The associated grid."""
        return _wrap_grid3d(self._grid3d_item.grid(), self)

    @property
    def location(self):
        """Which grid entity the values are attached to."""
        from hydrocouple.spatial import MeshLocation
        return MeshLocation(<int>self._grid3d_item.location())

    @property
    def x_cell_dimension(self):
        """The x-cell dimension (dimension 2 of ``shape``)."""
        return owned_by(wrap_dimension(self._grid3d_item.xCellDimension()),
                        self)

    @property
    def y_cell_dimension(self):
        """The y-cell dimension (dimension 1 of ``shape``)."""
        return owned_by(wrap_dimension(self._grid3d_item.yCellDimension()),
                        self)

    @property
    def z_cell_dimension(self):
        """The z-cell dimension (dimension 0 of ``shape``)."""
        return owned_by(wrap_dimension(self._grid3d_item.zCellDimension()),
                        self)

    @property
    def cell_face_dimension(self):
        """The per-cell face dimension, when values attach to cell faces."""
        return owned_by(wrap_dimension(self._grid3d_item.cellFaceDimension()),
                        self)

    @property
    def cell_vertex_dimension(self):
        """The per-cell vertex dimension, when values attach to vertices."""
        return owned_by(
            wrap_dimension(self._grid3d_item.cellVertexDimension()), self)


cdef class CppLayeredMeshComponentDataItemWrapper(
        CppPolyhedralSurfaceComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::ILayeredMeshComponentDataItem``:
    the surface item's accessors plus the layering."""

    cdef sp.ILayering* _layering

    @property
    def layer_dimension(self):
        """The layer dimension (dimension 1 of shape)."""
        return owned_by(wrap_dimension(self._layering.layerDimension()), self)

    @property
    def vertical_coordinate(self):
        """Where the layers are."""
        return _wrap_vertical(self._layering.verticalCoordinate(), self)


cdef class CppLayeredNetworkComponentDataItemWrapper(
        CppNetworkComponentDataItemWrapper):
    """Wrapper around a C++ ``Spatial::ILayeredNetworkComponentDataItem``:
    the network item's accessors plus the layering and cross-sections."""

    cdef sp.ILayeredNetworkComponentDataItem* _layered

    @property
    def layer_dimension(self):
        """The layer dimension (dimension 1 of shape)."""
        return owned_by(wrap_dimension(
            (<sp.ILayering*>self._layered).layerDimension()), self)

    @property
    def vertical_coordinate(self):
        """Where the layers are."""
        return _wrap_vertical(
            (<sp.ILayering*>self._layered).verticalCoordinate(), self)

    def cross_section(self, int64_t entity_index):
        """The channel shape at one entity, or ``None`` where there is none."""
        cdef sp.ICrossSection* section = self._layered.crossSection(
            entity_index)
        if section == NULL:
            return None
        cdef CppCrossSectionWrapper obj = CppCrossSectionWrapper.__new__(
            CppCrossSectionWrapper)
        obj._ptr = section
        obj._owner = self
        return obj


cdef int _require(void* ptr, str interface) except -1:
    if ptr == NULL:
        raise TypeError(f"the C++ item does not implement {interface}")
    return 0


cdef int _bind_item_view(CppComponentDataItemWrapper obj,
                         cpp.IComponentDataItem* p) except -1:
    """Bind a spatial item wrapper (or a subclass, such as a spatiotemporal
    one) to ``p``, setting every typed pointer its class has."""
    bind_data_item(obj, p)
    if isinstance(obj, CppGeometryComponentDataItemWrapper):
        (<CppGeometryComponentDataItemWrapper>obj)._geom_item = (
            sp.asGeometryItem(p))
        _require(<void*>sp.asGeometryItem(p), "IGeometryComponentDataItem")
    if isinstance(obj, CppNetworkComponentDataItemWrapper):
        (<CppNetworkComponentDataItemWrapper>obj)._net_item = (
            sp.asNetworkItem(p))
        _require(<void*>sp.asNetworkItem(p), "INetworkComponentDataItem")
    if isinstance(obj, CppPolyhedralSurfaceComponentDataItemWrapper):
        (<CppPolyhedralSurfaceComponentDataItemWrapper>obj)._surf_item = (
            sp.asPolyhedralSurfaceItem(p))
        _require(<void*>sp.asPolyhedralSurfaceItem(p),
                 "IPolyhedralSurfaceComponentDataItem")
    if isinstance(obj, CppTINComponentDataItemWrapper):
        (<CppTINComponentDataItemWrapper>obj)._tin_item = sp.asTINItem(p)
        _require(<void*>sp.asTINItem(p), "ITINComponentDataItem")
    if isinstance(obj, CppRasterComponentDataItemWrapper):
        (<CppRasterComponentDataItemWrapper>obj)._raster_item = (
            sp.asRasterItem(p))
        _require(<void*>sp.asRasterItem(p), "IRasterComponentDataItem")
    if isinstance(obj, CppRegularGrid2DComponentDataItemWrapper):
        (<CppRegularGrid2DComponentDataItemWrapper>obj)._grid2d_item = (
            sp.asRegularGrid2DItem(p))
        _require(<void*>sp.asRegularGrid2DItem(p),
                 "IRegularGrid2DComponentDataItem")
    if isinstance(obj, CppRegularGrid3DComponentDataItemWrapper):
        (<CppRegularGrid3DComponentDataItemWrapper>obj)._grid3d_item = (
            sp.asRegularGrid3DItem(p))
        _require(<void*>sp.asRegularGrid3DItem(p),
                 "IRegularGrid3DComponentDataItem")
    if isinstance(obj, CppLayeredMeshComponentDataItemWrapper):
        (<CppLayeredMeshComponentDataItemWrapper>obj)._layering = (
            <sp.ILayering*>sp.asLayeredMesh(p))
        _require(<void*>sp.asLayeredMesh(p), "ILayeredMeshComponentDataItem")
    if isinstance(obj, CppLayeredNetworkComponentDataItemWrapper):
        (<CppLayeredNetworkComponentDataItemWrapper>obj)._layered = (
            sp.asLayeredNetwork(p))
        _require(<void*>sp.asLayeredNetwork(p),
                 "ILayeredNetworkComponentDataItem")
    return 0


cdef cpp.IComponentDataItem* _plain_pointer(object item) except? NULL:
    """The IComponentDataItem* behind any core data-item wrapper."""
    if not isinstance(item, CppComponentDataItemWrapper):
        raise TypeError(
            "expected a wrapped C++ data item (CppComponentDataItemWrapper, "
            f"CppInputWrapper, CppOutputWrapper, ...), got "
            f"{type(item).__name__}")
    return (<CppComponentDataItemWrapper>item)._ptr


def _make_view(cls, item):
    """An instance of ``cls`` -- a spatial item wrapper class or a subclass
    of one -- viewing the same C++ object as ``item``, sharing its
    ownership. Raises ``TypeError`` if the object does not implement what
    ``cls`` wraps."""
    cdef cpp.IComponentDataItem* p = _plain_pointer(item)
    if p == NULL:
        raise ValueError("data item wrapper holds a null pointer")
    if not issubclass(cls, CppComponentDataItemWrapper):
        raise TypeError(f"{cls.__name__} is not a data-item wrapper class")
    view = cls.__new__(cls)
    _bind_item_view(<CppComponentDataItemWrapper>view, p)
    return owned_by(view, item)


def as_spatial(item):
    """The spatial view of a C++ data item, or ``None`` when it has none.

    Accepts any data-item wrapper the core bindings hand out (an output, an
    input, a result) and returns the most specific spatial wrapper the C++
    object implements: layered network or mesh, TIN, polyhedral surface,
    network, geometry, raster, or regular grid.
    """
    cdef cpp.IComponentDataItem* p = _plain_pointer(item)
    if p == NULL:
        return None
    if sp.asLayeredNetwork(p) != NULL:
        return _make_view(CppLayeredNetworkComponentDataItemWrapper, item)
    if sp.asLayeredMesh(p) != NULL:
        return _make_view(CppLayeredMeshComponentDataItemWrapper, item)
    if sp.asTINItem(p) != NULL:
        return _make_view(CppTINComponentDataItemWrapper, item)
    if sp.asPolyhedralSurfaceItem(p) != NULL:
        return _make_view(CppPolyhedralSurfaceComponentDataItemWrapper, item)
    if sp.asNetworkItem(p) != NULL:
        return _make_view(CppNetworkComponentDataItemWrapper, item)
    if sp.asGeometryItem(p) != NULL:
        return _make_view(CppGeometryComponentDataItemWrapper, item)
    if sp.asRasterItem(p) != NULL:
        return _make_view(CppRasterComponentDataItemWrapper, item)
    if sp.asRegularGrid3DItem(p) != NULL:
        return _make_view(CppRegularGrid3DComponentDataItemWrapper, item)
    if sp.asRegularGrid2DItem(p) != NULL:
        return _make_view(CppRegularGrid2DComponentDataItemWrapper, item)
    return None


def as_layered(item):
    """The layered view of a C++ data item, or ``None`` if it is not layered.

    Accepts what the core bindings hand out for a component's items -- a
    ``CppOutputWrapper``, ``CppInputWrapper`` or
    ``CppComponentDataItemWrapper`` -- and asks the C++ object which layered
    interface it implements: a
    :class:`CppLayeredNetworkComponentDataItemWrapper` or
    :class:`CppLayeredMeshComponentDataItemWrapper` when it is one of those,
    a bare :class:`CppLayeringWrapper` when it is layered some other way.
    The time-varying variants are recognised by
    ``_hydrocouple._spatiotemporal.as_time_layered``.
    """
    if not isinstance(item, CppComponentDataItemWrapper):
        plain = getattr(item, "data_item", None)
        if isinstance(plain, CppComponentDataItemWrapper):
            item = plain
    cdef cpp.IComponentDataItem* p = _plain_pointer(item)
    if p == NULL:
        return None
    if sp.asLayeredNetwork(p) != NULL:
        return _make_view(CppLayeredNetworkComponentDataItemWrapper, item)
    if sp.asLayeredMesh(p) != NULL:
        return _make_view(CppLayeredMeshComponentDataItemWrapper, item)
    cdef sp.ILayering* layering = sp.asLayering(p)
    if layering == NULL:
        return None
    cdef CppLayeringWrapper view = CppLayeringWrapper.__new__(
        CppLayeringWrapper)
    view._ptr = layering
    view._owner = item
    return view


#: Every (ABC, wrapper) pair this module registers. A registration asserts
#: that the wrapper implements the ABC; tests/test_wrapper_conformance.py holds
#: each wrapper to it, since ABC.register() itself checks nothing.
ABC_REGISTRATIONS = []


def _register(abc_class, wrapper):
    abc_class.register(wrapper)
    ABC_REGISTRATIONS.append((abc_class, wrapper))


def _register_abc_subclasses():
    """Register wrappers with the spatial ABCs."""
    import hydrocouple.spatial as abcs

    _register(abcs.ISpatialReferenceSystem, CppSpatialReferenceSystemWrapper)
    _register(abcs.IEnvelope, CppEnvelopeWrapper)
    _register(abcs.IGeometry, CppGeometryWrapper)
    _register(abcs.IPoint, CppPointWrapper)
    _register(abcs.IVertex, CppVertexWrapper)
    _register(abcs.ILineString, CppLineStringWrapper)
    _register(abcs.ISurface, CppSurfaceWrapper)
    _register(abcs.IPolygon, CppPolygonWrapper)
    _register(abcs.ITriangle, CppTriangleWrapper)
    _register(abcs.IPolyhedralSurface, CppPolyhedralSurfaceWrapper)
    _register(abcs.ITIN, CppTINWrapper)
    _register(abcs.IGeometryCollection, CppGeometryCollectionWrapper)
    _register(abcs.IMultiPoint, CppMultiPointWrapper)
    _register(abcs.IMultiCurve, CppMultiCurveWrapper)
    _register(abcs.IMultiLineString, CppMultiLineStringWrapper)
    _register(abcs.IMultiSurface, CppMultiSurfaceWrapper)
    _register(abcs.IMultiPolygon, CppMultiPolygonWrapper)
    _register(abcs.IEdge, CppEdgeWrapper)
    _register(abcs.IMeshView, CppMeshViewWrapper)
    _register(abcs.INetwork, CppNetworkWrapper)
    _register(abcs.IRaster, CppRasterWrapper)
    _register(abcs.IRasterBand, CppRasterBandWrapper)
    _register(abcs.IRegularGrid2D, CppRegularGrid2DWrapper)
    _register(abcs.IRegularGrid3D, CppRegularGrid3DWrapper)
    _register(abcs.IGeometryComponentDataItem,
              CppGeometryComponentDataItemWrapper)
    _register(abcs.INetworkComponentDataItem,
              CppNetworkComponentDataItemWrapper)
    _register(abcs.IPolyhedralSurfaceComponentDataItem,
              CppPolyhedralSurfaceComponentDataItemWrapper)
    _register(abcs.ITINComponentDataItem, CppTINComponentDataItemWrapper)
    _register(abcs.IRasterComponentDataItem, CppRasterComponentDataItemWrapper)
    _register(abcs.IRegularGrid2DComponentDataItem,
              CppRegularGrid2DComponentDataItemWrapper)
    _register(abcs.IRegularGrid3DComponentDataItem,
              CppRegularGrid3DComponentDataItemWrapper)
    _register(abcs.IVerticalCoordinate, CppVerticalCoordinateWrapper)
    _register(abcs.ICrossSection, CppCrossSectionWrapper)
    _register(abcs.ILayering, CppLayeringWrapper)
    _register(abcs.ILayeredMeshComponentDataItem,
              CppLayeredMeshComponentDataItemWrapper)
    _register(abcs.ILayeredNetworkComponentDataItem,
              CppLayeredNetworkComponentDataItemWrapper)


_register_abc_subclasses()
