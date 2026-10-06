# Round 3 report

**Composer `8c6d96e` · SDK `9b57e66` · macOS arm64, Qt 6.9.3**

Everything in §5 and §6 has now been run. §8 is reported at the end, with
what a screenshot cannot reach called out rather than glossed.

---

## Build

Both clean.

The documented SDK commands in §1 still do not work on this machine, for two
reasons that are new since your correction:

- **`macos-full` inherits `macos`, which specifies `Unix Makefiles`.** This
  machine's SDK build directory is Ninja, so the preset refuses it:
  `Error: generator : Unix Makefiles / Does not match the generator used
  previously: Ninja`. Using the preset means discarding the existing tree.
- **`CMakePresets.json` declares no `testPresets` at all**, so
  `ctest --preset macos-full` cannot run either.

I applied the same cache variables to the Ninja directory instead. The
resulting install is what matters and it is correct: both targets exported,
17 `NetCDFUGRIDWriter` symbols, the new mesh-view gate present.

## ctest

| | Result |
|---|---|
| SDK | **330 / 330** |
| Composer | **64 / 65** |

One failure, and it is a **regression introduced by `8c6d96e`**:

```
ShellTest.FramingSurvivesTheMapTabsFirstLayout
tests/gui/test_shell_smoke.cpp:389: Failure
Expected: (visible.width()) < (extent.width() * 3.0), actual: 7518.2578125 vs 3000
  the map is drawn far smaller than the space it has
```

**Cause established by A/B, not inference:**

| `linkViews` | Result |
|---|---|
| `OnTabSwitch` (default) | fails at 7518 |
| `Never` | passes |

You called the scene-bounds fault latent. The `extent()` fallback made it
live, in the *map* rather than the 3D view: every layer's extent now reaches
the 3D bounds, and U5's view linking pushes that framing to the map on a tab
switch, so the map opens at ~7.5× the layer extent. This is your own step 22,
failing automatically. The fallback is right in isolation; the question is
whether 3D bounds should feed map framing from a layer with no 3D form.

## Falsifiers

| Script | Result |
|---|---|
| `u1/falsify_welcome_tab` | **12 / 12** |
| `u3/falsify_markers` | **10 / 10** |
| `u4/falsify_gizmo` | **22 / 22** |
| `u5/falsify_navigation` | **18 / 18** |
| `u2/falsify_kinds` | **22 / 22** |
| `u2/falsify_dialogs` | **15 / 15** |
| SDK `u2s/falsify_typedarguments` | **14 / 14** |
| `u6/falsify_selection` | baseline **green**; 10 caught, **2 survived** |

The fixture repair worked — `u6`'s baseline is green for the first time, so
its results mean something now. Two survivors, different in kind:

```
SURVIVED H3 the re-entrancy guard is gone (recurses; caught by exit status)
SURVIVED V1 the map drops the modifiers, so a click always replaces
```

**V1 is a harness defect, not a missing gate.** The script's `rebuild()` and
`gate_ok()` are hard-coded to `test_selectionhub`:

```sh
rebuild() { cmake --build "$BUILD" --target test_selectionhub ...; }
gate_ok() { timeout 120 "$BUILD/tests/gui/test_selectionhub" --gtest_filter="$1" ...; }
```

V1's filter is `MapToolsTest.*`, and that binary contains **zero**
`MapToolsTest` cases — I checked. A gtest filter matching nothing exits 0, so
the script reads it as a pass. The Shift-adds gate you added is correct and
simply cannot be reached from here. V1 needs to build and run `test_maptools`.

**H3 is a real gate weakness.** I applied the mutation by hand, rebuilt and
ran the case: **with the guard deleted, `TheTwoDirectionsDoNotChaseEachOther`
passes.** The repaired fixture connects only `hub → scene`, so there is no
return path to recurse through. The window wires both directions, so the guard
still matters in the product and nothing now proves it. Wiring the scene's
selection change back into the hub would restore the loop the gate is named
for.

## ASan

**Zero AddressSanitizer errors, everywhere** — all eight Composer suites and
the SDK's typed arguments.

| Suite | |
|---|---|
| axisgizmo 17, gizmodraw 4, navigation 12, argumentkinds 11, argumentdialogs 11, scenemarkers 8 | pass |
| SDK `TypedArgumentTest` | 7 pass |
| `test_shell_smoke` | the framing regression above |
| `test_selectionhub` | see below |

`test_argumentdialogs` exited 137 in an earlier round with an empty log. It
passes 11/11 alone and in this round — that was the OS killing it under memory
pressure, not a fault. Disregard it.

**One Debug/Release divergence worth your eye.**
`SelectionHubTest.SelectingAComponentSelectsThatOneAndClearsTheRest` fails in
the ASan/Debug build (`first->isSelected()` is false at
`test_selectionhub.cpp:269`) and passes in Release. Consistent across runs and
independent of ordering; ASan reports no memory error. A case that disagrees
between builds is unreliable whichever answer is right.

## Docs — the count has grown, 6 → 12

You asked for the list if it grew. Four are the pre-existing ones. The new
ones, all from this stack:

```
include/configurator/componentconfigurator.h:90: too many @param commands for
  ComponentConfigurator::editArgument. Found 3 while function has 1 parameter
  (arguments 'message' and 'payload' are not in the list)
include/layers/featurelayer.h:207: FeatureLayer::resolvedMarkerSize has @param
  documentation sections but no arguments
include/map/mapcanvas.h:242: parameter of MapCanvas::selectIn(const QRect &,
  SelectionMode) is not documented
include/map/mapcanvas.h:255: parameter of MapCanvas::pickAndSelectAt(const QPoint &,
  SelectionMode) is not documented
```

`compositionscene.h`'s `@from` moved from line 62 to 74; same warning.

The two `mapcanvas.h` ones are the `SelectionMode` parameters U6 added — the
same parameters V1 exists to protect.

## §8 — the walkthrough

**What a capture cannot reach.** `QRhiWidget` cannot create a device under the
offscreen platform, so the 3D tab renders blank in any screenshot I can take.
Steps 11–18 cannot be judged from an image on this harness. The cue's pixels
are covered instead by `test_gizmodraw`, which renders through
`renderSceneToImage` — all four corners, the empty viewport, the opaque sheet,
and the camera turning. Both of §7's first two items are answered green there.

**Captured:** `artifacts/composer-scene.png`, `artifacts/composer-welcome.png`.
They confirm two items directly: the *Sync from 3D* ribbon face carries its
icon (step 19's group), and the Start tab shows its close button and is absent
in the closed capture (steps 1–2).

**Checked automatically:** step 22 — that is what found the framing
regression. Step 4 — the component search path is in the live preferences
domain and the palette loads from it. Step 32 — `test_argumentkinds` 11/11
against the rebuilt SDK.

**Covered by green gates, not walked by hand:** 1–4, 8–10, 19–21, 23–26,
28–32.

**Genuinely needs a person at the keyboard, and is not done:**

- **Step 17**, the depth squeeze against *real relief*. The gate uses a flat
  opaque sheet, which is not the same question.
- **Step 27**, Cancel on an argument window — you called it the one worth
  doing by hand.
- **Steps 13–16**, how the cue reads at exaggeration 10, straight down, and
  under clicks on and off the arms.

## The focus-out crash

Agreed and left alone. One occurrence, before `43370c2`, no recurrence since.
Report kept at `verification/walk/crash_focusout_215324.ips`.

## Housekeeping

- Composer falsifier scripts are executable now; **`u4/falsify_gizmo.sh` was
  not**, and the documented invocation failed with 126 rather than running. I
  ran it with `bash`. (It is `-rwx--x--x` in the SDK too — owner-executable
  only, which works but is odd.)
- `plans/TESTING_NOW_2026-09-21.md` is the operator-facing delta, kept current.
