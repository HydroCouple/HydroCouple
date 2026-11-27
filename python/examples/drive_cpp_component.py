"""
Example: drive a compiled C++ HydroCouple component from Python.

Loads a component shared library, runs its lifecycle, and pulls output
fields as NumPy arrays through the zero-copy hyperslab data plane.

    python examples/drive_cpp_component.py ./libMyModel.so
"""

from __future__ import annotations

import sys

import numpy as np

from hydrocouple.core import ComponentStatus, ErrorEntry
from hydrocouple.loader import load


def main(library_path: str) -> int:
    component, info, handle = load(library_path)
    print(f"loaded '{info.caption}' v{info.version} by {info.developer}")

    component.initialize()
    messages = component.validate()
    if messages and component.status == ComponentStatus.Invalid:
        for message in messages:
            print(f"  invalid: {message}")
        return 1
    component.prepare()

    outputs = component.outputs
    buffers = {
        out.id: np.empty(out.shape, dtype=np.float64) for out in outputs
    }

    while component.status not in (ComponentStatus.Done,
                                   ComponentStatus.Failed):
        component.update()  # GIL released while the C++ engine computes
        for out in outputs:
            zeros = (0,) * len(out.shape)
            ok, message = out.get_values_into(
                buffers[out.id], zeros, out.shape)
            if not ok:
                print(f"  read failed on {out.id}: {message}")

    if component.status == ComponentStatus.Failed:
        # The error queue is the normative failure channel.
        for entry in component.errors():
            print(f"  [{entry.severity.name}] {entry.source}: "
                  f"{entry.message} (code {entry.code})")
        return 1

    component.finish()
    for out_id, values in buffers.items():
        print(f"{out_id}: mean={values.mean():.4g} max={values.max():.4g}")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        raise SystemExit(2)
    raise SystemExit(main(sys.argv[1]))
