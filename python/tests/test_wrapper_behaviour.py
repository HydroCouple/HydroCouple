"""
The C++ wrappers do what their interfaces say, end to end.

``test_wrapper_conformance.py`` proves every member exists; this proves the
members work, against native C++ fixtures (``binding_test_fixtures.h``)
reached through the ordinary wrappers: signals delivered as the event-args
objects the ABCs promise, ownership of what C++ creates, dispatch to the
most specific wrapper, value definitions with units, exchange-item wiring,
adapted outputs, workflows, the temporal and spatial views, geometries and
rasters.
"""

import gc

import numpy as np
import pytest

from hydrocouple.core import (
    ArgumentInputType,
    ArgumentRole,
    ComponentStatus,
    DifferentialRole,
    DistanceUnits,
    FundamentalUnitDimension,
    IAdaptedOutput,
    IAdaptedOutputFactory,
    IComponentDataItemValueChanged,
    IComponentStatusChangeEventArgs,
    IMultiInput,
    IModelComponentInfo,
    IQuality,
    IQuantity,
    IWorkflowComponentStatusChangeEventArgs,
    ValueKind,
    WorkflowStatus,
    DimensionRole,
)
from hydrocouple.spatial import (
    GeometryType,
    IPolygon,
    IVertex,
    RasterDataType,
)
from hydrocouple.spatiotemporal import ITimeLayeredMeshComponentDataItem
from hydrocouple.temporal import (
    ITimeModelComponent,
    TimeExtrapolation,
    TimeInterpolation,
    TimeKind,
)

_core = pytest.importorskip("_hydrocouple._core")
_temporal = pytest.importorskip("_hydrocouple._temporal")
_spatial = pytest.importorskip("_hydrocouple._spatial")
_spatiotemporal = pytest.importorskip("_hydrocouple._spatiotemporal")
_testing = pytest.importorskip("_hydrocouple._testing")


@pytest.fixture
def native():
    return _testing.BindingFixture()


@pytest.fixture
def component(native):
    return native.component


# ---------------------------------------------------------------------------
# Component information and ownership of what C++ creates
# ---------------------------------------------------------------------------

class TestComponentInfo:
    def test_metadata(self, native):
        info = native.info
        assert isinstance(info, IModelComponentInfo)
        assert info.id == "fixture-info"
        assert (info.developer, info.version, info.license) == (
            "HydroCouple", "1.2.3", "MIT")
        assert info.tags == {"hydrology", "test"}
        assert info.documentation == ["README.md"]
        info.library_file_path = "/opt/lib/libmodel.so"
        assert info.library_file_path == "/opt/lib/libmodel.so"

    def test_a_created_instance_is_owned_by_its_wrapper(self, native):
        before = _testing.live_components()
        made = native.info.create_component_instance()
        assert made.id == "made-1"
        assert _testing.live_components() == before + 1
        del made
        gc.collect()
        assert _testing.live_components() == before

    def test_what_is_reached_through_a_created_instance_keeps_it_alive(
            self, native):
        before = _testing.live_components()
        made = native.info.create_component_instance()
        discharge = made.outputs[0]
        clock = _temporal.as_time_model_component(made)
        del made
        gc.collect()
        assert _testing.live_components() == before + 1   # the item holds it
        assert discharge.id == "discharge"
        assert discharge.model_component == clock
        del discharge
        gc.collect()
        assert _testing.live_components() == before + 1   # the view holds it
        del clock
        gc.collect()
        assert _testing.live_components() == before

    def test_factories(self, native):
        factories = native.info.adapted_output_factories
        assert len(factories) == 1
        assert isinstance(factories[0], IAdaptedOutputFactory)
        assert factories[0].id == "scaling-factory"


# ---------------------------------------------------------------------------
# Dispatch, equality, value definitions
# ---------------------------------------------------------------------------

class TestComponentItems:
    def test_items_come_back_as_their_most_specific_wrappers(self, component):
        assert isinstance(component.inputs[0], _core.CppMultiInputWrapper)
        assert isinstance(component.inputs[0], IMultiInput)
        assert isinstance(component.outputs[0], _core.CppOutputWrapper)
        assert isinstance(component.arguments[0], _core.CppArgumentWrapper)
        # A result that is an output in C++ is handed out as one.
        assert isinstance(component.results[0], _core.CppOutputWrapper)

    def test_two_wrappers_of_one_object_are_equal(self, component):
        first, second = component.inputs[0], component.inputs[0]
        assert first is not second
        assert first == second and hash(first) == hash(second)
        assert len({first, second}) == 1
        assert component.outputs[0] != component.inputs[0]
        assert component.outputs[0].model_component == component

    def test_a_quantity_carries_its_unit(self, component):
        value = component.outputs[0].value_definition
        assert isinstance(value, IQuantity)
        assert value.value_kind == ValueKind.Flux
        assert (value.min_value, value.max_value) == (0.0, 1.0e6)
        unit = value.unit
        assert unit.caption == "ft3/s"
        assert unit.conversion_factor_to_si == pytest.approx(0.0283168)
        assert unit.dimensions.power(FundamentalUnitDimension.Length) == 3
        assert unit.dimensions.power(FundamentalUnitDimension.Time) == -1
        assert unit.dimensions.power(FundamentalUnitDimension.Mass) == 0

    def test_a_quality_carries_its_categories(self, component):
        value = component.results[0].value_definition
        assert isinstance(value, IQuality)
        assert value.categories == ["water", "forest", "urban"]
        assert value.is_ordered is False

    def test_dimensions_carry_roles(self, component):
        (cells,) = component.outputs[0].dimensions
        assert cells.role == DimensionRole.Entity

    def test_argument(self, component):
        argument = component.arguments[0]
        assert argument.role == ArgumentRole.Parameter
        assert any("FixtureArgument" in name
                   for name in argument.valid_component_data_item_types)
        assert argument.initialize('{"n": 0.03}', ArgumentInputType.JSON) == (
            True, "")
        assert str(argument) == '{"n": 0.03}'
        assert argument.initialize("x", ArgumentInputType.YAML)[0] is False
        assert argument.initialize(component.outputs[0]) == (True, "")
        assert str(argument) == "from discharge"
        assert argument.serialize(ArgumentInputType.JSON) == (
            True, "from discharge", "")
        with pytest.raises(TypeError):
            argument.initialize("no type given")


# ---------------------------------------------------------------------------
# Signals
# ---------------------------------------------------------------------------

class TestSignals:
    def test_status_slots_receive_event_args(self, component):
        received = []
        component.connect(received.append)
        component.initialize()
        assert [(e.previous_status, e.status) for e in received] == [
            (ComponentStatus.Created, ComponentStatus.Initializing),
            (ComponentStatus.Initializing, ComponentStatus.Initialized)]
        event = received[0]
        assert isinstance(event, IComponentStatusChangeEventArgs)
        assert event.component == component
        assert event.has_progress_monitor and event.percent_progress == 50.0
        component.disconnect(received.append)

    def test_connecting_twice_connects_once(self, component):
        received = []
        component.connect(received.append)
        component.connect(received.append)
        component.prepare()
        assert len(received) == 2
        component.disconnect(received.append)

    def test_disconnect_through_another_wrapper_of_the_same_object(
            self, native):
        received = []
        native.component.connect(received.append)
        native.component.disconnect(received.append)  # a different wrapper
        native.component.initialize()
        assert received == []

    def test_block_signals(self, component):
        received = []
        component.connect(received.append)
        component.block_signals(True)
        component.initialize()
        assert received == []
        component.block_signals(False)
        component.validate()
        assert len(received) == 2
        component.disconnect(received.append)

    def test_property_changes_reach_a_handle(self, component):
        names = []
        handle = component.on_property_changed(names.append)
        component.caption = "Renamed"
        assert names == ["Caption"] and component.caption == "Renamed"
        handle.disconnect()
        assert not handle.connected
        component.caption = "Again"
        assert names == ["Caption"]

    def test_value_slots_receive_event_args(self, component):
        output = component.outputs[0]
        received = []
        output.connect(received.append)
        ok, message = output.set_values_from(np.array([9.0, 8.0]), [1], [2])
        assert ok, message
        (event,) = received
        assert isinstance(event, IComponentDataItemValueChanged)
        assert (event.start, event.count) == ([1], [2])
        assert event.component_data_item == output
        values = np.zeros(4)
        assert output.get_values_into(values, [0], [4])[0]
        np.testing.assert_allclose(values, [1, 9, 8, 4])
        output.disconnect(received.append)

    def test_workflow_status_slots_receive_event_args(self, native):
        workflow = native.workflow
        received = []
        workflow.connect(received.append)
        workflow.initialize()
        assert isinstance(received[0], IWorkflowComponentStatusChangeEventArgs)
        assert received[-1].status == WorkflowStatus.Initialized
        assert received[0].workflow_component == workflow
        workflow.disconnect(received.append)

    def test_a_slot_must_be_callable(self, component):
        with pytest.raises(TypeError):
            component.on_status_changed(42)


# ---------------------------------------------------------------------------
# Exchange items, adapted outputs
# ---------------------------------------------------------------------------

class TestExchangeItems:
    def test_provider_and_consumers(self, native, component):
        inflows, discharge = component.inputs[0], component.outputs[0]
        assert inflows.set_provider(discharge)
        assert inflows.provider == discharge
        discharge.add_consumer(inflows)
        assert discharge.consumers == [inflows]
        assert discharge.remove_consumer(inflows)
        assert inflows.set_provider(None) and inflows.provider is None

    def test_a_refused_consumer_raises(self, native, component):
        refused = native.refused_output
        assert component.inputs[0].can_consume(refused) == (
            False, "refused by fixture")
        with pytest.raises(ValueError, match="refused by fixture"):
            refused.add_consumer(component.inputs[0])

    def test_multi_input(self, native, component):
        inflows, discharge = component.inputs[0], component.outputs[0]
        (upstream,) = inflows.provider_labels
        assert upstream.id == "upstream"
        assert inflows.is_required_provider(upstream)
        assert inflows.add_provider(discharge, upstream)
        assert inflows.add_provider(native.refused_output)
        assert [p.id for p in inflows.providers] == ["discharge", "refused"]
        assert inflows.remove_provider(discharge)
        assert [p.id for p in inflows.providers] == ["refused"]

    def test_update_values_reaches_the_output(self, native, component):
        component.outputs[0].update_values(component.inputs[0])
        assert native.discharge_updates == 1
        assert native.discharge_queried_by(component.inputs[0])
        component.outputs[0].update_values()
        assert native.discharge_updates == 2

    def test_update_passes_required_outputs(self, native, component):
        component.update([component.outputs[0]])
        assert native.last_required == 1
        with pytest.raises(TypeError):
            component.update(["not an output"])

    def test_adapted_output_round_trip(self, native, component):
        discharge = component.outputs[0]
        (factory,) = native.info.adapted_output_factories
        (scale,) = factory.get_available_adapted_output_ids(discharge)
        before = _testing.live_adapted_outputs()
        adapted = factory.create_adapted_output(scale, discharge)
        assert isinstance(adapted, IAdaptedOutput)
        assert _testing.live_adapted_outputs() == before + 1
        assert adapted.adaptee == discharge
        assert adapted.adapted_output_factory == factory
        adapted.initialize()
        adapted.refresh()

        discharge.add_adapted_output(adapted)
        (chained,) = discharge.adapted_outputs
        assert isinstance(chained, _core.CppAdaptedOutputWrapper)
        assert chained == adapted
        assert discharge.remove_adapted_output(adapted)

        del adapted, chained
        gc.collect()
        assert _testing.live_adapted_outputs() == before

    def test_a_stateful_adapter_through_the_bindings(self, native, component):
        # The fixture adapter blends y <- (y + x) / 2: it lists itself as its
        # state, differentiates through it, and checkpoints it.
        discharge = component.outputs[0]
        (factory,) = native.info.adapted_output_factories
        (scale,) = factory.get_available_adapted_output_ids(discharge)
        adapted = factory.create_adapted_output(scale, discharge)
        assert adapted.states == [adapted]
        assert adapted.differentiable_states() == [adapted]
        assert adapted.differentiable_arguments() == []

        ok, message = discharge.set_values_from(np.full(4, 9.0), (0,), (4,))
        assert ok, message
        adapted.refresh()                      # y = ([1,2,3,4] + 9) / 2
        ok, token, message = adapted.save_state()
        assert ok, message
        saved = np.empty(4)
        assert adapted.get_values_into(saved, (0,), (4,))[0]
        adapted.refresh()
        moved = np.empty(4)
        assert adapted.get_values_into(moved, (0,), (4,))[0]
        assert not np.array_equal(moved, saved)
        assert adapted.restore_state(token) == (True, "")
        back = np.empty(4)
        assert adapted.get_values_into(back, (0,), (4,))[0]
        assert np.array_equal(back, saved)
        assert adapted.release_state(token) == (True, "")

        # A restored adapter has no refresh to differentiate until it
        # refreshes again; then both partials are 1/2.
        ok, message = adapted.vjp([], [])
        assert not ok and "no refresh" in message
        adapted.refresh()
        out_bar, state_bar = np.full(4, 2.0), np.arange(4.0)
        x_bar, y_bar = np.empty(4), np.empty(4)
        ok, message = adapted.vjp(
            [(adapted, DifferentialRole.Output, out_bar),
             (adapted, DifferentialRole.StateAfter, state_bar)],
            [(discharge, DifferentialRole.Input, x_bar),
             (adapted, DifferentialRole.StateBefore, y_bar)])
        assert ok, message
        assert np.array_equal(x_bar, 0.5 * (out_bar + state_bar))
        assert np.array_equal(y_bar, x_bar)
        # A buffer the adapter cannot read is refused, not overrun.
        for bad in (np.ones(4, dtype=np.float32), np.ones(3)):
            ok, message = adapted.vjp(
                [(adapted, DifferentialRole.Output, bad)], [])
            assert not ok and "Float64 [4]" in message
        y_dot = np.empty(4)
        ok, message = adapted.jvp(
            [(discharge, DifferentialRole.Input, np.ones(4))],
            [(adapted, DifferentialRole.Output, y_dot)])
        assert ok, message
        assert np.array_equal(y_dot, np.full(4, 0.5))


# ---------------------------------------------------------------------------
# Workflow
# ---------------------------------------------------------------------------

class TestWorkflow:
    def test_lifecycle_and_roles(self, native, component):
        workflow = native.workflow
        assert workflow.validate() == ["no components"]
        assert workflow.status == WorkflowStatus.Failed
        assert workflow.errors()[0].message == "no components"

        (driver,) = workflow.model_component_labels
        assert workflow.is_required_model_component(driver)
        assert workflow.add_model_component(component, driver)
        assert native.workflow_role_was_driver
        assert workflow.model_components == [component]
        assert component.workflow == workflow

        workflow.initialize()
        assert workflow.validate() == []
        workflow.prepare()
        workflow.update()
        assert workflow.status == WorkflowStatus.Updated
        workflow.request_pause()
        workflow.resume()
        workflow.request_stop()
        assert (native.pause_requested, native.resumed,
                native.stop_requested) == (True, True, True)
        assert workflow.remove_model_component(component)

    def test_component_workflow_setter(self, native, component):
        component.workflow = native.workflow
        assert component.workflow == native.workflow
        component.workflow = None
        assert component.workflow is None
        with pytest.raises(TypeError):
            component.workflow = "a workflow"


# ---------------------------------------------------------------------------
# Temporal views
# ---------------------------------------------------------------------------

class TestTemporal:
    def test_time_model_component(self, component):
        clock = _temporal.as_time_model_component(component)
        assert isinstance(clock, ITimeModelComponent)
        assert clock == component
        assert clock.current_date_time.julian_day == pytest.approx(2461000.5)
        period = clock.simulation_period
        assert (period.duration, period.end_julian_day) == (10.0, 2461010.5)
        clock.update()
        assert clock.current_date_time.julian_day == pytest.approx(2461001.5)
        assert clock.next_date_time_julian_day == pytest.approx(2461002.5)

    def test_time_series_semantics(self):
        layered = _testing.LayeredFixture()
        series = _temporal.as_time_series(layered.time_mesh)
        assert series.time_kind == TimeKind.Instantaneous
        assert series.time_interpolation == TimeInterpolation.NoInterpolation
        assert series.time_extrapolation == TimeExtrapolation.Refuse
        assert series.interval_length == 0.0
        assert series.time_count == 2
        assert series == layered.time_mesh
        assert _temporal.as_time_series(layered.plain) is None


# ---------------------------------------------------------------------------
# Spatial views: geometries, ownership of what C++ makes, rasters
# ---------------------------------------------------------------------------

class TestGeometry:
    def test_dispatch_and_reference_system(self, native):
        shapes = _spatial.as_spatial(native.geometry_item)
        assert isinstance(shapes, _spatial.CppGeometryComponentDataItemWrapper)
        assert shapes == native.geometry_item
        square, point = shapes.geometry(0), shapes.geometry(1)
        assert isinstance(square, IPolygon)
        assert isinstance(point, IVertex) and point.vertex_index == 1
        srs = square.spatial_reference_system
        assert (srs.auth_name, srs.auth_srid) == ("EPSG", 26917)
        assert (srs.vertical_auth_name, srs.vertical_auth_srid) == ("EPSG", 5703)
        assert srs.vertical_distance_units == DistanceUnits.Feet
        assert srs.distance_units == DistanceUnits.Meters

    def test_parts_and_predicates(self, native):
        square = _spatial.as_spatial(native.geometry_item).geometry(0)
        assert square.geometry_type == GeometryType.Polygon
        assert square.area == 4.0
        ring = square.exterior_ring
        assert ring.point_count == 5 and ring.is_closed and ring.length == 8.0
        assert (ring.point(1).x, ring.point(1).y) == (2.0, 0.0)
        assert square.equals(square) and not square.equals(ring)
        assert square.relate(ring, "T*F**FFF*")
        assert square.distance(ring) == 1.5
        assert square.get_wkb() == b"\x01\x02\x03"
        assert square.get_wkt() == "FIXTURE(square)"
        assert square.locate_along(0.5) is None
        with pytest.raises(IndexError):
            square.interior_ring(0)

    def test_operations_return_owned_geometries(self, native):
        square = _spatial.as_spatial(native.geometry_item).geometry(0)
        before = _testing.live_geometries()
        buffered = square.buffer(1.5)
        assert isinstance(buffered, IPolygon) and buffered.area == 9.0
        assert _testing.live_geometries() > before
        centroid = square.centroid
        assert (centroid.x, centroid.y) == (1.0, 1.0)
        assert square.union(buffered).area == 9.0
        assert isinstance(square.intersection(buffered), IVertex)
        del buffered, centroid
        gc.collect()
        assert _testing.live_geometries() == before

    def test_a_part_keeps_its_geometry_alive(self, native):
        square = _spatial.as_spatial(native.geometry_item).geometry(0)
        hull = square.convex_hull()       # owned by its wrapper
        ring = hull.exterior_ring
        before = _testing.live_geometries()
        del hull
        gc.collect()
        assert _testing.live_geometries() == before  # the ring holds it
        assert ring.point_count == 5
        del ring
        gc.collect()
        assert _testing.live_geometries() < before


class TestRaster:
    def test_raster_read_write(self, native):
        item = _spatial.as_spatial(native.raster_item)
        assert isinstance(item, _spatial.CppRasterComponentDataItemWrapper)
        assert item.x_dimension.role == DimensionRole.Column
        raster = item.raster
        assert (raster.x_size, raster.y_size) == (3, 2)
        np.testing.assert_allclose(raster.geo_transformation(),
                                   [100, 10, 0, 200, 0, -10])
        assert raster.spatial_reference_system.auth_srid == 26917
        band = raster.get_raster_band(0)
        assert band.data_type == RasterDataType.Float64
        assert band.raster == raster

        window = np.zeros((2, 3))
        assert band.read(0, 0, 3, 2, window) == (True, "")
        np.testing.assert_allclose(window, [[0, 1, 2], [10, 11, 12]])
        assert band.write(1, 1, 2, 1, np.array([[7.0, 8.0]]))[0]
        assert native.band_value(1, 1) == 7.0 and native.band_value(1, 2) == 8.0
        assert band.read(0, 0, 3, 2, np.zeros((2, 3), np.float32))[0] is False

        raster.add_raster_band(RasterDataType.Int32)
        assert raster.raster_band_count == 2
        assert raster.get_raster_band(1).data_type == RasterDataType.Int32


class TestSpatioTemporal:
    def test_time_layered_mesh_view(self):
        layered = _testing.LayeredFixture()
        mesh = _spatiotemporal.as_spatiotemporal(layered.time_mesh)
        assert isinstance(mesh, ITimeLayeredMeshComponentDataItem)
        assert isinstance(mesh, _spatial.CppLayeredMeshComponentDataItemWrapper)
        assert mesh.time_kind == TimeKind.Instantaneous
        assert mesh.time_extrapolation == TimeExtrapolation.Refuse
        assert mesh.time_count == 2
        assert mesh.vertical_coordinate.layer_count == 4
        assert mesh.shape == (2, 3, 4)
        assert mesh == layered.time_mesh
        assert mesh == _spatial.as_layered(layered.time_mesh)
        assert len({mesh, layered.time_mesh}) == 1
        assert _spatiotemporal.as_spatiotemporal(layered.plain) is None
