"""
The vocabulary under docs/vocabulary/ is generated from the C++ headers by
tools/generate_vocabulary.py. These tests keep the committed files equal to
what the headers produce, and check that the generator saw the whole standard
rather than whatever its parser happened to match.
"""

import os
import re
import sys

import pytest

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import generate_vocabulary as vocabulary  # noqa: E402

OUTPUTS = vocabulary.generate()
VOCABULARY_DIR = os.path.join(ROOT, "docs", "vocabulary")


@pytest.mark.parametrize("name", sorted(OUTPUTS))
def test_committed_file_matches_the_headers(name):
    path = os.path.join(VOCABULARY_DIR, name)
    with open(path, encoding="utf-8") as handle:
        committed = handle.read()
    assert committed == OUTPUTS[name], (
        f"docs/vocabulary/{name} is stale: run python tools/generate_vocabulary.py")


def _declared_in_headers(pattern):
    names = set()
    for header in vocabulary.HEADERS:
        with open(os.path.join(ROOT, "include", header), encoding="utf-8") as handle:
            text = handle.read()
        text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
        text = re.sub(r"//[^\n]*", "", text)
        names |= set(re.findall(pattern, text))
    return names


def test_every_interface_is_a_class():
    # A second, cruder reading of the headers: what it finds, the generator
    # must have found too.
    # ISignal/ISlot are the observer templates every interface inherits; the
    # vocabulary leaves that mechanism out on purpose.
    interfaces = {name for name in _declared_in_headers(r"\bclass\s+(I[A-Z]\w*)\b[^;{]*\{")}
    interfaces -= {"ISignal", "ISlot"}
    turtle = OUTPUTS["hydrocouple.ttl"]
    missing = sorted(n for n in interfaces if f"\nhc:{n}\n    a owl:Class" not in turtle)
    assert not missing, missing


def test_every_enumeration_is_a_concept_scheme():
    enums = _declared_in_headers(r"\benum\s+class\s+(\w+)")
    turtle = OUTPUTS["hydrocouple.ttl"]
    missing = sorted(n for n in enums
                     if f"\nhc:{n}\n    a owl:Class, skos:ConceptScheme" not in turtle)
    assert not missing, missing


def test_vocabulary_binding_and_lifecycle_are_present():
    turtle = OUTPUTS["hydrocouple.ttl"]
    for term in ("hc:IValueDefinition.standardName", "hc:IValueDefinition.conceptURI",
                 "hc:IUnit.symbol", "hc:IUnit.ucumCode", "hc:WorkflowStatus.Invalid"):
        assert f"\n{term}\n" in turtle, term
    assert "hc:mayTransitionTo hc:WorkflowStatus.Validating" in turtle


def test_every_term_resolves_to_an_anchor():
    page = OUTPUTS["index.html"]
    anchors = set(re.findall(r'id="([^"]+)"', page))
    terms = set(re.findall(r"^hc:(\S+)$", OUTPUTS["hydrocouple.ttl"], re.MULTILINE))
    assert terms and not terms - anchors, sorted(terms - anchors)[:10]


def test_turtle_and_jsonld_are_the_same_graph():
    rdflib = pytest.importorskip("rdflib")
    from rdflib.compare import isomorphic

    turtle = rdflib.Graph().parse(data=OUTPUTS["hydrocouple.ttl"], format="turtle")
    jsonld = rdflib.Graph().parse(data=OUTPUTS["hydrocouple.jsonld"], format="json-ld")
    assert len(turtle) > 1000
    assert isomorphic(turtle, jsonld)
