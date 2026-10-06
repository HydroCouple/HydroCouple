"""
Tests for the HydroCouple component loader.

Verifies that the loader is importable and correctly rejects
non-existent library paths.
"""

import pytest


class TestComponentLoader:
    def test_loader_importable(self):
        from hydrocouple.loader import load
        from _hydrocouple._core import load_component, LoadedLibrary

        assert callable(load)
        assert callable(load_component)

    def test_loader_rejects_missing_library(self):
        from hydrocouple.loader import load

        with pytest.raises(OSError, match="Cannot load library"):
            load("/nonexistent/libfoo.so")
