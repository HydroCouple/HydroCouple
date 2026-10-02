"""
Every Python ABC declares what its C++ interface declares.

The mirror drifted twice before this test existed: ``helpers.py`` kept the
ABI-2 transition table through two ABI revisions, and ``IWorkflowComponent``
lost ``validate``, ``prepare``, ``request_stop``, ``request_pause``,
``resume`` and ``errors`` when the C++ interface gained them. Here each ABC
whose name is a C++ interface class is compared member for member with the
pure virtuals that class declares itself, in the snake_case the mirror uses.

The few differences that are deliberate are listed in ``RENAMED`` (a C++
spelling Python cannot or does not use), ``NOT_MIRRORED`` (C++ members with
no Python counterpart by design) and ``PYTHON_ONLY``, each with its reason.
"""

import inspect
import os
import re
from abc import ABCMeta

import pytest

import hydrocouple.core as core
import hydrocouple.distributed as distributed
import hydrocouple.spatial as spatial
import hydrocouple.spatiotemporal as spatiotemporal
import hydrocouple.temporal as temporal

HEADER_DIR = os.path.normpath(
    os.path.join(os.path.dirname(__file__), "..", "..", "include"))
HEADERS = {name: open(os.path.join(HEADER_DIR, name), encoding="utf-8").read()
           for name in os.listdir(HEADER_DIR) if name.endswith(".h")}

#: (class, C++ member) -> Python member.
RENAMED = {
    ("IGeometry", "is3D"): "is_3d",          # snake_case of a digit
    ("IGeometry", "unionG"): "union",        # C++ cannot name it union
    ("IRegularGrid2D", "numXNodes"): "num_x_nodes",
    ("IRegularGrid2D", "numYNodes"): "num_y_nodes",
    ("IRegularGrid3D", "numXNodes"): "num_x_nodes",
    ("IRegularGrid3D", "numYNodes"): "num_y_nodes",
    ("IRegularGrid3D", "numZNodes"): "num_z_nodes",
    ("IArgument", "toString"): "__str__",
    ("ITINComponentDataItem", "TIN"): "tin",
    ("ITimeSeriesTINComponentDataItem", "TIN"): "tin",
    # The same rename C++ made, for the same reason: connect/disconnect are
    # the inherited signal methods.
    ("IProxyModelComponent", "connectToPeer"): "connect_remote",
    ("IProxyModelComponent", "disconnectFromPeer"): "disconnect_remote",
}

#: (class, C++ member) -> why Python has no counterpart.
NOT_MIRRORED = {
    ("ISignal", "emit"): "protected in C++: an implementation detail",
    ("IMultiInput", "canConsume"):
        "the role-aware overload; Python's can_consume is inherited",
}

#: (class, Python member) -> why Python declares a member C++ does not.
PYTHON_ONLY = {
    ("IVertex", "vertex_index"):
        "the vertex's index in its network, spelled apart from "
        "IGeometry.index for readability; both answer index()",
}

#: Python members whose C++ counterpart is a setter pair, written in Python
#: as a method rather than a property setter.
SETTER_METHODS = {"set_values_from", "set_provider"}


def _cpp_members(class_name):
    for text in HEADERS.values():
        start = re.search(r"\n\s*class\s+" + class_name + r"\b[^;{]*\{", text)
        if not start:
            continue
        body, depth = [], 1
        for char in text[start.end():]:
            depth += char == "{"
            depth -= char == "}"
            if depth == 0:
                break
            body.append(char)
        body = re.sub(r"/\*.*?\*/", "", "".join(body), flags=re.DOTALL)
        body = re.sub(r"//[^\n]*", "", body)
        return set(re.findall(r"virtual[^;(]*?\b(\w+)\s*\([^;]*?=\s*0\s*;", body))
    return None


def _snake(name):
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "_", name).lower()


def _own_abstract(abc_class):
    return {name for name, member in vars(abc_class).items()
            if getattr(getattr(member, "fget", member),
                       "__isabstractmethod__", False)}


def _mirrored_abcs():
    found = []
    for module in (core, temporal, spatial, spatiotemporal, distributed):
        for name, value in vars(module).items():
            if (inspect.isclass(value) and isinstance(value, ABCMeta)
                    and value.__module__ == module.__name__
                    and _cpp_members(name) is not None):
                found.append(value)
    return found


ABCS = _mirrored_abcs()


def test_the_scan_finds_the_interfaces():
    names = {abc.__name__ for abc in ABCS}
    assert {"IModelComponent", "IWorkflowComponent", "IGeometry",
            "ITimeSeriesComponentDataItem", "ICrossSection"} <= names
    assert len(ABCS) > 60


@pytest.mark.parametrize("abc_class", ABCS, ids=lambda a: a.__name__)
def test_abc_mirrors_its_interface(abc_class):
    name = abc_class.__name__
    expected = set()
    setters = set()
    for member in _cpp_members(name):
        if (name, member) in NOT_MIRRORED:
            continue
        python_name = RENAMED.get((name, member), _snake(member))
        if python_name.startswith("set_") and python_name not in SETTER_METHODS:
            setters.add(python_name[4:])
            continue
        expected.add(python_name)

    declared = _own_abstract(abc_class) - {
        member for (cls, member) in PYTHON_ONLY if cls == name}
    assert declared == expected, (
        f"{name}: missing {sorted(expected - declared)}, "
        f"extra {sorted(declared - expected)}")

    # A C++ setX is the setter of a Python property x, wherever in the MRO
    # the property is declared.
    for prop in setters:
        member = next((vars(k)[prop] for k in abc_class.__mro__
                       if prop in vars(k)), None)
        assert isinstance(member, property) and member.fset is not None, (
            f"{name}.{prop} has a C++ setter but no Python property setter")
