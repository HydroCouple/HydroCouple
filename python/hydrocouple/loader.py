"""Load compiled C++ HydroCouple components from shared libraries.

Example
-------
>>> from hydrocouple.loader import load
>>> component, info, handle = load("./libMyComponent.so")
>>> component.initialize()
>>> component.status
<ComponentStatus.Initialized: 2>
"""

from __future__ import annotations

import os
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from hydrocouple.core import IModelComponent, IModelComponentInfo


def load(
    library_path: str | os.PathLike,
    symbol_name: str = "CreateComponentInfo",
) -> tuple["IModelComponent", "IModelComponentInfo", object]:
    """Load a compiled C++ HydroCouple component from a shared library.

    Parameters
    ----------
    library_path
        Path to the shared library (``.so``, ``.dylib``, or ``.dll``).
    symbol_name
        Name of the exported ``extern "C"`` factory function that returns
        an ``IModelComponentInfo*``.  Defaults to ``"CreateComponentInfo"``.

    Returns
    -------
    component : IModelComponent
        The newly created model component instance.
    info : IModelComponentInfo
        Metadata about the component (developer, version, etc.).
    handle : object
        An opaque handle to the loaded shared library.  **You must keep
        this alive** for as long as the component is in use.

    Raises
    ------
    OSError
        If the library cannot be loaded or the symbol is not found.
    RuntimeError
        If the factory or ``createComponentInstance()`` returns null.
    """
    from _hydrocouple._core import load_component

    return load_component(str(library_path), symbol_name)
