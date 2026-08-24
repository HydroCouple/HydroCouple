"""
Enum-parity tests: parse the C++ headers and assert the Python IntEnums
match name-for-name and value-for-value, so the bindings can never drift
from the standard again.
"""

import os
import re

import pytest

import hydrocouple.core as core
import hydrocouple.spatial as spatial

HEADER_DIR = os.path.normpath(
    os.path.join(os.path.dirname(__file__), "..", "..", "include"))


def _parse_cpp_enum(header: str, enum_name: str) -> dict[str, int]:
    """Extract {name: value} from a C++ enum class definition.

    Handles implicit sequential values and explicit ``= N`` assignments;
    strips comments.
    """
    path = os.path.join(HEADER_DIR, header)
    text = open(path, encoding="utf-8").read()
    match = re.search(
        r"enum\s+class\s+" + re.escape(enum_name) + r"\s*(?::\s*[\w:]+\s*)?\{(.*?)\}",
        text, re.DOTALL)
    if not match:
        raise AssertionError(f"enum {enum_name} not found in {header}")
    body = re.sub(r"/\*.*?\*/", "", match.group(1), flags=re.DOTALL)
    body = re.sub(r"//[^\n]*", "", body)
    values: dict[str, int] = {}
    next_value = 0
    for entry in body.split(","):
        entry = entry.strip()
        if not entry:
            continue
        assign = re.match(r"(\w+)\s*=\s*(\d+)", entry)
        if assign:
            next_value = int(assign.group(2))
            values[assign.group(1)] = next_value
        else:
            name = re.match(r"(\w+)", entry)
            if name:
                values[name.group(1)] = next_value
        next_value += 1
    return values


def _assert_parity(py_enum, header: str, cpp_name: str):
    cpp_values = _parse_cpp_enum(header, cpp_name)
    py_values = {member.name: member.value for member in py_enum}
    assert py_values == cpp_values, (
        f"{py_enum.__name__} diverges from C++ {cpp_name}: "
        f"python={py_values} cpp={cpp_values}")


class TestCoreEnumParity:
    def test_byte_order(self):
        _assert_parity(core.ByteOrder, "hydrocouple.h", "ByteOrder")

    def test_data_kind(self):
        _assert_parity(core.DataKind, "hydrocouple.h", "DataKind")

    def test_memory_space(self):
        _assert_parity(core.MemorySpace, "hydrocouple.h", "MemorySpace")

    def test_capability(self):
        _assert_parity(core.Capability, "hydrocouple.h", "Capability")

    def test_component_status(self):
        _assert_parity(core.ComponentStatus, "hydrocouple.h",
                       "ComponentStatus")

    def test_argument_input_type(self):
        _assert_parity(core.ArgumentInputType, "hydrocouple.h",
                       "ArgumentInputType")

    def test_workflow_status(self):
        _assert_parity(core.WorkflowStatus, "hydrocouple.h", "WorkflowStatus")

    def test_length_type(self):
        _assert_parity(core.LengthType, "hydrocouple.h", "LengthType")

    def test_fundamental_unit_dimension(self):
        _assert_parity(core.FundamentalUnitDimension, "hydrocouple.h",
                       "FundamentalUnitDimension")

    def test_distance_units(self):
        _assert_parity(core.DistanceUnits, "hydrocouple.h", "DistanceUnits")

    def test_area_units(self):
        _assert_parity(core.AreaUnits, "hydrocouple.h", "AreaUnits")

    def test_error_severity(self):
        _assert_parity(core.ErrorEntry.Severity, "hydrocouple.h", "Severity")


class TestSpatialEnumParity:
    def test_mesh_data_object_type(self):
        _assert_parity(spatial.MeshDataObjectType, "hydrocouplespatial.h",
                       "MeshDataObjectType")

    def test_network_data_object_type(self):
        _assert_parity(spatial.NetworkDataObjectType, "hydrocouplespatial.h",
                       "NetworkDataObjectType")

    def test_spatial_data_type(self):
        _assert_parity(spatial.SpatialDataType, "hydrocouplespatial.h",
                       "SpatialDataType")

    def test_regular_grid_type(self):
        _assert_parity(spatial.RegularGridType, "hydrocouplespatial.h",
                       "RegularGridType")

    def test_geometry_type(self):
        _assert_parity(spatial.GeometryType, "hydrocouplespatial.h",
                       "GeometryType")

    def test_raster_data_type(self):
        _assert_parity(spatial.RasterDataType, "hydrocouplespatial.h",
                       "RasterDataType")


# ---------------------------------------------------------------------------
# Cython declaration parity
# ---------------------------------------------------------------------------
#
# The .pxd files restate the same enums a third time, for Cython. Nothing
# else checks them: the members are not referenced from any .pyx, so a stale
# declaration compiles cleanly and the parity tests above only look at the
# pure-Python enums. WorkflowStatus drifted here undetected for exactly that
# reason. Only membership and order are asserted -- the .pxd deliberately
# omits values, since the C++ header supplies them.

PXD_DIR = os.path.join(os.path.dirname(__file__), "..", "_hydrocouple")

# cdef enum class <CythonName> ["<C++::qualified::Name>"] [(storage)]:
# followed by one indented member per line. Nested C++ enums carry the
# qualified name as a C-name string, optionally after a line continuation;
# top-level ones omit it, and the Cython name is the C++ name.
_PXD_ENUM = re.compile(
    r"cdef enum class (\w+)\s*(?:\\\s*\n\s*)?(?:\"([^\"]+)\")?"
    r"\s*(?:\([^)]*\))?\s*:\n((?:[ \t]+\w+\n)+)")


def _parse_pxd_enums(pxd: str) -> dict[str, tuple[str, list[str]]]:
    """Extract {cython_name: (cpp_enum_name, [member, ...])} from a .pxd."""
    text = open(os.path.join(PXD_DIR, pxd), encoding="utf-8").read()
    enums: dict[str, tuple[str, list[str]]] = {}
    for cython_name, c_name, body in _PXD_ENUM.findall(text):
        cpp_name = c_name.split("::")[-1] if c_name else cython_name
        enums[cython_name] = (cpp_name, body.split())
    return enums


@pytest.mark.parametrize("pxd,header", [
    ("_core.pxd", "hydrocouple.h"),
    ("_spatial.pxd", "hydrocouplespatial.h"),
])
def test_pxd_enum_members_exist_in_order(pxd, header):
    """Every member a .pxd declares must exist in the header, in order.

    Deliberately a subsequence check, not equality: Cython declarations are
    allowed to be partial (``_spatial.pxd`` names 5 of GeometryType's 72),
    and members bind to C++ enumerators by name, so the header always
    supplies the values. That also means a .pxd missing newly inserted
    members is stale documentation rather than a defect -- the real
    value-level guard is the pure-Python parity above, since
    ``hydrocouple/core.py`` restates the integers and nothing checks it.

    What this catches: a member that no longer exists in the header, and a
    declaration reordered against it. Neither is caught by the compiler
    while the member goes unreferenced from any .pyx.
    """
    declared = _parse_pxd_enums(pxd)
    assert declared, f"no enum declarations parsed out of {pxd}"
    for cython_name, (cpp_name, members) in declared.items():
        cpp_members = list(_parse_cpp_enum(header, cpp_name))
        remaining = iter(cpp_members)
        assert all(m in remaining for m in members), (
            f"{pxd} {cython_name} is not an in-order subset of C++ "
            f"{cpp_name}: pxd={members} cpp={cpp_members}")
