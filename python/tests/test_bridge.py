"""
Tests for the PyComponentBridge round-trip mechanism.

Verifies that a Python IModelComponent can be wrapped in a
PyComponentBridge and that identity, lifecycle state, and
property-change signaling work correctly through the bridge.
"""

from hydrocouple.core import ComponentStatus

from conftest import DummyComponent


class TestPyComponentBridge:
    def test_bridge_wraps_python_object(self):
        from _hydrocouple._core import PyComponentBridge

        comp = DummyComponent()
        bridge = PyComponentBridge(comp)
        assert bridge.py_object is comp

    def test_bridge_preserves_identity(self):
        """Bridge py_object reflects the same state as the original."""
        from _hydrocouple._core import PyComponentBridge

        comp = DummyComponent()
        bridge = PyComponentBridge(comp)

        assert bridge.py_object.id == "dummy-001"
        assert bridge.py_object.caption == "Dummy"
        assert bridge.py_object.description == "A test component"
        assert bridge.py_object.reference_directory == "/tmp"
        assert bridge.py_object.status == ComponentStatus.Created

    def test_bridge_reflects_lifecycle_changes(self):
        """Lifecycle calls on the Python object are visible through bridge."""
        from _hydrocouple._core import PyComponentBridge

        comp = DummyComponent()
        bridge = PyComponentBridge(comp)

        comp.initialize()
        assert bridge.py_object.status == ComponentStatus.Initialized

        comp.validate()
        comp.prepare()
        comp.update()
        comp.finish()

        assert bridge.py_object.status == ComponentStatus.Finished
        assert comp._lifecycle == [
            "initialize", "validate", "prepare", "update", "finish"
        ]

    def test_multiple_bridges_independent(self):
        from _hydrocouple._core import PyComponentBridge

        comp1 = DummyComponent()
        comp2 = DummyComponent()
        comp2._id = "dummy-002"

        b1 = PyComponentBridge(comp1)
        b2 = PyComponentBridge(comp2)

        assert b1.py_object is comp1
        assert b2.py_object is comp2
        assert b1.py_object.id != b2.py_object.id

    def test_bridge_emit_property_changed(self):
        from _hydrocouple._core import PyComponentBridge

        comp = DummyComponent()
        bridge = PyComponentBridge(comp)
        # Should not raise — no C++ slots connected
        bridge.emit_property_changed("caption")
