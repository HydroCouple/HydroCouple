# distutils: language = c++
# cython: language_level = 3
"""
Cython declarations for the C++ v2.0.0 interfaces in ``hydrocouplespatial.h``.
"""

from libcpp cimport bool as bint
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.memory cimport unique_ptr
from libc.stdint cimport int64_t, uint8_t, uint64_t

cimport _hydrocouple._core as cpp


cdef extern from "<span>" namespace "std":
    cdef cppclass span_const_double "std::span<const double>":
        span_const_double()
        span_const_double(const double* data, size_t size)
        const double* data() const
        size_t size() const
    cdef cppclass span_double "std::span<double>":
        span_double()
        span_double(double* data, size_t size)
        double* data() const
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

    cdef enum class VerticalCoordinateKind(uint8_t):
        Unknown
        Sigma
        ZLevel
        ZStar
        Hybrid
        Isopycnal
        DepthBelowSurface

    cdef enum class CrossSectionKind(uint8_t):
        Unknown
        StationElevation
        WidthElevation
        Analytic

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
    cdef cppclass IPoint
    cdef cppclass IPolygon
    cdef cppclass IPolyhedralSurface
    cdef cppclass IMultiCurve
    cdef cppclass IMultiPolygon
    cdef cppclass IRasterBand

    # ------------------------------------------------------------------
    # SRS / envelope
    # ------------------------------------------------------------------
    cdef cppclass ISpatialReferenceSystem:
        int authSRID() const
        const string& authName() const
        const string& srText() const
        cpp.IUnit_DistanceUnits distanceUnits() const
        const string& verticalAuthName() const
        int verticalAuthSRID() const
        const string& verticalSrText() const
        cpp.IUnit_DistanceUnits verticalDistanceUnits() const

    cdef cppclass IEnvelope:
        double minX() const
        double maxX() const
        double minY() const
        double maxY() const
        double minZ() const
        double maxZ() const

    # ------------------------------------------------------------------
    # Geometry hierarchy
    # ------------------------------------------------------------------
    cdef cppclass IGeometry:
        const string& id() const
        int64_t index() const
        int dimension() const
        int coordinateDimension() const
        IGeometry_GeometryType geometryType() const
        ISpatialReferenceSystem* spatialReferenceSystem() const
        IEnvelope* envelope() const
        string getWKT() except + nogil const
        vector[unsigned char] getWKB() except + nogil const
        bint isEmpty() const
        bint isSimple() except + nogil const
        bint is3D() const
        bint isMeasured() const
        unique_ptr[IGeometry] boundary() except + nogil const
        bint equals(const IGeometry& geom) except + nogil const
        bint disjoint(const IGeometry& geom) except + nogil const
        bint intersects(const IGeometry& geom) except + nogil const
        bint touches(const IGeometry& geom) except + nogil const
        bint crosses(const IGeometry& geom) except + nogil const
        bint within(const IGeometry& geom) except + nogil const
        bint contains(const IGeometry& geom) except + nogil const
        bint overlaps(const IGeometry& geom) except + nogil const
        bint relate(const IGeometry& geom,
                    const string& intersectionPatternMatrix) except + nogil const
        unique_ptr[IGeometry] locateAlong(double value) except + nogil const
        unique_ptr[IGeometry] locateBetween(double mStart,
                                            double mEnd) except + nogil const
        double distance(const IGeometry& geom) except + nogil const
        unique_ptr[IGeometry] buffer(double bufferDistance) except + nogil const
        unique_ptr[IGeometry] convexHull() except + nogil const
        unique_ptr[IGeometry] intersection(const IGeometry& geom) except + nogil const
        unique_ptr[IGeometry] unionG(const IGeometry& geom) except + nogil const
        unique_ptr[IGeometry] difference(const IGeometry& geom) except + nogil const
        unique_ptr[IGeometry] symmetricDifference(
            const IGeometry& geom) except + nogil const

    cdef cppclass IGeometryCollection(IGeometry):
        int64_t geometryCount() const
        IGeometry* geometry(int64_t index) except + nogil const

    cdef cppclass IPoint(IGeometry):
        double x() const
        double y() const
        double z() const
        double m() const

    cdef cppclass IMultiPoint(IGeometryCollection):
        IPoint* point(int64_t index) except + nogil const

    cdef cppclass IVertex(IPoint):
        IEdge* edge() const

    cdef cppclass ICurve(IGeometry):
        double length() const
        IPoint* startPoint() const
        IPoint* endPoint() const
        bint isClosed() const
        bint isRing() const

    cdef cppclass IMultiCurve(IGeometryCollection):
        bint isClosed() const
        double length() const

    cdef cppclass ILineString(ICurve):
        int64_t pointCount() const
        IPoint* point(int64_t index) except + nogil const

    cdef cppclass IMultiLineString(IMultiCurve):
        ILineString* lineString(int64_t index) except + nogil const

    cdef cppclass ISurface(IGeometry):
        double area() const
        unique_ptr[IPoint] centroid() except + nogil const
        unique_ptr[IPoint] pointOnSurface() except + nogil const
        unique_ptr[IMultiCurve] boundaryMultiCurve() except + nogil const

    cdef cppclass IMultiSurface(IGeometryCollection):
        double area() const
        unique_ptr[IPoint] centroid() except + nogil const
        unique_ptr[IPoint] pointOnSurface() except + nogil const

    cdef cppclass IPolygon(ISurface):
        ILineString* exteriorRing() const
        int64_t interiorRingCount() const
        ILineString* interiorRing(int64_t index) except + nogil const
        IEdge* edge() const
        IPolyhedralSurface* polyhedralSurface() const

    cdef cppclass IMultiPolygon(IMultiSurface):
        IPolygon* polygon(int64_t index) except + nogil const

    cdef cppclass ITriangle(IPolygon):
        IVertex* vertex1() const
        IVertex* vertex2() const
        IVertex* vertex3() const
        IVertex* vertex(int64_t index) except + nogil const

    cdef cppclass IEdge:
        int64_t index() const
        IVertex* orig() const
        IVertex* dest() const
        IPolygon* left() const
        IPolygon* right() const
        IPolygon* face() const
        IEdge* rot() const
        IEdge* invRot() const
        IEdge* sym() const
        IEdge* origNext() const
        IEdge* origPrev() const
        IEdge* destNext() const
        IEdge* destPrev() const
        IEdge* leftNext() const
        IEdge* leftPrev() const
        IEdge* rightNext() const
        IEdge* rightPrev() const

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
        IEdge* edge(int64_t index) except + nogil const
        int64_t vertexCount() const
        IVertex* vertex(int64_t index) except + nogil const
        const IMeshView* meshView() const

    cdef cppclass IPolyhedralSurface(ISurface):
        int64_t patchCount() const
        IPolygon* patch(int64_t index) except + nogil const
        int64_t vertexCount() const
        IVertex* vertex(int64_t index) except + nogil const
        unique_ptr[IMultiPolygon] boundingPolygons(
            const IPolygon* polygon) except + nogil const
        bint isClosed() const
        const IMeshView* meshView() const

    cdef cppclass ITIN(IPolyhedralSurface):
        ITriangle* triangle(int64_t index) except + nogil const

    # ------------------------------------------------------------------
    # Raster
    # ------------------------------------------------------------------
    cdef cppclass IRaster(cpp.IIdentity):
        int64_t xSize() const
        int64_t ySize() const
        int64_t rasterBandCount() const
        void addRasterBand(IRaster_RasterDataType dataType) except +
        ISpatialReferenceSystem* spatialReferenceSystem() const
        void geoTransformation(double* transformationMatrix) except + nogil const
        IRasterBand* getRasterBand(int64_t bandIndex) except + nogil const

    cdef cppclass IRasterBand(cpp.IIdentity):
        int64_t xSize() const
        int64_t ySize() const
        IRaster* raster() const
        IRaster_RasterDataType dataType() const
        bint read(int64_t xOffset, int64_t yOffset, int64_t xSize,
                  int64_t ySize, const cpp.BufferDescriptor& destination,
                  string* message) except + nogil const
        bint write(int64_t xOffset, int64_t yOffset, int64_t xSize,
                   int64_t ySize, const cpp.BufferDescriptor& source,
                   string* message) except + nogil
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
        IGeometry* geometry(int64_t geometryIndex) except + nogil const
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

    # ------------------------------------------------------------------
    # Vertical structure: coordinates, layering, cross-sections
    # ------------------------------------------------------------------
    cdef cppclass IVerticalCoordinate:
        VerticalCoordinateKind kind() const
        int64_t layerCount() const
        int64_t columnCount() const
        bint isTimeVarying() const
        uint64_t geometryEpoch() const
        double interfaceElevation(int64_t cellIndex,
                                  int64_t interfaceIndex) except + nogil const
        span_const_double interfaceElevations() except + nogil const

    cdef cppclass ILayering:
        cpp.IDimension* layerDimension() const
        IVerticalCoordinate* verticalCoordinate() const

    cdef cppclass ILayeredMeshComponentDataItem(
            IPolyhedralSurfaceComponentDataItem, ILayering):
        pass

    cdef cppclass ICrossSection:
        CrossSectionKind kind() const
        double invertElevation() const
        double topWidth(double stage) except + nogil const
        double storageArea(double stage) except + nogil const
        double flowArea(double stage) except + nogil const
        double wettedPerimeter(double stage) except + nogil const
        void evaluate(span_const_double stages, span_double topWidths,
                      span_double storageAreas, span_double flowAreas,
                      span_double wettedPerimeters) except + nogil const
        int64_t stationCount() const
        void stations(double* stations, double* elevations) except + nogil const

    cdef cppclass ILayeredNetworkComponentDataItem(
            INetworkComponentDataItem, ILayering):
        ICrossSection* crossSection(int64_t entityIndex) except + nogil const


cdef extern from "layered_casts.h" namespace "HydroCouple::Python":
    ILayering* asLayering(cpp.IComponentDataItem* item)
    ILayeredMeshComponentDataItem* asLayeredMesh(cpp.IComponentDataItem* item)
    ILayeredNetworkComponentDataItem* asLayeredNetwork(
        cpp.IComponentDataItem* item)


cdef extern from "interface_casts.h" namespace "HydroCouple::Python":
    IPoint* asPoint(IGeometry* g)
    IVertex* asVertex(IGeometry* g)
    ILineString* asLineString(IGeometry* g)
    ISurface* asSurface(IGeometry* g)
    IPolygon* asPolygon(IGeometry* g)
    ITriangle* asTriangle(IGeometry* g)
    IPolyhedralSurface* asPolyhedralSurface(IGeometry* g)
    ITIN* asTIN(IGeometry* g)
    IGeometryCollection* asGeometryCollection(IGeometry* g)
    IMultiPoint* asMultiPoint(IGeometry* g)
    IMultiCurve* asMultiCurve(IGeometry* g)
    IMultiLineString* asMultiLineString(IGeometry* g)
    IMultiSurface* asMultiSurface(IGeometry* g)
    IMultiPolygon* asMultiPolygon(IGeometry* g)
    IGeometryComponentDataItem* asGeometryItem(cpp.IComponentDataItem* item)
    INetworkComponentDataItem* asNetworkItem(cpp.IComponentDataItem* item)
    IPolyhedralSurfaceComponentDataItem* asPolyhedralSurfaceItem(
        cpp.IComponentDataItem* item)
    ITINComponentDataItem* asTINItem(cpp.IComponentDataItem* item)
    IRasterComponentDataItem* asRasterItem(cpp.IComponentDataItem* item)
    IRegularGrid2DComponentDataItem* asRegularGrid2DItem(
        cpp.IComponentDataItem* item)
    IRegularGrid3DComponentDataItem* asRegularGrid3DItem(
        cpp.IComponentDataItem* item)
