==========
Quickstart
==========

Installation
============

Requires a C++20 compiler, Python ≥ 3.10, NumPy, and Cython ≥ 3.0. The
package version is single-sourced from the repository's
``include/version.h``.

.. code-block:: bash

   cd python
   pip install .
   python -m pytest    # includes enum-parity checks against the C++ headers

Drive a compiled C++ component
==============================

.. code-block:: python

   import numpy as np
   from hydrocouple.loader import load

   component, info, handle = load("./libMyModel.so")
   component.initialize()
   component.validate()
   component.prepare()

   output = component.outputs[0]
   values = np.empty(output.shape, dtype=np.float64)
   while component.status.name not in ("Done", "Failed"):
       component.update()                      # GIL released while C++ runs
       ok, msg = output.get_values_into(
           values, (0,) * len(output.shape), output.shape)
       assert ok, msg

   component.finish()

Selections are HDF5-style hyperslabs (``start``/``count`` per dimension)
and destinations may be non-contiguous views. Dimension orderings are
canonical per data-item type — time is always dimension 0; see each ABC's
docstring in the :doc:`api`.

On failure, drain the diagnostic queue — the normative failure channel of
the standard:

.. code-block:: python

   for entry in component.errors():
       print(entry.severity.name, entry.source, entry.message)

Implement a component in Python
===============================

Subclass the ABCs, then hand the component to C++ through the bridge:

.. code-block:: python

   from hydrocouple.core import ComponentStatus, IModelComponent
   from _hydrocouple._core import PyComponentBridge

   class MyModel(IModelComponent):
       ...  # lifecycle, arguments, inputs/outputs,
            # capabilities(), errors()

   bridge = PyComponentBridge(MyModel())
   # The bridge holds a C++ IModelComponent* that delegates every virtual
   # call (including capabilities() and errors()) back into Python.

Runnable examples live in ``python/examples/``:
``sine_wave_component.py`` (a pure-Python time-marching component whose
lifecycle is checked against the transition table) and
``drive_cpp_component.py`` (loading and running a compiled component).

Helpers
=======

:mod:`hydrocouple.helpers` mirrors the C++ ``hydrocouplehelpers.h`` — the
single sanctioned exception to the standard's no-implementation rule:
the ``DataKind ⇄ numpy.dtype`` maps,
:func:`~hydrocouple.helpers.is_valid_component_status_transition`
(the lifecycle state machine), typed scalar/slab conveniences with
raising variants, and Julian-day conversions matching the ``IDateTime``
convention.
