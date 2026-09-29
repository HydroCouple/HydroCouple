"""
PyTorch overlay: a differentiable HydroCouple component as a
``torch.autograd.Function``.

.. code-block:: python

    import hydrocouple.torch as hct

    reservoir = hct.Step(component)          # C++, Fortran, or Python
    state = reservoir.initial_state()
    for x in forcing:
        inflow = net(x)                          # a torch network
        (outflow,), state = reservoir(state, [inflow], [k])
        loss = loss + ((outflow - observed) ** 2).sum()
    loss.backward()     # reaches net.parameters(), k and the initial state

``forward`` runs the component's ``update()`` on the tensors' own memory
(DLPack, zero copy, any device); ``backward`` calls the component's
``vjp()`` with the incoming gradients as cotangent buffers. PyTorch's
autograd graph *is* the tape: gradients flow from a torch model into a
C++ model and out the other side, through time (the state), with nothing
HydroCouple-specific in the training loop.

This is the Python surface of Alt 6 / Alt 8b of
``plans/DIFFERENTIABLE_INTERFACE_ALTERNATIVES_2026-09-28.md``. The
interface itself knows nothing of PyTorch.
"""

from __future__ import annotations

from typing import Optional, Sequence

import torch

from hydrocouple.core import DataKind
from hydrocouple.differentiable import StepDriver

__all__ = ["Step"]

_TORCH_DTYPE = {
    DataKind.Float32: torch.float32,
    DataKind.Float64: torch.float64,
}


class _StepFunction(torch.autograd.Function):
    """One component step. Tensors: state..., inputs..., arguments..."""

    @staticmethod
    def forward(ctx, step: "Step", n_state: int, n_inputs: int, *tensors):
        driver = step.driver
        state = tensors[:n_state]
        inputs = tensors[n_state:n_state + n_inputs]
        arguments = tensors[n_state + n_inputs:]
        # Private copies: a replay in backward must see the values this
        # step saw, even if the caller later mutates its tensors in place.
        prim = [t.detach().clone(memory_format=torch.contiguous_format)
                for t in tensors]
        with torch.no_grad():
            outputs, new_state, record = driver.forward(
                prim[:n_state], prim[n_state:n_state + n_inputs],
                prim[n_state + n_inputs:])
        ctx.step = step
        ctx.record = record
        ctx.n_state = n_state
        ctx.n_inputs = n_inputs
        ctx.n_outputs = len(outputs)
        del state, inputs, arguments
        return tuple(outputs) + tuple(new_state)

    @staticmethod
    def backward(ctx, *grads):
        driver = ctx.step.driver
        n_out = ctx.n_outputs
        out_cot = [None if g is None else g.contiguous() for g in grads[:n_out]]
        state_cot = [None if g is None else g.contiguous()
                     for g in grads[n_out:]]
        needs = ctx.needs_input_grad[3:]
        n_s, n_i = ctx.n_state, ctx.n_inputs
        with torch.no_grad():
            s_bar, i_bar, a_bar = driver.vjp(
                ctx.record, out_cot, state_cot,
                want_state=any(needs[:n_s]),
                want_inputs=any(needs[n_s:n_s + n_i]),
                want_arguments=any(needs[n_s + n_i:]))
        grads_in = []
        for group, flags in ((s_bar, needs[:n_s]),
                             (i_bar, needs[n_s:n_s + n_i]),
                             (a_bar, needs[n_s + n_i:])):
            for g, wanted in zip(group, flags):
                grads_in.append(g if wanted else None)
        return (None, None, None, *grads_in)


class Step:
    """A differentiable component as a callable torch operation.

    :param component: a component wrapper declaring
        ``Capability.Differentiable`` (e.g. from :func:`hydrocouple.loader.load`).
    :param device: where the tensors this overlay allocates live (outputs,
        new state, gradients). Defaults to CPU; the component must be able to
        service that memory space.
    """

    def __init__(self, component, device: Optional[torch.device] = None):
        self.device = torch.device("cpu") if device is None else torch.device(device)
        self.driver = StepDriver(component, self._empty)

    def _empty(self, shape, kind):
        dtype = _TORCH_DTYPE.get(DataKind(kind))
        if dtype is None:
            raise TypeError(f"{DataKind(kind).name} items are not differentiable")
        return torch.empty(tuple(shape), dtype=dtype, device=self.device)

    # -- names, in the order the call uses them ------------------------------

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

    def initial_state(self, requires_grad: bool = False) -> tuple:
        """The component's current state as tensors (the start of a run)."""
        return tuple(t.requires_grad_(requires_grad)
                     for t in self.driver.read_state())

    def __call__(self, state: Sequence[torch.Tensor],
                 inputs: Sequence[torch.Tensor],
                 arguments: Sequence[torch.Tensor]):
        """One step: ``(outputs, new_state)``, both tuples of tensors."""
        state, inputs, arguments = tuple(state), tuple(inputs), tuple(arguments)
        flat = _StepFunction.apply(self, len(state), len(inputs),
                                   *state, *inputs, *arguments)
        n_out = len(self.driver.outputs)
        return tuple(flat[:n_out]), tuple(flat[n_out:])
