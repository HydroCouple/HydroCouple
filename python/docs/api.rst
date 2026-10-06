.. _api_documentation:

=============
API Reference
=============

.. contents:: Module Index
   :local:
   :depth: 2

The :mod:`hydrocouple` package mirrors the C++20 v2.0.0 interface
standard 1:1: same names (snake_cased), same inheritance, same
pure-abstract discipline. For the corresponding C++ declarations see the
`C++ interface docs <../index.html>`__ at the site root.

hydrocouple.core
================

The core standard: enums, the ``ErrorEntry`` diagnostic record, the
signal/slot contracts, identity and metadata chains, the model-component
lifecycle, and the typed hyperslab data plane on
:class:`~hydrocouple.core.IComponentDataItem`.

.. automodule:: hydrocouple.core
   :members:
   :undoc-members:
   :show-inheritance:

----

hydrocouple.temporal
====================

Date/time representation, time spans, time-marching model components, and
time-series data items (time is canonical dimension 0).

.. automodule:: hydrocouple.temporal
   :members:
   :undoc-members:
   :show-inheritance:

----

hydrocouple.spatial
===================

OGC Simple Features geometry, spatial reference systems, the UGRID-congruent
:class:`~hydrocouple.spatial.IMeshView` bulk structure-of-arrays view,
rasters, regular grids, and the spatial data items.

.. automodule:: hydrocouple.spatial
   :members:
   :undoc-members:
   :show-inheritance:

----

hydrocouple.spatiotemporal
==========================

Data items whose values vary in both time and space; typed combinations
of the temporal and spatial data items with time outermost.

.. automodule:: hydrocouple.spatiotemporal
   :members:
   :undoc-members:
   :show-inheritance:

----

hydrocouple.distributed
=======================

Transport-neutral distributed execution: message transports, distributed
components, proxies for remote components, and partitioned data items
with local virtual (ghost/halo) entity representation.

.. automodule:: hydrocouple.distributed
   :members:
   :undoc-members:
   :show-inheritance:

----

hydrocouple.helpers
===================

Non-normative conveniences mirroring the C++ ``hydrocouplehelpers.h``.

.. automodule:: hydrocouple.helpers
   :members:
   :undoc-members:

----

hydrocouple.loader
==================

Load compiled C++ HydroCouple components from shared libraries.

.. automodule:: hydrocouple.loader
   :members:
   :undoc-members:
