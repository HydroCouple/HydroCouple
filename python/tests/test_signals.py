"""
Tests for signal/slot handle types in the HydroCouple Cython bindings.

Verifies that all slot handle classes are importable, have the expected
attributes, and that wrapper types expose signal connection methods.
"""


class TestSignalSlotHandles:
    def test_handle_type_importable(self):
        from _hydrocouple._core import _SlotHandle

        assert hasattr(_SlotHandle, "disconnect")
        assert hasattr(_SlotHandle, "connected")

    def test_wrapper_has_signal_methods(self):
        from _hydrocouple._core import (
            CppModelComponentWrapper,
            CppComponentDataItemWrapper,
            CppWorkflowComponentWrapper,
        )

        assert hasattr(CppModelComponentWrapper, "on_status_changed")
        assert hasattr(CppModelComponentWrapper, "on_property_changed")
        assert hasattr(CppComponentDataItemWrapper, "on_value_changed")
        assert hasattr(CppComponentDataItemWrapper, "on_property_changed")
        assert hasattr(CppWorkflowComponentWrapper, "on_status_changed")
        assert hasattr(CppWorkflowComponentWrapper, "on_property_changed")
