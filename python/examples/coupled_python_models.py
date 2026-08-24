"""
Example: two pure-Python components coupled through the standard, in Python.

No C++ is involved anywhere -- no compiled component, no SDK, no bridge.
Both models subclass the ABCs in :mod:`hydrocouple.core`, and they exchange
values through the standard's own interfaces.

The pairing is a small hydrologic one:

    RainfallRunoffComponent      ChannelRoutingComponent
      3 catchments                 3 reaches
      produces runoff depth        consumes lateral inflow
      in mm/hr                     in m3/s

Those units do not match, which is the point. The example runs the same
exchange twice:

  Part 1 -- explicit driver loop. The script advances both components and
  copies values across itself, converting units inline. This shows the data
  plane (get_values_into / set_values_from) and nothing more: the coupling
  lives in the script, so nothing else can reuse it.

  Part 2 -- provider/consumer wiring. The same conversion moves into an
  IAdaptedOutput between the two components, the input declares its
  provider, and the driver advances only the *downstream* model. Routing
  pulls what it needs through the chain, which pulls the upstream model
  forward in turn -- the standard's request-reply mechanism. The coupling
  now lives in the wiring, and the input can refuse a provider whose units
  it cannot consume.

Run it:

    python examples/coupled_python_models.py
"""

from __future__ import annotations

import numpy as np

from hydrocouple import helpers
from hydrocouple.core import (
    ComponentStatus,
    DataKind,
    ErrorEntry,
    IAdaptedOutput,
    IInput,
    IModelComponent,
    IOutput,
    IQuantity,
    IUnit,
)

# Catchment areas in m2, one per catchment/reach. The unit conversion below
# is area-weighted, so this is genuine adapter configuration rather than a
# single hard-coded factor.
CATCHMENT_AREAS_M2 = np.array([2.5e6, 4.0e6, 1.5e6], dtype=np.float64)

MM_PER_HR_TO_M_PER_S = 1.0 / (1000.0 * 3600.0)


# ---------------------------------------------------------------------------
# Boilerplate shared by every interface object
# ---------------------------------------------------------------------------
#
# The standard gives every object identity and signals. Neither is
# interesting here, so both are absorbed once instead of being restated on
# each class -- this is example scaffolding, not part of the standard.


class _Described:
    """Identity (``IIdentity``) plus no-op signal methods."""

    def __init__(self, id_: str, caption: str, description: str = ""):
        self._id = id_
        self._caption = caption
        self._description = description

    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    @property
    def id(self):
        return self._id

    @property
    def caption(self):
        return self._caption

    @caption.setter
    def caption(self, value):
        self._caption = value

    @property
    def description(self):
        return self._description

    @description.setter
    def description(self, value):
        self._description = value


# ---------------------------------------------------------------------------
# Value definitions -- what makes the unit mismatch visible
# ---------------------------------------------------------------------------


class Unit(_Described, IUnit):
    """A unit expressed by its conversion to SI."""

    def __init__(self, id_: str, caption: str, factor_to_si: float,
                 offset_to_si: float = 0.0):
        _Described.__init__(self, id_, caption)
        self._factor = factor_to_si
        self._offset = offset_to_si

    @property
    def dimensions(self):
        # A full implementation returns IUnitDimensions (the L/M/T powers).
        # The example compares units by identity, so it stays None.
        return None

    @property
    def conversion_factor_to_si(self):
        return self._factor

    @property
    def offset_to_si(self):
        return self._offset


class Quantity(_Described, IQuantity):
    """A float64 quantity carrying a unit."""

    def __init__(self, id_: str, caption: str, unit: Unit):
        _Described.__init__(self, id_, caption)
        self._unit = unit

    @property
    def unit(self):
        return self._unit

    @property
    def type(self):
        return DataKind.Float64

    @property
    def missing_value(self):
        return float("nan")

    @property
    def default_value(self):
        return 0.0

    @property
    def min_value(self):
        return 0.0

    @property
    def max_value(self):
        return float("inf")


RUNOFF_DEPTH = Quantity(
    "runoff-depth", "Runoff depth",
    Unit("mm/hr", "millimetre per hour", MM_PER_HR_TO_M_PER_S))

VOLUMETRIC_FLOW = Quantity(
    "volumetric-flow", "Volumetric flow rate",
    Unit("m3/s", "cubic metre per second", 1.0))


# ---------------------------------------------------------------------------
# A one-dimensional float64 field, shared by every exchange item here
# ---------------------------------------------------------------------------


class Field1D(_Described):
    """Storage plus the hyperslab data plane for a rank-1 float64 item.

    Canonical ordering: the entity dimension is dimension 0.
    """

    def __init__(self, id_: str, caption: str, owner, size: int,
                 quantity: Quantity):
        _Described.__init__(self, id_, caption)
        self._owner = owner
        self._quantity = quantity
        self._values = np.zeros(size, dtype=np.float64)

    @property
    def model_component(self):
        return self._owner

    @property
    def dimensions(self):
        return []

    @property
    def shape(self):
        return self._values.shape

    @property
    def data_kind(self):
        return DataKind.Float64

    @property
    def value_definition(self):
        return self._quantity

    def get_values_into(self, destination, start, count):
        if helpers.data_kind_of(destination.dtype) != self.data_kind:
            return False, f"{self.id}: expected float64"
        if start[0] < 0 or start[0] + count[0] > self._values.size:
            return False, f"{self.id}: selection out of bounds"
        destination.reshape(count[0])[...] = (
            self._values[start[0]:start[0] + count[0]])
        return True, ""

    def set_values_from(self, source, start, count):
        if helpers.data_kind_of(source.dtype) != self.data_kind:
            return False, f"{self.id}: expected float64"
        if start[0] < 0 or start[0] + count[0] > self._values.size:
            return False, f"{self.id}: selection out of bounds"
        self._values[start[0]:start[0] + count[0]] = source.reshape(count[0])
        return True, ""


# ---------------------------------------------------------------------------
# Exchange items
# ---------------------------------------------------------------------------


class RunoffOutput(Field1D, IOutput):
    """Runoff depth per catchment, in mm/hr."""

    def __init__(self, owner, size: int):
        Field1D.__init__(self, "runoff", "Runoff depth per catchment",
                         owner, size, RUNOFF_DEPTH)
        self._consumers: list[IInput] = []
        self._adapted: list[IAdaptedOutput] = []

    @property
    def consumers(self):
        return list(self._consumers)

    def add_consumer(self, consumer):
        self._consumers.append(consumer)

    def remove_consumer(self, consumer):
        if consumer in self._consumers:
            self._consumers.remove(consumer)
            return True
        return False

    @property
    def adapted_outputs(self):
        return list(self._adapted)

    def add_adapted_output(self, adapted_output):
        self._adapted.append(adapted_output)

    def remove_adapted_output(self, adapted_output):
        if adapted_output in self._adapted:
            self._adapted.remove(adapted_output)
            return True
        return False

    def update_values(self, query_specifier):
        """Produce values the querier can read.

        This is the request-reply hinge: a consumer asking for values is
        what advances this component. The driver in part 2 never calls
        ``update()`` on the rainfall-runoff model at all.

        A component that has reached ``Done`` is not advanced again -- the
        lifecycle has no Done -> Updating edge, and asserting that here
        would be a poor way for a reader to discover it. It keeps serving
        the values it already holds, which is what a provider that has run
        out of horizon should do.
        """
        if self._owner.status == ComponentStatus.Updated:
            self._owner.update()


class LateralInflowInput(Field1D, IInput):
    """Lateral inflow per reach, in m3/s."""

    def __init__(self, owner, size: int):
        Field1D.__init__(self, "lateral-inflow", "Lateral inflow per reach",
                         owner, size, VOLUMETRIC_FLOW)
        self._provider: IOutput | None = None

    @property
    def provider(self):
        return self._provider

    def set_provider(self, provider):
        if provider is not None:
            ok, message = self.can_consume(provider)
            if not ok:
                raise ValueError(message)
        self._provider = provider
        return True

    def can_consume(self, provider):
        """Whether this input can take values from ``provider`` as they are.

        Shape, kind and unit are checked. The unit check is what makes the
        raw mm/hr output unusable here and the adapter necessary -- without
        it the mismatch would surface as plausible-looking wrong numbers.
        """
        if provider.data_kind != self.data_kind:
            return False, (f"{provider.id} is {provider.data_kind.name}, "
                           f"{self.id} needs {self.data_kind.name}")
        if tuple(provider.shape) != tuple(self.shape):
            return False, (f"{provider.id} has shape {tuple(provider.shape)}, "
                           f"{self.id} needs {tuple(self.shape)}")
        theirs = getattr(provider.value_definition, "unit", None)
        mine = self.value_definition.unit
        if theirs is None or theirs.id != mine.id:
            name = "none" if theirs is None else theirs.id
            return False, (f"{provider.id} delivers {name}, "
                           f"{self.id} needs {mine.id}")
        return True, ""


class AreaWeightedFlowAdapter(Field1D, IAdaptedOutput):
    """Converts runoff depth (mm/hr) to volumetric flow (m3/s).

        Q [m3/s] = depth [mm/hr] * area [m2] / (1000 * 3600)

    An adapted output *is* an output, so it can be a provider like any
    other, and the components on either side stay unaware of it.
    """

    def __init__(self, adaptee: IOutput, areas_m2: np.ndarray):
        Field1D.__init__(self, "runoff-to-flow", "Runoff depth to flow rate",
                         adaptee.model_component, areas_m2.size,
                         VOLUMETRIC_FLOW)
        self._adaptee = adaptee
        self._areas = areas_m2
        self._consumers: list[IInput] = []
        self._adapted: list[IAdaptedOutput] = []
        adaptee.add_adapted_output(self)

    # -- IAdaptedOutput ----------------------------------------------------
    @property
    def adaptee(self):
        return self._adaptee

    @property
    def adapted_output_factory(self):
        # Adapters are normally produced by an IAdaptedOutputFactory so a
        # host can discover them by id. This one is constructed directly.
        return None

    @property
    def arguments(self):
        # The areas are constructor state here; a discoverable adapter would
        # expose them as IArgument so a composition file could set them.
        return []

    def initialize(self):
        pass

    def refresh(self):
        """Re-read the adaptee and convert. Cascades to child adapters."""
        depth = helpers.get_values_or_raise(
            self._adaptee, (0,), self._adaptee.shape)
        self._values[...] = depth * self._areas * MM_PER_HR_TO_M_PER_S
        for child in self._adapted:
            child.refresh()

    # -- IOutput -----------------------------------------------------------
    def update_values(self, query_specifier):
        self._adaptee.update_values(query_specifier)
        self.refresh()

    @property
    def consumers(self):
        return list(self._consumers)

    def add_consumer(self, consumer):
        self._consumers.append(consumer)

    def remove_consumer(self, consumer):
        if consumer in self._consumers:
            self._consumers.remove(consumer)
            return True
        return False

    @property
    def adapted_outputs(self):
        return list(self._adapted)

    def add_adapted_output(self, adapted_output):
        self._adapted.append(adapted_output)

    def remove_adapted_output(self, adapted_output):
        if adapted_output in self._adapted:
            self._adapted.remove(adapted_output)
            return True
        return False


# ---------------------------------------------------------------------------
# Components
# ---------------------------------------------------------------------------


class _Component(_Described, IModelComponent):
    """Lifecycle bookkeeping shared by both models."""

    def __init__(self, id_: str, caption: str, steps: int):
        _Described.__init__(self, id_, caption)
        self._status = ComponentStatus.Created
        self._steps = steps
        self._step = 0
        self._error_queue: list[ErrorEntry] = []
        self._ref_dir = "."

    def _to(self, status: ComponentStatus):
        assert helpers.is_valid_component_status_transition(
            self._status, status), (
                f"{self.id}: illegal transition {self._status.name} -> "
                f"{status.name}")
        self._status = status

    @property
    def status(self):
        return self._status

    @property
    def component_info(self):
        return None

    @property
    def arguments(self):
        return []

    @property
    def results(self):
        return []

    @property
    def workflow(self):
        return None

    @workflow.setter
    def workflow(self, value):
        pass

    def capabilities(self):
        return set()

    def errors(self, clear_after_read=False):
        entries = list(self._error_queue)
        if clear_after_read:
            self._error_queue.clear()
        return entries

    @property
    def reference_directory(self):
        return self._ref_dir

    @reference_directory.setter
    def reference_directory(self, value):
        self._ref_dir = value

    def initialize(self):
        self._to(ComponentStatus.Initializing)
        self._to(ComponentStatus.Initialized)

    def validate(self):
        self._to(ComponentStatus.Validating)
        self._to(ComponentStatus.Valid)
        return []

    def prepare(self):
        self._to(ComponentStatus.Preparing)
        self._to(ComponentStatus.Updated)

    def finish(self):
        self._to(ComponentStatus.Finishing)
        self._to(ComponentStatus.Finished)


class RainfallRunoffComponent(_Component):
    """Turns a rainfall hyetograph into runoff depth per catchment."""

    #: mm/hr of rainfall, one entry per step.
    HYETOGRAPH = (0.0, 4.0, 12.0, 18.0, 9.0, 3.0, 0.0, 0.0)

    #: Fraction of rainfall that becomes runoff, per catchment.
    RUNOFF_COEFFICIENTS = np.array([0.35, 0.55, 0.20], dtype=np.float64)

    def __init__(self, steps: int = len(HYETOGRAPH)):
        super().__init__("rainfall-runoff", "Rainfall-runoff model", steps)
        self._runoff = RunoffOutput(self, CATCHMENT_AREAS_M2.size)

    @property
    def inputs(self):
        return []

    @property
    def outputs(self):
        return [self._runoff]

    def update(self, required_outputs=None):
        self._to(ComponentStatus.Updating)
        rain = self.HYETOGRAPH[min(self._step, len(self.HYETOGRAPH) - 1)]
        self._runoff._values[...] = rain * self.RUNOFF_COEFFICIENTS
        self._step += 1
        self._to(ComponentStatus.Done if self._step >= self._steps
                 else ComponentStatus.Updated)


class ChannelRoutingComponent(_Component):
    """Routes lateral inflow through linear reservoirs, one per reach.

        S' = S + (Q_in - Q_out) * dt,    Q_out = S / k
    """

    def __init__(self, steps: int, k_seconds: float = 7200.0,
                 dt_seconds: float = 3600.0):
        super().__init__("channel-routing", "Channel routing model", steps)
        self._inflow = LateralInflowInput(self, CATCHMENT_AREAS_M2.size)
        self._storage = np.zeros(CATCHMENT_AREAS_M2.size, dtype=np.float64)
        self._discharge = np.zeros(CATCHMENT_AREAS_M2.size, dtype=np.float64)
        self._k = k_seconds
        self._dt = dt_seconds

    @property
    def inputs(self):
        return [self._inflow]

    @property
    def outputs(self):
        return []

    @property
    def discharge(self):
        """Outflow per reach, m3/s (read by the driver for reporting)."""
        return self._discharge.copy()

    def _pull(self):
        """Fetch values from the provider -- the consumer side of
        request-reply. Only used in part 2; with no provider set, the
        driver is expected to have pushed values in already."""
        provider = self._inflow.provider
        if provider is None:
            return
        self._to(ComponentStatus.WaitingForData)
        provider.update_values(self._inflow)
        values = helpers.get_values_or_raise(provider, (0,), provider.shape)
        helpers.set_values_or_raise(self._inflow, values, (0,), provider.shape)
        self._to(ComponentStatus.Updating)

    def update(self, required_outputs=None):
        self._to(ComponentStatus.Updating)
        self._pull()
        inflow = helpers.get_values_or_raise(
            self._inflow, (0,), self._inflow.shape)
        self._discharge = self._storage / self._k
        self._storage = np.maximum(
            self._storage + (inflow - self._discharge) * self._dt, 0.0)
        self._step += 1
        self._to(ComponentStatus.Done if self._step >= self._steps
                 else ComponentStatus.Updated)


# ---------------------------------------------------------------------------
# Part 1 -- the driver does the coupling itself
# ---------------------------------------------------------------------------


def run_explicit() -> np.ndarray:
    runoff_model = RainfallRunoffComponent()
    routing_model = ChannelRoutingComponent(
        steps=len(RainfallRunoffComponent.HYETOGRAPH))

    for component in (runoff_model, routing_model):
        component.initialize()
        component.validate()
        component.prepare()

    output = runoff_model.outputs[0]
    input_item = routing_model.inputs[0]
    history = []

    while routing_model.status != ComponentStatus.Done:
        runoff_model.update()

        # Read the producer, convert, write the consumer. Correct, but the
        # conversion is stranded in this loop: nothing else can reuse it,
        # and no interface records that a conversion happened at all.
        depth = helpers.get_values_or_raise(output, (0,), output.shape)
        flow = depth * CATCHMENT_AREAS_M2 * MM_PER_HR_TO_M_PER_S
        helpers.set_values_or_raise(input_item, flow, (0,), input_item.shape)

        routing_model.update()
        history.append(routing_model.discharge)

    for component in (runoff_model, routing_model):
        component.finish()

    return np.array(history)


# ---------------------------------------------------------------------------
# Part 2 -- the wiring does the coupling
# ---------------------------------------------------------------------------


def run_pull_driven() -> np.ndarray:
    runoff_model = RainfallRunoffComponent()
    routing_model = ChannelRoutingComponent(
        steps=len(RainfallRunoffComponent.HYETOGRAPH))

    output = runoff_model.outputs[0]
    input_item = routing_model.inputs[0]

    # The raw output is refused: right shape, right kind, wrong unit.
    ok, message = input_item.can_consume(output)
    assert not ok, "expected the unit mismatch to be caught"
    print(f"  link refused: {message}")

    # The adapter reconciles the units, and is itself an IOutput.
    adapter = AreaWeightedFlowAdapter(output, CATCHMENT_AREAS_M2)
    adapter.initialize()
    ok, message = input_item.can_consume(adapter)
    assert ok, message
    print(f"  link accepted via {adapter.id}: "
          f"{output.value_definition.unit.id} -> "
          f"{adapter.value_definition.unit.id}")

    input_item.set_provider(adapter)
    adapter.add_consumer(input_item)

    for component in (runoff_model, routing_model):
        component.initialize()
        component.validate()
        component.prepare()

    history = []
    # Only the downstream model is driven. Routing pulls through the
    # adapter, which pulls the rainfall-runoff model forward.
    while routing_model.status != ComponentStatus.Done:
        routing_model.update()
        history.append(routing_model.discharge)

    for component in (runoff_model, routing_model):
        component.finish()

    return np.array(history)


def main():
    np.set_printoptions(precision=3, suppress=True)

    print("Part 1 - explicit driver loop")
    explicit = run_explicit()
    print(f"  discharge, final step (m3/s): {explicit[-1]}")

    print("\nPart 2 - provider/consumer wiring")
    pulled = run_pull_driven()
    print(f"  discharge, final step (m3/s): {pulled[-1]}")

    assert np.allclose(explicit, pulled), (
        "the two couplings should produce identical results")
    print("\nBoth couplings agree over every step and reach.")


if __name__ == "__main__":
    main()
