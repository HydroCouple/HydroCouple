"""
End-to-end tests for the v2 HydroCouple Python bindings.

Covers, in one place:
1. Pure-Python ABC implementation and lifecycle.
2. Cython wrapper ABC registration across all four extension modules.
3. PyComponentBridge round-trip of a Python component through C++,
   including the v2 capabilities() and errors() contracts.
4. The typed hyperslab data plane driven through the ABCs.
5. Loader plumbing.
"""

import numpy as np
import pytest

from hydrocouple.core import (
    Capability,
    ComponentStatus,
    ErrorEntry,
    IComponentDataItem,
    IModelComponent,
)
from hydrocouple import helpers

from conftest import DummyComponent, GridDataItem


# ======================================================================
# 1. Pure ABCs
# ======================================================================
class TestPureABCs:
    def test_lifecycle_and_isinstance(self):
        comp = DummyComponent()
        assert isinstance(comp, IModelComponent)
        comp.initialize()
        comp.validate()
        comp.prepare()
        comp.update()
        comp.finish()
        assert comp.status == ComponentStatus.Finished

    def test_grid_item_is_component_data_item(self):
        assert isinstance(GridDataItem(2, 2), IComponentDataItem)


# ======================================================================
# 2. Wrapper registration across all extension modules
# ======================================================================
class TestABCRegistration:
    def test_core_wrappers(self):
        from _hydrocouple._core import (
            CppArgumentWrapper,
            CppComponentDataItemWrapper,
            CppModelComponentWrapper,
        )
        from hydrocouple.core import (
            IArgument,
            IComponentDataItem,
            IModelComponent,
        )

        assert issubclass(CppArgumentWrapper, IArgument)
        assert issubclass(CppComponentDataItemWrapper, IComponentDataItem)
        assert issubclass(CppModelComponentWrapper, IModelComponent)

    def test_all_extension_modules_import(self):
        import _hydrocouple._core  # noqa: F401
        import _hydrocouple._spatial  # noqa: F401
        import _hydrocouple._spatiotemporal  # noqa: F401
        import _hydrocouple._temporal  # noqa: F401


# ======================================================================
# 3. PyComponentBridge round-trip
# ======================================================================
class TestPyComponentBridge:
    def test_round_trip_identity_and_lifecycle(self):
        from _hydrocouple._core import PyComponentBridge

        comp = DummyComponent()
        bridge = PyComponentBridge(comp)
        assert bridge.py_object is comp

        comp.initialize()
        assert bridge.py_object.status == ComponentStatus.Initialized

    def test_bridge_carries_v2_contracts(self):
        """The bridged Python object exposes capabilities() and errors()."""
        from _hydrocouple._core import PyComponentBridge

        comp = DummyComponent()
        comp._errors.append(ErrorEntry(
            severity=ErrorEntry.Severity.Error, code=42,
            source="dummy-001", message="boom"))
        bridge = PyComponentBridge(comp)

        assert bridge.py_object.capabilities() == set()
        entries = bridge.py_object.errors()
        assert entries[0].code == 42
        assert entries[0].severity == ErrorEntry.Severity.Error

    def test_emit_property_changed_without_slots(self):
        from _hydrocouple._core import PyComponentBridge

        bridge = PyComponentBridge(DummyComponent())
        bridge.emit_property_changed("caption")  # must not raise


# ======================================================================
# 4. Typed hyperslab data plane
# ======================================================================
class TestDataPlane:
    def test_hyperslab_through_abc(self, grid_item):
        out = np.zeros((2, 3), dtype=np.float64)
        ok, message = grid_item.get_values_into(out, (1, 2), (2, 3))
        assert ok, message
        np.testing.assert_array_equal(
            out, [[12.0, 13.0, 14.0], [22.0, 23.0, 24.0]])

    def test_helpers_round_trip(self, grid_item):
        block = helpers.get_values_or_raise(grid_item, (0, 0), (4, 5))
        block2 = block.copy()
        block2 += 1000.0
        helpers.set_values_or_raise(grid_item, block2, (0, 0), (4, 5))
        again = helpers.get_values_or_raise(grid_item, (0, 0), (4, 5))
        np.testing.assert_array_equal(again, block + 1000.0)

    def test_dtype_enforcement(self, grid_item):
        wrong = np.zeros((2, 3), dtype=np.float32)
        ok, message = grid_item.get_values_into(wrong, (0, 0), (2, 3))
        assert not ok


# ======================================================================
# 5. Loader plumbing
# ======================================================================
class TestComponentLoader:
    def test_loader_importable(self):
        from _hydrocouple._core import LoadedLibrary, load_component
        from hydrocouple.loader import load

        assert callable(load)
        assert callable(load_component)
        assert LoadedLibrary is not None

    def test_loader_rejects_missing_library(self):
        from hydrocouple.loader import load

        with pytest.raises(OSError, match="Cannot load library"):
            load("/nonexistent/libfoo.so")
