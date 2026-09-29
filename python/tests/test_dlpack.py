"""
Phase G0 gates: tensors cross the HydroCouple boundary by DLPack, without
copies, on any device, in both directions.

The central claim is *zero copy*: when Python hands a tensor to a C++ data
item, the C++ item receives a BufferDescriptor pointing at the tensor's own
memory. Values landing in the tensor would not prove that -- a
copy-in/copy-out binding would also deliver them -- so the gates compare
*addresses*: a native C++ ProbeDataItem records the descriptor it was
handed, and the test asserts the address is the tensor's ``data_ptr()``.

Device gates run everywhere by construction: a BufferView over a fake
device address carries a Device descriptor through the binding into the
probe, which records it and refuses it (host-only, per the interface)
without ever touching the memory. The tests at the bottom that need a real
accelerator skip unless one is present; see plans/G0_HANDOFF.
"""

import gc

import numpy as np
import pytest

from hydrocouple.core import DataKind, MemorySpace
from hydrocouple import dlpack as hdl
from hydrocouple.dlpack import BufferView, DLDeviceType

torch = pytest.importorskip("torch")

from _hydrocouple import _core  # noqa: E402
from _hydrocouple._testing import Probe, inspect_dlpack  # noqa: E402

FAKE_DEVICE_ADDRESS = 0x7F0000000000


@pytest.fixture(autouse=True)
def cuda_accelerator():
    """Every test starts and ends with the default accelerator."""
    hdl.set_accelerator(DLDeviceType.CUDA)
    yield
    hdl.set_accelerator(DLDeviceType.CUDA)


# ======================================================================
# Python drives C++: the tensor's own memory reaches the C++ item
# ======================================================================
class TestZeroCopyOutbound:
    def test_a_torch_destination_is_written_in_place(self):
        probe = Probe()
        backing = torch.zeros((3, 8), dtype=torch.float64)
        view = backing[:, ::2]                      # non-contiguous
        ok, msg = probe.item.get_values_into(view, (0, 0), (3, 4))
        assert ok, msg
        seen = probe.last
        assert seen["address"] == view.data_ptr()
        assert seen["strides_bytes"] == (64, 16)
        assert seen["shape"] == (3, 4)
        assert seen["space"] == int(MemorySpace.Host)
        np.testing.assert_array_equal(view.numpy(), probe.values())
        # The gaps the view skips were never written.
        assert torch.count_nonzero(backing[:, 1::2]) == 0

    def test_a_torch_source_is_read_in_place(self):
        probe = Probe()
        source = -torch.arange(12, dtype=torch.float64).reshape(3, 4)
        ok, msg = probe.item.set_values_from(source, (0, 0), (3, 4))
        assert ok, msg
        assert probe.last["address"] == source.data_ptr()
        np.testing.assert_array_equal(probe.values(), source.numpy())

    def test_an_interior_hyperslab_reaches_the_right_cells(self):
        probe = Probe()
        dest = torch.full((2, 2), -1.0, dtype=torch.float64)
        ok, msg = probe.item.get_values_into(dest, (1, 2), (2, 2))
        assert ok, msg
        np.testing.assert_array_equal(dest.numpy(), [[12, 13], [22, 23]])

    def test_numpy_keeps_its_own_path_and_is_still_zero_copy(self):
        probe = Probe()
        dest = np.zeros((3, 4))
        ok, msg = probe.item.get_values_into(dest, (0, 0), (3, 4))
        assert ok, msg
        assert probe.last["address"] == dest.ctypes.data

    def test_a_jax_source_is_read_in_place(self):
        jax = pytest.importorskip("jax")
        jax.config.update("jax_enable_x64", True)
        import jax.numpy as jnp

        probe = Probe()
        source = jnp.arange(12, dtype=jnp.float64).reshape(3, 4) * 2.0
        ok, msg = probe.item.set_values_from(source, (0, 0), (3, 4))
        assert ok, msg
        assert probe.last["address"] == inspect_dlpack(source)["data"]
        np.testing.assert_array_equal(probe.values(), np.asarray(source))

    def test_a_jax_destination_is_refused_because_jax_arrays_are_immutable(self):
        jax = pytest.importorskip("jax")
        import jax.numpy as jnp

        probe = Probe()
        dest = jnp.zeros((3, 4))
        with pytest.raises(ValueError, match="immutable"):
            probe.item.get_values_into(dest, (0, 0), (3, 4))
        assert probe.last["calls"] == 0

    def test_the_contiguous_case_passes_real_strides(self):
        # DLPack >= 1.2 producers always send strides; they must arrive as
        # bytes, not elements.
        probe = Probe()
        dest = torch.zeros((3, 4), dtype=torch.float64)
        ok, msg = probe.item.get_values_into(dest, (0, 0), (3, 4))
        assert ok, msg
        assert probe.last["strides_bytes"] == (32, 8)


class TestElementTypes:
    @pytest.mark.parametrize("dtype, kind", [
        ("int8", DataKind.Int8), ("uint8", DataKind.UInt8),
        ("int16", DataKind.Int16), ("uint16", DataKind.UInt16),
        ("int32", DataKind.Int32), ("uint32", DataKind.UInt32),
        ("int64", DataKind.Int64), ("uint64", DataKind.UInt64),
        ("float32", DataKind.Float32), ("float64", DataKind.Float64),
        ("bool", DataKind.Boolean),
    ])
    def test_every_kind_maps_from_torch(self, dtype, kind):
        probe = Probe()
        dest = torch.zeros((3, 4), dtype=getattr(torch, dtype))
        ok, msg = probe.item.get_values_into(dest, (0, 0), (3, 4))
        assert probe.last["kind"] == int(kind)
        assert ok == (kind == DataKind.Float64), msg

    @pytest.mark.parametrize("dtype", ["bfloat16", "complex128", "float16"])
    def test_types_the_standard_lacks_are_refused_before_the_call(self, dtype):
        probe = Probe()
        dest = torch.zeros((3, 4), dtype=getattr(torch, dtype))
        with pytest.raises(ValueError, match="no HydroCouple DataKind"):
            probe.item.get_values_into(dest, (0, 0), (3, 4))
        assert probe.last["calls"] == 0


# ======================================================================
# Devices: the mapping, proven without a device
# ======================================================================
class TestDeviceMapping:
    def fake(self, space, device_id=3, writable=True):
        return BufferView(FAKE_DEVICE_ADDRESS, DataKind.Float64, (3, 4),
                          None, space, device_id, writable)

    def test_a_device_tensor_reaches_cpp_as_a_device_descriptor(self):
        probe = Probe()
        ok, msg = probe.item.get_values_into(
            self.fake(MemorySpace.Device), (0, 0), (3, 4))
        assert not ok and msg == "probe is host-only"
        seen = probe.last
        assert seen["space"] == int(MemorySpace.Device)
        assert seen["device_id"] == 3
        assert seen["address"] == FAKE_DEVICE_ADDRESS

    @pytest.mark.parametrize("space", [MemorySpace.HostPinned,
                                       MemorySpace.Unified,
                                       MemorySpace.Device])
    def test_each_space_round_trips_under_cuda(self, space):
        probe = Probe()
        probe.item.get_values_into(self.fake(space), (0, 0), (3, 4))
        assert probe.last["space"] == int(space)
        assert probe.last["device_id"] == 3

    def test_the_accelerator_decides_what_device_means_on_export(self):
        assert hdl.device_of(MemorySpace.Device, 2) == (DLDeviceType.CUDA, 2)
        hdl.set_accelerator(DLDeviceType.ROCM)
        assert hdl.device_of(MemorySpace.Device, 2) == (DLDeviceType.ROCM, 2)
        assert hdl.device_of(MemorySpace.HostPinned, 0)[0] == DLDeviceType.ROCMHost
        hdl.set_accelerator(DLDeviceType.Metal)
        assert hdl.device_of(MemorySpace.HostPinned, 0)[0] == DLDeviceType.CPU
        assert hdl.device_of(MemorySpace.Unified, 0)[0] == DLDeviceType.Metal

    def test_import_forgets_the_vendor(self):
        for accelerator in (DLDeviceType.CUDA, DLDeviceType.ROCM,
                            DLDeviceType.OneAPI, DLDeviceType.Metal):
            assert hdl.space_of(accelerator) == MemorySpace.Device
        assert hdl.space_of(DLDeviceType.CPU) == MemorySpace.Host
        assert hdl.space_of(DLDeviceType.CUDAManaged) == MemorySpace.Unified
        assert hdl.space_of(99) is None

    def test_only_accelerators_can_be_the_accelerator(self):
        for not_one in (DLDeviceType.CPU, DLDeviceType.CUDAHost,
                        DLDeviceType.CUDAManaged):
            with pytest.raises(ValueError):
                hdl.set_accelerator(not_one)
        assert hdl.accelerator() == DLDeviceType.CUDA

    def test_a_rocm_device_tensor_arrives_as_device(self):
        hdl.set_accelerator(DLDeviceType.ROCM)
        probe = Probe()
        probe.item.get_values_into(self.fake(MemorySpace.Device, 1),
                                   (0, 0), (3, 4))
        assert probe.last["space"] == int(MemorySpace.Device)
        assert probe.last["device_id"] == 1


class TestByteOffset:
    def capsule_producer(self, device_type, offset):
        class Producer:
            def __dlpack_device__(self):
                return (device_type, 0)

            def __dlpack__(self, **kwargs):
                return _core._make_capsule(
                    FAKE_DEVICE_ADDRESS, int(DataKind.Float64), (3, 4), None,
                    int(device_type), 0, False, True, offset)
        return Producer()

    def test_an_address_offset_is_folded_into_the_pointer(self):
        probe = Probe()
        probe.item.get_values_into(
            self.capsule_producer(DLDeviceType.CUDA, 256), (0, 0), (3, 4))
        assert probe.last["address"] == FAKE_DEVICE_ADDRESS + 256

    def test_an_offset_on_an_opaque_handle_is_refused(self):
        probe = Probe()
        with pytest.raises(ValueError, match="opaque handle"):
            probe.item.get_values_into(
                self.capsule_producer(DLDeviceType.Metal, 256), (0, 0), (3, 4))
        assert probe.last["calls"] == 0


# ======================================================================
# Lifetime, mutability, protocol versions
# ======================================================================
class TestLifetimeAndMutability:
    def test_every_borrowed_capsule_is_released_exactly_once(self):
        probe = Probe()
        backing = np.arange(12, dtype=np.float64).reshape(3, 4)
        assert _core._live_exports() == 0
        for _ in range(50):
            view = BufferView(backing.ctypes.data, DataKind.Float64, (3, 4),
                              None, MemorySpace.Host)
            ok, msg = probe.item.set_values_from(view, (0, 0), (3, 4))
            assert ok, msg
        assert _core._live_exports() == 0

    def test_a_framework_holding_an_export_keeps_it_alive_until_it_lets_go(self):
        backing = np.arange(12, dtype=np.float64).reshape(3, 4)
        view = BufferView(backing.ctypes.data, DataKind.Float64, (3, 4),
                          None, MemorySpace.Host)
        t = torch.from_dlpack(view)
        assert _core._live_exports() == 1
        del t
        gc.collect()
        assert _core._live_exports() == 0

    def test_an_unconsumed_capsule_frees_itself(self):
        backing = np.zeros(4)
        view = BufferView(backing.ctypes.data, DataKind.Float64, (4,),
                          None, MemorySpace.Host)
        capsule = view.__dlpack__(max_version=(1, 0))
        assert _core._live_exports() == 1
        del capsule
        gc.collect()
        assert _core._live_exports() == 0

    def test_a_read_only_producer_cannot_be_a_destination(self):
        backing = np.zeros((3, 4))
        view = BufferView(backing.ctypes.data, DataKind.Float64, (3, 4),
                          None, MemorySpace.Host, writable=False)
        probe = Probe()
        with pytest.raises(ValueError, match="read-only"):
            probe.item.get_values_into(view, (0, 0), (3, 4))
        assert probe.last["calls"] == 0
        # ...but is a perfectly good source.
        ok, msg = probe.item.set_values_from(view, (0, 0), (3, 4))
        assert ok, msg

    def test_a_read_only_view_refuses_a_legacy_consumer(self):
        backing = np.zeros(4)
        view = BufferView(backing.ctypes.data, DataKind.Float64, (4,),
                          None, MemorySpace.Host, writable=False)
        with pytest.raises(BufferError, match="read-only"):
            view.__dlpack__()

    def test_a_legacy_producer_still_works(self):
        backing = np.arange(12, dtype=np.float64).reshape(3, 4)

        class Legacy:
            """A pre-1.0 producer: no keywords at all, legacy capsule."""

            def __dlpack__(self):
                return BufferView(backing.ctypes.data, DataKind.Float64,
                                  (3, 4), None, MemorySpace.Host).__dlpack__()

            def __dlpack_device__(self):
                return (1, 0)

        probe = Probe()
        ok, msg = probe.item.set_values_from(Legacy(), (0, 0), (3, 4))
        assert ok, msg
        assert probe.last["address"] == backing.ctypes.data
        assert probe.last["strides_bytes"] == (32, 8)
        assert _core._live_exports() == 0

    def test_a_consumed_capsule_cannot_be_consumed_again(self):
        backing = np.zeros(4)
        view = BufferView(backing.ctypes.data, DataKind.Float64, (4,),
                          None, MemorySpace.Host)
        capsule = view.__dlpack__(max_version=(1, 0))

        class Replay:
            def __dlpack__(self, **kwargs):
                return capsule

        probe = Probe()
        ok, msg = probe.item.set_values_from(Replay(), (0,), (4,))
        assert (ok, msg) == (False, "rank mismatch")   # consumed, then refused
        assert _core._live_exports() == 0              # ...and released
        # The first consumption renamed it; a second is refused.
        with pytest.raises(BufferError, match="already consumed"):
            probe.item.set_values_from(Replay(), (0,), (4,))


# ======================================================================
# The producer side: a BufferView is a real DLPack tensor to real consumers
# ======================================================================
class TestBufferViewProducer:
    def test_torch_aliases_a_strided_view(self):
        backing = np.arange(12, dtype=np.float64).reshape(3, 4)
        view = BufferView(backing.ctypes.data, DataKind.Float64, (3, 2),
                          (32, 16), MemorySpace.Host)
        t = torch.from_dlpack(view)
        np.testing.assert_array_equal(t.numpy(), backing[:, ::2])
        t[0, 1] = 99.0
        assert backing[0, 2] == 99.0

    def test_numpy_and_jax_read_it_too(self):
        backing = np.arange(6, dtype=np.float32)
        view = BufferView(backing.ctypes.data, DataKind.Float32, (6,),
                          None, MemorySpace.Host)
        np.testing.assert_array_equal(np.from_dlpack(view), backing)
        jax = pytest.importorskip("jax")
        np.testing.assert_array_equal(
            np.asarray(jax.numpy.from_dlpack(view)), backing)

    def test_the_capsule_says_what_the_descriptor_said(self):
        view = BufferView(FAKE_DEVICE_ADDRESS, DataKind.Int32, (5, 7),
                          (4 * 7 * 2, 4), MemorySpace.Device, 4)
        info = inspect_dlpack(view)
        assert info["data"] == FAKE_DEVICE_ADDRESS
        assert info["device"] == (int(DLDeviceType.CUDA), 4)
        assert info["dtype"] == (0, 32, 1)
        assert info["shape"] == (5, 7)
        assert info["strides"] == (14, 1)
        assert info["flags"] == 0

    def test_a_view_refuses_to_copy_or_move(self):
        view = BufferView(FAKE_DEVICE_ADDRESS, DataKind.Float64, (2,),
                          None, MemorySpace.Device, 0)
        with pytest.raises(BufferError, match="cannot copy"):
            view.__dlpack__(copy=True)
        with pytest.raises(BufferError, match="another device"):
            view.__dlpack__(dl_device=(1, 0))

    def test_strides_that_split_an_element_are_refused(self):
        view = BufferView(FAKE_DEVICE_ADDRESS, DataKind.Float64, (2,),
                          (12,), MemorySpace.Device, 0)
        with pytest.raises(BufferError, match="whole number of elements"):
            view.__dlpack__(max_version=(1, 0))


# ======================================================================
# C++ drives Python: device memory arrives as a DLPack view, never an ndarray
# ======================================================================
class _Recorder:
    """A Python data item that records what C++ hands it."""

    def __init__(self):
        self.received = None
        self.kept = None

    def __call__(self, destination, start, count):
        self.received = destination
        self.kept = destination
        return False, "recorded"


def _python_component_with(recorder):
    from test_interop import PyInteropComponent

    comp = PyInteropComponent()
    comp.field.get_values_into = recorder
    comp.field.set_values_from = recorder
    return comp


class TestInbound:
    def test_device_memory_arrives_as_a_buffer_view(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_call_result

        recorder = _Recorder()
        bridge = PyComponentBridge(_python_component_with(recorder))
        ok, msg = cpp_call_result(
            bridge, 0, True, FAKE_DEVICE_ADDRESS, int(DataKind.Float64),
            (10,), None, int(MemorySpace.Device), 2, (0,), (10,))
        assert (ok, msg) == (False, "recorded")
        view = recorder.received
        assert isinstance(view, BufferView)
        assert view.data_ptr == FAKE_DEVICE_ADDRESS
        assert view.__dlpack_device__() == (DLDeviceType.CUDA, 2)
        assert view.writable

    def test_a_view_is_revoked_when_the_call_returns(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_call_result

        recorder = _Recorder()
        bridge = PyComponentBridge(_python_component_with(recorder))
        cpp_call_result(bridge, 0, True, FAKE_DEVICE_ADDRESS,
                        int(DataKind.Float64), (10,), None,
                        int(MemorySpace.Device), 0, (0,), (10,))
        assert not recorder.kept.valid
        with pytest.raises(BufferError, match="outlived"):
            recorder.kept.__dlpack__(max_version=(1, 0))

    def test_a_write_request_lends_a_read_only_view(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_call_result

        recorder = _Recorder()
        bridge = PyComponentBridge(_python_component_with(recorder))
        cpp_call_result(bridge, 0, False, FAKE_DEVICE_ADDRESS,
                        int(DataKind.Float64), (10,), None,
                        int(MemorySpace.Device), 0, (0,), (10,))
        assert not recorder.received.writable

    @pytest.mark.parametrize("space", [MemorySpace.Host,
                                       MemorySpace.HostPinned,
                                       MemorySpace.Unified])
    def test_host_accessible_memory_still_arrives_as_an_ndarray(self, space):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_call_result

        backing = np.zeros(10)
        recorder = _Recorder()
        bridge = PyComponentBridge(_python_component_with(recorder))
        cpp_call_result(bridge, 0, True, backing.ctypes.data,
                        int(DataKind.Float64), (10,), None, int(space), 0,
                        (0,), (10,))
        assert type(recorder.received) is np.ndarray
        assert recorder.received.ctypes.data == backing.ctypes.data

    def test_a_torch_component_writes_straight_into_the_callers_memory(self):
        """A Python item written in torch, called from C++ with host memory:
        torch.from_dlpack over the lent view writes the caller's buffer."""
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_call_result

        backing = np.zeros(10)

        def torch_get(destination, start, count):
            t = torch.from_dlpack(destination)
            t.copy_(torch.arange(10, dtype=torch.float64) * 3.0)
            return True, ""

        comp = _python_component_with(_Recorder())
        comp.field.get_values_into = torch_get
        bridge = PyComponentBridge(comp)
        ok, msg = cpp_call_result(bridge, 0, True, backing.ctypes.data,
                                  int(DataKind.Float64), (10,), None,
                                  int(MemorySpace.Host), 0, (0,), (10,))
        assert ok, msg
        np.testing.assert_array_equal(backing, np.arange(10) * 3.0)


# ======================================================================
# Real accelerators (skipped here; run by the hand-off verifier)
# ======================================================================
def _accelerator():
    if torch.cuda.is_available():
        return "cuda"
    return None


needs_device = pytest.mark.skipif(_accelerator() is None,
                                  reason="no CUDA/ROCm device")


@needs_device
class TestRealDevice:
    def test_a_device_tensor_is_lent_not_copied(self):
        probe = Probe()
        t = torch.zeros((3, 4), dtype=torch.float64, device=_accelerator())
        ok, msg = probe.item.get_values_into(t, (0, 0), (3, 4))
        assert (ok, msg) == (False, "probe is host-only")
        assert probe.last["address"] == t.data_ptr()
        assert probe.last["space"] == int(MemorySpace.Device)
        assert probe.last["device_id"] == t.device.index

    def test_a_torch_component_receives_device_memory_it_can_write(self):
        from _hydrocouple._core import PyComponentBridge
        from _hydrocouple._testing import cpp_call_result

        target = torch.zeros(10, dtype=torch.float64, device=_accelerator())

        def torch_get(destination, start, count):
            assert isinstance(destination, BufferView)
            t = torch.from_dlpack(destination)
            assert t.data_ptr() == target.data_ptr()
            t.copy_(torch.arange(10, dtype=torch.float64, device=t.device))
            return True, ""

        comp = _python_component_with(_Recorder())
        comp.field.get_values_into = torch_get
        bridge = PyComponentBridge(comp)
        ok, msg = cpp_call_result(bridge, 0, True, target.data_ptr(),
                                  int(DataKind.Float64), (10,), None,
                                  int(MemorySpace.Device),
                                  target.device.index or 0, (0,), (10,))
        assert ok, msg
        torch.cuda.synchronize()
        assert torch.equal(target.cpu(), torch.arange(10, dtype=torch.float64))

    def test_an_explicit_stream_is_accepted(self):
        probe = Probe()
        s = torch.cuda.Stream()
        with torch.cuda.stream(s):
            t = torch.ones((3, 4), dtype=torch.float64, device="cuda")
        ok, msg = probe.item.set_values_from(t, (0, 0), (3, 4),
                                             stream=s.cuda_stream)
        assert probe.last["address"] == t.data_ptr()
