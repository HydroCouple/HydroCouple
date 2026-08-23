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
