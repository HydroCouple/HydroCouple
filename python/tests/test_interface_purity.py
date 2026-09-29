"""
The interface stays header-only and depends on the C++ standard library
alone -- the invariant the differentiable-interface plan is built around
(plans/hydrocouple/DIFFERENTIABLE_INTERFACE_ALTERNATIVES_2026-09-28.md, "Two
invariants"). DLPack, PyTorch, JAX and Python.h belong to the bindings
and the SDK, never to include/.
"""

import re
import shutil
import subprocess
from pathlib import Path

import pytest

INCLUDE_DIR = Path(__file__).parent.parent.parent / "include"
HEADERS = sorted(INCLUDE_DIR.glob("*.h"))

# A standard header (<cstdint>, <span>, <unordered_map>, the C-compat
# <stdint.h>) or a sibling interface header.
ALLOWED = re.compile(
    r'#\s*include\s*(<[a-z_]+(\.h)?>|"hydrocouple[a-z]*\.h"|"version\.h")\s*$')
STANDARD_C_COMPAT = {"<stdint.h>", "<stddef.h>"}


def include_lines(header: Path):
    for number, line in enumerate(header.read_text().splitlines(), 1):
        if re.match(r"\s*#\s*include\b", line):
            yield number, line.strip()


def test_the_headers_are_there():
    names = {h.name for h in HEADERS}
    assert "hydrocouple.h" in names and len(HEADERS) >= 7


@pytest.mark.parametrize("header", HEADERS, ids=lambda h: h.name)
def test_every_include_is_standard_or_a_sibling(header):
    offenders = []
    for number, line in include_lines(header):
        match = ALLOWED.match(line)
        if not match:
            offenders.append(f"{header.name}:{number}: {line}")
            continue
        angle = match.group(1)
        if angle.startswith("<") and angle.endswith(".h>") \
                and angle not in STANDARD_C_COMPAT:
            offenders.append(f"{header.name}:{number}: {line}")
    assert not offenders, "non-standard include in the interface:\n" + \
        "\n".join(offenders)


def test_the_interface_compiles_with_nothing_but_itself(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++") or shutil.which("c++")
    if compiler is None:
        pytest.skip("no C++ compiler available")
    tu = tmp_path / "alone.cpp"
    tu.write_text("".join(f'#include "{h.name}"\n' for h in HEADERS
                          if h.name != "version.h")
                  + "int main() { return 0; }\n")
    result = subprocess.run(
        [compiler, "-std=c++20", "-fsyntax-only", f"-I{INCLUDE_DIR}", str(tu)],
        capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


def test_the_differentiation_contract_is_in_the_interface():
    text = (INCLUDE_DIR / "hydrocouple.h").read_text()
    for name in ("class IDifferentiableModelComponent",
                 "class IDifferentiableAdaptedOutput",
                 "struct DifferentialEntry",
                 "using DifferentialSet = std::span<const DifferentialEntry>",
                 "enum class DifferentialRole",
                 "Differentiable        //!<"):
        assert name in text, name
