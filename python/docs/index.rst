.. image:: images/hydrocouple_logo.png
   :alt: HydroCouple Logo
   :width: 200px

===========
HydroCouple
===========

**HydroCouple — Python bindings for the component-based modeling
framework interface standard.**

HydroCouple is a header-only C++20 interface standard for coupling
hydrologic, hydraulic, and environmental model components, designed for
HPC, GPU, and cloud execution. The :mod:`hydrocouple` package mirrors the
v2.0.0 standard as Python abstract base classes, and the ``_hydrocouple``
Cython bridge connects Python to compiled C++ components — in both
directions.

Field data moves through the typed hyperslab data plane
(:meth:`~hydrocouple.core.IComponentDataItem.get_values_into` /
:meth:`~hydrocouple.core.IComponentDataItem.set_values_from`) as NumPy
arrays: an ndarray carries exactly what the C++ ``BufferDescriptor``
encodes — data pointer, dtype, shape, byte strides — so the bridge
marshals zero-copy in both directions and releases the GIL around C++
compute.

.. grid:: 2

    .. grid-item-card::  Implement a component in Python
        :link: quickstart
        :link-type: doc

        Subclass the :mod:`hydrocouple.core` ABCs exactly as C++
        developers implement the pure-virtual interfaces, then hand the
        component to C++ workflows through ``PyComponentBridge``.

    .. grid-item-card::  Drive a compiled C++ component
        :link: quickstart
        :link-type: doc

        :func:`hydrocouple.loader.load` brings a compiled component into
        Python; exchange fields as NumPy arrays through the zero-copy
        hyperslab API.

----

Where to start
==============

.. grid:: 2

    .. grid-item-card:: Quickstart
        :link: quickstart
        :link-type: doc

        Install, load or implement a component, exchange NumPy arrays.

    .. grid-item-card:: API Reference
        :link: api
        :link-type: doc

        Full class and method reference for every module: core, temporal,
        spatial, spatiotemporal, distributed, helpers, loader.

The C++ side of this site — the interface standard itself, with the full
Doxygen reference — lives at the site root:
`HydroCouple C++ interface docs <https://hydrocouple.org/HydroCouple/>`_
(or ``../index.html`` when browsing this co-deployed site).

----

.. toctree::
   :maxdepth: 2
   :hidden:

   quickstart
   api
