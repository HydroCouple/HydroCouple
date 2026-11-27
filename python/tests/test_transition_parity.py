"""
The lifecycle tables exist twice: normatively in hydrocouplehelpers.h and
again in hydrocouple/helpers.py. The Python copy drifted from the C++ one
for a whole ABI round without any test noticing, so this compiles the C++
tables and compares every (from, to) pair against the Python functions.
"""

import shutil
import subprocess
from pathlib import Path

import pytest

from hydrocouple import helpers
from hydrocouple.core import ComponentStatus, WorkflowStatus

INCLUDE_DIR = Path(__file__).parent.parent.parent / "include"

_PROGRAM = r"""
#include "hydrocouplehelpers.h"
#include <cstdio>
using namespace HydroCouple;
int main()
{
    for (int f = 0; f < %(nc)d; ++f)
        for (int t = 0; t < %(nc)d; ++t)
            std::printf("C %%d %%d %%d\n", f, t,
                Helpers::isValidComponentStatusTransition(
                    static_cast<IModelComponent::ComponentStatus>(f),
                    static_cast<IModelComponent::ComponentStatus>(t)) ? 1 : 0);
    for (int f = 0; f < %(nw)d; ++f)
        for (int t = 0; t < %(nw)d; ++t)
            std::printf("W %%d %%d %%d\n", f, t,
                Helpers::isValidWorkflowStatusTransition(
                    static_cast<IWorkflowComponent::WorkflowStatus>(f),
                    static_cast<IWorkflowComponent::WorkflowStatus>(t)) ? 1 : 0);
    return 0;
}
"""


@pytest.fixture(scope="module")
def cpp_tables(tmp_path_factory):
    compiler = shutil.which("g++") or shutil.which("clang++") or shutil.which("c++")
    if compiler is None:
        pytest.skip("no C++ compiler available")
    work = tmp_path_factory.mktemp("parity")
    source = work / "tables.cpp"
    source.write_text(_PROGRAM % {"nc": len(ComponentStatus),
                                  "nw": len(WorkflowStatus)})
    binary = work / "tables"
    build = subprocess.run([compiler, "-std=c++20", f"-I{INCLUDE_DIR}",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert build.returncode == 0, build.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True,
                         check=True)
    tables = {"C": {}, "W": {}}
    for line in run.stdout.splitlines():
        which, f, t, legal = line.split()
        tables[which][(int(f), int(t))] = legal == "1"
    return tables


def test_component_table_matches_cpp(cpp_tables):
    mismatches = [
        (f.name, t.name, cpp_tables["C"][(f.value, t.value)])
        for f in ComponentStatus for t in ComponentStatus
        if helpers.is_valid_component_status_transition(f, t)
        != cpp_tables["C"][(f.value, t.value)]]
    assert not mismatches, f"(from, to, C++ says) differ: {mismatches}"


def test_workflow_table_matches_cpp(cpp_tables):
    mismatches = [
        (f.name, t.name, cpp_tables["W"][(f.value, t.value)])
        for f in WorkflowStatus for t in WorkflowStatus
        if helpers.is_valid_workflow_status_transition(f, t)
        != cpp_tables["W"][(f.value, t.value)]]
    assert not mismatches, f"(from, to, C++ says) differ: {mismatches}"
