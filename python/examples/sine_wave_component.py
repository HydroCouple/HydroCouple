"""
Example: a pure-Python HydroCouple component with a typed data plane.

A minimal time-marching component producing a sine-wave signal on a
10-cell domain, exposing its state through the v2 hyperslab API. Run it
directly to see the lifecycle, error queue, and data plane in action:

    python examples/sine_wave_component.py
"""

from __future__ import annotations

import math

import numpy as np

from hydrocouple.core import (
    Capability,
    ComponentStatus,
    DataKind,
    ErrorEntry,
    IComponentDataItem,
    IModelComponent,
)
from hydrocouple import helpers


class SineField(IComponentDataItem):
    """State of the sine-wave model: one float64 value per cell.

    Canonical ordering: the cell dimension is dimension 0 of ``shape``.
    """

    def __init__(self, owner: "SineWaveComponent", cells: int):
        self._owner = owner
        self._values = np.zeros(cells, dtype=np.float64)

    # -- signals (no listeners needed for this example) --------------------
    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    # -- identity ----------------------------------------------------------
    @property
    def id(self):
        return "sine-field"

    @property
    def caption(self):
        return "Sine wave field"

    @caption.setter
    def caption(self, value):
        pass

    @property
    def description(self):
        return "sin(2*pi*(t + x/n)) per cell"

    @description.setter
    def description(self, value):
        pass

    # -- data plane --------------------------------------------------------
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
        return None

    def get_values_into(self, destination, start, count):
        if helpers.data_kind_of(destination.dtype) != self.data_kind:
            return False, "kind mismatch"
        if start[0] < 0 or start[0] + count[0] > self._values.size:
            return False, "selection out of bounds"
        destination.reshape(count[0])[...] = (
            self._values[start[0]:start[0] + count[0]])
        return True, ""

    def set_values_from(self, source, start, count):
        if helpers.data_kind_of(source.dtype) != self.data_kind:
            return False, "kind mismatch"
        self._values[start[0]:start[0] + count[0]] = source.reshape(count[0])
        return True, ""


class SineWaveComponent(IModelComponent):
    """A minimal time-marching component."""

    def __init__(self, cells: int = 10, steps: int = 8):
        self._status = ComponentStatus.Created
        self._steps = steps
        self._step = 0
        self._field = SineField(self, cells)
        self._error_queue: list[ErrorEntry] = []
        self._ref_dir = "."

    # -- signals -----------------------------------------------------------
    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    # -- identity ----------------------------------------------------------
    @property
    def id(self):
        return "sine-wave"

    @property
    def caption(self):
        return "Sine wave"

    @caption.setter
    def caption(self, value):
        pass

    @property
    def description(self):
        return "Example time-marching component"

    @description.setter
    def description(self, value):
        pass

    # -- lifecycle ---------------------------------------------------------
    def _transition(self, to: ComponentStatus):
        assert helpers.is_valid_component_status_transition(
            self._status, to), f"illegal transition {self._status} -> {to}"
        self._status = to

    @property
    def component_info(self):
        return None

    @property
    def status(self):
        return self._status

    @property
    def arguments(self):
        return []

    @property
    def inputs(self):
        return []

    @property
    def outputs(self):
        return []

    @property
    def results(self):
        return [self._field]

    def initialize(self):
        self._transition(ComponentStatus.Initializing)
        self._transition(ComponentStatus.Initialized)

    def validate(self):
        self._transition(ComponentStatus.Validating)
        self._transition(ComponentStatus.Valid)
        return []

    def prepare(self):
        self._transition(ComponentStatus.Preparing)
        self._transition(ComponentStatus.Updated)

    def update(self, required_outputs=None):
        self._transition(ComponentStatus.Updating)
        t = self._step / self._steps
        n = self._field._values.size
        for i in range(n):
            self._field._values[i] = math.sin(2.0 * math.pi * (t + i / n))
        self._step += 1
        self._transition(ComponentStatus.Done if self._step >= self._steps
                         else ComponentStatus.Updated)

    def finish(self):
        self._transition(ComponentStatus.Finishing)
        self._transition(ComponentStatus.Finished)

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


def main():
    comp = SineWaveComponent(cells=10, steps=4)
    comp.initialize()
    comp.validate()
    comp.prepare()

    field = comp.results[0]
    while comp.status != ComponentStatus.Done:
        comp.update()
        values = helpers.get_values_or_raise(field, (0,), field.shape)
        print(f"step values: {np.array2string(values, precision=2)}")

    comp.finish()
    print(f"final status: {comp.status.name}")


if __name__ == "__main__":
    main()
