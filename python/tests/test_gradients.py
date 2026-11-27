"""
Gradients cross model boundaries: Phases G1 (the contract) and G3-lite
(the PyTorch and JAX overlays).

The native fixture ``cpp_component/reservoir_component.cpp`` is a C++
component with a hand-written adjoint. The oracle for every gate is
**central finite differences through forward runs only** -- a path that
never calls ``vjp`` or ``jvp`` -- so an adjoint that is wrong cannot agree
with it by construction.

The flagship (``TestTheChainTrains*``) is the question the plan was asked:
a PyTorch (or JAX) network produces the inflow of one C++ reservoir, whose
outflow is the inflow of a second C++ reservoir, whose outflow feeds a
torch/JAX output head; the loss is a misfit summed over six time steps.
One ``loss.backward()`` / ``jax.grad`` must deliver, and FD must confirm,
gradients for:

* the network's weights -- upstream of *two* C++ models;
* each reservoir's own parameter ``k``;
* the head's scale -- downstream of both;
* the first reservoir's initial storage -- which only reaches the loss
  *through time*, via the state cotangent.

Falsifiers (``verification/g0g1/falsify.sh``) rebuild the fixture
with a deliberate adjoint fault and require these gates to fail.
"""

import os
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
import pytest

from hydrocouple.core import Capability, DifferentialRole as R

torch = pytest.importorskip("torch")

TESTS_DIR = Path(__file__).parent
INCLUDE_DIR = TESTS_DIR.parent.parent / "include"
CELLS = 4
STEPS = 6
FEATURES = 3


# ======================================================================
# Fixture: the differentiable C++ component
# ======================================================================
@pytest.fixture(scope="session")
def reservoir_lib(tmp_path_factory):
    compiler = shutil.which("g++") or shutil.which("clang++") or shutil.which("c++")
    if compiler is None:
        pytest.skip("no C++ compiler available")
    build_dir = tmp_path_factory.mktemp("reservoir")
    suffix = ".dylib" if sys.platform == "darwin" else ".so"
    lib = build_dir / f"libreservoir{suffix}"
    defines = shlex.split(os.environ.get("HC_FIXTURE_DEFINES", ""))
    cmd = [compiler, "-std=c++20", "-shared", "-fPIC", f"-I{INCLUDE_DIR}",
           *defines, str(TESTS_DIR / "cpp_component" / "reservoir_component.cpp"),
           "-o", str(lib)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        pytest.fail(f"fixture compile failed:\n{result.stderr}")
    return lib


_handles = []


def fresh(lib):
    """A new, prepared reservoir instance (storage 10, k 0.3)."""
    from hydrocouple.loader import load

    component, _info, handle = load(str(lib))
    _handles.append(handle)   # keep the library mapped for the session
    component.initialize()
    component.validate()
    component.prepare()
    return component


def items(component):
    return (component.differentiable_inputs()[0],
            component.differentiable_arguments()[0],
            component.differentiable_outputs()[0],
            component.differentiable_states()[0])


def whole(item, values):
    ok, msg = item.set_values_from(np.asarray(values, dtype=np.float64),
                                   (0,), (CELLS,))
    assert ok, msg


def read(item):
    out = np.empty(CELLS)
    ok, msg = item.get_values_into(out, (0,), (CELLS,))
    assert ok, msg
    return out


# ======================================================================
# G1: the contract, one step, against finite differences
# ======================================================================
RNG = np.random.default_rng(20260928)
S0 = np.array([10.0, 4.0, 7.5, 1.0])
I0 = np.array([1.0, 0.5, 2.0, 0.1])
K0 = np.array([0.3, 0.1, 0.45, 0.2])


def one_step(lib, s, i, k):
    """Forward only: (outflow, storage after) for one update from (s, i, k)."""
    c = fresh(lib)
    inflow, kk, outflow, storage = items(c)
    whole(storage, s)
    whole(inflow, i)
    whole(kk, k)
    c.update()
    return read(outflow), read(storage)


def fd_jacobian_product(lib, u_q, u_s, eps=1e-6):
    """Cotangent of (s, i, k) for seeds (u_q, u_s), by central differences."""
    base = [S0.copy(), I0.copy(), K0.copy()]
    grads = []
    for which in range(3):
        g = np.zeros(CELLS)
        for j in range(CELLS):
            plus = [b.copy() for b in base]
            minus = [b.copy() for b in base]
            plus[which][j] += eps
            minus[which][j] -= eps
            qp, sp = one_step(lib, *plus)
            qm, sm = one_step(lib, *minus)
            g[j] = (u_q @ (qp - qm) + u_s @ (sp - sm)) / (2 * eps)
        grads.append(g)
    return grads   # [s_bar, i_bar, k_bar]


def stepped(lib):
    c = fresh(lib)
    inflow, kk, outflow, storage = items(c)
    whole(storage, S0)
    whole(inflow, I0)
    whole(kk, K0)
    c.update()
    return c, (inflow, kk, outflow, storage)


class TestContractOneStep:
    def test_the_fixture_declares_the_capability(self, reservoir_lib):
        c = fresh(reservoir_lib)
        assert Capability.Differentiable in c.capabilities()
        assert [i.id for i in items(c)] == ["inflow", "k", "outflow", "storage"]

    def test_vjp_matches_finite_differences(self, reservoir_lib):
        u_q, u_s = RNG.normal(size=CELLS), RNG.normal(size=CELLS)
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        sb, ib, kb = np.zeros(CELLS), np.zeros(CELLS), np.zeros(CELLS)
        ok, msg = c.vjp([(outflow, R.Output, u_q), (storage, R.StateAfter, u_s)],
                        [(storage, R.StateBefore, sb), (inflow, R.Input, ib),
                         (kk, R.Argument, kb)])
        assert ok, msg
        fd_s, fd_i, fd_k = fd_jacobian_product(reservoir_lib, u_q, u_s)
        np.testing.assert_allclose(sb, fd_s, rtol=1e-6, atol=1e-9)
        np.testing.assert_allclose(ib, fd_i, rtol=1e-6, atol=1e-9)
        np.testing.assert_allclose(kb, fd_k, rtol=1e-6, atol=1e-9)

    def test_jvp_matches_finite_differences(self, reservoir_lib):
        ds, di, dk = (RNG.normal(size=CELLS) for _ in range(3))
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        dq, dsa = np.zeros(CELLS), np.zeros(CELLS)
        ok, msg = c.jvp([(storage, R.StateBefore, ds), (inflow, R.Input, di),
                         (kk, R.Argument, dk)],
                        [(outflow, R.Output, dq), (storage, R.StateAfter, dsa)])
        assert ok, msg
        eps = 1e-6
        qp, sp = one_step(reservoir_lib, S0 + eps * ds, I0 + eps * di, K0 + eps * dk)
        qm, sm = one_step(reservoir_lib, S0 - eps * ds, I0 - eps * di, K0 - eps * dk)
        np.testing.assert_allclose(dq, (qp - qm) / (2 * eps), rtol=1e-6, atol=1e-9)
        np.testing.assert_allclose(dsa, (sp - sm) / (2 * eps), rtol=1e-6, atol=1e-9)

    def test_vjp_and_jvp_are_adjoint(self, reservoir_lib):
        """<u, J v> == <J^T u, v>: the two products describe one Jacobian."""
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        u_q, u_s = RNG.normal(size=CELLS), RNG.normal(size=CELLS)
        v_s, v_i, v_k = (RNG.normal(size=CELLS) for _ in range(3))
        sb, ib, kb = np.zeros(CELLS), np.zeros(CELLS), np.zeros(CELLS)
        assert c.vjp([(outflow, R.Output, u_q), (storage, R.StateAfter, u_s)],
                     [(storage, R.StateBefore, sb), (inflow, R.Input, ib),
                      (kk, R.Argument, kb)])[0]
        dq, dsa = np.zeros(CELLS), np.zeros(CELLS)
        assert c.jvp([(storage, R.StateBefore, v_s), (inflow, R.Input, v_i),
                      (kk, R.Argument, v_k)],
                     [(outflow, R.Output, dq), (storage, R.StateAfter, dsa)])[0]
        lhs = u_q @ dq + u_s @ dsa
        rhs = sb @ v_s + ib @ v_i + kb @ v_k
        assert lhs == pytest.approx(rhs, rel=1e-12)

    def test_results_are_written_not_accumulated(self, reservoir_lib):
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        seeds = [(outflow, R.Output, np.ones(CELLS))]
        clean, dirty = np.zeros(CELLS), np.full(CELLS, 1e6)
        assert c.vjp(seeds, [(kk, R.Argument, clean)])[0]
        assert c.vjp(seeds, [(kk, R.Argument, dirty)])[0]
        np.testing.assert_array_equal(clean, dirty)

    def test_an_absent_seed_is_zero(self, reservoir_lib):
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        with_zero, without = np.zeros(CELLS), np.zeros(CELLS)
        assert c.vjp([(outflow, R.Output, np.ones(CELLS)),
                      (storage, R.StateAfter, np.zeros(CELLS))],
                     [(kk, R.Argument, with_zero)])[0]
        assert c.vjp([(outflow, R.Output, np.ones(CELLS))],
                     [(kk, R.Argument, without)])[0]
        np.testing.assert_array_equal(with_zero, without)

    def test_derivatives_do_not_move_the_component(self, reservoir_lib):
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        before = read(storage), read(outflow)
        assert c.vjp([(outflow, R.Output, np.ones(CELLS))],
                     [(kk, R.Argument, np.zeros(CELLS))])[0]
        assert c.jvp([(kk, R.Argument, np.ones(CELLS))],
                     [(outflow, R.Output, np.zeros(CELLS))])[0]
        np.testing.assert_array_equal(read(storage), before[0])
        np.testing.assert_array_equal(read(outflow), before[1])

    def test_a_wrong_role_is_refused(self, reservoir_lib):
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        ok, msg = c.vjp([(inflow, R.Output, np.ones(CELLS))],
                        [(kk, R.Argument, np.zeros(CELLS))])
        assert not ok and "does not differentiate" in msg
        ok, msg = c.vjp([(outflow, R.Output, np.ones(CELLS))],
                        [(outflow, R.Output, np.zeros(CELLS))])
        assert not ok and "does not differentiate" in msg

    def test_there_is_nothing_to_differentiate_before_a_step(self, reservoir_lib):
        c = fresh(reservoir_lib)
        _, kk, outflow, _ = items(c)
        ok, msg = c.vjp([(outflow, R.Output, np.ones(CELLS))],
                        [(kk, R.Argument, np.zeros(CELLS))])
        assert not ok and "update()" in msg

    def test_torch_buffers_serve_as_cotangents(self, reservoir_lib):
        c, (inflow, kk, outflow, storage) = stepped(reservoir_lib)
        seed = torch.ones(CELLS, dtype=torch.float64)
        via_torch = torch.zeros(CELLS, dtype=torch.float64)
        via_numpy = np.zeros(CELLS)
        assert c.vjp([(outflow, R.Output, seed)], [(kk, R.Argument, via_torch)])[0]
        assert c.vjp([(outflow, R.Output, seed.numpy())],
                     [(kk, R.Argument, via_numpy)])[0]
        np.testing.assert_array_equal(via_torch.numpy(), via_numpy)


# ======================================================================
# The flagship: torch net -> C++ -> C++ -> torch head, six steps
# ======================================================================
FORCING = RNG.normal(size=(STEPS, FEATURES))
OBSERVED = np.abs(RNG.normal(size=(STEPS, CELLS))) * 0.5 + 0.5
W0 = RNG.normal(size=(CELLS, FEATURES)) * 0.3
B0 = RNG.normal(size=CELLS) * 0.1
KA0 = np.array([0.30, 0.25, 0.40, 0.15])
KB0 = np.array([0.20, 0.35, 0.10, 0.30])
SCALE0 = np.array([1.3])
SA0 = np.array([10.0, 4.0, 7.5, 1.0])

PARAMS = ("W", "b", "kA", "kB", "scale", "sA0")


def _params_numpy():
    return {"W": W0.copy(), "b": B0.copy(), "kA": KA0.copy(),
            "kB": KB0.copy(), "scale": SCALE0.copy(), "sA0": SA0.copy()}


def chain_loss_numpy_forward(lib, p):
    """The chain's loss by forward runs only (no overlay, no vjp)."""
    a, b = fresh(lib), fresh(lib)
    ia, ka, qa, sa = items(a)
    ib, kb, qb, sb = items(b)
    whole(sa, p["sA0"])
    loss = 0.0
    for t in range(STEPS):
        z = p["W"] @ FORCING[t] + p["b"]
        inflow = np.logaddexp(0.0, z)                 # softplus
        whole(ia, inflow)
        whole(ka, p["kA"])
        a.update()
        whole(ib, read(qa))
        whole(kb, p["kB"])
        b.update()
        pred = read(qb) * p["scale"][0]
        loss += np.sum((pred - OBSERVED[t]) ** 2)
    return loss


def finite_difference_gradients(lib, eps=1e-6):
    grads = {}
    for name in PARAMS:
        base = _params_numpy()
        g = np.zeros_like(base[name])
        for j in np.ndindex(g.shape):
            plus, minus = _params_numpy(), _params_numpy()
            plus[name][j] += eps
            minus[name][j] -= eps
            g[j] = (chain_loss_numpy_forward(lib, plus)
                    - chain_loss_numpy_forward(lib, minus)) / (2 * eps)
        grads[name] = g
    return grads


@pytest.fixture(scope="module")
def fd_grads(reservoir_lib):
    return finite_difference_gradients(reservoir_lib)


def torch_chain(lib):
    import hydrocouple.torch as hct

    A, B = hct.Step(fresh(lib)), hct.Step(fresh(lib))
    t = lambda a: torch.tensor(a, dtype=torch.float64, requires_grad=True)
    p = {k: t(v) for k, v in _params_numpy().items()}
    state_a = (p["sA0"],)
    state_b = B.initial_state()
    loss = torch.zeros((), dtype=torch.float64)
    for step in range(STEPS):
        x = torch.tensor(FORCING[step], dtype=torch.float64)
        inflow = torch.nn.functional.softplus(p["W"] @ x + p["b"])
        (qa,), state_a = A(state_a, [inflow], [p["kA"]])
        (qb,), state_b = B(state_b, [qa], [p["kB"]])
        pred = qb * p["scale"]
        loss = loss + ((pred - torch.tensor(OBSERVED[step])) ** 2).sum()
    return loss, p, (A, B)


class TestTheChainTrainsInTorch:
    def test_the_loss_is_the_forward_models_loss(self, reservoir_lib):
        loss, _, _ = torch_chain(reservoir_lib)
        assert loss.item() == pytest.approx(
            chain_loss_numpy_forward(reservoir_lib, _params_numpy()), rel=1e-12)

    def test_every_gradient_matches_finite_differences(self, reservoir_lib,
                                                       fd_grads):
        loss, p, _ = torch_chain(reservoir_lib)
        loss.backward()
        for name in PARAMS:
            np.testing.assert_allclose(
                p[name].grad.numpy(), fd_grads[name], rtol=1e-5, atol=1e-7,
                err_msg=f"d loss / d {name}")

    def test_zero_is_not_a_gradient(self, reservoir_lib):
        """A dropped tape reads as 'no sensitivity'. Every parameter here is
        sensitive, so every gradient entry must be non-zero."""
        loss, p, _ = torch_chain(reservoir_lib)
        loss.backward()
        for name in PARAMS:
            assert np.all(np.abs(p[name].grad.numpy()) > 1e-8), name

    def test_the_network_upstream_of_two_cpp_models_learns(self, reservoir_lib):
        """Recover a network from data the coupled model generated with a
        different one: the only way the upstream weights can learn is
        through both C++ adjoints."""
        import hydrocouple.torch as hct

        W_true = W0 + np.random.default_rng(7).normal(size=W0.shape) * 0.5
        observed = np.zeros((STEPS, CELLS))
        a, b = fresh(reservoir_lib), fresh(reservoir_lib)
        ia, ka, qa, sa = items(a)
        ib, kb, qb, sb = items(b)
        whole(sa, SA0)
        for step in range(STEPS):
            whole(ia, np.logaddexp(0.0, W_true @ FORCING[step] + B0))
            whole(ka, KA0)
            a.update()
            whole(ib, read(qa))
            whole(kb, KB0)
            b.update()
            observed[step] = read(qb) * SCALE0[0]

        W = torch.tensor(W0, requires_grad=True)
        opt = torch.optim.Adam([W], lr=0.05)
        losses = []
        for epoch in range(40):
            A, B = hct.Step(fresh(reservoir_lib)), hct.Step(fresh(reservoir_lib))
            state_a = (torch.tensor(SA0),)
            state_b = B.initial_state()
            loss = torch.zeros((), dtype=torch.float64)
            for step in range(STEPS):
                x = torch.tensor(FORCING[step])
                inflow = torch.nn.functional.softplus(W @ x + torch.tensor(B0))
                (qa_t,), state_a = A(state_a, [inflow], [torch.tensor(KA0)])
                (qb_t,), state_b = B(state_b, [qa_t], [torch.tensor(KB0)])
                loss = loss + ((qb_t * SCALE0[0]
                                - torch.tensor(observed[step])) ** 2).sum()
            opt.zero_grad()
            loss.backward()
            opt.step()
            losses.append(loss.item())
        assert losses[-1] < 0.05 * losses[0], losses

    def test_backward_replays_superseded_steps(self, reservoir_lib):
        loss, _, (A, B) = torch_chain(reservoir_lib)
        clock_before = A.driver._clock, B.driver._clock
        loss.backward()
        # Every step but each component's last is replayed exactly once.
        assert A.driver._clock - clock_before[0] == STEPS - 1
        assert B.driver._clock - clock_before[1] == STEPS - 1

    def test_a_second_backward_agrees_with_the_first(self, reservoir_lib):
        loss, p, _ = torch_chain(reservoir_lib)
        loss.backward(retain_graph=True)
        first = {k: v.grad.clone() for k, v in p.items()}
        for v in p.values():
            v.grad = None
        loss.backward()
        for k in PARAMS:
            torch.testing.assert_close(p[k].grad, first[k], rtol=1e-12, atol=0)

    def test_without_checkpointing_only_the_last_step_differentiates(self, reservoir_lib):
        from hydrocouple.differentiable import DifferentiationError

        loss, _, (A, B) = torch_chain(reservoir_lib)
        A.driver.checkpointable = False
        B.driver.checkpointable = False
        with pytest.raises(DifferentiationError, match="Checkpointing"):
            loss.backward()


# ======================================================================
# The same chain in JAX; it must agree with FD and with PyTorch
# ======================================================================
@pytest.fixture(scope="module")
def jax_modules():
    jax = pytest.importorskip("jax")
    jax.config.update("jax_enable_x64", True)
    import jax.numpy as jnp
    return jax, jnp


def jax_chain_loss(lib, jax, jnp):
    import hydrocouple.jax as hcj

    A, B = hcj.Step(fresh(lib)), hcj.Step(fresh(lib))
    state_b0 = B.initial_state()

    def loss(p):
        state_a = (p["sA0"],)
        state_b = state_b0
        total = 0.0
        for step in range(STEPS):
            inflow = jax.nn.softplus(p["W"] @ FORCING[step] + p["b"])
            (qa,), state_a = A(state_a, [inflow], [p["kA"]])
            (qb,), state_b = B(state_b, [qa], [p["kB"]])
            total = total + jnp.sum((qb * p["scale"][0] - OBSERVED[step]) ** 2)
        return total

    return loss


class TestTheChainTrainsInJax:
    def test_every_gradient_matches_finite_differences(self, reservoir_lib,
                                                       fd_grads, jax_modules):
        jax, jnp = jax_modules
        loss = jax_chain_loss(reservoir_lib, jax, jnp)
        p = {k: jnp.asarray(v) for k, v in _params_numpy().items()}
        value, grads = jax.value_and_grad(loss)(p)
        assert float(value) == pytest.approx(
            chain_loss_numpy_forward(reservoir_lib, _params_numpy()), rel=1e-12)
        for name in PARAMS:
            np.testing.assert_allclose(np.asarray(grads[name]), fd_grads[name],
                                       rtol=1e-5, atol=1e-7,
                                       err_msg=f"d loss / d {name}")

    def test_jax_and_torch_agree(self, reservoir_lib, jax_modules):
        jax, jnp = jax_modules
        loss = jax_chain_loss(reservoir_lib, jax, jnp)
        p = {k: jnp.asarray(v) for k, v in _params_numpy().items()}
        jgrads = jax.grad(loss)(p)
        tloss, tp, _ = torch_chain(reservoir_lib)
        tloss.backward()
        for name in PARAMS:
            np.testing.assert_allclose(np.asarray(jgrads[name]),
                                       tp[name].grad.numpy(), rtol=1e-12,
                                       atol=1e-14, err_msg=name)

    def test_jit_preserves_step_order(self, reservoir_lib, jax_modules):
        """Under jit the callbacks are ordered by the state's data
        dependence; the gradient must equal the eager one."""
        jax, jnp = jax_modules
        p = {k: jnp.asarray(v) for k, v in _params_numpy().items()}
        eager = jax.grad(jax_chain_loss(reservoir_lib, jax, jnp))(p)
        jitted = jax.jit(jax.grad(jax_chain_loss(reservoir_lib, jax, jnp)))(p)
        for name in PARAMS:
            # XLA fuses and reassociates the JAX-side arithmetic, so the
            # last few bits differ; step order errors would be O(1).
            np.testing.assert_allclose(np.asarray(jitted[name]),
                                       np.asarray(eager[name]), rtol=1e-10,
                                       atol=1e-15, err_msg=name)


# ======================================================================
# Checkpoints are released, never leaked (ICheckpointable::releaseState)
# ======================================================================
def live_checkpoints(component) -> int:
    """The fixture's count of checkpoints saved and not yet released."""
    item = [r for r in component.results if r.id == "live_checkpoints"][0]
    out = np.empty(CELLS)
    ok, msg = item.get_values_into(out, (0,), (CELLS,))
    assert ok, msg
    return int(out[0])


class TestCheckpointsAreReleased:
    def test_a_released_token_is_dead(self, reservoir_lib):
        c = fresh(reservoir_lib)
        ok, token, msg = c.save_state()
        assert ok, msg
        assert live_checkpoints(c) == 1
        assert c.release_state(token) == (True, "")
        assert live_checkpoints(c) == 0
        ok, msg = c.restore_state(token)
        assert not ok and "released" in msg
        ok, msg = c.release_state(token)
        assert not ok and "already released" in msg

    def test_torch_releases_every_step_when_the_graph_goes(self, reservoir_lib):
        import gc

        loss, p, (A, B) = torch_chain(reservoir_lib)
        a, b = A.driver.component, B.driver.component
        assert live_checkpoints(a) == STEPS
        assert live_checkpoints(b) == STEPS
        loss.backward()
        del loss, p, A, B
        gc.collect()
        assert live_checkpoints(a) == 0
        assert live_checkpoints(b) == 0

    def test_jax_releases_every_step_once_backward_has_run(self, reservoir_lib,
                                                           jax_modules):
        import gc
        import hydrocouple.jax as hcj

        jax, jnp = jax_modules
        comp = fresh(reservoir_lib)
        step = hcj.Step(comp)

        def loss(k):
            state = step.initial_state()
            total = 0.0
            for n in range(STEPS):
                (q,), state = step(state, [jnp.full(CELLS, 1.0 + n)], [k])
                total = total + jnp.sum(q ** 2)
            return total

        jax.grad(loss)(jnp.full(CELLS, 0.3))
        gc.collect()
        assert live_checkpoints(comp) == 0
