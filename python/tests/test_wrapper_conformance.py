"""
Every C++ wrapper implements the ABC it is registered as.

``ABC.register(Wrapper)`` is a promise: ``isinstance(w, IThing)`` turns true
and code written against ``IThing`` will call whatever ``IThing`` declares.
Python checks none of it -- registration bypasses the abstract-method check a
subclass would get -- so a wrapper missing ``time_kind`` passes every
``isinstance`` test and fails at the first attribute access, far from the
cause. This test is the check registration does not make.

Two things are compared for every abstract member of the ABC (including the
ones it inherits):

- the wrapper has it, and
- it is the same *kind* of member: a property where the ABC declares a
  property, a method where it declares a method. ``w.layer_count()`` on an
  int property fails just as surely as a missing name.
"""

import inspect

import pytest

_core = pytest.importorskip("_hydrocouple._core")
_temporal = pytest.importorskip("_hydrocouple._temporal")
_spatial = pytest.importorskip("_hydrocouple._spatial")
_spatiotemporal = pytest.importorskip("_hydrocouple._spatiotemporal")

REGISTRATIONS = [pair
                 for module in (_core, _temporal, _spatial, _spatiotemporal)
                 for pair in module.ABC_REGISTRATIONS]


def _declared(abc_class, name):
    """The ABC's own declaration of ``name``, wherever in its MRO it is."""
    for klass in abc_class.__mro__:
        if name in vars(klass):
            return vars(klass)[name]
    raise AssertionError(f"{abc_class.__name__}.{name} not found")


def _is_property(member) -> bool:
    return isinstance(member, property) or inspect.isgetsetdescriptor(member) \
        or inspect.isdatadescriptor(member) and not callable(member)


def _wrapper_member(wrapper, name):
    for klass in wrapper.__mro__:
        if name in vars(klass):
            return vars(klass)[name]
    return None


def test_every_module_registers_something():
    for module in (_core, _temporal, _spatial, _spatiotemporal):
        assert module.ABC_REGISTRATIONS, module.__name__


@pytest.mark.parametrize(
    "abc_class,wrapper", REGISTRATIONS,
    ids=[f"{w.__name__}-as-{a.__name__}" for a, w in REGISTRATIONS])
def test_wrapper_implements_its_abc(abc_class, wrapper):
    missing, wrong_kind = [], []
    for name in sorted(abc_class.__abstractmethods__):
        member = _wrapper_member(wrapper, name)
        if member is None:
            missing.append(name)
            continue
        wants_property = _is_property(_declared(abc_class, name))
        if wants_property != _is_property(member):
            wrong_kind.append(
                f"{name} ({'property' if wants_property else 'method'} "
                f"in the ABC)")
    assert not missing and not wrong_kind, (
        f"{wrapper.__name__} is registered as {abc_class.__name__} but "
        f"lacks {missing}" + (f"; wrong kind: {wrong_kind}" if wrong_kind else ""))
