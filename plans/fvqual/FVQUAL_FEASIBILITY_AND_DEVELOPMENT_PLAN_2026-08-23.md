# FVQUAL — Feasibility Assessment & Development Plan

**A fully 3D, unstructured, finite-volume hydrodynamic and water-quality model — the HydroCouple-native successor to CE-QUAL-W2.**

- **Date:** 2026-08-23
- **Status:** DRAFT for review
- **Component id (proposed):** `org.hydrocouple.fvqual`
- **Repo:** `cbuahin_github/FVQUAL`
- **Conventions:** This document follows the house plan style (decision ids `D-F#`, phased work packages with verify criteria, status boxes ⬜/✅). It builds on — and does not replace — `HydroCouple/plans/sdk/SDK_MODERNIZATION_PLAN.md` and `openswmm.engine/plans/transport/UNIFIED_TRANSPORT_MASTER_PLAN.md`.

---

## 1. Executive Summary

CE-QUAL-W2 is the workhorse laterally averaged (2D longitudinal–vertical) reservoir/river hydrodynamics and water-quality model, but it is structurally limited: lateral averaging, structured segment×layer grids, serial Fortran with global module state, and a monolithic ~60-constituent kinetics module. FVQUAL replaces it with:

1. **A fully 3D hydrostatic finite-volume core** on an unstructured **tri-quad horizontal mesh** with **terrain-following sigma (hybrid sigma-z capable) vertical layers**, with an architectural seam for a later **non-hydrostatic pressure-correction module**.
2. **Explicit, Kokkos-portable kernels** using barotropic–baroclinic **mode splitting** (explicit subcycled external mode) plus **implicit vertical diffusion** (batched per-column tridiagonal) — with a decision gate against a semi-implicit free-surface alternative.
3. **Two transport engines behind one interface**: Eulerian FV with MUSCL reconstruction + Zalesak FCT (primary, strictly conservative), and a Lagrangian family (ELM semi-Lagrangian and RWPT particles) for large-time-step screening and diagnostics — mirroring the openswmm `QUALITY_SOLVER` pattern and its 3-engine cross-check gate.
4. **The openswmm multispecies reaction engine (MSX-convention `.rxn`)** reused as the kinetics subsystem. Its numerical core (`ReactionIntegrator`/`ReactionExpression`) is already mesh-agnostic and drops into a 3D cell loop with a thin new binding.
5. **UGRID-1.0/CF IO** via the HydroCoupleSDK writers (layered-2D convention, Delft3D-FM/xugrid-compatible), and a **HydroCouple 2.0 `IModelComponent`** wrapper so FVQUAL composes with openswmm, GW, and stream components — with the legacy `CEQUALW2Component` as a side-by-side validation partner.

**Feasibility verdict: high.** Nearly every enabling subsystem already exists in your ecosystem: sigma-grid extrusion (`SDK tools/sigmagrid.h` → `LayeredMesh`), UGRID NetCDF/HDF5 writers, a Kokkos device backend and GPU plugin ABI precedent, an explicit unstructured 2D FV solver with LTS, a separable reaction VM with four integrators, and MPI/halo infrastructure for later scale-out. The genuinely new work is the 3D hydrostatic core (mode splitting, baroclinic pressure gradient, turbulence closure) and the transport engines on the layered mesh. Estimated effort to a W2-comparable validated release: **~18–24 months of phased development**, with a usable hydrodynamic+temperature model at ~9–12 months.

---

## 2. Motivation & Scope

### 2.1 Why succeed CE-QUAL-W2

| W2 limitation | FVQUAL answer |
|---|---|
| Laterally averaged — cannot represent wind-driven gyres, lateral inflow plumes, wide/dendritic waterbodies | Fully 3D on unstructured tri-quad meshes that follow shoreline and dendritic arms |
| Structured segment×layer grid; branch bookkeeping | Single unstructured mesh; branches are just geometry |
| Serial Fortran, global COMMON/module state (the `CEQUALW2Component` wrapper must `dlopen` a fresh DLL per instance just to clone) | Modern C++20, instance-safe, thread-parallel, GPU-portable via Kokkos |
| Monolithic hard-coded kinetics | Declarative MSX-style `.rxn` kinetics compiled to bytecode; W2 parity as curated template libraries |
| Bespoke ASCII IO | UGRID/CF NetCDF+HDF5, GeoPackage; standard tooling (xarray/xugrid, QGIS, Paraview) |
| Standalone model | HydroCouple component: couples to openswmm networks, groundwater, stream temperature components |

### 2.2 Target applications

Reservoirs and lakes (stratification, eutrophication, DO management, selective withdrawal), river reaches and run-of-river impoundments, estuaries (salinity-capable equation of state), and coupled watershed–reservoir systems via HydroCouple composition.

### 2.3 Initial out-of-scope (deferred, tracked in §6)

Non-hydrostatic solver (seam only), MPI multi-domain (SDK `Distributed` layer exists; enable later), ice cover, CEMA-style sediment diagenesis (simple SOD + benthic species first), `.msx` file reader (use `.rxn` dialect), curvilinear-only workflows (tri-quad covers it).

---

## 3. Asset Inventory — What Already Exists

| Asset | Where | Reuse in FVQUAL |
|---|---|---|
| HydroCouple v2.0.0-alpha interface standard (C++17, no Qt) | `HydroCouple/include/hydrocouple*.h` | Component contract, exchange items, spatial/spatiotemporal data items |
| HydroCoupleSDK v2 (C++20, CMake+vcpkg, MIT) | `cbuahin_github/HydroCoupleSDK` | Base classes, data plane (`BufferDescriptor` hyperslabs), workflows |
| **Sigma-grid generator** — Song–Haidvogel stretching, min-thickness enforcement, hybrid z-sigma transition, extrusion of tri/quad meshes into layered prism/hex `LayeredMesh` (CSR cells) | SDK `tools/sigmagrid.h` | The FVQUAL mesh, nearly as-is |
| CDT triangulator, quad-dominant mesher, terrain sampler (IDW/GDAL) | SDK `tools/` | Mesh generation from bathymetry |
| **UGRID-1.0/CF-1.11 NetCDF + HDF5 writers**, IOThread (async, back-pressure), Snapshot, `MeshDefinition` | SDK `io/` | Output stack; extend with layer dimension + sigma formula terms |
| Kokkos `DeviceBackend`, buffer registry, space transfer | SDK `device/` | GPU abstraction (vendor-neutral, Kokkos backend #1) |
| MPI transport, `PartitionedDataItem`, `HaloExchanger`, flux+derivative payloads | SDK `distributed/` | Deferred scale-out path |
| **Multispecies reaction engine** — shunting-yard → RPN bytecode, allocation-free evaluator, EUL/RK5/ROS2/BDF2 + damped-Newton EQUIL, SoA hot/cold split | `openswmm.engine/src/engine/transport/components/ReactionModule/` | Kinetics subsystem (see §4.5) |
| Species registry, `[PROCESS_COMPONENTS]` config plumbing, `.rxn` format | openswmm engine | Species/kinds, config conventions |
| Transport numerics recipes: MUSCL/QUICKEST + **Zalesak FCT**, implicit tridiagonal dispersion, Lie splitting, donor-upwinded coupling tuples, 3-engine cross-check gate | openswmm ARD/LARD plans + code | Algorithms (not data structures) for the 3D Eulerian engine |
| Explicit unstructured 2D FV solver: SoA triangle mesh, unique-face fluxes, tiered LTS, wet/dry (VFR), Kokkos GPU **plugin ABI** (OMP/CUDA/HIP/SYCL shims, core never links Kokkos) | `openswmm.engine/src/engine/2d/` | Design template for the external (barotropic) mode + GPU packaging pattern |
| Heat-flux formulations (latent/sensible/radiative/sediment) | openswmm HEAT plan; `CSHComponent` | Surface thermodynamics module |
| FVHMComponent — 2D SWE on TIN, SIMPLE/SIMPLEC/PISO, TVD, HYPRE AMG, wet/dry | `HydroCouple/FVHMComponent` | Legacy reference only (Qt/C++11; implicit pressure-based approach informs the semi-implicit fallback) |
| **CEQUALW2Component + vendored W2 v4.1.0 Fortran + DeGray Reservoir / Columbia Slough examples** | `HydroCouple/CEQUALW2Component`, `HydroCouple/CE-QUAL-W2-4.1.0` | Side-by-side validation partner and test datasets |

**Known gap (interface):** the HydroCouple spatial standard has `IPolyhedralSurface`/`ITIN` (2D surface patches) and `IRegularGrid3D` (structured), but **no first-class layered-unstructured-mesh data item**. §5.5 proposes the extension.

---

## 4. Feasibility Assessments

### 4.1 Mesh & vertical coordinate — tri-quad horizontal + terrain-following sigma

**Feasible now; mostly built.** The SDK's `SigmaGridGenerator` already extrudes a 2D tri-quad mesh into a `LayeredMesh` of prisms/hexes with Song–Haidvogel stretching, per-column interface elevations, and minimum layer thickness in shallows — exactly the tri-quad + terrain-following-sigma target. A **hybrid z-sigma transition depth** is specified in the SDK plan (Phase 7b item 4) but not yet visible in `sigmagrid.h`; confirm or implement it in FVQUAL Phase 1 (F1.1).

Design points:

- **Indexing.** Treat the 3D mesh as *(2D face) × (layer)*: `cell(c,k) = c*nLayers + k` (column-major by layer). Column sweeps (vertical implicit solves, light attenuation, settling) become contiguous; horizontal face loops iterate per-layer over the fixed 2D edge list. This avoids general 3D polyhedral connectivity entirely — vertical neighbors are `±1`, horizontal neighbors come from the 2D topology. It is also the natural UGRID layered representation and the natural Kokkos layout.
- **Free surface & moving layers.** With sigma, layers stretch/compress with the free surface — no W2-style layer add/subtract logic. Costs: (a) thin-layer stiffness in shallow arms → min-thickness clamping + implicit vertical terms; (b) the **sigma pressure-gradient error** over steep bathymetry (§4.2 mitigation list); (c) wet/dry handled per column (VFR-style closure from the 2D solver generalizes).
- **Known sigma risks and mitigations:** enforce a hydrostatic-consistency (Haney-number) mesh check in the mesh pipeline; hybrid z-sigma below the transition depth for steep old river channels; density-Jacobian baroclinic gradient (§5.3).

**IO (UGRID).** UGRID-1.0 formally standardizes 2D mesh topology; the established practice for 3D layered models (Delft3D-FM, FVCOM outputs) is **2D topology + a vertical layer dimension + CF `ocean_sigma_coordinate`/`ocean_s_coordinate` formula terms**, which xugrid/ugrid-checks understand. Full 3D volume topology exists in the spec but has poor tool support. **Decision D-F2: use the layered-2D UGRID convention.** The SDK writers already emit CF-1.11 UGRID-1.0 with `mesh`/`location` tagging; the extension is a layer dimension, interface/center distinction, and sigma formula-term variables. Input side: implement `UGRIDMeshReader` (already designed in SDK `IMPLEMENTATION_PLAN.md` §18 but writer-only today).

### 4.2 Hydrostatic vs non-hydrostatic

**Recommendation (D-F3): hydrostatic core with a designed-in non-hydrostatic seam; non-hydrostatic as a later optional module.**

Rationale:

- **Where hydrostatics suffices.** W2, ELCOM/AEM3D, EFDC, Delft3D, FVCOM — the entire reservoir/estuary WQ modeling tradition — are hydrostatic. Seasonal stratification, wind setup, differential heating, density-inflow plunging (at grid-resolvable scales), selective withdrawal, basin-scale internal seiches: all hydrostatic-valid because horizontal scales ≫ vertical scales.
- **Where it fails.** Non-hydrostatic pressure matters when horizontal and vertical scales converge: nonlinear internal solitary waves and their breaking, Kelvin–Helmholtz billows, convective plumes (destratification bubblers), flow over steep sills at high resolution. The literature comparison is consistent: hydrostatic models cannot represent the nonlinear–dispersive balance of internal solitons but capture basin-scale dynamics well.
- **Cost.** Non-hydrostatic requires prognostic vertical momentum plus a 3D elliptic pressure-Poisson solve each step — typically the dominant runtime cost, needing an AMG-preconditioned Krylov solver. That directly conflicts with the "explicit, dependency-light, Kokkos-first" design for v1. σ-coordinate non-hydrostatic formulations are proven feasible (pressure defined directly in σ gives results equivalent to z-defined pressure), so nothing about the mesh choice forecloses it.
- **The seam (cheap now, expensive to retrofit):** (a) carry vertical velocity `w` as a full prognostic-capable state array (in hydrostatic mode it is diagnosed from continuity); (b) route all pressure through a `PressureClosure` interface (hydrostatic integral now; hydrostatic + q-correction later); (c) keep face-normal flux assembly agnostic to where the pressure came from. Cost of the seam: one indirection and one array.

**Decision gate G3 (post-Phase 8):** revisit non-hydrostatic go/no-go against actual use cases (soliton-scale internal wave studies, aeration plume modeling). If go: fractional-step q-correction with a matrix-free CG/AMG solve, reusing HYPRE experience from FVHMComponent or a Kokkos-native solver.

### 4.3 Explicit finite volume + Kokkos

**Feasible with one crucial qualification: "fully explicit" cannot mean explicit in everything.** Two hard CFL constraints dictate the time-integration architecture:

1. **Surface (barotropic) gravity waves:** c = √(gH) ≈ 22–31 m/s at 50–100 m depth. At Δx = 25 m that is Δt ≈ 0.8–1.1 s — ruinous for multi-year WQ runs if the whole 3D state advances at that rate.
2. **Vertical diffusion in thin sigma layers:** Δz ~ 0.1 m with Kz ~ 10⁻² m²/s gives explicit Δt ~ 0.5 s, and Δz shrinks with drawdown.

Standard, GPU-proven resolution — **D-F4**:

- **Barotropic–baroclinic mode splitting (FVCOM-style):** a cheap explicit 2D external mode (η, depth-averaged U,V — reusing the design of the openswmm 2D marcher) subcycled ~20–40× inside each internal step; the 3D internal mode advances at the internal-wave/advective CFL (Δt ≈ 10–30 s at Δx = 25 m, since internal wave speeds are ~0.5–1.5 m/s). Everything stays explicit → ideal Kokkos flat parallelism; the known mode-splitting instabilities are managed with standard dissipation/averaging of the external mode.
- **Implicit vertical column solves** for vertical diffusion of momentum and scalars (and vertical advection near the surface): batched Thomas tridiagonal, one thread per column, contiguous by construction of the §4.1 layout — a canonically GPU-friendly pattern.
- **Fallback (decision gate G1, end of Phase 3):** if external-mode subcycling proves fragile with aggressive wet/dry, switch to a **theta-method semi-implicit free surface** (UnTRIM/SCHISM/ELCOM lineage): one sparse SPD 2D system in η per internal step (size = #columns, CG-solvable, still Kokkos-viable). Even this fallback needs no heavy solver: the η system is diagonally dominant and well-conditioned, so hand-rolled matrix-free CG with Jacobi preconditioning suffices — HYPRE-class AMG enters the picture only with the future non-hydrostatic 3D Poisson solve (Phase 9/G3). FVHMComponent's SIMPLE/PISO experience and HYPRE integration remain in reserve for that case. The transport and reaction layers are unaffected by G1's outcome.

**Practicality verdict — explicit vs implicit in one table.** Three families are on the table; the fully implicit CFD-style approach is ruled out for v1, and the real contest is θ-semi-implicit vs explicit mode splitting:

| | Fully implicit pressure-based (SIMPLE/PISO — FVHM lineage) | Semi-implicit θ free surface (UnTRIM/SCHISM/ELCOM) | Explicit mode split + implicit vertical (**D-F4**) |
|---|---|---|---|
| Δt regime | Accuracy-limited (minutes), but with outer iterations per step | Baroclinic-accuracy-limited, typically **1–5 min** | Internal advective CFL, **~10–30 s** at Δx = 25 m |
| Per-step cost | 3D momentum solves + pressure correction; AMG/Krylov per outer iteration | One 2D SPD solve in η (CG, matrix-free viable) + column sweeps | Cheap flat kernels + ~20–40 very cheap 2D external substeps |
| Linear-solver dependency | Heavy (HYPRE/AMG) | Light | **None** |
| Kokkos/GPU fit | Poor–moderate (solver dominates, global sync) | Moderate (η solve is the sync point) | **Excellent** (TUFLOW FV commercial proof) |
| Wet/dry robustness | Robust but iteration-fragile | Very robust (θ-scheme tradition) | Needs care in subcycling |
| Kernel complexity / testability | Highest | Medium | **Lowest** |
| Multi-year WQ cost, serial/OpenMP CPU | Uncompetitive (iteration cost eats the Δt win) | **Often best** (~6–15× fewer steps than explicit) | More steps, but each far cheaper |
| Multi-year WQ cost, GPU | Poor | Good | **Typically best** — 10–50× kernel speedups swamp the step-count gap |

So the answer is hardware-dependent: on CPU the θ scheme's step-count advantage usually wins; on GPU the explicit path wins or ties because *every* kernel ports. Given the scoping decision (CPU-first but Kokkos-ready, GPU as the destination), D-F4 leads with explicit mode splitting and keeps θ as the G1 fallback. **G1 is decided on measurements, not taste:** (i) wall-clock per simulated day on a DeGray-scale mesh (8-core CPU), (ii) achieved stable internal Δt through a drawdown wet/dry stress test, (iii) implementation-defect rate during Phase 3. Because transport and reactions sit behind Δt-agnostic interfaces, promoting θ later touches only the hydro core.

**Local timestepping (LTS) — yes, and there is in-house precedent.** The openswmm 2D marcher already ships tiered LTS with active-cell sets; reservoir meshes are the *ideal* LTS case: fine shallow arms and littoral cells set the global CFL while the deep main basin — most of the volume — could step 4–8× larger. Design for FVQUAL:

- **Tier unit = the water column** (all layers of a column share a tier; vertical implicit solves are column-local anyway, so tiers never split a tridiagonal system).
- Power-of-two tiers with an enforced **2:1 ratio between neighboring columns**; tier map from local CFL (edge length, depth, |u|+c_internal), rebuilt every N steps with hysteresis to prevent tier flapping.
- **Conservation preserved by construction:** faces on tier boundaries accumulate the fine side's fluxes over its substeps (flux buffering), so the coarse update sees the time-integrated fine flux — mass closure stays at machine precision; FCT limiting is applied per substep on the fine side.
- **Applicability:** external mode (direct reuse of the openswmm tiered design, F2.2); internal-mode advection and Eulerian transport (flux-buffered, F5.5); baroclinic pressure gradient evaluated at tier sync points (the 2:1 ratio keeps neighbor states within one substep of each other). **Reactions need no LTS** — `ReactionIntegrator` already does adaptive per-cell substepping, which is LTS for kinetics. ELM and RWPT are Δt-flexible by nature.
- **Expected gain:** 2–5× wall-clock on typical dendritic reservoir meshes (literature + openswmm experience). GPU caveat: tiers shrink batch sizes, so process tiers as compacted index lists (the openswmm active-set pattern) and expect LTS gains to be larger on CPU than on GPU — which is exactly the phase (pre-GPU) where the speedup is most needed.

**Kokkos packaging.** Follow the proven openswmm pattern: core library never links Kokkos; kernels are written device-portable-by-construction (SoA, fixed-size inline arrays, no virtual dispatch in hot loops); GPU backends ship as a dlopen'd plugin behind a C ABI (`GpuPluginAbi` precedent) with OMP/CUDA/HIP/SYCL shims; vcpkg features `gpu`, `gpu-cuda`, `gpu-hip`, `gpu-sycl`. Per the scoping decision: **serial+OpenMP first, Kokkos-ready layouts from day one, GPU parity harness in Phase 7.** Current Kokkos ecosystem practice (batched kernels for many small systems, ArborX-style neighbor search) matches FVQUAL's workload shapes.

### 4.4 Transport — Eulerian FV vs LARD (ELM / RWPT)

Terminology anchor: in openswmm, **LARD = Lagrangian Advection–Reaction–Dispersion** (EPANET-style plug-flow segments in 1D, with RWPT dispersion planned). In 3D the Lagrangian family splits into **ELM** (semi-Lagrangian backtracking with interpolation) and **RWPT** (mass-carrying particles). Assessment:

| Criterion | Eulerian FV (MUSCL + Zalesak FCT) | ELM (semi-Lagrangian) | RWPT (particles) |
|---|---|---|---|
| Mass conservation | Machine precision, local & global | **Not conservative** — interpolation at departure points drifts mass; corrections (local remap, constraints) add complexity and remain approximate | Exact global (particles carry mass); local concentration is statistical |
| Stability / Δt | Advective CFL (already paid by internal mode → **zero extra Δt penalty**) | Unconditionally stable — Δt limited only by trajectory accuracy | Unconditionally stable |
| Numerical diffusion | Controlled by limiter; FCT preserves sharp fronts | Interpolation diffusion, significant at low order on unstructured meshes | **None** (its superpower) |
| Nonlinear kinetics coupling | Clean — smooth cell fields | Clean fields, but mass bias feeds kinetics error over long runs | Poor — reaction rates on noisy reconstructed concentrations (noise ~ 1/√N per cell); nonlinear kinetics amplify noise |
| N-species scaling | Shared reconstruction; species-major SoA blocks amortize well | One backtracked trajectory serves all species — excellent | Cost ∝ particles × species; whole-reservoir eutrophication needs prohibitive counts |
| Wet/dry & boundaries | Well understood (FCT + positivity) | Fragile near moving boundaries; trajectory clipping ad hoc | Reflection/absorption rules ad hoc at bed/surface |
| GPU | Flat face/cell loops — ideal | Gather-heavy cell-walk location; doable but irregular | Embarrassingly parallel; sorting/binning needed |
| Precedent | FVCOM, Delft3D, TUFLOW FV, W2's ULTIMATE-QUICKEST | SCHISM/UnTRIM (primarily for **momentum**, where strict conservation matters less) | Tracking/age/plume studies |

**Recommendation (D-F5):**

1. **Eulerian FV-FCT is the primary WQ transport engine.** For eutrophication/DO budgets, strict mass conservation is non-negotiable (the entire W2 user expectation is closed constituent budgets). It reuses the validated openswmm recipe (MUSCL reconstruction + Zalesak FCT + implicit vertical dispersion, Lie-split reactions) on the new mesh, and its Δt rides free on the internal mode.
2. **ELM as an optional momentum-advection scheme and a "screening" transport mode** — the SCHISM lesson: use Lagrangian advection where conservation is less critical (momentum) or where a fast, stable, approximate answer is the goal (long planning runs, ≥5–10× internal Δt for transport-only). Ship behind the same engine interface; document the conservation caveat loudly.
3. **RWPT as a diagnostics module, not a WQ engine**: water age, source attribution, spill/plume forensics, residence-time distributions — extending the openswmm LARD/RWPT design (counter-based RNG, divergence-corrected drift) to 3D.
4. **Cross-check gate** (mirrors openswmm X4): every release must pass uniform-tracer preservation, machine-precision Eulerian mass budgets, engine-vs-engine tracer comparisons, and analytical age solutions on all engines.

This gives the requested LARD-vs-Eulerian evaluation an operational answer: *both ship, with clearly assigned roles, behind one `TRANSPORT_SOLVER` switch* — the same philosophy as openswmm's `QUALITY_SOLVER = LEGACY | EULERIAN_ARD | LAGRANGIAN`.

### 4.5 Reactions — reusing the openswmm multispecies (MSX-convention) engine

**Highly feasible; this is the single biggest reuse win.** Audit findings from the openswmm codebase:

- The numerical core is **already mesh-agnostic**: `ReactionIntegrator::step(const ReactionData&, bool tank, double dt, double* species, double* hydvar, RxWorkspace&, const double* pollutants)` touches no SWMM data structures — only a species block, a 9-slot hydraulic-variable array (the documented "ABI" of the evaluator), and a caller-owned workspace. It is allocation-free and `noexcept` on the hot path. A 3D binding is a ~200-line gather/scatter adapter, same as the existing ARD/LEGACY bindings.
- The expression compiler (shunting-yard → flat RPN pool, pre-resolved indices), the four integrators (EUL/RK5 default/ROS2/BDF2 + damped-Newton EQUIL), and the SoA hot/cold `ReactionData` all carry over unchanged. D-R7 (no SUNDIALS; many tiny independent per-cell systems) is *exactly* the 3D workload too — and the fixed-size, allocation-free design is Kokkos-portable with modest effort (the RPN evaluator is a natural device kernel).

Required extensions (tracked as work items in Phase 6):

| Item | Description |
|---|---|
| **Packaging** | Extract `ReactionModule` + `ReactionData`/`ReactionTokens`/`SpeciesRegistry` into a shared library (`msxkinetics`) consumed by both openswmm and FVQUAL — or short-term, vendor via git subtree with a sync script. Upstream extraction is the right long-term answer; propose it as an openswmm plan addendum. |
| **Scope tokens** | Replace `bool tank` with a scope enum; add `WATERCOLUMN`, `BENTHIC` (the openswmm plans already reserve `SURFACE2D`/`SUBSURFACE`, so the enum is anticipated). |
| **Environment variables** | Extend `RxHydVar` for 3D: PAR/light at cell (computed top-down per column via Beer–Lambert with chlorophyll/ISS self-shading — a transport-side service), DEPTH, DENSITY, USTAR/shear, WIND (surface cells), plus existing D/U/DT semantics reinterpreted per cell. Temperature stays a **transported species** (`__TEMPERATURE__`), per the HEAT plan's D-UT5. |
| **Settling & benthic exchange** | Per-species settling velocity = additional vertical advective flux in the *transport* operator (not the reaction), with bed-flux handoff to BENTHIC species (2D fields on bottom faces). WALL-species semantics finally get a physical home. |
| **W2 parity kinetics** | Curated `.rxn` template libraries, phased: (1) conservative tracer + age + first-order decay; (2) DO–CBOD–N–P–3 algal groups–DOM/POM (labile/refractory)–ISS with SOD; (3) pH/carbonate, zooplankton, macrophytes, CEMA-style diagenesis. Validated species-by-species against W2 and against openswmm reference results. |

Also reusable with minor generalization: the `[PROCESS_COMPONENTS]` external-config registration pattern and the `.rxn`/`.ard`-style `.inp`-dialect section parser. **Lesson imported as a hard rule (D-F8): FVQUAL is SI-internal everywhere** — the openswmm plans repeatedly flag internal US-customary units as a recurring defect source.

### 4.6 Peer-model landscape (context for the decisions above)

| Model | Mesh | Vertical | Free surface | Transport | Relevance |
|---|---|---|---|---|---|
| FVCOM | Unstructured tri | Sigma | Explicit mode split | Eulerian FV | Closest architectural cousin to D-F4 |
| SCHISM | Hybrid tri-quad | Hybrid S-Z | Semi-implicit | **ELM momentum** + FV/TVD tracers | Validates the ELM role split in D-F5 |
| Delft3D-FM | Tri-quad-poly | Sigma/z | Semi-implicit | Eulerian FV | UGRID layered-IO convention source (D-F2) |
| TUFLOW FV | Tri-quad | Sigma/z hybrid | Explicit FV | Eulerian FV, GPU | Proof: explicit tri-quad sigma FV + GPU is commercial-grade |
| ELCOM/AEM3D | Structured z | z-layer | Semi-implicit | Eulerian + mixing | The incumbent 3D reservoir-WQ coupling (CAEDYM) to beat |
| CE-QUAL-W2 v4.5+ | Structured 2D (x,z) | z-layer add/sub | Implicit tridiag | ULTIMATE-QUICKEST | Validation partner; kinetics parity target |
| SUNTANS | Unstructured tri | z | Semi-implicit | Eulerian | Non-hydrostatic reference for the G3 gate |

---

## 5. Proposed Architecture

### 5.1 Layering & build targets

```
FVQUAL/
├── src/core/          libfvqual-core     — mesh, state, hydro, transport, services
│   ├── mesh/          LayeredMesh adoption (from SDK tools), geometry precompute, Haney check
│   ├── state/         SoA state stores (see 5.2), snapshots
│   ├── hydro/         external mode, internal mode, PressureClosure, turbulence, wet/dry
│   ├── transport/     EngineInterface: EulerianFCT | ELM | RWPT; light/settling services
│   ├── kinetics/      msxkinetics binding (WATERCOLUMN/BENTHIC scopes)
│   └── forcing/       met, inflows/withdrawals, structures (selective withdrawal), BCs
├── src/io/            UGRID reader/writer ext., restart (HDF5), IOThread use, optional GeoPackage summaries
├── src/component/     libfvqual-hc       — HydroCouple IModelComponent wrapper, exchange items
├── src/gpu/           Kokkos plugin (OMP/CUDA/HIP/SYCL shims behind C ABI) — core never links Kokkos
├── apps/fvqual        CLI runner (YAML/JSON composition via SDK ModelInitializer)
├── tests/             GTest unit + verification suite (§7)
├── examples/          DeGray, Columbia Slough, analytical cases
└── plans/             this document + phase subplans
```

Dependency rules: `core` depends on nothing HydroCouple-facing (SDK `tools`/`io` linked as libraries); `component` depends on core + HydroCoupleSDK; kinetics comes from the shared `msxkinetics` library (§4.5). CMake + vcpkg manifest + CMakePresets, features mirroring SDK/openswmm conventions (`tests`, `netcdf`, `hdf5`, `gdal`, `geopackage`, `gpu*`, `mpi` reserved). C++20. MIT. CI from day one (Linux/macOS/Windows).

### 5.2 Data model & memory layout

- **Mesh:** immutable geometry SoA — 2D topology (CSR face-node, unique-edge list with normals/lengths, face areas/centroids) + per-column layer interface elevations (dynamic, sigma-updated) + derived cell volumes/face areas per layer. Cell id = `column*nLayers + k`.
- **State:** SoA blocks — `eta[nCol]`, `Ubar,Vbar[nCol]` (external), `u,v[nCell]`, `w[interfaces]` (prognostic-capable per D-F3), species-major `species[s][nCell]` (T and age are reserved species), `rho[nCell]`, `Km,Kz[interfaces]`. All hot arrays allocation-stable and device-mirrorable via the SDK `DeviceBufferRegistry`.
- **No virtuals in kernels**; engines selected via function tables at setup (openswmm `PerformStepFunction` precedent).

### 5.3 Numerical formulation summary (internal mode)

- Incompressible RANS + Boussinesq; hydrostatic pressure = surface + baroclinic integral; **baroclinic gradient in sigma via density-Jacobian (Shchepetkin–McWilliams-type) with reference-profile subtraction** — the primary defense against sigma pressure-gradient error, verified by the quiescent-stratified-basin test (§7).
- Continuity → diagnostic `w`; free surface from external mode (or theta-solve under G1 fallback).
- Momentum: explicit horizontal advection (TVD/MUSCL on faces) + optional Coriolis + explicit horizontal diffusion (Smagorinsky) + **implicit vertical diffusion** + pressure via `PressureClosure`.
- Turbulence closure: Phase 3 ships Richardson-damped analytical mixing (Munk–Anderson, W2-comparable); Phase 5 adds a native **GLS (k-ε/k-ω/gen)** column module (implicit, batched — same tridiagonal machinery). No GOTM dependency (Fortran).
- Equation of state: freshwater ρ(T, TDS, ISS) with UNESCO/TEOS-10-simplified salinity extension for estuaries.
- Surface thermodynamics: shortwave with Beer–Lambert penetration (two-band), longwave/latent/sensible per the openswmm HEAT/CSH formulations; evaporation mass flux optional.
- Structures & operations: selective withdrawal envelopes (W2 theory), point/distributed inflows with **density-seeking tributary placement**, weirs/gates/pipes via rating-style closures (reuse openswmm hydraulic-structure experience).

### 5.4 IO

- **Input:** UGRID 2D mesh (tri-quad, `face_node_connectivity` with fill values) + sigma spec; YAML/JSON model composition via SDK `ModelInitializer` targeting `IArgument`s; `.rxn` kinetics; met/inflow time series (CSV/NetCDF).
- **Output (D-F12): UGRID NetCDF + HDF5 is the sole full-field results format** — layered-2D convention via the SDK writers on the SDK `IOThread` (bounded queue, snapshot deep-copy only on write path, chunked+deflated hyperslab appends). GeoPackage is **not viable at 3D scale** (row-per-cell-per-timestep SQLite writes vs. chunked array appends) and is demoted to an optional, feature-gated **derived-summary export only** — station time series, time-aggregated statistics, min/max surfaces for GIS — produced post-run or at low cadence, never on the hot IO path. HDF5 restart/hotstart with exact-state round-trip guarantee.
- CI validation: `ugrid-checks` + xugrid round-trip in the test suite.

### 5.5 HydroCouple integration & the interface gap

- `FVQUALComponent : AbstractModelComponent` (SDK v2 contract — `shape()/dataKind()/getValuesInto()/setValuesFrom()`, no variants).
- **Exchange items (initial):** inputs — inflow flow/temperature/species at named inlets (id-based time series, exactly the pattern `CEQUALW2Component` exposes, enabling drop-in A/B testing), met forcing, boundary WSE/stage; outputs — withdrawal/outlet flow, temperature and species at outlets, surface fields (η, surface T) as `ITimeSeriesPolyhedralSurfaceComponentDataItem`, full 3D fields via file IO initially.
- **Interface gap & proposal:** HydroCouple has no layered-unstructured 3D data item. Propose `ILayeredMeshComponentDataItem` (2D polyhedral topology + layer dimension + vertical coordinate metadata — the in-memory twin of the D-F2 UGRID convention) as an interface-review item for HydroCouple v2 co-evolution (SDK Phase 3 channel). Interim: id-based and polyhedral-surface items suffice for coupling; 3D state moves via UGRID files.
- **Coupling scenarios to prove:** openswmm outfall → FVQUAL inflow (watershed→reservoir); FVQUAL outlet → CSHComponent river temperature; side-by-side `CEQUALW2Component` vs `FVQUALComponent` in one composition for validation.

---

## 6. Development Plan — Phases & Work Packages

Statuses: ⬜ not started. Each phase ends with its verify criteria green before the next begins (gates may overlap engineering prep). Rough efforts assume focused development with AI assistance.

### Phase 0 — Project scaffolding ⬜ (~2 weeks)
- F0.1 Repo `FVQUAL` (MIT; CMake+vcpkg+presets mirroring SDK/openswmm conventions; C++20; CI matrix Linux/macOS/Windows; GTest).
- F0.2 Dependency wiring: HydroCoupleSDK (tools/io/device as libs), netcdf/hdf5 features; **SI-units rule (D-F8) documented in CONTRIBUTING**.
- F0.3 `plans/` seeded with this doc + phase-subplan stubs.
- **Verify:** clean configure/build/test on all three platforms in CI; empty component links against SDK.

### Phase 1 — Mesh & data model ⬜ (~4–6 weeks)
- F1.1 Adopt `SigmaGridGenerator`/`LayeredMesh`; FVQUAL mesh wrapper with geometry precompute (edge normals, volumes, non-orthogonality weights), Haney/hydrostatic-consistency mesh check.
- F1.2 UGRID 2D mesh **reader** (tri-quad, mixed connectivity) + sigma spec input; terrain sampling from bathymetry rasters (SDK terrainsampler).
- F1.3 State stores (§5.2); snapshot/restart skeleton.
- F1.4 Layered-2D UGRID **writer extension** (layer dim + `ocean_sigma_coordinate` formula terms) over SDK writers.
- **Verify:** mesh invariant tests; UGRID round-trip read→write→xugrid/ugrid-checks pass; DeGray bathymetry meshed tri-quad and extruded (visual + metric checks).

### Phase 2 — External (barotropic) mode ✅ (delivered 2026-08-23)
- F2.1 ✅ Explicit 2D depth-averaged SWE on the tri-quad mesh: HLLC face fluxes in the face-normal frame, **Audusse hydrostatic reconstruction** for well-balancedness, semi-implicit Manning friction, wind stress, positivity-preserving wet/dry, reflective walls. No linear solver, no per-step allocation.
- F2.2 ✅ Adaptive CFL Δt with `advance()` substepping that lands exactly on the target time (the clock contract mode splitting needs). **Tiered LTS deferred** — see the deviation note below.
- **Verify ✅:** lake at rest over irregular *and* emergent bathymetry exact to 1e-13; mass closure < 1e-10 in a sloshing basin; seiche period vs `T = 2L/√(gH)` within 5%; wind setup vs `τ/(ρgH)` with an unforced control proving the test has teeth; Ritter dam-break **refinement study** (200→800 cells) confirming first-order convergence, 1.0% error at 3200 cells.

**Deviations from the original plan, and why:**

| Item | Decision | Rationale |
|---|---|---|
| MacDonald SWE benchmarks | Replaced with **Ritter dam-break convergence study** | MacDonald cases require ODE-integrated bed profiles to manufacture each solution — substantial fixture work that verifies steady friction-slope balance. Ritter exercises the harder property for this phase (shock capturing and a wetting front against a closed-form solution) and, run as a refinement study, distinguishes first-order accuracy from a bug in a way no single tolerance can. MacDonald remains worthwhile for steady-flow validation and is deferred to Phase 4, where structures and steady operations make it directly relevant. |
| Tiered LTS (D-F11) | Deferred to a follow-on within Phase 2 | Its conservation gate is defined *against a global-Δt reference*, which only now exists. Building flux-buffered tier interfaces on an unvalidated solver would have meant debugging two things at once. The marcher's face-loop structure is unchanged by the addition. |
| MUSCL reconstruction | Not attempted; scheduled with Phase 5 | First-order is sufficient for every Phase 2 gate and keeps the well-balanced proof simple. It is, however, the cause of the one known limitation below. |

**Known limitation recorded:** the dry-bed front in a Ritter dam break trails theory by ~15 m *independent of refinement*, while the depth field converges cleanly at first order. The vanishing tip is a degenerate point where a first-order scheme's front speed converges at a reduced rate. This is expected behaviour, is pinned by a bounded test rather than hidden, and is a concrete driver for the Phase 5 MUSCL work.

### Phase 2 addenda — F2.3 open boundaries, F2.4 MacDonald ✅ (2026-08-23)

**F2.3 — characteristic-based open boundaries.** Prescribed-discharge `Inflow`, prescribed-elevation `Stage`, and zero-gradient `Transmissive`, per boundary edge and settable mid-run. How many quantities may be imposed is *counted by the characteristics*, not chosen: subcritical flow sends one out along `u + c`, so one condition is imposed and the rest follows from the outgoing invariant `u + 2c`; supercritical outflow takes nothing, supercritical inflow both. Subcritical inflow solves `−q/h + 2√(gh) = R` by bisection — the left side is monotone in `h`, so the root is unique and bisection cannot fail where Newton could near `h → 0`.

*Two gates exist only because earlier versions were shown to be worthless.* I first asserted a fixed-stage boundary should be non-reflecting; it should not — pinning `η` is a Dirichlet condition and reflects with inverted phase, much as a wall reflects unchanged. Non-reflection belongs to a radiation boundary, not to well-posedness. And the rarefaction gate, written at 5%, passed with the characteristic treatment deliberately removed, because the HLLC solver downstream does much of the same work from the ghost state. Measuring both gave **98.7% of exact** for the invariant treatment against **96.5%** for copying the interior velocity, so the bound is now 2% — tight enough to separate them, and set by measurement.

**F2.4 — MacDonald manufactured steady flow.** Ritter tests an unsteady shock; it says nothing about the balance that dominates riverine hydraulics, where a steady discharge, a bed slope, and friction hold each other in equilibrium. MacDonald's construction inverts the problem: choose a smooth `h(x)`, integrate `dz_b/dx = (q²/(gh³) − 1)h′ − S_f` for the bed that makes it exact.

The subtlety that decides whether the benchmark measures anything: `S_f` must be **exactly** the friction law the solver applies. FVQUAL uses the wide-channel Manning form `S_f = n²u|u|/h^{4/3}`, so the manufactured bed uses `n²q²/h^{10/3}` — *not* the hydraulic-radius form MacDonald's paper writes. Using the paper's form would manufacture a bed for a different model and measure the mismatch between two friction laws while appearing to measure the solver.

*Measured, and the tolerances set from the measurements rather than from comfort:*

| Gate | Result |
|---|---|
| Exact profile is a fixed point (200 cells, 2000 s) | mean depth error **0.57 mm** on a 2 m depth |
| Recovered from a flat start (20 000 s) | **the same** 0.57 mm — it finds the same discrete steady state |
| Refinement 50 → 100 → 200 cells | 2.21 → 1.13 → 0.57 mm, ratios **1.97, 1.98** — clean first order |

First order is the correct expectation, not a shortfall: MUSCL reconstruction was added to *scalar transport*, not to the shallow-water depth and momentum. The refinement gate asserts the **rate** rather than a floor, because a scheme stalled at a fixed error passes any single tolerance. A deliberate 10% error in the friction coefficient drives the error to 10.8 mm — 19× the bound — so the gates have teeth. The fixture also checks itself: differencing the integrated bed must return the slope it was integrated from, since an error there would quietly pose a different problem with a different exact answer.

### Phase 3 — 3D hydrostatic internal mode ✅ (delivered 2026-08-23)

**Delivered and verified:**
- F3.1 ✅ Mode-splitting driver: external subcycling with time-averaged flux accumulation, mode adjustment, flux adjustment, continuity-diagnosed vertical velocity, implicit vertical viscosity (batched Thomas), upwind horizontal/vertical momentum advection.
- F3.2 ✅ `IPressureClosure` seam (D-F3) + equation of state (freshwater density maximum, UNESCO salinity, suspended solids).
- F3.3 ✅ Conservative scalar transport on mass-consistent fluxes, implicit vertical diffusion, Pacanowski–Willebrand-style Richardson-damped mixing with convective adjustment, prescribed surface heat flux.
- **Verified:** homogeneous basin over steep bathymetry holds rest to **5.6e-12 m/s** over 600 s; stratified basin on a flat bed holds rest to **1.6e-13 m/s** with stratification intact; free-surface continuity residual **1e-14** throughout; uniform tracer stays uniform to 1e-9; tracer mass and water volume conserved to <1e-10; baroclinic pressure gradient **exactly zero** for a horizontally uniform ocean on a flat bed and convergent under vertical refinement over a sloping bed (1.6e-5 → 4.8e-6 → 1.3e-6 m/s² at 10/20/40 layers).

**Baroclinic dynamics ✅ — measured against theory:**

| Gate | Result | Theory |
|---|---|---|
| Quiescent stratified basin over steep bathymetry (R1) | **1.1e-2 m/s**, bounded | 0 |
| Two-layer internal seiche period | **7240 s** (+18.6%) | 6105 s |
| Lock-exchange front speed | **0.217 m/s** (−19.0%) | 0.268 m/s |

The two ~19% biases share one cause and one sign: first-order upwind transport thickens the density interface, and a thicker interface both slows a gravity current and lengthens a seiche period. MUSCL reconstruction in Phase 5 is the fix, exactly as for the Ritter dry-front lag in Phase 2.

**Four bugs found, each by measurement rather than inspection:**

1. **Barotropic contamination of the baroclinic pressure gradient — the one that mattered.** The column integral used the full density instead of the anomaly ρ−ρ₀. Since ∫ᶻ^η ρ dz′ = ρ₀(η−z) + ∫ρ′dz′, the "baroclinic" term silently carried the whole barotropic surface-slope term ρ₀∂η/∂x. The external mode already computes that through its own pressure flux, so it received a *duplicate of its own restoring force, lagged* across an entire subcycling interval — a resonantly forced oscillator. A free surface that should have tilted by millimetres reached several metres. **Invisible to every rest-state test**, because a flat surface hides the term completely; only a tilted surface exposes it. Now pinned by `TiltedSurfaceOverHomogeneousWaterIsExactlyZero`.
2. **Missing baroclinic forcing of the external mode.** The depth-averaged baroclinic acceleration was never handed to the barotropic solver, so the modes were decoupled and mode adjustment *erased* the barotropic response instead of reconciling it. Fixed by `BarotropicSolver::setBodyAcceleration`.
3. **Time step blind to horizontal density contrast.** Internal wave speed was estimated from each column's *vertical* density range, which is identically zero for a lock exchange — every column starts internally uniform. The solver took its 300 s maximum step and was unstable from step one.
4. **No vertical Courant limit**, though sigma layers are metres thick where cells are tens of metres wide, so vertical advection usually binds first.

**Elimination trail** (kept because it is what located bug 1, and because it records what *not* to re-suspect): the pressure closure returns exactly zero for a horizontally uniform ocean; the splitting machinery holds a homogeneous basin over steep bathymetry at rest to 5.6e-12 m/s; the instability reproduced on a **flat bed**, ruling out sigma coordinates; the growing mode carried only 3.5% grid-scale energy, ruling out checkerboard decoupling; horizontal viscosity swept 0→20 m²/s changed nothing; and freezing the density field reproduced the instability exactly, which localized it to the momentum/barotropic path and pointed at the surface term.

**Two mitigations built, measured, and found ineffective** — documented in `pressureclosure.h` so they are not re-attempted: reference-profile subtraction (cancels identically in this Green-Gauss form; removed) and piecewise-linear density reconstruction (worth <1% of the spurious velocity, not the order of magnitude assumed; retained as a cheap option). Note the first argument applies only to a *z-dependent* profile — subtracting the constant ρ₀ is mandatory, per bug 1.

**Deferred with the phase:** F3.4 (inflows, withdrawals, density-seeking placement, 3-D wetting and drying — the internal mode requires every column wet and says so), the DeGray/W2 seasonal-stratification comparison, and GLS turbulence.

**Note on the Haney gate:** the steep-bathymetry case that exposed the instability *passed* r ≤ 0.2 screening (r = 0.112). The gate is necessary but not sufficient, and should not be read as certifying a mesh.

### Phase 3 addenda — F3.5 heat budget ✅, F3.6 computed radiation ✅, F3.7 spatially varying forcing ⬜

**F3.5 ✅ Surface heat budget from meteorology.** A prescribed flux cannot be wrong in an interesting way — it is whatever the modeller typed. A computed one depends on the water temperature it is trying to set, which is the negative feedback that gives a lake an equilibrium temperature at all. Net shortwave, Swinbank atmospheric longwave with cloud enhancement, back radiation, and latent/sensible through a shared wind function and the psychrometric constant. Penetrating shortwave is distributed down the column through the same Beer–Lambert march the kinetics use, because handing it all to the surface layer builds a hot skin rather than a thermocline. `equilibriumTemperature` is exposed and gated.

**F3.6 ✅ Computed solar radiation and a general wind function.** W2 computes rather than reads shortwave when `SROC` is off, which is how DeGray is configured — its met file has no shortwave column, so a budget that can only be driven by a measured record cannot run that model. Cooper declination, air-mass transmittance `0.7^(m^0.678)`, cloud `1 − 0.65C²`. The wind function gained an exponent because calibrated forms are routinely quadratic.

**What the DeGray run found that 247 unit tests did not** — both worth keeping because they are the same *kind* of defect:

1. The evaporative wind function was ~8× too weak. **Every heat-budget gate computed its expectation from the same function it was testing**, so the budget stayed self-limiting, stayed conservative, and converged on "its" equilibrium — all true, all useless. Peak surface temperature 124 °C. Now gated against magnitudes from outside the code.
2. Cloud cover units: W2 records tenths, the budget wants a fraction, and the cloud factor is quadratic, so a 10 multiplied atmospheric longwave by eighteen. The underlying fault was two functions in one file disagreeing about what a cloud fraction is.

**F3.7 ⬜ Spatially varying meteorological forcing — new scope, motivated by measurement.**

Comparison against DeGray's measured profiles (`prf_dam.npt`, constituent 22, 24 casts through 1980) exposed that the driver sets `windSpeed` for the heat budget and **never sets `windStressX/Y`**, which stayed zero for the whole run. Wind drove evaporation and drove nothing mechanically. The reservoir has a 14 m wind-stirred epilimnion in April; the model has 3 m. The met file even carries `PHI`, wind direction, unread.

That is a wiring defect and cheap to fix. The requirement it exposes is broader: forcing must be specifiable **per column and globally**, because shading, fetch, and sheltering vary along a reservoir, and because a coupled atmospheric model delivers fields rather than scalars.

**The defect is worse than a thin epilimnion, and this is the part worth remembering.** Vertical mixing is Richardson-damped: `Ri = N²/max(shear, 1e-12)`, `damping = 1 + 5 Ri`, `k_z = k_m/damping + backgroundDiffusivity`. With no wind stress there is no shear in a reservoir with negligible throughflow, so `Ri` is enormous, the damping is enormous, and `k_z` collapses to the background **1e-7 m²/s** — molecular — wherever the column is stable. **The entire mixing scheme was inert for the whole DeGray run.** The only vertical mixing left was convective adjustment. A parameterisation can be switched off by an unset input in a different module and nothing reports it; the model runs, conserves, and closes its budgets. Worth a diagnostic that refuses silence: report the fraction of interfaces sitting at background diffusivity, and say so when it is ~1.

*Design — uniform or per column, chosen per variable:*

```
ForcingField:  setUniform(v) | setPerColumn(vector) | at(column) | isUniform()
Meteorology  → MeteorologyField, one ForcingField per variable
               at(column) materialises a Meteorology for the flux call
```

The pure function `surfaceHeatFlux(const Meteorology &, double)` **does not change**, which matters: every F3.5 gate tests it directly, and those gates keep their meaning if only the supplier changes.

*Scope:*
- `ForcingField` for air temperature, dew point, wind speed, wind direction, cloud cover, shortwave, pressure, albedo, surface absorption.
- **Wind stress derived in the engine** from speed and direction via `τ = ρ_air C_D W²`, so wind is specified once and reaches both the heat budget and the momentum equation. The present split — one consumer wired, one not — is the defect that motivated this.
- Per-column incident irradiance in `LightField` (`LightOptions::incidentShortwave` is currently one value for the whole mesh).
- Per-column granularity, not per cell: meteorology acts at the air–water interface, so the surface face is the natural unit. Per-cell is already available where it means something — `LightField` attenuates per cell with self-shading.

*Rejected: a callback per column.* Fine serially, awkward under OpenMP, and unpleasant across the C API and Python without a GIL dance. Data in, not code in.

*Verify:*
- A uniform field reproduces the scalar path **bit for bit** — the existing 254 gates must not move by one ulp.
- A per-column field of a constant equals the uniform path exactly.
- Two columns given different forcing diverge in the right direction and by an amount computed outside the code — the F3.5 lesson, applied up front rather than after a whole-reservoir run.
- Per-column energy budget still closes.
- A wrong-length vector is refused with a message naming both lengths.
- **Ablation:** shading one half of a basin must change its temperature; if it does not, the field is not being read.

### Phase 3 — original scope ⬜ (~10–14 weeks) — **the heart of the project**
- F3.1 Mode splitting (external subcycling, dissipative averaging); internal momentum with TVD horizontal advection; implicit vertical diffusion (batched Thomas).
- F3.2 `PressureClosure` seam; baroclinic gradient via density-Jacobian + reference-profile subtraction; equation of state; diagnostic `w`.
- F3.3 T (and S) as transported scalars with provisional upwind advection pending Phase 5 engines; surface heat-flux module; Ri-damped vertical mixing.
- F3.4 Wet/dry per column; inflow/withdrawal source terms; density-seeking inflow placement (first cut).
- **Verify:** internal two-layer seiche period vs analytic; lock-exchange front speed vs literature (±10%); **quiescent stratified basin over steep bathymetry: spurious |u| below threshold** (sigma-PG-error gate); seasonal stratification cycle on DeGray vs W2 within agreed T-profile RMSE; energy/mass budgets closed.
- **Decision gate G1:** mode splitting vs semi-implicit free surface — judged on wet/dry robustness, achieved Δt, wall-clock on a DeGray-scale mesh.

### Phase 4 — Structures & operations ◐ (sources, structures, multi-port operation, and internal transfers delivered 2026-08-23; W2 comparison open)

**Delivered — F3.4 (deferred from Phase 3) and the core of F4.1.** `Forcing::SourceSet`: inflows and withdrawals as volumetric source terms, with vertical placement the substantive part rather than the bookkeeping:

- **Density-matched (plunging) inflow** — the tributary descends until the ambient density matches its own and spreads there. Overflow, interflow, and underflow all fall out of one scan.
- **Selective withdrawal** — an envelope centred on the outlet with half-height `(|Q|/N)^(1/3)` for a point sink or `(|Q|/(width·N))^(1/2)` for a line sink, and a quadratic approach-velocity profile within it. This is the operational lever the whole phase exists for.
- Uniform, surface, bed, and fixed-elevation placements alongside; `setDischarge` applies an operating schedule mid-run and refuses to reverse a source's physics.

Wired through **both** modes: a per-column source on the barotropic continuity (with the physically asymmetric momentum treatment — inflow arrives with no momentum and dilutes, outflow leaves at the local velocity), per-cell sources in the continuity integration, and the scalar source inside the transport's conservation update.

**Verify ✅ (9 gates):** distribution sums exactly to the column total; plunging inflow reaches overflow/interflow/underflow at the right depths; envelope narrows with stratification and peaks at the outlet; volume budget closes under balanced throughflow; net inflow raises the surface by exactly `Q·t`; an inflow delivers `Q·c·t` of constituent; a withdrawal exports at ambient concentration. **The free-surface continuity residual stays below 1e-10 with sources active** — the mass-consistency contract now covers them.

**Two bugs found by these gates:**
1. **The scalar source was applied after transport rather than inside it.** Continuity had already carried the source into the vertical flux field, so the divergence changed the concentration whether or not anything was done about it: a withdrawal removed water but not its load, concentrating what remained, and the exported mass came out as *zero*. Fixed by moving the `S·c_source` term into `applyDivergence`, where the scalar conservation equation actually puts it.
2. **The withdrawal envelope used the density gradient local to the outlet.** A deep port below a sharp thermocline sits in uniform cold water where that gradient vanishes, so the envelope was reported as unbounded and the outlet appeared to draw from the surface — the opposite of what a deep port does. Now uses the column-scale buoyancy frequency, since it is the contrast the outlet works *against* that confines the flow.

**Also worth recording:** a test expectation of mine was wrong in a way the code was right about. A 2 °C inflow does *not* sink into a 5 °C hypolimnion — it floats, because water is densest at 4 °C. That is winter inverse stratification, and it is exactly why the equation of state carries the full polynomial.

**Delivered — F4.1 rating-curve structures (2026-08-23).** `Forcing::StructureSet`: weirs (`Q = C L H^{3/2}` with Villemonte submergence), orifices and gates (`Q = C A g_open √(2g ΔH)`, driving head switching to the difference across the outlet once submerged), pipes (always head-difference driven), and pumps (prescribed capacity with a start/stop deadband). Each structure registers a paired `Source`, so a rating curve composes with selective withdrawal: the gate decides *how much*, the envelope decides *from which layers*. Outlet works release only — a submerged gate would physically admit flow, but modelling that means inventing the returning water's composition, so it closes instead.

**Where the rating is evaluated turned out to be the whole design question, and I got it wrong first.** The obvious placement — rate each structure once per internal step — is wrong under mode splitting, and wrong by a lot. With uniform density the internal step is bounded only by advection and `maxTimeStep`, so it runs at ~200 s while the external mode subcycles at ~2 s. A spillway rated once for the whole interval releases at its starting head for over three minutes after that head has gone, and the reservoir draws down **below its own crest** — a state the rating curve calls impossible.

*The wrong fix, and what it cost.* My first correction clamped the release to the water standing above the crest **in the structure's own column**. It removed the overshoot and all nine gates passed. It was still wrong: over a 200 s internal step the barotropic mode refills that 3,000 m² column from the whole 36,000 m² basin many times over, so the single-column-isolation assumption does not hold. Probing the drawdown *curve* rather than its endpoint exposed it — the reported discharge **rose** from 36 to 59 m³/s while the head **fell** from 2.30 to 1.78 m, which no monotone rating curve can do, and the basin took 10,000 s to do what should take 300 s. The clamp had traded a 6% over-drain for a 7× under-drain. An endpoint tolerance passed it; the curve did not.

*The right fix.* A free-surface-driven release is a **barotropic** process, so it belongs in the subcycle. `BarotropicSolver` gained a per-substep `ColumnSourceUpdater` hook; structures are re-rated against the surface each substep, and the head feedback holds the level at the crest by physics with no limiter at all. Only the *column totals* are refreshed at that cadence — the external mode consumes nothing else, and repeating the envelope and density work a hundred times per step would cost far more than the placement moves. Transport is then handed the **time-mean** release the subcycle actually delivered, not the end-of-step rating, or the two modes would disagree about a spillway by exactly the amount the head changed.

**Verify ✅ (11 gates).** Rating laws against their closed forms; submergence monotonicity; gate opening as a linear multiplier; no reverse flow; malformed structures and inverted deadbands refused at configuration; pump hysteresis holds state inside the deadband. The two that carry the physics:

- **Drawdown against the closed form.** A weir draining a constant-area pool integrates exactly to `h(t) = h₀ / (1 + C L √h₀ t / 2A)²`, approaching the crest as `1/t²` and never reaching it. The model tracks it to within 2–6% from t = 100 s onward, and the instantaneous discharge agrees with the rating on the head the structure actually sees to within 2%.
- **The basin takes time to level.** Before a gravity wave has crossed the 600 m basin (~50 s), the model should *not* match the level-pool law and should release less — the far end has not yet responded. Asserted as the physical effect it is, rather than excluded as noise. This is also why the closed-form comparison starts at two basin transits.

Ablating the subcycle hook fails both gates, and fails them with the diagnostic signature of the original bug ("discharge and head disagree at t = 100 s").

**Delivered — F4.2 multi-port outlet operation (2026-08-23).** `Forcing::OutletGroupSet`: ports at several elevations operated together to a target release temperature. This is the payoff for the selective-withdrawal work — a deep port draws cold hypolimnetic water, a surface port warm epilimnetic water, and blending hits a downstream temperature neither reaches alone. Release temperature is regulated below many dams, so it is the operational lever the phase exists to provide.

*The split follows from heat conservation, not from a heuristic.* With ports delivering `T_w` and `T_c` bracketing the target `T*`, the warm fraction is `f_w = (T* − T_c)/(T_w − T_c)`. The model picks the **tightest** bracketing pair, because a blend of two ports near the target is far less sensitive to error in either one's delivered temperature than a blend of the coldest and warmest.

*Three design points worth recording:*

1. **A scheduled structure's rating curve changes role.** Left alone, a structure passes what the head drives — right for an ungated spillway, wrong for anything an operator touches. Under a group schedule the rating becomes the *capacity*: the release is the demand or the capacity, whichever is smaller. Without this the group could not command anything.
2. **The group decides once per internal step, not per substep.** The choice is driven by the temperature field, which transport advances once per step; re-deciding it inside the barotropic subcycle would repeat every port's envelope calculation against a field that has not moved. Capacity limiting *does* happen each substep, because the head does move — the two cadences are split along the timescales that actually differ (cf. D-F16).
3. **Asking what a *shut* port would deliver requires a probe discharge.** The withdrawal half-height scales as `Q^{1/3}`, so a closed port has no envelope and would report the temperature of whichever single layer it sits in. `SourceSet::withdrawalMean` therefore takes the discharge as an argument rather than reading the source's own.

**Verify ✅ (10 gates).** Malformed groups refused — including a port listed twice, which would silently release double the demand. Scheduled release honoured below capacity and clipped to capacity above it. The substantive ones:

- **Ports at different depths deliver different water** (deep port < 12 °C, shallow > 18 °C in a 24→6 °C profile) — the precondition, and the gate that fails if the envelope ever stops depending on outlet elevation.
- **The split matches the mixing law** to 1e-6, checked against the port temperatures the model itself reports, so it tests the split and not the envelope.
- **Achieved temperature tracks the target** monotonically across a sweep, at full demand.
- **Missing the target is visible.** A target colder than the whole reservoir opens the coldest port alone and reports an achieved temperature *above* it; a demand beyond the combined capacity reports a shortfall equal to the summed capacities, rather than inventing water. A blending model that always reports success is useless to an operator.
- **Stratification is what makes blending possible.** In a well-mixed column every port draws the same water and the lever disappears — which is why a dam that can manage release temperature in August cannot in February.
- **End to end through a step**, staying mass-consistent (surface-flux residual < 1e-10). Ablating the solver wiring releases 0 m³ and fails this gate.

**Delivered — F4.3 internal transfers (2026-08-23).** `Forcing::TransferSet`: water moved from one column to another inside the model — a pumped-storage return, a bypass around a dam segment, a destratification pump lifting hypolimnetic water, a withdrawal tower discharging into another branch.

*What makes a transfer more than two independent sources.* A hand-registered inflow carries a composition the modeller states. A transfer cannot: the water arriving at the receiving column **is** the water that left the donating one, so its temperature and every constituent it carries are fixed by where in the donor's column it was drawn from. That coupling is the whole content of the feature; the discharges are bookkeeping.

*Conservation is an identity here, not a tolerance.* The withdrawal removes `dt·Q·Σ_k w_k c_k` through the vertical weights the source distribution applied; the inflow adds `dt·Q·c_in`. Setting `c_in` to the mean through those **same cached** weights makes the two cancel term by term. This is why `SourceSet` now exposes `weightedMean()` (cached weights, used by transfers) separately from `withdrawalMean()` (probed at a stated discharge, used by outlet groups): recomputing the weights against the newer post-`updateSurface` geometry would leave a residual that no tolerance could justify. The gate asserts relative drift below 1e-12 rather than a physical tolerance, and the ablation drifts heat by 5.8e3 and tracer mass by 2.8e5.

*Cadence.* Discharges are written once per internal step before distribution, so both halves are placed against the same state. Composition is set per species at transport time, after `distribute()` has cached the weights and **before** that species is advected — the withdrawal removes mass at the pre-advection values, so the arriving water has to be that same water.

**Verify ✅ (7 gates):** malformed transfers refused (a column to itself, negative rate, negative index) and the registered pair confirmed equal and opposite so a transfer can never be a net source; volume moves between columns with none lost and the donor sitting below the recipient; **constituent mass conserved exactly** for temperature and an independent depth-varying tracer; intake elevation sets what is delivered (deep < 13 °C, shallow > 17 °C in a 22→8 °C profile, same reservoir and rate); the delivered water resembles the donor's deep water and not the column it arrives in; a destratifying pump measurably cools the column it feeds; and an idle transfer perturbs nothing — the control that catches one half of a pair being written without the other.

**F4.4 — selective withdrawal reconciled against CE-QUAL-W2 (2026-08-23). This found a real error in delivered work.**

Approaching the DeGray comparison, the first thing worth checking was not a curve but the *formulation*: does FVQUAL compute the same withdrawal zone W2 does? It did not.

*What was wrong.* The zone limits I had implemented were `d = (|Q|/N)^{1/3}` (point) and `(|Q|/(W·N))^{1/2}` (line) with `N` evaluated once at column scale. The published theory (Bohan & Grace 1973; Smith et al. 1987, as restated in the W2 V5.0 technical note, eqs. 1a–1b) carries leading coefficients I had dropped, and — more fundamentally — evaluates `N` **locally and implicitly**, between the outlet layer and each candidate layer over their own separation, marching outward until the separation catches the thickness.

*Why no existing gate caught it.* Every Phase 4 selective-withdrawal test asserted the **sign** of selectivity — a deep port draws colder than a shallow one — which holds for any positive coefficient. None asserted its **magnitude**. This is the characteristic blind spot of self-consistent verification, and precisely what comparison against a reference formulation is for.

*A wrong intermediate fix, recorded because it was instructive.* Reading the technical note first, I applied its `θ = π` and `c_bi = 8` on top of my existing column-scale `N`. That combined constants from one formulation with a frequency from another and produced a 36 m withdrawal half-height in a 24 m column — a zone drawing from water that is not there. The local implicit `N` and these coefficients are not separable; checking the shipped `withdrawal.f90` showed it uses `c_bi = 1` (2 near a boundary) with the implicit `N`, and that pairing is coherent.

*What is now implemented*, matching the shipped reference: march outward from the outlet layer with `N(H) = sqrt(g|Δρ|/(H ρ_str))` and `d = (c_bi Q/N)^{1/3}` (point) or `(2 c_bi Q/(W N))^{1/2}` (line); `c_bi = 2` where the outlet sits in the top or bottom tenth of the column; and a velocity profile parabolic in the **density anomaly** relative to the outlet layer, normalised separately above and below, rather than in distance from it.

*Consequences that show the change is real.* A port inside a well-mixed hypolimnion now draws **uniformly** across it and **nothing at all** across a sharp thermocline — where the distance-based profile tapered through water physically indistinguishable from the port's own. And the fixture scales were exposed as unphysical: 60 m³/s through a 4-hectare pond genuinely does draw the whole column, which the too-thin envelope had been hiding.

**Verify ✅ (3 new gates, 2 rewritten):** the zone limit satisfies the published Froude condition, recomputed independently in the test from the density field alone and bracketed (satisfied at the limit, not one layer inside) so no monotone rule could fake it — a 4× coefficient error moves the limit three layers and fails it; a thermocline excludes the epilimnion exactly; a well-mixed layer is drawn uniformly; and under linear stratification the draw does peak at the port and fall away on both sides.

**F4.5 — V5.0 withdrawal updates: one adopted, one refused (2026-08-23).**

*Adopted — the shifted velocity maximum (Bohan & Grace 1973, eq. 4).* The fastest approach velocity sits at the port only when the zone is symmetric about it. Where stratification lets the zone grow further on one side, the maximum moves:

`y₁ = H sin²(1.57 z₁/H)`

with `z₁` the distance from the port down to the lower limit and `y₁` the height of the maximum above that limit. A symmetric zone gives `z₁/H = ½` and `sin²(π/4) = ½`, returning the port — so this generalises the previous behaviour rather than replacing it. The parabolic profile is now normalised about the fastest layer rather than the port layer, which is what the reference does. Applied only where the zone develops freely; once it reaches the surface or bed the relation no longer holds, and the reference skips it too.

*Refused — the boundary-interference bisection (note eqs. 2a–2c, 3b).* Not implemented, deliberately, because the two authorities disagree and I cannot tell which is right from the material available:

1. **The coefficients contradict each other.** The distributed technical note's eq. 1a *multiplies* by the withdrawal angle and uses `c_bi` = 8 intermediate / 2 at a boundary. The shipped v5 code path *divides* by `θ` — `HSWT = (COEF·Q/(RHOF·θ))^{1/3}`, with the older un-angled line commented out directly above it — and uses `COEF` = 1 / 2. At `θ = π` the two differ by about a factor of 2 in zone thickness. Both are labelled the same algorithm.
2. **The bisection's result changes origin.** `WD_ZONE_EQN` solves for `D'`, which the note defines as the distance from the *interference boundary* to the free limit, and the caller assigns it straight into `hswts`/`hswbs` — quantities that are then compared against separations measured from the *port*. Those origins differ by `b`, the port-to-boundary distance. Either it is an undocumented convention or a defect, and guessing would mean shipping a number I could not defend.

Implementing either reading would be presenting a guess as fidelity. The classic `c_bi` = 2 treatment at boundaries stays; the discrepancy is recorded here and is one of the things the DeGray whole-reservoir comparison would settle, since it is exactly the regime — a low-level port near the bed — where the two readings diverge most.

**Gap found while scoping the MacDonald benchmarks: there are no open boundary conditions.** Every domain edge in the barotropic solver is a reflective wall, and no phase of this plan ever scheduled otherwise — the Phase 2 header says "inflows, withdrawals, and open boundaries arrive with the internal mode", but F3.4 delivered only the first two. Volumetric sources are not a substitute: a source injects water with no momentum, whereas a prescribed-discharge inlet carries momentum flux `ρQ²/A`, and no source can hold a downstream *stage*, which is what a subcritical outflow boundary does.

This blocks the MacDonald cases, whose whole content is a steady balance between bed slope and friction driven by a discharge inlet and a stage outlet. It equally blocks any riverine application, the DeGray tributary inflows, and a large part of the Phase 8 validation. It should be a scheduled item — **F2.3, characteristic-based open boundaries**: subcritical inflow specifying discharge, subcritical outflow specifying stage, supercritical inflow specifying both, supercritical outflow specifying neither, with the Riemann invariant supplying the missing information in each case.

**F2.3 ✅ open boundaries and F2.4 ✅ MacDonald benchmarks (2026-08-23).** Both delivered; see the Phase 2 section.

**Still open ⬜:** the DeGray whole-reservoir run against W2 output (`str_br1.csv`, `prf_dam.npt` are available locally) — this needs bathymetry ingest and meteorology, and belongs with the Phase 8 validation release; the MacDonald steady-flow benchmarks deferred here from Phase 2.

### Phase 5 — Transport engines ◐ **F5.1 delivered 2026-08-23; engines 2 and 3 open**

**F5.1 ✅ EulerianFCT** — `Transport::EulerianTransport`: inverse-distance least-squares cell gradients with Barth–Jespersen limiting horizontally, minmod slopes vertically, MUSCL face reconstruction from the upwind cell, and Zalesak flux correction blending against the first-order upwind flux. Boundedness is a property of the algorithm, not of a tolerance. Implicit vertical diffusion remains in the internal mode; Lie-split reaction hooks arrive with Phase 6.

*Measured effect, and where it does and does not help:*

| Gate | First-order upwind | MUSCL + FCT |
|---|---|---|
| Two-layer internal seiche period | +18.6% | **+7.9%** |
| L1 error, translated square pulse | baseline | **less than half** |
| Lock-exchange front speed | −11.1% | −11.1% (unchanged) |

The lock-exchange result is the informative one. A gravity current's front speed is set by integral buoyancy–inertia balance, not by how sharply the interface is resolved, and at the front itself the limiter correctly degrades to upwind — so no improvement is expected there, and none is seen. That front converges under mesh refinement instead: −11.1%, −6.2%, −4.3% at 100, 200, 400 cells. The seiche, whose period depends on interface thickness over a long smooth propagation, is exactly where second-order accuracy pays.

*Note on the engine interface:* the plan calls for a `TransportEngine` abstraction with three implementations behind it. It is deliberately not introduced yet — extracting an interface from a single implementation guesses at the seam, and the second engine is what will reveal where it belongs.

*Also unchanged:* MUSCL was added to **scalar transport**, not to the shallow-water depth and momentum, so the Ritter dry-front lag in the external mode stands. Extending reconstruction to the barotropic solver is separate work.

**F5.2 ELM and F5.3 RWPT — deferred indefinitely (decision D-F14).**

Both Lagrangian engines are shelved on mass-conservation grounds. ELM does not conserve mass: interpolation at backtracked departure points leaks, and the remedies (local remapping, global rescaling) are approximate corrections to a structural defect. For a model whose entire purpose is closing constituent budgets — a user asking where the phosphorus went does not want an answer that is 2% short — an engine that cannot answer exactly is not worth the second implementation, the second set of gates, and the abstraction needed to host it. RWPT conserves mass exactly but delivers it as statistical noise, which nonlinear kinetics amplify.

Consequences of the deferral, deliberately accepted:
- No large-time-step screening mode; long planning runs pay the advective CFL.
- Water age, source attribution, and residence time must be produced Eulerian-style — as additional transported species — rather than by particle tracking. The reserved `__AGE__` species already supports this, and it keeps every diagnostic on the same conservative footing as the constituents.
- The `TransportEngine` interface of D-F5 is not built. With one engine it would be an abstraction guessing at its own seam.

Revisit if a use case appears that genuinely needs Δt freedom (multi-decade scenario ensembles) or zero numerical diffusion (spill forensics, sharp plume delineation).

**F5.4 k-epsilon closure ✅ (delivered 2026-08-26).** Two-equation closure carrying TKE and dissipation as transported quantities, Galperin quasi-equilibrium stability functions, wall conditions from the wind stress F3.7 supplies and the bed drag the momentum solve already applies. Ten gates against results that exist independently of the code: log-layer energy `u*²/√c_μ`, quadratic scaling in `u*`, decay with nothing to sustain it, stratification suppressing mixing, convection enhancing it, boundedness under a 3600 s step. Behind `useTurbulenceClosure`, **off by default**.

*Measured against DeGray's casts, matched runs, 30 layers:*

| | overall RMSE | 5–10 m | 20–30 m | 30–60 m |
|---|---|---|---|---|
| no wind stress | **2.29 °C** | −3.91 | −0.83 | −0.55 |
| Richardson + wind | 3.59 °C | −1.11 | +3.22 | +3.65 |
| k-epsilon + wind | **3.42 °C** | −1.23 | +2.95 | +3.39 |
| no mixing at all + wind | 5.87 °C | +0.84 | +5.78 | +6.21 |

**The last row is the finding.** Removing mixing makes the interior *worse*, so the heat reaching DeGray's hypolimnion is being **advected** down by the wind-driven overturning cell, not mixed down — and mixing partly counteracts it by holding heat near the surface where the budget can shed it. Every earlier reading of this, including several in these notes, called it over-mixing and looked to the closure to fix it. Two schemes differing by 5% while removing mixing entirely costs 63% is what settled it.

*Why the cell is too strong:* a laterally-averaged schematisation is one segment wide, so every return path is forced through the vertical. `InternalMode.ALaterallyAveragedBasinOverturnsHarderThanAWideOne` measures it directly — same basin, same 9 m/s wind, varying only cells across: **0.0874 °C of warming below 20 m at one cell wide against 0.0531 °C at eight, 39% less.** The effect is real and large. Whether it accounts for *all* of DeGray's +3 to +4 °C cannot be decided on a dataset that is one segment wide by construction.

**Four defects found on the way, each by measurement rather than inspection:**

1. **A data race in the closure's tridiagonal workspace** — six scratch vectors shared across a parallel column loop. The *second* time this exact bug has been written here; F7.1b found it in the momentum and scalar solves and recorded that it "corrupts results silently rather than crashing". It survives unit testing both times because the gates call the solver from one thread.
2. **Wind stress routed through the body-force path** as `τ/(ρH)`, which the solver multiplies back by depth — but the body acceleration is fixed for an internal step while `h` evolves through ~108 substeps, so the applied stress became `τh/(ρH₀)`. A column gaining depth gained stress. Now on the surface-stress path, which is depth-independent by definition.
3. **The barotropic Courant limit used the plan area at full pool**, not the storage area at the current stage — too permissive by exactly the ratio wherever a basin narrows with depth.
4. **The vertical Courant limit read a scaled velocity.** `m_verticalFlux` stores `volumeFlux/planArea`, a true velocity only where interface area equals plan area; under hypsography DeGray's near-bed interfaces are 0.8% of it, so the limit under-restricted by up to 125×, never bound, and the near-bed momentum diverged to 1e70 m/s. **This was the DeGray wind failure.** Fixed at the point of use, leaving the mass-consistency identity untouched.

**A methodological note worth keeping.** Six readings of that failure were wrong before the right one, and each was corrected by a bisection or a diagnostic, never by thinking harder. The decisive step was making the solver report *which* constraint bound and *which cell* — after two runs had traced columns chosen by guesswork and found them healthy. Two interventions were made, failed to do what they were added for, and were reverted.

**Remaining Phase 5 scope ⬜**
- Reaction source hooks, with Phase 6.
- F5.2 **ELM engine**: RK trajectory backtracking with cell-walk location, linear/quadratic interpolation, optional local mass correction; documented conservation caveats; transport-Δt decoupling.
- F5.3 **RWPT diagnostics**: counter-based RNG, divergence-corrected drift, age/source-attribution outputs (3D extension of the openswmm LARD/RWPT design).
- F5.4 GLS turbulence module (column implicit) replacing/augmenting Ri-mixing.
- F5.5 Tiered LTS for internal-mode advection + Eulerian transport (column tiers, 2:1 neighbor ratio, flux-buffered tier interfaces per §4.3) — experimental flag; **verify:** mass closure vs global-Δt reference at machine precision, tracer-error bounds vs global-Δt run, speedup report on the DeGray mesh.
- **Verify (cross-check gate, mirrors openswmm X4):** uniform-tracer invariance on all engines; Eulerian global mass error ≤ 1e-12 relative; rotating-cone/slotted-cylinder advection quality metrics; analytical vertical-diffusion profiles; engine-vs-engine tracer envelopes on DeGray; water age vs analytical solutions in idealized basins.

### Phase PY — Python bindings & C API ⬜ (~10–12 weeks; detailed sub-plan: `PYTHON_BINDINGS_PLAN_2026-08-23.md`)

A `fvqual` Python package orchestrating the entire engine, modelled on `openswmm.engine/python/`: a new **C API layer** (`include/fvqual/capi/`, opaque handles, error codes, C89 ABI) as the stability firewall, **Cython** bindings over it, scikit-build-core + cibuildwheel packaging inheriting the openswmm lessons wholesale. One deliberate improvement over the template: **true zero-copy numpy state views**, which FVQUAL's allocation-stable stores (D-F7) make safe where SWMM's reallocating engine could not — guarded by a generation counter against re-initialization.

- PY0 C API foundation → PY1 mesh/species/options → PY2 lifecycle + `nogil` stepping + callbacks → PY3 zero-copy state views → PY4 physics-parity gate (Python-scripted seiche and lock exchange must match `fvqual-verify` within 0.1%) → PY5 wheels/typing/docs.
- **Sequencing rule:** PY0 lands before Phase 6, so the kinetics C API grows inside an established convention. From PY0 onward, C API coverage is part of every engine phase's definition of done.
- MCP server explicitly out of scope, planned as a sibling repo layering on the package (openswmm.mcp precedent).

### Phase 6 — Kinetics integration ✅ (delivered 2026-08-24/25)

**Delivered:** F6.1b reaction network with operator-split RK45 kinetics and a shunting-yard expression parser; F6.2a light field with Beer–Lambert self-shading and layer-mean irradiance; F6.2b `RateScope` (water column / bed / surface) with areal-to-volumetric conversion; F6.2c implicit upwind settling with benthic handoff; F6.3a air–water exchange scope; F6.3b `.rxn` text configuration; F6.4 template library (water age, first-order decay, DO–CBOD–N–P–algae).

Rather than vendoring `msxkinetics` as D-F6 anticipated, the network was written against FVQual's own species registry and 3-D environment. The MSX conventions carried over; the code did not. Revisit if drift against openswmm becomes real rather than hypothetical.

**Lesson worth keeping:** the scope check was, for a time, untested — a zero conversion factor away from the bed enforced scope a *second* time, so ablating the check changed nothing. Two mechanisms enforcing one rule means neither is gated. Removing the duplicate made the check fail four gates when ablated, which is what a gate is for.

### Phase 6 — original scope ⬜
- F6.1 Extract/vendor `msxkinetics` (shared library or subtree; upstream proposal to openswmm plans).
- F6.2 Scope enum (`WATERCOLUMN`/`BENTHIC`), 3D `RxHydVar` extension, per-column PAR service (Beer–Lambert + self-shading), settling as vertical advective flux with benthic handoff.
- F6.3 `.rxn` parsing reuse; temperature-as-species alignment with the HEAT plan; water-age reserved species.
- F6.4 Template library v1: tracer/age/decay + DO–CBOD–N–P–algae suite.
- **Verify:** reaction unit tests bit-compare against openswmm reference outputs for shared cases; batch-reactor analytics; Lie-split convergence study; DO sag + algal bloom idealized cases vs W2 kinetics behavior.

### Phase 7 — Kokkos acceleration ◐ (F7.2 baseline delivered 2026-08-23)

**F7.2 profiling baseline — delivered first, because it reorders the phase.** `StepProfile` records per-phase wall time inside the internal step, and `benchmarks/fvqual-bench` sweeps a stratified basin with everything switched on. Measured on 4 cores, 25 steps per size:

| Phase | 3 200 cells | 19 200 cells | 102 400 cells |
|---|---|---|---|
| **external (barotropic subcycle)** | **70.9%** | **63.7%** | **58.0%** |
| transport | 20.2% | 24.8% | 28.0% |
| pressure | 3.4% | 5.0% | 6.3% |
| momentum | 2.6% | 3.4% | 4.1% |
| density, fluxes, geometry, forcing | < 2% each | | |

**This contradicts the port order F7.1 assumed.** The plan targeted "face loops, batched column solves, FCT passes" — the three-dimensional kernels. Those are transport + pressure + momentum + density ≈ 40% of runtime, so by Amdahl a *perfect* port of all of them caps the speedup at **1.7×**. The 2-D external mode is the majority of a 3-D run.

*Why a 2-D solver dominates a 3-D run:* it subcycles. The measurement shows ~**108 external substeps per internal step at every size**, and a flat **73–74 ns per column-substep** independent of mesh size. External cost is `columns × substeps × 74 ns`, and the substep ratio does not improve with refinement — both `dt` limits scale with `dx` together, so the ratio is fixed at roughly `√(gH)/u`.

*Micro-optimisation was tried and rejected on measurement.* The friction term calls `pow(h, 4/3)` per column per substep, which looked like an obvious target. Measured: `pow(h, 4/3)` costs 6.05 ns and the "faster" `h·cbrt(h)` replacement is **slower** at 6.85 ns; `pow(x, 2.0)` already compiles to `x*x`; swapping `hypot` for `sqrt(x²+y²)` saves 1.4 ns. Total libm is ~11% of the per-column cost, so the 74 ns is memory traffic and HLLC edge work. There is no cheap serial win, and the intuition that there was one would have cost a day.

**Revised order:** parallelise the external mode *first*, then transport, then the rest. The obstacle is structural rather than arithmetic — the edge loop scatters into both adjacent cells (`m_dh[left] -= …; m_dh[r] += …`), which races. The mesh already carries a CSR cell→edge adjacency (`faceEdgeOffsets`/`faceEdges`), so the fix is a two-pass gather: compute and store per-edge fluxes in an edge-parallel pass, then accumulate per cell in a cell-parallel pass. Race-free by construction, no colouring structure, and it is the form Kokkos and a GPU both want.

**F7.1a — the barotropic edge loop is now race-free (2026-08-23).** The scattering sweep (`m_dh[left] -= …; m_dh[r] += …`) is split into an edge-parallel pass that computes and stores per-edge fluxes and a cell-parallel gather over the mesh's existing `faceEdges` CSR adjacency. No colouring, no atomics on the cell accumulators, and the same shape Kokkos and a GPU want. The one surviving scatter is the per-boundary flux tally, which touches a handful of groups rather than every cell and takes an atomic. Correct: all 126 gates pass threaded and unthreaded, including the well-balancedness residuals at 1e-15, so the gather reproduces the scatter. Cost: ~7% serially (74 → 79 ns per column-substep) for the extra edge traffic, which is the price of admission.

OpenMP is wired behind `FVQUAL_USE_OPENMP` across seven loops in the external mode.

**The scaling measurement is inconclusive on this hardware, and saying otherwise would be dishonest.** External phase, ms/step:

| threads | 3 200 cells | 19 200 cells | 102 400 cells |
|---|---|---|---|
| 1 | 2.99 | 12.56 | 44.91 |
| 2 | 2.71 | 8.20 | 32.16 |
| 3 | 2.44 | **7.40** | **28.98** |
| 4 | 2.61 | 9.33 | 32.22 |

**Three threads beat four at every size.** That is the signature of oversubscription rather than of the algorithm: the container reports four CPUs but exposes no core topology in `/proc/cpuinfo` (no `core id`, no `physical id`), so it is most likely two physical cores with SMT, or a shared VM. Peak observed is **1.55× on the external phase**, which against a probable two-core ceiling is reasonable efficiency — but the honest statement is that this sandbox cannot evaluate parallel scaling and the numbers must be retaken on real hardware before any conclusion is drawn from them.

**F7.1b — transport and the 3-D kernels threaded (2026-08-23).** Every remaining hot loop is now parallel. They needed no restructuring: transport was already written as a gather (column loops with CSR inner sums), as were the pressure closure and the momentum advection, so the work was auditing each for shared state rather than rewriting it.

Two pieces of shared state did have to be fixed:

- **The vertical-solve workspace.** Six member scratch vectors were shared by the two per-column tridiagonal loops. Threading them naively is a race that corrupts results silently rather than crashing — every column would solve against whatever tridiagonal its neighbours were half-way through assembling. Each thread now takes its own slice of a buffer sized `layers × maxThreads()`.
- **`maxRichardson`** was a shared read-modify-write inside a threaded loop. It is only a diagnostic, but a racing diagnostic reports a number nobody can reproduce. Now a `max` reduction.

*A real bug, caught by the gates rather than by inspection.* After wiring the per-thread slices, ten physics tests failed at four threads and passed at one — uniform-tracer preservation, mass conservation, the seiche, the lock exchange. The cause was mine and mechanical: the transport solve wrote its result into its own thread's slice but **read it back from `m_solution[k]`**, which is always thread 0's. I had fixed the momentum loop's write-back and missed its twin. Bisecting by disabling one pragma at a time located it in minutes; the uniform-tracer gate is what made it visible at all, since a corrupted vertical solve still produces plausible-looking output.

*Measured, 102 400 cells, 1 → 4 threads:*

| Phase | 1T (ms/step) | 4T (ms/step) | Speedup |
|---|---|---|---|
| pressure | 4.07 | 1.17 | **3.47×** |
| density | 0.76 | 0.27 | 2.81× |
| transport | 19.71 | 7.31 | 2.70× |
| external | 44.93 | 26.67 | 1.68× |
| momentum | 2.97 | 2.20 | 1.35× |
| fluxes | 1.00 | 1.00 | 1.00× (still serial) |
| **total** | **75.39** | **40.22** | **1.87×** |

Throughput 1.36 → 2.55 Mcell-steps/s. The external mode is now 67% of the threaded runtime and scales worst, which is the granularity story the transport contrast already implied: its regions are ~5 000 columns entered 108 times per step, against transport's 102 400 cells entered once.

*Known next steps, in order:*
1. **Hoist one `omp parallel` region around the whole subcycle.** Now evidence-backed rather than assumed: transport scales 2.70× and external only 1.68× on the same hardware, and the difference between them is region size. Seven parallel regions × ~108 substeps is ~750 fork/joins per internal step, against work of only ~1 300 columns per thread per region. A single region with `omp for` inside removes almost all of that.
2. `computeLayerFluxes` / `diagnoseVerticalFlux` (1.0 ms, still serial).
3. Re-measure on a machine whose topology is known — four threads still underperform three on parts of this container.

- F7.1 Kernel ports in measured order — external mode first (see F7.2), then transport, then the 3-D remainder; GPU plugin behind C ABI (openswmm `GpuPluginAbi` pattern); OMP backend first, CUDA next, HIP/SYCL as CI targets.
- F7.2 CPU/GPU parity harness (tolerance-based per-kernel compare); perf baselines (initial targets: DeGray-scale ≥ 200× realtime on workstation CPU; 1M-cell mesh tractable on a single workstation GPU — refine after profiling).
- **Verify:** parity suite green on OMP+CUDA; profiling report; no Kokkos symbols in core library.

### Phase 8 — HydroCouple component & validation release ⬜ (~8–10 weeks)
- F8.1 `FVQUALComponent` + exchange items (§5.5); YAML composition; IOThread async UGRID outputs; optional GeoPackage summary export (D-F12).
  *Expanded 2026-09-03 by the provider-pipeline program*
  (`HydroCouple/plans/sdk/PROVIDER_PIPELINE_PLAN_2026-09-03.md`, slices
  S3.1-S3.4): S3.1 `FVQualComponentInfo` + `HYDROCOUPLE_DECLARE_COMPONENT`
  plugin entry point (Composer-loadable); S3.2 mesh argument
  (`PolyhedralSurfaceArgument`: UGRID file *and* live
  `initialize(IComponentDataItem&)` from a mesh-generator component's
  output, default rectangular basin preserved when absent); S3.3
  whole-series met argument (argument *and* the live `air_temperature`
  exchange input both stay — user decision) + kinetics `.rxn` File
  argument wrapping `ReactionFile`; S3.4 `fvqual` CLI composition run with
  RunRecorder + manifest so results are consumable downstream via
  `ExecutionMode::Open`.
- F8.2 `ILayeredMeshComponentDataItem` proposal to the HydroCouple interface review.
- F8.3 Coupled demos: openswmm→FVQUAL; FVQUAL→CSHComponent; **side-by-side CEQUALW2Component vs FVQUALComponent on DeGray** (flow, T profiles, DO) with a published comparison report.
- F8.4 Docs (Doxygen + user guide); CHANGELOG per release policy; v0.1.0 tag.
- **Verify:** composition runs headless from YAML; W2-comparison acceptance metrics met; all cross-check gates green in CI.

### Phase 9 — Exploratory tracks ⬜ (post-v0.1)
- F9.1 **Non-hydrostatic q-correction** module through the `PressureClosure` seam (G3 gate).
- F9.2 MPI multi-domain via SDK `Distributed` (`PartitionedDataItem`/`HaloExchanger`; flux+derivative payloads for implicit-boundary stability).
- F9.3 W2 parity kinetics v2 (pH/carbonate, zooplankton, macrophytes, diagenesis); ice cover; estuarine validation case.
- F9.4 ELM momentum-advection option; higher-order reconstruction study.

**Critical path:** P0 → P1 → P2 → P3 → P5 → P6 → P8; P4 and P7 parallelize against P5/P6. Phase PY parallelizes against P4/P6 after PY0, which should precede P6. **First externally useful milestone** (hydro + temperature on real reservoirs): end of Phase 3, ~6–8 months in.

---

## 7. Verification & Validation Plan

Layered V&V, automated in CI where feasible (every capability lands with its verify check):

1. **Unit/property tests:** mesh invariants, well-balancedness, positivity, conservation identities, reaction bit-parity vs openswmm, restart exactness.
2. **Analytical benchmarks:** barotropic/internal seiche, wind setup, MacDonald SWE set, lock exchange, diffusion profiles, rotating advection shapes, age solutions, batch reactors.
3. **Sigma-specific gates:** steep-bathymetry quiescent basin (PG error), thin-layer drawdown stress test, Haney-number mesh screening.
4. **Model-to-model:** CE-QUAL-W2 v4.x on DeGray Reservoir and Columbia Slough (vendored examples) — temperature profiles, outlet temperatures, DO; ELCOM/AEM3D literature cases where published data allow.
5. **Field validation (release-gating for v1.0):** at least one well-instrumented stratified reservoir with multi-year thermistor + WQ data.
6. **Performance regression:** wall-clock per simulated day tracked in CI on a fixed reference mesh (CPU), plus GPU parity/perf after Phase 7.

---

## 8. Risks & Mitigations

| # | Risk | L | I | Mitigation |
|---|---|---|---|---|
| R1 | Sigma pressure-gradient error corrupts baroclinic circulation over steep old river channels | M | H | Density-Jacobian scheme + reference-profile subtraction; hybrid z-sigma below transition depth (specified in SDK plan; implement in F1.1 if absent); Haney mesh gate; dedicated CI test |
| R2 | External-mode subcycling unstable with aggressive wet/dry | M | M | G1 fallback to semi-implicit θ free surface (SPD CG solve); FVHM/HYPRE experience in reserve |
| R3 | Reaction-engine fork drift between openswmm and FVQUAL | H | M | Shared `msxkinetics` library (preferred) or subtree + sync script + shared bit-parity fixtures; upstream plan addendum |
| R4 | UGRID layered-3D ambiguity hurts interoperability | L | M | Follow Delft3D-FM layered-2D convention (D-F2); xugrid/ugrid-checks in CI |
| R5 | Explicit Δt still too slow for multi-year WQ planning runs | M | M | Mode splitting already amortizes; transport rides internal Δt; ELM screening mode for planning runs; tiered LTS (D-F11, F2.2/F5.5, 2–5× expected); GPU (Phase 7); θ semi-implicit promotion via G1 if CPU-era wall-clock is unacceptable |
| R6 | Scope creep toward full W2 constituent parity too early | H | M | Phased `.rxn` template libraries (scoping decision); parity v2 explicitly Phase 9 |
| R7 | Kokkos/MSVC/Windows friction | M | L | openswmm GPU plugin ABI precedent (core never links Kokkos); OMP backend first |
| R8 | HydroCouple interface gap stalls 3D exchange | M | L | Interim id-based/surface items + file IO; `ILayeredMeshComponentDataItem` through interface review (SDK Phase 3 channel) |
| R9 | Single-developer bus factor / long critical path | M | H | Strict phase gates with shippable intermediate milestones (P3 = usable thermal model); plans-as-documents per house rules |
| R10 | Unit-system defects | M | M | D-F8: SI everywhere internally; conversion only at IO boundaries; unit-annotated accessors |

---

## 9. Decision Log

| Id | Decision | Rationale |
|---|---|---|
| D-F1 | New standalone repo `FVQUAL`; core library independent of HydroCouple; component wrapper a separate target | Reuse without lock-in; mirrors openswmm component packaging (user scoping decision) |
| D-F2 | UGRID layered-2D convention (2D topology + layer dim + CF sigma formula terms), not full 3D volume topology | Tool support (xugrid, ugrid-checks, Delft3D-FM practice); matches (face×layer) indexing |
| D-F3 | Hydrostatic core; `PressureClosure` seam + prognostic-capable `w` for a future non-hydrostatic module | Covers W2-class applications; avoids 3D Poisson solve in v1; retrofit made cheap (G3 gate) |
| D-F4 | Explicit barotropic–baroclinic mode splitting + implicit vertical column solves; semi-implicit θ free surface as G1 fallback | Explicit-everywhere is infeasible (surface CFL, thin-layer diffusion); this keeps Kokkos-friendly flat parallelism |
| D-F5 | Eulerian MUSCL+FCT primary WQ transport; ELM as momentum/screening option; RWPT as diagnostics; one engine interface + cross-check gate | Mass conservation non-negotiable for WQ budgets; SCHISM lesson on ELM roles; openswmm 3-engine precedent |
| D-F6 | Reuse openswmm reaction engine via shared `msxkinetics` library; scope enum + 3D env vars; settling lives in transport | Core already mesh-agnostic; D-R7 workload identical; single kinetics codebase across 1D/2D/3D |
| D-F7 | Serial+OpenMP first with Kokkos-ready SoA layouts; GPU via dlopen'd plugin ABI (core never links Kokkos) | User scoping decision; proven openswmm pattern; physics validated before porting |
| D-F8 | SI units internally; conversions only at IO boundaries | Recurring-defect lesson from openswmm plans |
| D-F9 | Temperature and age are reserved transported species; heat fluxes from CSH/HEAT formulations | Alignment with openswmm D-UT5/HEAT plan; one transport path for everything |
| D-F10 | Kinetics scope: tracer/age/decay + DO–CBOD–N–P–algae first; W2 parity v2 later | User scoping decision; de-risks Phase 6 |
| D-F11 | Tiered LTS (column tiers, 2:1 neighbor ratio, flux-buffered interfaces): external mode in P2, internal mode + Eulerian transport in P5, both experimental until the conservation gate passes; reactions rely on the integrator's existing per-cell adaptive substepping | Reservoir meshes are the ideal LTS case (shallow arms set global CFL); openswmm tiered-marcher precedent; 2–5× expected on CPU, where it is most needed pre-GPU |
| D-F12 | UGRID NetCDF/HDF5 is the sole full-field results format; GeoPackage demoted to optional feature-gated derived-summary export (stations, aggregates), off the hot IO path | SQLite row-wise writes cannot sustain 3D field output at scale; UGRID chunked hyperslab appends can (user decision) |
| D-F13 | Reimplement W2 physics from the theory manual with cited equation references; treat the MIT-licensed v4.5 ERDC release as the provenance basis; `THIRD_PARTY_LICENSES` entry for any code-adapted portion | W2 is MIT (§10) so both paths are permitted; formulation-level reimplementation keeps provenance clean and suits a 3D unstructured target better than translating width-averaged structured code |
| D-F20 | Adopt eq. 4's shifted velocity maximum; **refuse** the V5.0 boundary-interference bisection until the contradiction between the technical note and the shipped source is resolved | Eq. 4 is unambiguous and reduces correctly to the port for a symmetric zone. The bisection is not: the note and the code disagree on whether θ multiplies or divides and on `c_bi` (8/2 vs 1/2), and the solved `D'` is measured from the interference boundary but consumed as a distance from the port. A wrong choice here is invisible in every self-consistent test and would only surface against real W2 output |
| D-F19 | Selective withdrawal follows the **shipped W2 variant**: zone limits from a march outward with a locally evaluated *implicit* buoyancy frequency and `c_bi` of 1 (2 near a boundary), and a velocity profile parabolic in density anomaly rather than distance | The leading coefficients and the frequency's evaluation scale are not independent choices — the technical note's `θ π` and `c_bi = 8` combined with a column-scale `N` give a 36 m zone in a 24 m column. Matching the reference means matching the pairing. The V5.0 bisection form for boundary interference (note eqs. 2–3b) and the shifted maximum-velocity location (eq. 4) remain unimplemented and are recorded as limitations |
| D-F18 | An internal transfer's receiving inflow takes its composition from the donor's **cached** withdrawal weights, not from a stated value and not from recomputed weights | The arriving water is the departing water, so its composition is not the modeller's to state; and using the same weights the withdrawal removes mass through makes conservation an algebraic identity rather than something to be bounded by a tolerance |
| D-F17 | Multi-port outlets are operated to a **target release temperature** by heat-conserving blend of the tightest bracketing pair; total demand is supplied by the caller and rating curves act as per-port capacity limits, with any shortfall reported rather than redistributed | Release temperature is the regulated quantity below a dam and the reason selective withdrawal was implemented; total release is set by water rights, generation, and flood control, none of which the reservoir model knows. A shortfall silently absorbed would surface later as an apparent mass-balance error in an unrelated module |
| D-F16 | Level-driven structures are re-rated inside the **barotropic subcycle**, not once per internal step, via a `ColumnSourceUpdater` hook on `BarotropicSolver`; transport receives the time-mean release over the step | The head that drives a spillway changes on the gravity-wave timescale the external mode already resolves, while the internal step may be 100× longer; a stale head drains a reservoir past its own crest. Clamping the release to the local column's storage was tried and is worse — it under-drains ~7× because the barotropic mode refills that column from the whole basin within one internal step |
| D-F15 | Python bindings via a new C API + Cython, per the detailed sub-plan `PYTHON_BINDINGS_PLAN_2026-08-23.md`; true zero-copy state views; base wheel dependency- and Kokkos-free | The openswmm three-layer stack is proven and its packaging lessons are already paid for; allocation-stable stores (D-F7) enable the zero-copy upgrade; an engine without bindings has one user |
| D-F21 | Meteorological forcing is **per column or uniform, chosen per variable**, through a `ForcingField` that stores either one value or a vector. Wind **stress** is derived inside the engine from speed and direction rather than supplied alongside them. Granularity is the surface face, not the cell | A reservoir is small against a weather system but not against its own topography — shading, fetch and sheltering vary along it, and a coupled atmospheric model delivers fields rather than scalars. Keeping the uniform case a single value means the common configuration costs nothing and the existing gates stay bit-identical. Deriving stress in the engine closes the defect that motivated this: the DeGray driver set wind for the heat budget and left `windStressX/Y` at zero, so wind evaporated water and moved none, and the model's April epilimnion was 3 m against a measured 14 m. Two consumers of one variable, one wired and one not, is a class of defect no unit test catches — only a whole-model comparison against measurement did |
| D-F14 | **Supersedes part of D-F5:** ship only the Eulerian engine. ELM and RWPT are deferred indefinitely, and the `TransportEngine` interface with them. Age and source attribution become transported species instead of particle diagnostics | ELM cannot conserve mass, and closed constituent budgets are the product; RWPT conserves it as noise that nonlinear kinetics amplify. Neither earns its second implementation, gates, and abstraction (user decision) |

---

## 10. Licensing & Attribution

*Not legal advice — confirm with counsel before release if anything here carries commercial risk.*

**CE-QUAL-W2 is MIT-licensed.** The ERDC GitHub distribution (`CE-QUAL-W2-ERDC/CE-QUAL-W2`, v4.5) carries `LICENSE.md` = MIT, © Environmental Modeling Team, Environmental Laboratory, ERDC, USACE; cequalw2.org states the same ("source code is released under the MIT License… originally developed with public funding"). MIT is permissive: use, modify, translate, sublicense, and commercialize are all allowed, with one obligation — **retain the copyright notice and license text in copies or substantial portions**.

Two distinct questions, with different answers:

1. **Adapting the formulations (equations, kinetics rate laws, closure relations, selective-withdrawal theory).** Copyright protects *expression*, not ideas, mathematics, or algorithms. Governing equations and published kinetic formulations are not copyrightable subject matter, so reimplementing W2 physics from the theory manual is unrestricted regardless of the code license — and MIT makes it doubly clear. Academic norms still apply: **cite the model and the manual**. Verbatim reproduction of manual *figures, tables, or prose* is a separate copyright question — redraw and rewrite rather than copy.
2. **Copying or translating source code.** A Fortran→C++ translation is a derivative work. MIT permits it, but the notice obligation attaches: any file substantially derived from W2 source must carry the ERDC copyright + MIT text, recorded in a `NOTICE`/`THIRD_PARTY_LICENSES` file. MIT is compatible with FVQUAL's MIT license, so no license conflict arises.

**Practices adopted (D-F13):**
- Work from the **v4.5 ERDC release** as the licensing reference. The v4.1.0 copy vendored under `HydroCouple/` carries **no license header in its source files** — the licensing was clarified in the later release, so do not treat the old copy as the provenance basis for anything code-derived.
- **Prefer formulation-level reimplementation** from the theory manual (with citations in code comments naming manual section/equation numbers) over code translation. This is better engineering anyway — FVQUAL is 3D unstructured FV, so W2's width-averaged structured-grid code rarely transfers directly — and it keeps provenance clean.
- Where W2 source *is* consulted or adapted (e.g. selective-withdrawal envelopes, specific kinetic coefficient sets), record it per-file and add the ERDC MIT notice to `THIRD_PARTY_LICENSES`.
- **Note on the existing wrapper:** `CEQUALW2Component` is **LGPL-3.0** (your own licensing choice for the wrapper, unrelated to W2's MIT terms). Reusing code from it in an MIT-licensed FVQUAL requires relicensing that code — straightforward since you are the author, but do it deliberately rather than by copy-paste.
- Cite CE-QUAL-W2 in FVQUAL's `CITATION.cff`, docs, and any validation publications.

**Also verify before shipping:** the vendored example datasets (DeGray, Columbia Slough) used for validation — bundled examples are distributed with the model, but confirm redistribution terms if FVQUAL ships copies rather than download scripts.

## 11. References

**Internal (paths relative to `~/Documents/Projects`):**
- `cbuahin_github/HydroCouple/plans/sdk/SDK_MODERNIZATION_PLAN.md` — sigma tools, UGRID writers, Kokkos backend, MPI/halo, IO thread
- `cbuahin_github/HydroCoupleSDK/include/hydrocouplesdk/tools/sigmagrid.h` — `SigmaGridGenerator`, `LayeredMesh`
- `cbuahin_github/openswmm.engine/plans/transport/UNIFIED_TRANSPORT_MASTER_PLAN.md` (+ EULERIAN_ARD, MULTISPECIES_REACTIONS_MSX, LARD_AGE_EXPEDITE 2026-08-23, HEAT, TWOD plans)
- `cbuahin_github/openswmm.engine/src/engine/transport/components/ReactionModule/` — reaction-engine audit basis
- `cbuahin_github/openswmm.engine/src/engine/2d/` — explicit FV marcher, LTS, GPU plugin ABI
- `HydroCouple/CEQUALW2Component`, `HydroCouple/CE-QUAL-W2-4.1.0` — W2 wrapper + DeGray/Columbia Slough examples
- `HydroCouple/FVHMComponent` — implicit 2D SWE reference

**External:**
- CE-QUAL-W2 (PSU Water Quality Research Group): https://www.cequalw2.org/ ; https://www.ce.pdx.edu/w2/
- UGRID conventions tooling: xugrid https://pypi.org/project/xugrid/ ; ugrid-checks https://pypi.org/project/ugrid-checks
- Kokkos ecosystem status: KUG 2026 report https://kokkos.org/blog/2026-03-KUG-report/
- Non-hydrostatic σ-coordinate feasibility: Keilegavlen & Berntsen, *Non-hydrostatic pressure in σ-coordinate ocean models*, Ocean Modelling (2009) https://www.sciencedirect.com/science/article/abs/pii/S1463500309000286 ; σ-coordinate non-hydrostatic DG coastal model https://www.sciencedirect.com/science/article/abs/pii/S1463500320302341 ; 3D unstructured non-hydrostatic model for internal waves https://link.springer.com/article/10.1007/s10236-016-0980-9
- Unstructured non-hydrostatic generalized-vertical-coordinate ocean model: https://arxiv.org/pdf/2109.07467
- Semi-Lagrangian conservation analysis: https://tellusjournal.org/articles/10.1111/j.1600-0870.2007.00293.x ; https://arxiv.org/pdf/1910.06476
