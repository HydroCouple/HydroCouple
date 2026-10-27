"""
The examples run.

They are the first code a reader copies, and nothing ran them: both pure
Python examples stopped instantiating when ABI 4 added ``value_kind`` and
``states`` to the interfaces, and no test noticed. Each runs here as a
script, the way a reader runs it.
"""

import os
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

PYTHON_DIR = Path(__file__).resolve().parent.parent
EXAMPLES = PYTHON_DIR / "examples"
INCLUDE_DIR = PYTHON_DIR.parent / "include"


def _run(script, *args):
    env = dict(os.environ)
    env["PYTHONPATH"] = os.pathsep.join(
        [str(PYTHON_DIR)] + ([env["PYTHONPATH"]] if env.get("PYTHONPATH") else []))
    return subprocess.run([sys.executable, str(EXAMPLES / script), *args],
                          capture_output=True, text=True, timeout=120, env=env)


def test_coupled_python_models():
    result = _run("coupled_python_models.py")
    assert result.returncode == 0, result.stderr
    assert "Both couplings agree over every step and reach." in result.stdout


def test_sine_wave_component():
    result = _run("sine_wave_component.py")
    assert result.returncode == 0, result.stderr
    assert "final status: Finished" in result.stdout


def test_drive_cpp_component(tmp_path):
    pytest.importorskip("_hydrocouple._core")
    compiler = shutil.which("g++") or shutil.which("clang++") or shutil.which("c++")
    if compiler is None:
        pytest.skip("no C++ compiler available")
    suffix = ".dylib" if sys.platform == "darwin" else ".so"
    library = tmp_path / f"libcpp_sine{suffix}"
    source = PYTHON_DIR / "tests" / "cpp_component" / "test_component.cpp"
    built = subprocess.run(
        [compiler, "-std=c++20", "-shared", "-fPIC", f"-I{INCLUDE_DIR}",
         str(source), "-o", str(library)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = _run("drive_cpp_component.py", str(library))
    assert result.returncode == 0, result.stderr + result.stdout
