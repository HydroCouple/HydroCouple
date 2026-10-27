"""
JAX overlay: a differentiable HydroCouple component as a ``jax.custom_vjp``
function.

.. code-block:: python

    import hydrocouple.jax as hcj

    reservoir = hcj.Step(component)
    def loss(params, k):
        state = reservoir.initial_state()
        total = 0.0
        for x, obs in zip(forcing, observed):
            (outflow,), state = reservoir(state, [mlp(params, x)], [k])
            total = total + jnp.sum((outflow - obs) ** 2)
        return total
    grads = jax.grad(loss, argnums=(0, 1))(params, k)

JAX has no C++ implementation to adopt: a component is opaque to its
tracer, so each step enters JAX as a host callback (``jax.pure_callback``)
and its backward rule is the component's ``vjp()``. The contract has
exactly the shape of ``custom_vjp``'s ``bwd``, which is why nothing in the
interface prefers PyTorch.

Scope (Phase G3-lite): eager ``jax.grad`` / ``jax.vjp`` / ``jax.value_and_grad``
on CPU. The component is stateful, so under ``jax.jit`` the order of its
steps rests on the data dependence through the state (which does order a
time loop) -- see ``test_jit_preserves_step_order`` -- and device-resident
JAX arrays are staged through the host by the callback. An ordered-effect
FFI path is the G3 follow-up.
"""

from __future__ import annotations

import itertools
import threading
from typing import Sequence

import jax
import jax.numpy as jnp
import numpy as np

from hydrocouple.core import DataKind
from hydrocouple.differentiable import StepDriver
from hydrocouple.helpers import DATA_KIND_TO_DTYPE

__all__ = ["Step"]


def _numpy_empty(shape, kind):
    kind = DataKind(kind)
    if kind not in (DataKind.Float32, DataKind.Float64):
        raise TypeError(f"{kind.name} items are not differentiable")
    return np.empty(tuple(shape), dtype=DATA_KIND_TO_DTYPE[kind])


class Step:
    """A differentiable component as a JAX function with a custom VJP.

    Calling it returns ``(outputs, new_state)`` as tuples of JAX arrays.
    """

    def __init__(self, component):
        self.driver = StepDriver(component, _numpy_empty)
        self._records = {}
        self._ids = itertools.count(1)
        self._lock = threading.Lock()
        d = self.driver
        self._out_specs = tuple(self._spec(i) for i in d.outputs)
        self._state_specs = tuple(self._spec(i) for i in d.states)
        self._fn = self._build()

    @staticmethod
    def _spec(item):
        return jax.ShapeDtypeStruct(tuple(item.shape),
                                    DATA_KIND_TO_DTYPE[DataKind(item.data_kind)])

    @property
    def state_items(self):
        return self.driver.states

    @property
    def input_items(self):
        return self.driver.inputs

    @property
    def argument_items(self):
        return self.driver.arguments

    @property
    def output_items(self):
        return self.driver.outputs

    def initial_state(self) -> tuple:
        """The component's current state as JAX arrays."""
        return tuple(jnp.asarray(a) for a in self.driver.read_state())

    # -- host callbacks ----------------------------------------------------

    def _split(self, flat):
        d = self.driver
        n_s, n_i = len(d.states), len(d.inputs)
        arrays = [np.ascontiguousarray(a) for a in flat]
        return arrays[:n_s], arrays[n_s:n_s + n_i], arrays[n_s + n_i:]

    def _host_primal(self, *flat):
        state, inputs, arguments = self._split(flat)
        outputs, new_state, _ = self.driver.forward(state, inputs, arguments,
                                                    record=False)
        return tuple(outputs) + tuple(new_state)

    def _host_forward(self, *flat):
        state, inputs, arguments = self._split(flat)
        outputs, new_state, record = self.driver.forward(
            [a.copy() for a in state], [a.copy() for a in inputs],
            [a.copy() for a in arguments])
        with self._lock:
            rid = next(self._ids)
            self._records[rid] = record
        return tuple(outputs) + tuple(new_state) + (np.int32(rid),)

    def _host_backward(self, rid, *cotangents):
        with self._lock:
            record = self._records.pop(int(rid))
        n_out = len(self.driver.outputs)
        out_cot = [np.ascontiguousarray(c) for c in cotangents[:n_out]]
        state_cot = [np.ascontiguousarray(c) for c in cotangents[n_out:]]
        s_bar, i_bar, a_bar = self.driver.vjp(record, out_cot, state_cot)
        return tuple(s_bar) + tuple(i_bar) + tuple(a_bar)

    # -- the custom_vjp function ---------------------------------------------

    def _build(self):
        d = self.driver
        out_specs = self._out_specs + self._state_specs
        in_specs = tuple(self._spec(i)
                         for i in (*d.states, *d.inputs, *d.arguments))
        rid_spec = jax.ShapeDtypeStruct((), np.int32)

        @jax.custom_vjp
        def step(*flat):
            return jax.pure_callback(self._host_primal, out_specs, *flat)

        def step_fwd(*flat):
            res = jax.pure_callback(self._host_forward,
                                    out_specs + (rid_spec,), *flat)
            primal, rid = tuple(res[:-1]), res[-1]
            return primal, rid

        def step_bwd(rid, cotangents):
            return tuple(jax.pure_callback(self._host_backward, in_specs, rid,
                                           *cotangents))

        step.defvjp(step_fwd, step_bwd)
        return step

    def __call__(self, state: Sequence, inputs: Sequence,
                 arguments: Sequence):
        """One step: ``(outputs, new_state)``."""
        flat = self._fn(*state, *inputs, *arguments)
        n_out = len(self.driver.outputs)
        return tuple(flat[:n_out]), tuple(flat[n_out:])
