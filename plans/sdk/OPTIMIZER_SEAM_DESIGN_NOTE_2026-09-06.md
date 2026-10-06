# Optimizer seam — CalibrationWorkflowComponent design note (ARGGEN P7)

**Status:** DESIGN NOTE ONLY (user decision 2026-09-03: no optimizer code
this program) · **Date:** 2026-09-06 · **Author:** Caleb Buahin (with
Claude) · **Grounded against:** HydroCouple `dev` (hydrocouple.h as of
`bef95cb`), HydroCoupleSDK `removing_qt` (through `46a164a`+),
HydroCoupleComposer `v2` (through `a3174ca`), ARGGEN P1-P5 as executed.

## Problem

Calibration/optimization runs the SAME composition many times against a
parameter set the optimizer chooses, scores each run, and picks the next
set. Today every piece of substrate exists EXCEPT the orchestrator:

- `ICloneableModelComponent` (hydrocouple.h:981) — deep clone including
  arguments; contract already states clones write outputs to a DIFFERENT
  location, cloning happens only after the parent initialized, and clones
  must themselves be initialized. That is exactly a per-trial simulator.
- `@from` bindings + `bindingStages()` (spec v1.1, S1.1/S1.2) — a value
  produced by one component feeding another component's ARGUMENT before it
  initializes. That is exactly "parameter set into a clone".
- Run manifests + `ExecutionMode::Open` (P5-proven) — a trial's recorded
  results reopen with no model library, which is how the objective can be
  evaluated after the fact and how a finished calibration republishes its
  best run.
- `IWorkflowComponent` / `IWorkflowComponentInfo` (hydrocouple.h:2176,
  2198) — workflows are already interface-level components with an info
  type, so a CALIBRATION workflow can ship as a plugin like any model.

## Architecture

One new SDK class (a future program's deliverable), no interface changes
beyond the two prerequisites below:

```
CalibrationWorkflowComponent : AbstractWorkflowComponent
  roles (assigned by component id, recorded in the workflow block):
    "simulator"  — an ICloneableModelComponent; cloned per trial
    "objective"  — consumes simulator outputs, serves a scalar "score"
    "sampler"    — serves a parameter-set output per trial; consumes the
                   score; declares Done when converged
```

Per trial: sampler.update() → emits parameter outputs → the workflow
clones the simulator (`clone()` with a per-trial subdirectory in the
clone-optional arguments; contract hydrocouple.h:1000), applies the
parameter outputs to the clone's arguments through the SAME
`IArgument::initialize(const IComponentDataItem&)` pathway `@from` uses
(no new value-transport machinery), initializes the clone, drives it to
Done/Finished with the inner strategy (time-stepped for FVQual-class
simulators), lets the objective read the clone's outputs, hands the score
back to the sampler, disposes or archives the clone. The sampler's Done
ends the workflow; its best-trial output is the result.

### Document shape (schema sketch — additive, v1.x)

```json
"workflow": {
  "strategy": "calibration",
  "roles": { "simulator": "basin", "objective": "nse", "sampler": "dds" },
  "inner_strategy": "time_stepped",
  "max_trials": 200,
  "record_trials": "best"        // none | best | all
}
```

`workflowStrategyFromName` grows one spelling; everything else is data the
host already round-trips. Roles name COMPONENT IDS — the same identity
discipline connections use, so `bindingStages()`/validation extend
naturally (a role naming an undeclared component is refused at parse, the
way connection endpoints are).

### Why roles, not conventions

The sampler and objective are ordinary model components (a NSE objective
is just a component with inputs and one scalar output) — so they load
through the SAME plugin loader, palette, and configurator as everything
else, and a calibration document differs from a plain one only in its
workflow block. No "optimizer framework": the optimizer is a component.

## Prerequisites (each its own small slice, before any optimizer code)

1. **`AbstractWorkflowComponentInfo`** — the SDK ships
   `AbstractModelComponentInfo` and
   `AbstractAdaptedOutputFactoryComponentInfo`, but NO workflow-info base
   (component/ has only abstractmodelcomponentinfo.h). Mirror the model
   one: identity + caption + validation boilerplate, so a workflow plugin
   is one subclass + `HYDROCOUPLE_DECLARE_COMPONENT`.
2. **`IWorkflowComponentInfo::createComponentInstance()` returns a RAW
   pointer** (hydrocouple.h:2189) — the same pre-modern signature
   `IAdaptedOutputFactoryComponentInfo` had before A0 moved it to
   `std::unique_ptr`. Amend identically, while there are still zero
   implementers (the A0 window argument applies verbatim).
3. **Registry/palette kind** — Composer's
   `ComponentRegistry::ComponentKind{Model, AdapterFactory, Other}`
   (B1) classifies a workflow info as `Other` today. Add `Workflow` +
   `createWorkflowComponent()`, palette section "Workflows", and refuse
   canvas drops the way adapter factories are refused (they attach to the
   document's workflow block, not to the canvas).
4. **SimulationManager strategy dispatch** — `start()` currently builds
   the built-in workflows from `WorkflowSpec.strategy`; a registry-loaded
   strategy name must win when a workflow plugin claims it (component-info
   factories shadowing standalone ones is the precedent — A4's documented
   lookup order).
5. **Clone-aware run recording** — P5's finding stands: manifests catalog
   by instance id(), and ONE document id ("basin") now maps to MANY clone
   instances. Decide the catalog key up front: document id + trial ordinal
   (`basin#17`, matching the adapter-instance `#ordinal` convention from
   A2) — never bare instance ids.

## Open questions (for the future program's AskUserQuestion round)

- Parallel trials: `clone()` is synchronous and the contract is silent on
  concurrent clones of one parent. Serial trials first; parallelism needs
  a thread-safety statement in the interface docs before anything else.
- Trial persistence default (`record_trials`): "best" balances
  reviewability (CLAUDE.md §4.1) against disk; "all" is the reproducible-
  research mode.
- Whether the sampler's parameter output feeds the clone by ARGUMENT ID
  convention (output id == argument id) or by an explicit mapping table in
  the workflow block. The mapping table is more typing but survives
  components whose argument ids the sampler cannot know; lean mapping
  table, authored by the Composer UI.
- Objective over recorded-vs-live outputs: reading the clone LIVE avoids
  manifest churn per trial, but scoring a REOPENED manifest reuses the
  P5-proven pathway and makes trials individually inspectable. Lean live
  for speed, with `record_trials` opting into manifests.

## Out of scope (this note records, nothing more)

No CalibrationWorkflowComponent implementation, no sampler/objective
components, no schema change, no Composer UI. The next program picks this
note up whole.
