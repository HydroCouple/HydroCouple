# Plans — the single home for HydroCouple-ecosystem plans

**Convention (from 2026-09-28):** every plan, design note, hand-off and
verification report for the HydroCouple ecosystem is written here, in the
HydroCouple repository's `plans/` folder — not in the SDK's, the
Composer's or FVQual's. New documents go here from now on. The folder is
**not tracked by git** (by choice); back it up with the rest of the
working tree.

Layout: one subfolder per project, and every plan, design note, hand-off
and verification report goes into the subfolder of the project whose code
it changes.

| Where | What |
|---|---|
| `hydrocouple/` | The HydroCouple interface and its Python bindings — including the differentiable-interface / AI plan and the G0/G1 hand-off |
| `sdk/` | HydroCoupleSDK — including the G2.x hand-offs (the SDK's reverse engine) |
| `composer/` | HydroCoupleComposer plans, Phase U hand-offs, build/verify rounds |
| `fvqual/` | FVQual plans and status |

A programme that spans projects (the differentiable-interface series does)
keeps its plan with the project that defines it (`hydrocouple/`) and each
hand-off with the project it changed.

Verification *harnesses* (falsifier scripts, logs, Mac verification
records) stay in each repo's `verification/` folder, next to the code they
mutate.

## The differentiable-interface series

| Document | What it is |
|---|---|
| `hydrocouple/DIFFERENTIABLE_INTERFACE_ALTERNATIVES_2026-09-28.md` | The alternatives plan (rev 3): contract in the interface, LibTorch as an optional engine, phases G0–G5 with status table |
| `hydrocouple/G0_G1_HANDOFF_2026-09-28.md` | DLPack, the derivative contract, torch/JAX overlays — Mac-verified (record in `HydroCouple/verification/g0g1/`) |
| `sdk/G2_1_HANDOFF_2026-09-28.md` | The Torch-free reverse engine in the SDK (`DifferentiableWorkflow`) — Mac-verified (record in `HydroCoupleSDK/verification/g2/`) |
| `sdk/G2_2_HANDOFF_2026-09-29.md` | Logarithmic checkpointing for that engine (O(log N) memory, identical gradients) |
| `hydrocouple/RELEASE_STATE_HANDOFF_2026-09-29.md` | `ICheckpointableModelComponent::releaseState` (ABI 3): interface, bindings and the SDK engine release every dropped checkpoint |
| `sdk/G2_3_HANDOFF_2026-09-29.md` | The optional LibTorch layer: ATen-native components differentiable by construction, a composition as one LibTorch autograd node, `rewind()` |
| `hydrocouple/G2_4_STATEFUL_ADAPTERS_PLAN_2026-09-29.md` | Gradients through stateful adapters: plan, and its outcome (implemented 2026-10-02 inside ABI 4) |
| `sdk/G2_4_HANDOFF_2026-10-02.md` | G2.4 and the v2.0.0-alpha.2 release: what changed, the Mac checklist, commits and tags |
| `hydrocouple/ABI4_MAC_VERIFICATION_HANDOFF_2026-09-29.md` | Instructions for a Mac agent: set up the `hydrocouple` conda env, verify the ABI 4 round and SDK port, fix within limits, commit, report |

Cross-references inside the moved documents were rewritten to these
paths. Older text may still say `plans/<file>` meaning the originating
repository's folder: look in the matching subfolder here.
