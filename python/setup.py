"""
Build script for the HydroCouple Python bindings.

Compiles the Cython extension modules in ``_hydrocouple/`` that bridge
the C++ v2.0.0 header-only HydroCouple interfaces to Python. The package
version is single-sourced from the repository's ``include/version.h``.
"""

import os
import re

import numpy as np
from Cython.Build import cythonize
from setuptools import Extension, setup

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
HERE = os.path.dirname(os.path.abspath(__file__))
HYDROCOUPLE_INCLUDE = os.path.normpath(os.path.join(HERE, "..", "include"))
CYTHON_INCLUDE = os.path.join(HERE, "_hydrocouple", "include")

INCLUDE_DIRS = [
    HYDROCOUPLE_INCLUDE,
    CYTHON_INCLUDE,
    np.get_include(),
]


# ---------------------------------------------------------------------------
# Version — single-sourced from the repository, converted to PEP 440
# ---------------------------------------------------------------------------
def _semver_to_pep440(version: str) -> str:
    """Convert a semver pre-release (2.0.0-alpha.1) to PEP 440 (2.0.0a1)."""
    return (version
            .replace("-alpha.", "a")
            .replace("-beta.", "b")
            .replace("-rc.", "rc"))


def _read_version() -> str:
    # CMakeLists.txt is the authoritative, always-in-repo source
    # (the generated version.h is gitignored and may be stale).
    cmake_lists = os.path.normpath(
        os.path.join(HERE, "..", "CMakeLists.txt"))
    try:
        text = open(cmake_lists, encoding="utf-8").read()
        base = re.search(r'^\s*VERSION\s+([0-9.]+)\s*$', text, re.MULTILINE)
        suffix = re.search(
            r'set\(HYDROCOUPLE_VERSION_SUFFIX\s+"([^"]*)"\)', text)
        if base:
            return _semver_to_pep440(
                base.group(1) + (suffix.group(1) if suffix else ""))
    except OSError:
        pass
    # Fallback for sdist layouts without the repository root.
    version_h = os.path.join(HYDROCOUPLE_INCLUDE, "version.h")
    try:
        with open(version_h, encoding="utf-8") as fh:
            match = re.search(r'#define\s+PROJECT_VERSION\s+"([^"@]+)"',
                              fh.read())
            if match:
                return _semver_to_pep440(match.group(1))
    except OSError:
        pass
    return "2.0.0a1"


# ---------------------------------------------------------------------------
# Compiler flags
# ---------------------------------------------------------------------------
if os.name == "nt":
    EXTRA_COMPILE = ["/std:c++20", "/EHsc"]
    EXTRA_LINK = []
else:
    EXTRA_COMPILE = ["-std=c++20"]
    EXTRA_LINK = []

DEFINE_MACROS = [("NPY_NO_DEPRECATED_API", "NPY_1_7_API_VERSION")]

# ---------------------------------------------------------------------------
# Extension definitions
# ---------------------------------------------------------------------------


def _ext(name: str) -> Extension:
    return Extension(
        f"_hydrocouple.{name}",
        sources=[os.path.join("_hydrocouple", f"{name}.pyx")],
        include_dirs=INCLUDE_DIRS,
        define_macros=DEFINE_MACROS,
        language="c++",
        extra_compile_args=EXTRA_COMPILE,
        extra_link_args=EXTRA_LINK,
    )


EXTENSIONS = [
    _ext("_core"),
    _ext("_temporal"),
    _ext("_spatial"),
    _ext("_spatiotemporal"),
    _ext("_testing"),
]

setup(
    version=_read_version(),
    ext_modules=cythonize(
        EXTENSIONS,
        language_level=3,
        include_path=[HERE],
    ),
)
