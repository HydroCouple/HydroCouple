"""
Internal support for the C++ -> Python bridge.

Constructs NumPy array views over raw memory described by a C++
``BufferDescriptor`` so that a Python component's typed data plane can be
driven from C++ with zero element copies. Not part of the public API.
"""

from __future__ import annotations

import ctypes

import numpy as np

from hydrocouple.core import DataKind
from hydrocouple.helpers import DATA_KIND_TO_DTYPE


def ndarray_over(address: int, kind: int, shape, strides_bytes, writable: bool):
    """Create an ndarray view over foreign memory (zero-copy).

    :param address: base address of element (0, ..., 0).
    :param kind: integer value of the C++ ``DataKind``.
    :param shape: extent per dimension.
    :param strides_bytes: byte stride per dimension, or ``None`` for
        C-contiguous. Negative strides are not supported (the standard's
        descriptors point at element (0, ..., 0)).
    :param writable: whether the view should be writable.
    :returns: an ndarray sharing the foreign memory. The caller guarantees
        the memory outlives the view.
    """
    dtype = DATA_KIND_TO_DTYPE[DataKind(kind)]
    shape = tuple(int(s) for s in shape)

    if strides_bytes is None:
        strides = None
        span = dtype.itemsize
        for extent in shape:
            span *= extent
    else:
        strides = tuple(int(s) for s in strides_bytes)
        if any(s < 0 for s in strides):
            raise ValueError("negative strides are not supported")
        span = dtype.itemsize
        for extent, stride in zip(shape, strides):
            if extent > 0:
                span += (extent - 1) * stride

    buffer = (ctypes.c_char * max(span, dtype.itemsize)).from_address(address)
    arr = np.ndarray(shape, dtype=dtype, buffer=buffer, strides=strides)
    if not writable:
        arr = arr.view()
        arr.flags.writeable = False
    return arr
