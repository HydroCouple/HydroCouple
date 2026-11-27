"""
Build script for the HydroCouple Python bindings.

Compiles the Cython extension modules in ``_hydrocouple/`` that bridge
the C++ v2.0.0 header-only HydroCouple interfaces to Python. The package
version is single-sourced from the repository's ``CMakeLists.txt``.

In the repository the interface headers are read from ``../include``. An sdist
carries its own copy in ``_hcinclude/`` (with the version in
``_hcinclude/hydrocouple_version.txt``), written by the ``sdist`` command below,
so a wheel can be built from the sdist alone. (Not ``VERSION``: in an include
directory on a case-insensitive filesystem that file would shadow ``<version>``.)
"""

import glob
import os
import re
import shutil

import numpy as np
from Cython.Build import cythonize
from setuptools import Extension, setup
from setuptools.command.sdist import sdist as _sdist

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
HERE = os.path.dirname(os.path.abspath(__file__))
REPOSITORY_INCLUDE = os.path.normpath(os.path.join(HERE, "..", "include"))
VENDORED_INCLUDE = os.path.join(HERE, "_hcinclude")
HYDROCOUPLE_INCLUDE = (REPOSITORY_INCLUDE
                       if os.path.exists(os.path.join(REPOSITORY_INCLUDE, "hydrocouple.h"))
                       else VENDORED_INCLUDE)
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
    # (the generated hydrocoupleversion.h is gitignored and may be stale).
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
    # An sdist has no repository root: its sdist step recorded the version.
    try:
        with open(os.path.join(VENDORED_INCLUDE, "hydrocouple_version.txt"),
                  encoding="utf-8") as fh:
            return fh.read().strip()
    except OSError:
        pass
    raise RuntimeError("cannot determine the HydroCouple version: neither "
                       "../CMakeLists.txt nor _hcinclude/hydrocouple_version.txt is present")


class sdist(_sdist):
    """Vendor the interface headers and the version, so the sdist builds alone."""

    def run(self):
        if os.path.exists(os.path.join(REPOSITORY_INCLUDE, "hydrocouple.h")):
            shutil.rmtree(VENDORED_INCLUDE, ignore_errors=True)
            os.makedirs(VENDORED_INCLUDE)
            # The interface headers only (a stale generated header may sit beside them).
            for header in glob.glob(os.path.join(REPOSITORY_INCLUDE, "hydrocouple*.h")):
                shutil.copy2(header, VENDORED_INCLUDE)
            with open(os.path.join(VENDORED_INCLUDE, "hydrocouple_version.txt"), "w",
                      encoding="utf-8") as fh:
                fh.write(_read_version() + "\n")
        super().run()


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
    cmdclass={"sdist": sdist},
    ext_modules=cythonize(
        EXTENSIONS,
        language_level=3,
        include_path=[HERE],
    ),
)
