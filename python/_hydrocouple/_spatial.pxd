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
    cdef enum class MeshLocation(uint8_t):
        Node
        Edge
        Face
        Volume

    cdef enum class VectorBasis(uint8_t):
        Unknown
        Cartesian
        EastNorthUp
        NormalTangential
        AlongEntity

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
        const string& verticalAuthName() const
        int verticalAuthSRID() const
        const string& verticalSrText() const

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
        int64_t index() const
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
        int64_t pointCount() const
        IPoint* point(int64_t index) const
        bint isClosed() const

    cdef cppclass IPolygon(IGeometry):
        double area() const
        ILineString* exteriorRing() const
        int64_t interiorRingCount() const

    cdef cppclass ITriangle(IPolygon):
        IVertex* vertex1() const
        IVertex* vertex2() const
        IVertex* vertex3() const
        IVertex* vertex(int64_t index) const

    cdef cppclass IEdge:
        int64_t index() const
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
        span_const_int64 faceEdgeOffsets() const
        span_const_int64 faceEdges() const
        span_const_int64 edgeFaces() const
        span_const_double faceX() const
        span_const_double faceY() const
        span_const_double faceAreas() const
        span_const_double edgeLengths() const
        span_const_double edgeNormalX() const
        span_const_double edgeNormalY() const

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
        int64_t xSize() const
        int64_t ySize() const
        int64_t rasterBandCount() const
        ISpatialReferenceSystem* spatialReferenceSystem() const
        IRasterBand* getRasterBand(int64_t bandIndex) const

    cdef cppclass IRasterBand(cpp.IIdentity):
        int64_t xSize() const
        int64_t ySize() const
        IRaster* raster() const
        IRaster_RasterDataType dataType() const
        double noData() const

    # ------------------------------------------------------------------
    # Regular grids
    # ------------------------------------------------------------------
    cdef cppclass IRegularGrid2D(cpp.IIdentity):
        ISpatialReferenceSystem* spatialReferenceSystem() const
        RegularGridType gridType() const
        int64_t numXNodes() const
        int64_t numYNodes() const
        double xNodeLocation(int64_t xNodeIndex, int64_t yNodeIndex) const
        double yNodeLocation(int64_t xNodeIndex, int64_t yNodeIndex) const
        span_const_double nodeXs() const
        span_const_double nodeYs() const
        bint isActive(int64_t xCellIndex, int64_t yCellIndex) const
        span_const_uint8 activeCells() const

    cdef cppclass IRegularGrid3D(cpp.IIdentity):
        ISpatialReferenceSystem* spatialReferenceSystem() const
        RegularGridType gridType() const
        int64_t numXNodes() const
        int64_t numYNodes() const
        int64_t numZNodes() const
        double xNodeLocation(int64_t xNodeIndex, int64_t yNodeIndex) const
        double yNodeLocation(int64_t xNodeIndex, int64_t yNodeIndex) const
        double zNodeLocation(int64_t xNodeIndex, int64_t yNodeIndex, int64_t zNodeIndex) const
        span_const_double nodeXs() const
        span_const_double nodeYs() const
        span_const_double nodeZs() const
        bint isActive(int64_t xCellIndex, int64_t yCellIndex, int64_t zCellIndex) const
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
        MeshLocation location() const
        SpatialDataType networkDataType() const
        VectorBasis vectorBasis() const
        cpp.IDimension* entityDimension() const

    cdef cppclass IPolyhedralSurfaceComponentDataItem(cpp.IComponentDataItem):
        MeshLocation location() const
        SpatialDataType meshDataType() const
        VectorBasis vectorBasis() const
        IPolyhedralSurface* polyhedralSurface() const
        cpp.IDimension* entityDimension() const

    cdef cppclass ITINComponentDataItem(IPolyhedralSurfaceComponentDataItem):
        ITIN* TIN() const

    cdef cppclass IRasterComponentDataItem(cpp.IComponentDataItem):
        IRaster* raster() const
        cpp.IDimension* xDimension() const
        cpp.IDimension* yDimension() const
        cpp.IDimension* bandDimension() const

    cdef cppclass IRegularGrid2DComponentDataItem(cpp.IComponentDataItem):
        IRegularGrid2D* grid() const
        MeshLocation location() const
        cpp.IDimension* xCellDimension() const
        cpp.IDimension* yCellDimension() const
        cpp.IDimension* cellEdgeDimension() const
        cpp.IDimension* cellVertexDimension() const

    cdef cppclass IRegularGrid3DComponentDataItem(cpp.IComponentDataItem):
        IRegularGrid3D* grid() const
        MeshLocation location() const
        cpp.IDimension* xCellDimension() const
        cpp.IDimension* yCellDimension() const
        cpp.IDimension* zCellDimension() const
        cpp.IDimension* cellFaceDimension() const
        cpp.IDimension* cellVertexDimension() const
