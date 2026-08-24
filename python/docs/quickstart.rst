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
lifecycle is checked against the transition table),
``drive_cpp_component.py`` (loading and running a compiled component), and
``coupled_python_models.py`` (two Python components coupled to each other,
covered next).

Couple two Python models together
=================================

Nothing about coupling requires C++. Two components that subclass the ABCs
can exchange values through the standard's own interfaces, in Python alone
— no compiled component, no SDK, no bridge.
``examples/coupled_python_models.py`` runs a rainfall-runoff model
(producing runoff depth in **mm/hr** over three catchments) into a channel
routing model (consuming lateral inflow in **m³/s** over three reaches),
and shows the same exchange done two ways.

The direct way is to copy values in the driver loop:

.. code-block:: python

   depth = helpers.get_values_or_raise(output, (0,), output.shape)
   flow = depth * areas_m2 / (1000.0 * 3600.0)      # mm/hr -> m3/s
   helpers.set_values_or_raise(input_item, flow, (0,), input_item.shape)

That is correct, and it is where most couplings start. But the conversion
is stranded in the loop: nothing else can reuse it, and no interface
records that a conversion happened — so a reader of either model cannot
tell that the units disagree.

Moving it into the wiring changes that. An :class:`~hydrocouple.core.IInput`
decides for itself whether a provider is usable, and here the raw output is
refused:

.. code-block:: python

   ok, message = input_item.can_consume(output)
   # False: "runoff delivers mm/hr, lateral-inflow needs m3/s"

An :class:`~hydrocouple.core.IAdaptedOutput` reconciles them. Because an
adapted output *is* an output, it can serve as a provider like any other,
and neither component knows it is there:

.. code-block:: python

   adapter = AreaWeightedFlowAdapter(output, areas_m2)   # IAdaptedOutput
   adapter.initialize()
   assert input_item.can_consume(adapter)[0]

   input_item.set_provider(adapter)
   adapter.add_consumer(input_item)

With the link established, only the *downstream* model is driven. Routing
pulls what it needs through the adapter, which pulls the rainfall-runoff
model forward — the standard's request-reply mechanism, and the reason
``update_values()`` takes the querying input as its argument:

.. code-block:: python

   while routing.status != ComponentStatus.Done:
       routing.update()        # pulls upstream; nothing calls runoff.update()

Inside ``routing.update()`` the consumer side is three calls — and the
component parks in ``WaitingForData`` while the request is outstanding,
which is what that status is for:

.. code-block:: python

   provider = self._inflow.provider
   provider.update_values(self._inflow)                  # request
   values = helpers.get_values_or_raise(provider, (0,), provider.shape)
   helpers.set_values_or_raise(self._inflow, values, (0,), provider.shape)

The example asserts that both couplings produce identical discharge at
every step and reach, so the adapter is checked against the arithmetic it
replaced rather than merely asserted to work.

Two details worth copying into real components. A provider that has reached
``Done`` must not be advanced again — there is no ``Done → Updating`` edge
in the transition table — so it keeps serving the values it holds once its
horizon runs out. And a discoverable adapter would normally be produced by
an :class:`~hydrocouple.core.IAdaptedOutputFactory` and expose its
configuration as :class:`~hydrocouple.core.IArgument`, rather than taking
it through the constructor as this one does.

Helpers
=======

:mod:`hydrocouple.helpers` mirrors the C++ ``hydrocouplehelpers.h`` — the
single sanctioned exception to the standard's no-implementation rule:
the ``DataKind ⇄ numpy.dtype`` maps,
:func:`~hydrocouple.helpers.is_valid_component_status_transition`
(the lifecycle state machine), typed scalar/slab conveniences with
raising variants, and Julian-day conversions matching the ``IDateTime``
convention.
