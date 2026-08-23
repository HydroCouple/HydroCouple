"""
End-to-end interop proof, both directions:

Direction A — C++ consumes a Python component: a pure-Python
IModelComponent is wrapped in PyComponentBridge and driven entirely from
C++ (cpp_test_harness.h) through the IModelComponent* virtual interface —
lifecycle, capabilities(), errors(), and hyperslab reads/writes against
the Python data item with C++-constructed BufferDescriptors.

Direction B — Python consumes a C++ component: a native C++20 component
(tests/cpp_component/test_component.cpp) is compiled to a shared library,
loaded via hydrocouple.loader.load(), and driven from Python with NumPy
hyperslab exchange.
"""

import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
import pytest

from hydrocouple.core import (
    Capability,
    ComponentStatus,
    DataKind,
    ErrorEntry,
    IComponentDataItem,
    IModelComponent,
)

TESTS_DIR = Path(__file__).parent
INCLUDE_DIR = TESTS_DIR.parent.parent / "include"


# ======================================================================
# Direction A fixtures: a pure-Python component with a working data plane
# ======================================================================
class PyField(IComponentDataItem):
    """[10] Float64 field: value = 100 * step + index."""

    def __init__(self, owner):
        self._owner = owner
        self.values = np.zeros(10, dtype=np.float64)

    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    @property
    def caption(self):
        return "Python field"

    @caption.setter
    def caption(self, value):
        pass

    @property
    def description(self):
        return ""

    @description.setter
    def description(self, value):
        pass

    @property
    def id(self):
        return "py-field"

    @property
    def model_component(self):
        return self._owner

    @property
    def dimensions(self):
        return []

    @property
    def shape(self):
        return (10,)

    @property
    def data_kind(self):
        return DataKind.Float64

    @property
    def value_definition(self):
        return None

    def get_values_into(self, destination, start, count):
        if destination.dtype != np.float64:
            return False, "kind mismatch"
        if start[0] < 0 or start[0] + count[0] > 10:
            return False, "selection out of bounds"
        destination.reshape(count[0])[...] = \
            self.values[start[0]:start[0] + count[0]]
        return True, ""

    def set_values_from(self, source, start, count):
        if source.dtype != np.float64:
            return False, "kind mismatch"
        if start[0] < 0 or start[0] + count[0] > 10:
            return False, "selection out of bounds"
        self.values[start[0]:start[0] + count[0]] = source.reshape(count[0])
        return True, ""


class PyInteropComponent(IModelComponent):
    """Time-marching Python component: Done after 3 updates."""

    def __init__(self):
        self._status = ComponentStatus.Created
        self._step = 0
        self.field = PyField(self)
        self.error_queue = []

    def connect(self, slot):
        pass

    def disconnect(self, slot):
        pass

    def block_signals(self, block):
        pass

    @property
    def caption(self):
        return "Python interop component"

    @caption.setter
    def caption(self, value):
        pass

    @property
    def description(self):
        return ""

    @description.setter
    def description(self, value):
        pass

    @property
    def id(self):
        return "py-interop"

    @property
    def component_info(self):
        return None

    @property
    def status(self):
        return self._status

    @property
    def arguments(self):
        return []

    @property
    def inputs(self):
        return []

    @property
    def outputs(self):
        return []

    @property
    def results(self):
        return [self.field]

    def initialize(self):
        self._status = ComponentStatus.Initialized

    def validate(self):
        self._status = ComponentStatus.Valid
        return []

    def prepare(self):
        self.field.values[:] = np.arange(10, dtype=np.float64)
        self._status = ComponentStatus.Updated

    def update(self, required_outputs=None):
        self._step += 1
        self.field.values[:] = 100.0 * self._step + np.arange(10)
        self._status = (ComponentStatus.Done if self._step >= 3
                        else ComponentStatus.Updated)

    def finish(self):
        self._status = ComponentStatus.Finished

    @property
    def workflow(self):
        return None

    @workflow.setter
    def workflow(self, value):
        pass

    def capabilities(self):
        return {Capability.Checkpointing, Capability.Cloneable}

    def errors(self, clear_after_read=False):
        entries = list(self.error_queue)
        if clear_after_read:
            self.error_queue.clear()
        return entries

    @property
    def reference_directory(self):
        return "."

    @reference_directory.setter
    def reference_directory(self, value):
        pass


# ======================================================================
# Direction A: C++ consumes the Python component
# ======================================================================
class TestCppConsumesPython:
    def test_cpp_drives_full_lifecycle(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_drive_lifecycle

        comp = PyInteropComponent()
        bridge = PyComponentBridge(comp)
        trace = cpp_drive_lifecycle(bridge)

        assert "id=py-interop" in trace
        assert "caption=Python interop component" in trace
        assert "start=Created" in trace
        assert "after_init=Initialized" in trace
        assert "validate_msgs=0" in trace
        assert "after_prepare=Updated" in trace
        assert "updates=3" in trace
        assert "after_updates=Done" in trace
        assert "final=Finished" in trace
        # The Python object really went through the lifecycle.
        assert comp.status == ComponentStatus.Finished

    def test_cpp_reads_python_data_plane(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_drive_lifecycle, cpp_read_result

        comp = PyInteropComponent()
        bridge = PyComponentBridge(comp)
        cpp_drive_lifecycle(bridge)  # leaves step == 3

        ok, values, message = cpp_read_result(bridge, 0, (0,), (10,))
        assert ok, message
        np.testing.assert_array_equal(values, 300.0 + np.arange(10))

        # Interior hyperslab, C++-side descriptor
        ok, values, message = cpp_read_result(bridge, 0, (2,), (5,))
        assert ok, message
        np.testing.assert_array_equal(values, [302.0, 303.0, 304.0, 305.0, 306.0])

        # Bounds rejection surfaces the Python message through C++
        ok, _, message = cpp_read_result(bridge, 0, (8,), (5,))
        assert not ok
        assert message == "selection out of bounds"

    def test_cpp_writes_python_data_plane(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_write_result

        comp = PyInteropComponent()
        bridge = PyComponentBridge(comp)

        block = np.array([7.0, 8.0, 9.0])
        ok, message = cpp_write_result(bridge, 0, block, (4,), (3,))
        assert ok, message
        np.testing.assert_array_equal(comp.field.values[4:7], block)
        assert comp.field.values[0] == 0.0  # neighbors untouched

    def test_cpp_observes_capabilities_and_errors(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import (
            cpp_drain_errors,
            cpp_query_capabilities,
        )

        comp = PyInteropComponent()
        comp.error_queue.append(ErrorEntry(
            severity=ErrorEntry.Severity.Error, code=42,
            source="py-interop", message="numerical blow-up"))
        bridge = PyComponentBridge(comp)

        caps = cpp_query_capabilities(bridge)
        assert caps == sorted([int(Capability.Checkpointing),
                               int(Capability.Cloneable)])

        rows = cpp_drain_errors(bridge)
        assert rows == ["2|42|py-interop|numerical blow-up"]
        # C++ drained with clearAfterRead=true
        assert comp.errors() == []


# ======================================================================
# Direction B: Python consumes a native C++ component
# ======================================================================
@pytest.fixture(scope="session")
def cpp_component_lib(tmp_path_factory):
    """Compile the native C++ test component to a shared library."""
    compiler = shutil.which("g++") or shutil.which("clang++") or shutil.which("c++")
    if compiler is None:
        pytest.skip("no C++ compiler available")

    build_dir = tmp_path_factory.mktemp("cpp_component")
    suffix = ".dylib" if sys.platform == "darwin" else ".so"
    lib_path = build_dir / f"libcpp_sine{suffix}"
    source = TESTS_DIR / "cpp_component" / "test_component.cpp"
    cmd = [compiler, "-std=c++20", "-shared", "-fPIC",
           f"-I{INCLUDE_DIR}", str(source), "-o", str(lib_path)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        pytest.fail(f"fixture compile failed:\n{result.stderr}")
    return lib_path


class TestPythonConsumesCpp:
    def test_load_and_metadata(self, cpp_component_lib):
        from hydrocouple.loader import load

        component, info, handle = load(str(cpp_component_lib))
        assert component.id == "cpp-sine"
        assert info.developer == "HydroCouple interop test"
        assert info.version == "2.0.0-alpha.1"
        assert isinstance(component, IModelComponent)

    def test_lifecycle_and_hyperslab_reads(self, cpp_component_lib):
        from hydrocouple.loader import load

        component, info, handle = load(str(cpp_component_lib))
        component.initialize()
        assert component.status == ComponentStatus.Initialized
        assert component.validate() == []
        component.prepare()
        assert component.status == ComponentStatus.Updated

        field = component.results[0]
        assert field.id == "cpp-sine-field"
        assert field.shape == (10,)
        assert field.data_kind == DataKind.Float64

        # After prepare: values are 0..9
        out = np.zeros(10, dtype=np.float64)
        ok, message = field.get_values_into(out, (0,), (10,))
        assert ok, message
        np.testing.assert_array_equal(out, np.arange(10, dtype=np.float64))

        # Run to Done (3 steps)
        steps = 0
        while component.status != ComponentStatus.Done:
            component.update()
            steps += 1
        assert steps == 3

        ok, message = field.get_values_into(out, (0,), (10,))
        assert ok, message
        np.testing.assert_array_equal(out, 3000.0 + np.arange(10))

        # Interior hyperslab
        part = np.zeros(5, dtype=np.float64)
        ok, message = field.get_values_into(part, (2,), (5,))
        assert ok, message
        np.testing.assert_array_equal(part, 3000.0 + np.arange(2, 7))

        component.finish()
        assert component.status == ComponentStatus.Finished

    def test_kind_and_bounds_rejection(self, cpp_component_lib):
        from hydrocouple.loader import load

        component, info, handle = load(str(cpp_component_lib))
        component.initialize()
        component.validate()
        component.prepare()
        field = component.results[0]

        wrong = np.zeros(10, dtype=np.float32)
        ok, message = field.get_values_into(wrong, (0,), (10,))
        assert not ok
        assert message == "kind mismatch"

        out = np.zeros(5, dtype=np.float64)
        ok, message = field.get_values_into(out, (8,), (5,))
        assert not ok
        assert message == "selection out of bounds"

    def test_write_back_into_cpp(self, cpp_component_lib):
        from hydrocouple.loader import load

        component, info, handle = load(str(cpp_component_lib))
        component.initialize()
        component.validate()
        component.prepare()
        field = component.results[0]

        block = np.array([5.5, 6.5], dtype=np.float64)
        ok, message = field.set_values_from(block, (3,), (2,))
        assert ok, message

        out = np.zeros(10, dtype=np.float64)
        ok, _ = field.get_values_into(out, (0,), (10,))
        assert ok
        np.testing.assert_array_equal(out[3:5], block)
        assert out[0] == 0.0

    def test_error_queue_translation(self, cpp_component_lib):
        from hydrocouple.loader import load

        component, info, handle = load(str(cpp_component_lib))
        component.initialize()
        component.validate()
        component.prepare()  # queues one Warning

        entries = component.errors(clear_after_read=True)
        assert len(entries) == 1
        assert entries[0].severity == ErrorEntry.Severity.Warning
        assert entries[0].code == 7
        assert entries[0].source == "cpp-sine"
        assert component.errors() == []

    def test_capabilities_empty(self, cpp_component_lib):
        from hydrocouple.loader import load

        component, info, handle = load(str(cpp_component_lib))
        assert component.capabilities() == set()
