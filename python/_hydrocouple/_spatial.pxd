# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ v2.0.0 interfaces in ``hydrocouplespatial.h``.
"""

from libcpp cimport bool as bint
from libcpp.vector cimport vector
from libcpp.string cimport string
from libc.stdint cimport int64_t, uint8_t

cimport _hydrocouple._core as cpp


cdef extern from "<span>" namespace "std":
    cdef cppclass span_const_double "std::span<const double>":
        span_const_double()
        const double* data() const
        size_t size() const
    cdef cppclass span_const_int64 "std::span<const int64_t>":
        span_const_int64()
        const int64_t* data() const
        size_t size() const
    cdef cppclass span_const_uint8 "std::span<const uint8_t>":
        span_const_uint8()
        const uint8_t* data() const
        size_t size() const


cdef extern from "hydrocouplespatial.h" namespace "HydroCouple::Spatial":

    # ------------------------------------------------------------------
    # Enums
    # ------------------------------------------------------------------
    cdef enum class MeshDataObjectType:
        Cell
        Vertex
        Edge
        Face

    cdef enum class NetworkDataObjectType:
        Node
        Edge

    cdef enum class SpatialDataType:
        Scalar
        MultiScalar
        Vector
        Tensor

    cdef enum class RegularGridType:
        Cartesian
        Rectilinear
        Curvilinear

    cdef enum class IGeometry_GeometryType "HydroCouple::Spatial::IGeometry::GeometryType":
        Geometry
        Point
        LineString
        Polygon
        Triangle

    cdef enum class IRaster_RasterDataType "HydroCouple::Spatial::IRaster::RasterDataType":
        Unknown
        Byte
        UInt16
        Int16
        UInt32
        Int32
        Float32
        Float64

    # ------------------------------------------------------------------
    # Forward declarations
    # ------------------------------------------------------------------
    cdef cppclass IEdge
    cdef cppclass IMeshView
    cdef cppclass IPolygon
    cdef cppclass IPolyhedralSurface
    cdef cppclass IRasterBand

    # ------------------------------------------------------------------
    # SRS / envelope
    # ------------------------------------------------------------------
    cdef cppclass ISpatialReferenceSystem:
        int authSRID() const
        const string& authName() const
        const string& srText() const

    cdef cppclass IEnvelope:
        double minX() const
        double maxX() const
        double minY() const
        double maxY() const
        double minZ() const
        double maxZ() const

    # ------------------------------------------------------------------
    # Geometry hierarchy (subset used by the wrappers)
    # ------------------------------------------------------------------
    cdef cppclass IGeometry:
        const string& id() const
        unsigned int index() const
        int dimension() const
        int coordinateDimension() const
        IGeometry_GeometryType geometryType() const
        ISpatialReferenceSystem* spatialReferenceSystem() const
        IEnvelope* envelope() const
        string getWKT() const
        bint isEmpty() const
        bint is3D() const
        bint isMeasured() const

    cdef cppclass IPoint(IGeometry):
        double x() const
        double y() const
        double z() const
        double m() const

    cdef cppclass IVertex(IPoint):
        IEdge* edge() const

    cdef cppclass ILineString(IGeometry):
        double length() const
        int pointCount() const
        IPoint* point(int index) const
        bint isClosed() const

    cdef cppclass IPolygon(IGeometry):
        double area() const
        ILineString* exteriorRing() const
        int interiorRingCount() const

    cdef cppclass ITriangle(IPolygon):
        IVertex* vertex1() const
        IVertex* vertex2() const
        IVertex* vertex3() const
        IVertex* vertex(int index) const

    cdef cppclass IEdge:
        unsigned int index() const
        IVertex* orig() const
        IVertex* dest() const
        IPolygon* left() const
        IPolygon* right() const
        IEdge* sym() const
        IEdge* origNext() const
        IEdge* destNext() const

    # ------------------------------------------------------------------
    # Bulk mesh view
    # ------------------------------------------------------------------
    cdef cppclass IMeshView:
        int64_t nodeCount() const
        int64_t edgeCount() const
        int64_t faceCount() const
        span_const_double nodeX() const
        span_const_double nodeY() const
        span_const_double nodeZ() const
        span_const_int64 faceNodeOffsets() const
        span_const_int64 faceNodes() const
        span_const_int64 edgeNodes() const

    # ------------------------------------------------------------------
    # Network / polyhedral surface / TIN
    # ------------------------------------------------------------------
    cdef cppclass INetwork(cpp.IIdentity):
        int64_t edgeCount() const
        IEdge* edge(int64_t index) const
        int64_t vertexCount() const
        IVertex* vertex(int64_t index) const
        const IMeshView* meshView() const

    cdef cppclass IPolyhedralSurface(IGeometry):
        int64_t patchCount() const
        IPolygon* patch(int64_t index) const
        int64_t vertexCount() const
        IVertex* vertex(int64_t index) const
        bint isClosed() const
        const IMeshView* meshView() const

    cdef cppclass ITIN(IPolyhedralSurface):
        ITriangle* triangle(int64_t index) const

    # ------------------------------------------------------------------
    # Raster
    # ------------------------------------------------------------------
    cdef cppclass IRaster(cpp.IIdentity):
        int xSize() const
        int ySize() const
        int rasterBandCount() const
        ISpatialReferenceSystem* spatialReferenceSystem() const
        IRasterBand* getRasterBand(int bandIndex) const

    cdef cppclass IRasterBand(cpp.IIdentity):
        int xSize() const
        int ySize() const
        IRaster* raster() const
        IRaster_RasterDataType dataType() const
        double noData() const

    # ------------------------------------------------------------------
    # Regular grids
    # ------------------------------------------------------------------
    cdef cppclass IRegularGrid2D(cpp.IIdentity):
        ISpatialReferenceSystem* spatialReferenceSystem() const
        RegularGridType gridType() const
        int numXNodes() const
        int numYNodes() const
        double xNodeLocation(int xNodeIndex, int yNodeIndex) const
        double yNodeLocation(int xNodeIndex, int yNodeIndex) const
        span_const_double nodeXs() const
        span_const_double nodeYs() const
        bint isActive(int xCellIndex, int yCellIndex) const
        span_const_uint8 activeCells() const

    cdef cppclass IRegularGrid3D(cpp.IIdentity):
        ISpatialReferenceSystem* spatialReferenceSystem() const
        RegularGridType gridType() const
        int numXNodes() const
        int numYNodes() const
        int numZNodes() const
        double xNodeLocation(int xNodeIndex, int yNodeIndex) const
        double yNodeLocation(int xNodeIndex, int yNodeIndex) const
        double zNodeLocation(int xNodeIndex, int yNodeIndex, int zNodeIndex) const
        span_const_double nodeXs() const
        span_const_double nodeYs() const
        span_const_double nodeZs() const
        bint isActive(int xCellIndex, int yCellIndex, int zCellIndex) const
        span_const_uint8 activeCells() const

    # ------------------------------------------------------------------
    # Spatial component data items (metadata; data plane inherited)
    # ------------------------------------------------------------------
    cdef cppclass IGeometryComponentDataItem(cpp.IComponentDataItem):
        IGeometry_GeometryType geometryType() const
        int64_t geometryCount() const
        IGeometry* geometry(int64_t geometryIndex) const
        cpp.IDimension* geometryDimension() const
        IEnvelope* envelope() const

    cdef cppclass INetworkComponentDataItem(cpp.IComponentDataItem):
        INetwork* network() const
        NetworkDataObjectType networkDataObjectType() const
        SpatialDataType networkDataType() const
        cpp.IDimension* edgeDimension() const
        cpp.IDimension* vertexDimension() const

    cdef cppclass IPolyhedralSurfaceComponentDataItem(cpp.IComponentDataItem):
        MeshDataObjectType meshDataObjectType() const
        SpatialDataType meshDataType() const
        IPolyhedralSurface* polyhedralSurface() const
        cpp.IDimension* patchDimension() const
        cpp.IDimension* edgeDimension() const
        cpp.IDimension* vertexDimension() const

    cdef cppclass ITINComponentDataItem(IPolyhedralSurfaceComponentDataItem):
        ITIN* TIN() const

    cdef cppclass IRasterComponentDataItem(cpp.IComponentDataItem):
        IRaster* raster() const
        cpp.IDimension* xDimension() const
        cpp.IDimension* yDimension() const
        cpp.IDimension* bandDimension() const

    cdef cppclass IRegularGrid2DComponentDataItem(cpp.IComponentDataItem):
        IRegularGrid2D* grid() const
        MeshDataObjectType meshDataObjectType() const
        cpp.IDimension* xCellDimension() const
        cpp.IDimension* yCellDimension() const
        cpp.IDimension* cellEdgeDimension() const
        cpp.IDimension* cellVertexDimension() const

    cdef cppclass IRegularGrid3DComponentDataItem(cpp.IComponentDataItem):
        IRegularGrid3D* grid() const
        MeshDataObjectType meshDataObjectType() const
        cpp.IDimension* xCellDimension() const
        cpp.IDimension* yCellDimension() const
        cpp.IDimension* zCellDimension() const
        cpp.IDimension* cellFaceDimension() const
        cpp.IDimension* cellVertexDimension() const
