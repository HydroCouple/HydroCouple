"""
Vertical structure in the Python mirror: IVerticalCoordinate, ILayering,
ICrossSection, the layered mesh/network items and their time-varying forms.

Three things are pinned here:

- the ABCs say what the C++ interfaces say -- member for member, parsed out
  of ``hydrocouplespatial.h`` so the two cannot drift;
- a pure-Python implementation is held to the full contract;
- a native C++ layered item reaches Python the way a loaded component's
  output does -- as a plain ``IComponentDataItem`` -- and ``as_layered`` /
  ``as_time_layered`` find the layered interfaces on it, with elevations
  and cross-section tables moving between the languages without a copy.
"""

import os
import re

import numpy as np
import pytest

from hydrocouple.core import DimensionRole, IComponentDataItem
from hydrocouple.spatial import (
    CrossSectionKind,
    ICrossSection,
    ILayeredMeshComponentDataItem,
    ILayeredNetworkComponentDataItem,
    ILayering,
    INetworkComponentDataItem,
    IPolyhedralSurfaceComponentDataItem,
    IVerticalCoordinate,
    MeshLocation,
    VerticalCoordinateKind,
)
from hydrocouple.spatiotemporal import (
    ITimeLayeredMeshComponentDataItem,
    ITimeLayeredNetworkComponentDataItem,
    ITimeNetworkComponentDataItem,
    ITimeSeriesPolyhedralSurfaceComponentDataItem,
)

HEADER = os.path.normpath(os.path.join(
    os.path.dirname(__file__), "..", "..", "include", "hydrocouplespatial.h"))


# ---------------------------------------------------------------------------
# The ABCs mirror the C++ interfaces member for member
# ---------------------------------------------------------------------------

def _cpp_members(class_name: str) -> set[str]:
    """Pure-virtual member names a C++ interface declares itself."""
    text = open(HEADER, encoding="utf-8").read()
    start = re.search(r"\n\s*class\s+" + class_name + r"\b[^;]*?\{", text)
    assert start, f"class {class_name} not found in hydrocouplespatial.h"
    body, depth = [], 1
    for char in text[start.end():]:
        depth += char == "{"
        depth -= char == "}"
        if depth == 0:
            break
        body.append(char)
    body = re.sub(r"/\*.*?\*/", "", "".join(body), flags=re.DOTALL)
    return set(re.findall(r"virtual[^;(]*?\b(\w+)\s*\([^;]*?=\s*0\s*;", body))


def _snake(name: str) -> str:
    return re.sub(r"(?<!^)(?=[A-Z])", "_", name).lower()


def _python_members(abc) -> set[str]:
    """Abstract members an ABC declares itself (not inherited)."""
    return {name for name, member in vars(abc).items()
            if getattr(getattr(member, "fget", member),
                       "__isabstractmethod__", False)}


@pytest.mark.parametrize("cpp_name,abc", [
    ("IVerticalCoordinate", IVerticalCoordinate),
    ("ILayering", ILayering),
    ("ICrossSection", ICrossSection),
    ("ILayeredNetworkComponentDataItem", ILayeredNetworkComponentDataItem),
    ("ILayeredMeshComponentDataItem", ILayeredMeshComponentDataItem),
])
def test_abc_declares_what_the_interface_declares(cpp_name, abc):
    expected = {_snake(name) for name in _cpp_members(cpp_name)}
    assert _python_members(abc) == expected


def test_layered_items_compose_the_plan_geometry_with_layering():
    assert issubclass(ILayeredMeshComponentDataItem,
                      IPolyhedralSurfaceComponentDataItem)
    assert issubclass(ILayeredMeshComponentDataItem, ILayering)
    assert issubclass(ILayeredNetworkComponentDataItem,
                      INetworkComponentDataItem)
    assert issubclass(ILayeredNetworkComponentDataItem, ILayering)
    assert issubclass(ITimeLayeredMeshComponentDataItem,
                      ITimeSeriesPolyhedralSurfaceComponentDataItem)
    assert issubclass(ITimeLayeredMeshComponentDataItem,
                      ILayeredMeshComponentDataItem)
    assert issubclass(ITimeLayeredNetworkComponentDataItem,
                      ITimeNetworkComponentDataItem)
    assert issubclass(ITimeLayeredNetworkComponentDataItem,
                      ILayeredNetworkComponentDataItem)


def test_layering_is_a_mixin_not_a_data_item():
    # A consumer that needs only "values by layer" tests for ILayering and
    # stays indifferent to the plan geometry; it is not itself an item.
    assert not issubclass(ILayering, IComponentDataItem)


# ---------------------------------------------------------------------------
# A pure-Python implementation is held to the contract
# ---------------------------------------------------------------------------

class PySigma(IVerticalCoordinate):
    """Sigma layers over a flat bed, in pure Python."""

    def __init__(self, columns=2, layers=3, bed=-6.0, stage=0.0):
        self._columns, self._layers, self._bed = columns, layers, bed
        self._stage, self._epoch = stage, 0

    @property
    def kind(self):
        return VerticalCoordinateKind.Sigma

    @property
    def layer_count(self):
        return self._layers

    @property
    def column_count(self):
        return self._columns

    @property
    def is_time_varying(self):
        return True

    @property
    def geometry_epoch(self):
        return self._epoch

    def interface_elevation(self, cell_index, interface_index):
        return self._stage + interface_index / self._layers * (
            self._bed - self._stage)

    @property
    def interface_elevations(self):
        profile = np.linspace(self._stage, self._bed, self._layers + 1)
        return np.tile(profile, (self._columns, 1))


class PyRectangle(ICrossSection):
    """A closed-form rectangular channel, in pure Python."""

    def __init__(self, width=5.0, invert=1.0):
        self._width, self._invert = width, invert

    @property
    def kind(self):
        return CrossSectionKind.Analytic

    @property
    def invert_elevation(self):
        return self._invert

    def top_width(self, stage):
        return 0.0 if stage < self._invert else self._width

    def storage_area(self, stage):
        return max(stage - self._invert, 0.0) * self._width

    def flow_area(self, stage):
        return self.storage_area(stage)

    def wetted_perimeter(self, stage):
        depth = stage - self._invert
        return 0.0 if depth < 0 else self._width + 2.0 * depth

    def evaluate(self, stages, top_widths=None, storage_areas=None,
                 flow_areas=None, wetted_perimeters=None):
        for out, method in ((top_widths, self.top_width),
                            (storage_areas, self.storage_area),
                            (flow_areas, self.flow_area),
                            (wetted_perimeters, self.wetted_perimeter)):
            if out is not None:
                out[:] = [method(s) for s in stages]

    @property
    def station_count(self):
        return 0

    def stations(self):
        return np.empty(0), np.empty(0)


class TestPurePythonImplementations:
    def test_complete_implementations_instantiate(self):
        sigma = PySigma()
        assert isinstance(sigma, IVerticalCoordinate)
        assert sigma.interface_elevations.shape == (2, 4)
        assert sigma.interface_elevation(1, 3) == pytest.approx(-6.0)
        assert isinstance(PyRectangle(), ICrossSection)

    def test_a_missing_member_is_refused(self):
        class NoBulkProfile(IVerticalCoordinate):
            kind = layer_count = column_count = None
            is_time_varying = geometry_epoch = None

            def interface_elevation(self, cell_index, interface_index):
                return 0.0

        with pytest.raises(TypeError, match="interface_elevations"):
            NoBulkProfile()

        class NoEvaluate(ICrossSection):
            kind = invert_elevation = station_count = None

            def top_width(self, stage): return 0.0
            def storage_area(self, stage): return 0.0
            def flow_area(self, stage): return 0.0
            def wetted_perimeter(self, stage): return 0.0
            def stations(self): return np.empty(0), np.empty(0)

        with pytest.raises(TypeError, match="evaluate"):
            NoEvaluate()

    def test_evaluate_fills_only_what_is_asked_for(self):
        stages = np.array([0.0, 1.0, 3.0])
        widths = np.full(3, -1.0)
        perimeters = np.full(3, -1.0)
        PyRectangle().evaluate(stages, top_widths=widths,
                               wetted_perimeters=perimeters)
        np.testing.assert_allclose(widths, [0.0, 5.0, 5.0])
        np.testing.assert_allclose(perimeters, [0.0, 5.0, 9.0])


# ---------------------------------------------------------------------------
# Native C++ items through the bindings
# ---------------------------------------------------------------------------

_spatial = pytest.importorskip("_hydrocouple._spatial")
_spatiotemporal = pytest.importorskip("_hydrocouple._spatiotemporal")
_testing = pytest.importorskip("_hydrocouple._testing")

BED, STAGE, INVERT = -10.0, 2.0, 1.0
COLUMNS, LAYERS = 3, 4


@pytest.fixture
def fixture():
    return _testing.LayeredFixture()


def _trapezoid(stage):
    """Closed form of the native surveyed trapezoid (bottom 4, 1:1 sides,
    banks 2 up, vertical walls above, a quarter ineffective)."""
    y = stage - INVERT
    if y < 0:
        return 0.0, 0.0, 0.0, 0.0
    if y <= 2.0:
        width, area = 4.0 + 2.0 * y, (4.0 + y) * y
        perimeter = 4.0 + 2.0 * np.sqrt(2.0) * y
    else:
        width, area = 8.0, 12.0 + 8.0 * (y - 2.0)
        perimeter = 4.0 + 4.0 * np.sqrt(2.0) + 2.0 * (y - 2.0)
    return width, area, 0.75 * area, perimeter


class TestWrapperRegistration:
    def test_wrappers_are_registered_with_the_abcs(self):
        assert issubclass(_spatial.CppVerticalCoordinateWrapper,
                          IVerticalCoordinate)
        assert issubclass(_spatial.CppCrossSectionWrapper, ICrossSection)
        assert issubclass(_spatial.CppLayeringWrapper, ILayering)
        assert issubclass(_spatial.CppLayeredMeshComponentDataItemWrapper,
                          ILayeredMeshComponentDataItem)
        assert issubclass(_spatial.CppLayeredNetworkComponentDataItemWrapper,
                          ILayeredNetworkComponentDataItem)
        assert issubclass(
            _spatiotemporal.CppTimeLayeredMeshComponentDataItemWrapper,
            ITimeLayeredMeshComponentDataItem)
        assert issubclass(
            _spatiotemporal.CppTimeLayeredNetworkComponentDataItemWrapper,
            ITimeLayeredNetworkComponentDataItem)

    def test_layered_wrappers_extend_the_plan_geometry_wrappers(self):
        assert issubclass(_spatial.CppLayeredNetworkComponentDataItemWrapper,
                          _spatial.CppNetworkComponentDataItemWrapper)
        assert issubclass(_spatial.CppLayeredMeshComponentDataItemWrapper,
                          _spatial.CppPolyhedralSurfaceComponentDataItemWrapper)
        assert issubclass(
            _spatiotemporal.CppTimeLayeredMeshComponentDataItemWrapper,
            _spatial.CppLayeredMeshComponentDataItemWrapper)

    def test_exchange_item_wrappers_expose_their_data_item(self):
        from _hydrocouple._core import (
            CppArgumentWrapper, CppInputWrapper, CppOutputWrapper)
        for wrapper in (CppArgumentWrapper, CppInputWrapper, CppOutputWrapper):
            assert hasattr(wrapper, "data_item")


class TestAsLayered:
    def test_finds_the_layered_network_on_a_plain_item(self, fixture):
        network = _spatial.as_layered(fixture.network)
        assert isinstance(network,
                          _spatial.CppLayeredNetworkComponentDataItemWrapper)
        assert isinstance(network, ILayeredNetworkComponentDataItem)
        assert network.id == "layered_network"
        assert network.location == MeshLocation.Node
        assert network.layer_dimension.role == DimensionRole.Layer
        assert network.entity_dimension.role == DimensionRole.Entity

    def test_a_time_layered_mesh_is_also_a_layered_mesh(self, fixture):
        mesh = _spatial.as_layered(fixture.time_mesh)
        assert isinstance(mesh, _spatial.CppLayeredMeshComponentDataItemWrapper)
        assert mesh.location == MeshLocation.Volume
        assert mesh.vertical_coordinate.kind == VerticalCoordinateKind.Sigma

    def test_an_item_that_is_not_layered_is_none(self, fixture):
        assert _spatial.as_layered(fixture.plain) is None
        assert _spatiotemporal.as_time_layered(fixture.plain) is None
        assert _spatiotemporal.as_time_layered(fixture.network) is None

    def test_anything_carrying_a_data_item_is_accepted(self, fixture):
        class Carrier:
            data_item = fixture.network

        assert _spatial.as_layered(Carrier()) is not None

    def test_something_else_is_a_type_error(self):
        with pytest.raises(TypeError):
            _spatial.as_layered("not an item")
        with pytest.raises(TypeError):
            _spatiotemporal.as_time_layered(42)

    def test_time_layered_view_adds_time(self, fixture):
        mesh = _spatiotemporal.as_time_layered(fixture.time_mesh)
        assert isinstance(
            mesh, _spatiotemporal.CppTimeLayeredMeshComponentDataItemWrapper)
        assert mesh.time_count == 2
        np.testing.assert_allclose(mesh.times, [2461000.5, 2461000.75])
        assert not mesh.times.flags.writeable
        assert mesh.time_dimension.role == DimensionRole.Time
        assert mesh.layer_dimension.role == DimensionRole.Layer


class TestVerticalCoordinate:
    def test_scalars(self, fixture):
        vertical = _spatial.as_layered(fixture.network).vertical_coordinate
        assert isinstance(vertical, IVerticalCoordinate)
        assert vertical.kind == VerticalCoordinateKind.Sigma
        assert vertical.layer_count == LAYERS
        assert vertical.column_count == COLUMNS
        assert vertical.is_time_varying

    def test_bulk_profile_is_shaped_top_first(self, fixture):
        vertical = _spatial.as_layered(fixture.network).vertical_coordinate
        profile = vertical.interface_elevations
        assert profile.shape == (COLUMNS, LAYERS + 1)
        assert profile.dtype == np.float64
        expected = np.linspace(STAGE, BED, LAYERS + 1)
        for column in profile:
            np.testing.assert_allclose(column, expected)
        assert vertical.interface_elevation(2, LAYERS) == pytest.approx(BED)

    def test_bulk_profile_is_a_read_only_view_of_the_producers_storage(
            self, fixture):
        vertical = _spatial.as_layered(fixture.network).vertical_coordinate
        profile = vertical.interface_elevations
        epoch = vertical.geometry_epoch
        with pytest.raises(ValueError):
            profile[0, 0] = 99.0

        fixture.set_stage(0.0)

        # The same array, not a fresh read, now shows the lowered surface.
        assert vertical.geometry_epoch == epoch + 1
        np.testing.assert_allclose(profile[1], np.linspace(0.0, BED, LAYERS + 1))

    def test_a_cpp_exception_becomes_a_python_one(self, fixture):
        vertical = _spatial.as_layered(fixture.network).vertical_coordinate
        with pytest.raises(IndexError):
            vertical.interface_elevation(COLUMNS, 0)


class TestCrossSection:
    def test_where_there_is_no_section_there_is_none(self, fixture):
        network = _spatial.as_layered(fixture.network)
        assert network.cross_section(1) is None
        with pytest.raises(IndexError):
            network.cross_section(3)

    def test_surveyed_points(self, fixture):
        section = _spatial.as_layered(fixture.network).cross_section(0)
        assert isinstance(section, ICrossSection)
        assert section.kind == CrossSectionKind.StationElevation
        assert section.invert_elevation == INVERT
        assert section.station_count == 4
        stations, elevations = section.stations()
        np.testing.assert_allclose(stations, [0.0, 2.0, 6.0, 8.0])
        np.testing.assert_allclose(elevations, [3.0, 1.0, 1.0, 3.0])

    def test_an_analytic_section_has_no_points_and_still_answers(self, fixture):
        section = _spatial.as_layered(fixture.network).cross_section(2)
        assert section.kind == CrossSectionKind.Analytic
        assert section.station_count == 0
        stations, elevations = section.stations()
        assert stations.size == 0 and elevations.size == 0
        assert section.storage_area(INVERT + 2.0) == pytest.approx(10.0)

    def test_per_stage_accessors_match_the_closed_form(self, fixture):
        section = _spatial.as_layered(fixture.network).cross_section(0)
        for stage in (0.0, INVERT, 1.5, 3.0, 4.5):
            width, area, flow, perimeter = _trapezoid(stage)
            assert section.top_width(stage) == pytest.approx(width)
            assert section.storage_area(stage) == pytest.approx(area)
            assert section.flow_area(stage) == pytest.approx(flow)
            assert section.wetted_perimeter(stage) == pytest.approx(perimeter)

    def test_at_the_invert_widths_take_their_limit_from_above(self, fixture):
        section = _spatial.as_layered(fixture.network).cross_section(0)
        assert section.storage_area(INVERT) == 0.0
        assert section.top_width(INVERT) == pytest.approx(4.0)
        assert section.wetted_perimeter(INVERT) == pytest.approx(4.0)

    def test_evaluate_fills_the_callers_arrays_in_place(self, fixture):
        section = _spatial.as_layered(fixture.network).cross_section(0)
        stages = np.array([0.0, INVERT, 1.5, 3.0, 4.5])
        widths, areas = np.zeros(5), np.zeros(5)
        flows, perimeters = np.zeros(5), np.zeros(5)
        same = widths

        assert section.evaluate(stages, widths, areas, flows, perimeters) is None

        assert same is widths
        expected = np.array([_trapezoid(s) for s in stages])
        np.testing.assert_allclose(widths, expected[:, 0])
        np.testing.assert_allclose(areas, expected[:, 1])
        np.testing.assert_allclose(flows, expected[:, 2])
        np.testing.assert_allclose(perimeters, expected[:, 3])

    def test_evaluate_leaves_unrequested_outputs_alone(self, fixture):
        section = _spatial.as_layered(fixture.network).cross_section(0)
        flows = np.zeros(2)
        section.evaluate([1.5, 3.0], flow_areas=flows)
        np.testing.assert_allclose(flows, [_trapezoid(1.5)[2], _trapezoid(3.0)[2]])
        section.evaluate(np.empty(0))  # nothing asked, nothing to do

    @pytest.mark.parametrize("bad,error", [
        (np.zeros(3, dtype=np.float32), ValueError),
        (np.zeros(4), ValueError),
        (np.zeros((2, 2)), ValueError),
        (np.zeros(6)[::2], ValueError),
        ([0.0, 0.0, 0.0], TypeError),
    ])
    def test_evaluate_refuses_outputs_it_could_only_fill_by_copying(
            self, fixture, bad, error):
        section = _spatial.as_layered(fixture.network).cross_section(0)
        with pytest.raises(error):
            section.evaluate(np.array([1.0, 2.0, 3.0]), top_widths=bad)

    def test_evaluate_refuses_a_read_only_output(self, fixture):
        section = _spatial.as_layered(fixture.network).cross_section(0)
        frozen = np.zeros(3)
        frozen.flags.writeable = False
        with pytest.raises(ValueError, match="writable"):
            section.evaluate(np.array([1.0, 2.0, 3.0]), storage_areas=frozen)
