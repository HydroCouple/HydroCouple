"""
Tests for pure-Python ABCs in HydroCouple core (v2).
"""

from hydrocouple.core import (
    Capability,
    ComponentStatus,
    DataKind,
    ErrorEntry,
    IDescription,
    IIdentity,
    IModelComponent,
    IPropertyChanged,
    ISignal,
    MemorySpace,
)

from conftest import DummyComponent


class TestPureABCs:
    def test_dummy_is_model_component(self):
        comp = DummyComponent()
        assert isinstance(comp, IModelComponent)
        assert isinstance(comp, IIdentity)
        assert isinstance(comp, IDescription)
        assert isinstance(comp, IPropertyChanged)
        assert isinstance(comp, ISignal)

    def test_lifecycle(self):
        comp = DummyComponent()
        comp.initialize()
        comp.validate()
        comp.prepare()
        comp.update()
        comp.finish()
        assert comp._lifecycle == [
            "initialize", "validate", "prepare", "update", "finish"
        ]
        assert comp.status == ComponentStatus.Finished

    def test_capabilities_default_empty(self):
        comp = DummyComponent()
        assert comp.capabilities() == set()

    def test_error_queue(self):
        comp = DummyComponent()
        comp._errors.append(ErrorEntry(
            severity=ErrorEntry.Severity.Warning,
            code=7, source="dummy-001", message="low storage"))
        entries = comp.errors()
        assert len(entries) == 1
        assert entries[0].severity == ErrorEntry.Severity.Warning
        assert comp.errors(clear_after_read=True)  # still there
        assert comp.errors() == []  # now drained


class TestEnums:
    def test_component_status_has_checkpointing(self):
        assert ComponentStatus.Checkpointing == 10
        assert ComponentStatus.Failed == 14

    def test_data_kind_values(self):
        assert DataKind.Unknown == 0
        assert DataKind.Float64 == 10
        assert DataKind.Opaque == 13

    def test_memory_space_values(self):
        assert MemorySpace.Host == 0
        assert MemorySpace.Unified == 3

    def test_capability_values(self):
        assert Capability.DeviceBuffers == 0
        assert Capability.Licensing == 6

    def test_no_variant_symbol(self):
        import hydrocouple.core as core
        assert not hasattr(core, "HydroCoupleVariant")


class TestErrorEntry:
    def test_defaults(self):
        e = ErrorEntry()
        assert e.severity == ErrorEntry.Severity.Information
        assert e.code == 0
        assert e.source == ""
        assert e.message == ""
