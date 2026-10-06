"""
Framework-neutral driver for one differentiable component's step.

:mod:`hydrocouple.torch` and :mod:`hydrocouple.jax` are thin overlays on
:class:`StepDriver`: they own the tape (PyTorch's autograd graph, JAX's
``custom_vjp``); the driver owns the component. It knows three things the
frameworks do not:

1. **What a step is.** ``forward`` writes the state, the inputs and the
   arguments it is given into the component through the ordinary data
   plane, calls ``update()`` once, and reads the outputs and the new state
   back. The step is thereby a function of explicit arrays, which is what a
   framework can differentiate.

2. **Where the linearization point is.** The interface differentiates the
   *most recent* ``update()``. Reverse mode visits steps last-first, so
   every step but the last must be *replayed* before its ``vjp``: the
   driver restores the checkpoint it saved before that step
   (``ICheckpointableModelComponent``), writes the step's state, inputs and
   arguments again, and calls ``update()``. A component without
   ``Capability.Checkpointing`` can have only its most recent step
   differentiated, and the driver says so rather than guess.

3. **How buffers cross.** Arrays go in and come out through DLPack -- torch
   tensors on any device, or NumPy arrays -- never through a copy the
   binding makes.

After a backward pass the component sits at whichever step was replayed
last; restore a checkpoint (or ``prepare()`` again) before running forward.

Checkpoints are released, not leaked: the token saved before a step is
handed back through ``release_state()`` when the step's record is garbage
collected -- when the framework drops the graph (PyTorch) or the backward
pass has consumed it (JAX). A component whose tokens name files gets them
back as soon as nothing can replay that step any more.
"""

from __future__ import annotations

import itertools
import warnings
import weakref
from dataclasses import dataclass, field
from typing import Any, Callable, Optional, Sequence

from hydrocouple.core import Capability, DataKind, DifferentialRole

__all__ = ["StepDriver", "StepRecord", "DifferentiationError"]


class DifferentiationError(RuntimeError):
    """A component refused, or cannot provide, a derivative."""


@dataclass
class StepRecord:
    """Everything needed to take one step's derivative later."""

    clock: int                  # driver clock value right after the step
    token: Optional[str]        # checkpoint saved just before the step
    state: tuple                # primal values fed to the step
    inputs: tuple
    arguments: tuple
    extra: dict = field(default_factory=dict)


def _release(component, token: str) -> None:
    """Finalizer of a StepRecord: give the component its checkpoint back."""
    try:
        ok, msg = component.release_state(token)
    except Exception as exc:  # a finalizer must never raise
        ok, msg = False, str(exc)
    if not ok:
        warnings.warn(f"release_state refused a checkpoint token: {msg}",
                      RuntimeWarning, stacklevel=2)


class StepDriver:
    """Drives one C++ (or bridged) ``IDifferentiableModelComponent``.

    :param component: a component wrapper with ``Capability.Differentiable``.
    :param empty: ``empty(shape, data_kind) -> array`` allocating a writable
        buffer in the caller's framework (a torch tensor, a NumPy array).
    """

    _ids = itertools.count(1)

    def __init__(self, component, empty: Callable[[tuple, DataKind], Any]):
        caps = component.capabilities()
        if Capability.Differentiable not in caps:
            raise DifferentiationError(
                f"component '{component.id}' does not declare "
                "Capability.Differentiable")
        self.component = component
        self.checkpointable = Capability.Checkpointing in caps
        self.inputs = list(component.differentiable_inputs())
        self.arguments = list(component.differentiable_arguments())
        self.outputs = list(component.differentiable_outputs())
        self.states = list(component.differentiable_states())
        self._empty = empty
        self._clock = 0          # update() calls this driver has made
        self._linearized_at = 0  # clock value of the step vjp would describe

    # -- helpers -------------------------------------------------------------

    @staticmethod
    def _whole(item):
        shape = tuple(item.shape)
        return tuple(0 for _ in shape), shape

    def _write(self, items, values, what):
        if len(values) != len(items):
            raise ValueError(f"expected {len(items)} {what}, got {len(values)}")
        for item, value in zip(items, values):
            start, count = self._whole(item)
            ok, msg = item.set_values_from(value, start, count)
            if not ok:
                raise DifferentiationError(
                    f"{what} '{item.id}' refused its values: {msg}")

    def _read(self, items, what):
        out = []
        for item in items:
            start, count = self._whole(item)
            buffer = self._empty(count, item.data_kind)
            ok, msg = item.get_values_into(buffer, start, count)
            if not ok:
                raise DifferentiationError(
                    f"{what} '{item.id}' could not be read: {msg}")
            out.append(buffer)
        return tuple(out)

    def read_state(self) -> tuple:
        """The component's current state, one array per state item."""
        return self._read(self.states, "state")

    # -- the step ------------------------------------------------------------

    def forward(self, state: Sequence, inputs: Sequence,
                arguments: Sequence, record: bool = True):
        """Take one step. Returns ``(outputs, new_state, record)``."""
        token = None
        if record and self.checkpointable:
            ok, token, msg = self.component.save_state()
            if not ok:
                raise DifferentiationError(f"checkpoint failed: {msg}")
        self._write(self.states, state, "state")
        self._write(self.inputs, inputs, "input")
        self._write(self.arguments, arguments, "argument")
        self.component.update()
        self._clock += 1
        self._linearized_at = self._clock
        outputs = self._read(self.outputs, "output")
        new_state = self._read(self.states, "state")
        rec = (StepRecord(self._clock, token, tuple(state), tuple(inputs),
                          tuple(arguments)) if record else None)
        if rec is not None and token is not None:
            release = weakref.finalize(rec, _release, self.component, token)
            # Not at interpreter exit: the component's library may already be
            # unloaded by then, and the process is taking the storage anyway.
            release.atexit = False
        return outputs, new_state, rec

    def _linearize(self, rec: StepRecord) -> None:
        """Make ``rec``'s step the component's most recent update()."""
        if self._linearized_at == rec.clock:
            return
        if not self.checkpointable or rec.token is None:
            raise DifferentiationError(
                f"component '{self.component.id}' lacks "
                "Capability.Checkpointing, so only its most recent step can "
                "be differentiated; this step has been superseded")
        ok, msg = self.component.restore_state(rec.token)
        if not ok:
            raise DifferentiationError(f"restore failed: {msg}")
        self._write(self.states, rec.state, "state")
        self._write(self.inputs, rec.inputs, "input")
        self._write(self.arguments, rec.arguments, "argument")
        self.component.update()
        self._clock += 1
        # The replay reproduces rec's step exactly; mark it as rec's so a
        # second backward through the same record needs no replay.
        self._linearized_at = rec.clock = self._clock

    def vjp(self, rec: StepRecord, output_cotangents: Sequence,
            state_cotangents: Sequence, want_state=True, want_inputs=True,
            want_arguments=True):
        """Cotangents of (state, inputs, arguments) for one recorded step.

        ``None`` among the cotangents means zero. Returns three tuples; an
        entry the caller did not want is ``None``.
        """
        self._linearize(rec)
        R = DifferentialRole
        seeds = [(item, R.Output, c)
                 for item, c in zip(self.outputs, output_cotangents)
                 if c is not None]
        seeds += [(item, R.StateAfter, c)
                  for item, c in zip(self.states, state_cotangents)
                  if c is not None]
        wanted = []
        for flag, items, role in ((want_state, self.states, R.StateBefore),
                                  (want_inputs, self.inputs, R.Input),
                                  (want_arguments, self.arguments,
                                   R.Argument)):
            group = []
            for item in items:
                group.append((item, role, self._empty(tuple(item.shape),
                                                      item.data_kind))
                             if flag else None)
            wanted.append(group)
        results = [entry for group in wanted for entry in group
                   if entry is not None]
        ok, msg = self.component.vjp(seeds, results)
        if not ok:
            raise DifferentiationError(
                f"'{self.component.id}'.vjp refused: {msg}")
        return tuple(tuple(None if e is None else e[2] for e in group)
                     for group in wanted)

    def jvp(self, rec: StepRecord, state_tangents: Sequence,
            input_tangents: Sequence, argument_tangents: Sequence):
        """Tangents of (outputs, new_state) for one recorded step."""
        self._linearize(rec)
        R = DifferentialRole
        seeds = []
        for items, role, tangents in (
                (self.states, R.StateBefore, state_tangents),
                (self.inputs, R.Input, input_tangents),
                (self.arguments, R.Argument, argument_tangents)):
            seeds += [(item, role, t) for item, t in zip(items, tangents)
                      if t is not None]
        outs = [(item, R.Output, self._empty(tuple(item.shape),
                                             item.data_kind))
                for item in self.outputs]
        sts = [(item, R.StateAfter, self._empty(tuple(item.shape),
                                                item.data_kind))
               for item in self.states]
        ok, msg = self.component.jvp(seeds, outs + sts)
        if not ok:
            raise DifferentiationError(
                f"'{self.component.id}'.jvp refused: {msg}")
        return tuple(e[2] for e in outs), tuple(e[2] for e in sts)
