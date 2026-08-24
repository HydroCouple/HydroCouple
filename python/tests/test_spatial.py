"""
Tests for spatial ABCs, wrapper registration, and wrapper surfaces (v2).
"""

from hydrocouple.spatial import (
    GeometryType,
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
    RasterDataType,
)


class TestSpatialABCRegistration:
    def test_geometry_wrappers_registered(self):
        from _hydrocouple._spatial import (
            CppEdgeWrapper,
            CppEnvelopeWrapper,
            CppGeometryWrapper,
            CppLineStringWrapper,
            CppNetworkWrapper,
            CppPointWrapper,
            CppPolygonWrapper,
            CppPolyhedralSurfaceWrapper,
            CppSpatialReferenceSystemWrapper,
            CppTINWrapper,
            CppTriangleWrapper,
            CppVertexWrapper,
        )

        assert issubclass(CppEnvelopeWrapper, IEnvelope)
        assert issubclass(CppGeometryWrapper, IGeometry)
        assert issubclass(CppPointWrapper, IPoint)
        assert issubclass(CppLineStringWrapper, ILineString)
        assert issubclass(CppPolygonWrapper, IPolygon)
        assert issubclass(CppVertexWrapper, IVertex)
        assert issubclass(CppEdgeWrapper, IEdge)
        assert issubclass(CppTriangleWrapper, ITriangle)
        assert issubclass(CppPolyhedralSurfaceWrapper, IPolyhedralSurface)
        assert issubclass(CppTINWrapper, ITIN)
        assert issubclass(CppNetworkWrapper, INetwork)
        assert issubclass(CppSpatialReferenceSystemWrapper,
                          ISpatialReferenceSystem)

    def test_mesh_view_wrapper_registered(self):
        from _hydrocouple._spatial import CppMeshViewWrapper

        assert issubclass(CppMeshViewWrapper, IMeshView)
        for attr in ("node_count", "edge_count", "face_count", "node_x",
                     "node_y", "node_z", "face_node_offsets", "face_nodes",
                     "edge_nodes"):
            assert hasattr(CppMeshViewWrapper, attr)

    def test_raster_wrappers_registered(self):
        from _hydrocouple._spatial import (
            CppRasterBandWrapper,
            CppRasterWrapper,
            CppRegularGrid2DWrapper,
            CppRegularGrid3DWrapper,
        )

        assert issubclass(CppRasterWrapper, IRaster)
        assert issubclass(CppRasterBandWrapper, IRasterBand)
        assert issubclass(CppRegularGrid2DWrapper, IRegularGrid2D)
        assert issubclass(CppRegularGrid3DWrapper, IRegularGrid3D)

    def test_component_data_item_wrappers_registered(self):
        from _hydrocouple._spatial import (
            CppGeometryComponentDataItemWrapper,
            CppNetworkComponentDataItemWrapper,
            CppPolyhedralSurfaceComponentDataItemWrapper,
            CppRasterComponentDataItemWrapper,
            CppRegularGrid2DComponentDataItemWrapper,
            CppRegularGrid3DComponentDataItemWrapper,
            CppTINComponentDataItemWrapper,
        )

        assert issubclass(CppGeometryComponentDataItemWrapper,
                          IGeometryComponentDataItem)
        assert issubclass(CppNetworkComponentDataItemWrapper,
                          INetworkComponentDataItem)
        assert issubclass(CppPolyhedralSurfaceComponentDataItemWrapper,
                          IPolyhedralSurfaceComponentDataItem)
        assert issubclass(CppTINComponentDataItemWrapper,
                          ITINComponentDataItem)
        assert issubclass(CppRasterComponentDataItemWrapper,
                          IRasterComponentDataItem)
        assert issubclass(CppRegularGrid2DComponentDataItemWrapper,
                          IRegularGrid2DComponentDataItem)
        assert issubclass(CppRegularGrid3DComponentDataItemWrapper,
                          IRegularGrid3DComponentDataItem)


class TestSpatialWrapperMethods:
    def test_geometry_wrapper_surface(self):
        from _hydrocouple._spatial import CppGeometryWrapper

        for attr in ("id", "geometry_type", "get_wkt", "envelope",
                     "is_empty", "is_3d"):
            assert hasattr(CppGeometryWrapper, attr)

    def test_edge_wrapper_quad_edge_navigation(self):
        from _hydrocouple._spatial import CppEdgeWrapper

        for attr in ("index", "orig", "dest", "sym", "orig_next",
                     "dest_next"):
            assert hasattr(CppEdgeWrapper, attr)

    def test_grid_wrapper_bulk_accessors(self):
        from _hydrocouple._spatial import CppRegularGrid2DWrapper

        for attr in ("node_xs", "node_ys", "active_cells",
                     "num_x_nodes", "num_y_nodes"):
            assert hasattr(CppRegularGrid2DWrapper, attr)

    def test_network_wrapper_mesh_view(self):
        from _hydrocouple._spatial import CppNetworkWrapper

        assert hasattr(CppNetworkWrapper, "mesh_view")


class TestSpatialEnums:
    def test_geometry_type_codes(self):
        assert GeometryType.Point == 1
        assert GeometryType.Triangle == 17
        assert GeometryType.PointZ == 1001
        assert GeometryType.TINZM == 3016

    def test_raster_data_type(self):
        assert RasterDataType.Float64 == 7
