# Testing the running build — 2026-09-21

Built at **`8c6d96e`**, against HydroCoupleSDK `9b57e66` installed with its
full IO feature set. Suite: **64 of 65**. Falsifiers all green except `u6`,
which has two survivors (one a harness fault, one a real gate gap). ASan is
clean of memory errors everywhere. Full results: `plans/ROUND3_REPORT_2026-09-21.md`.

Follow §8 of `plans/BUILD_AND_VERIFY_2026-09-20.md` for the 31 steps. This file
is only the delta.

---

## Correction to the earlier version of this file

An earlier revision of this file said the orientation cue lands in the wrong
corner and cannot be switched off. **That was wrong and is withdrawn.** The old
`test_gizmodraw` counted absolute pixels in the corner, so it was counting the
lit sheet behind the cue and reported a full 96×96 block both in the opposite
corner and with the cue switched off. The gate's measurement was broken, not the
renderer; `43370c2` replaces it with a difference between two renders plus a
control, and all four corner cases now pass. Nothing in the renderer changed.

The blank *Sync from 3D* ribbon face is also fixed — that commit adds the five
missing Navigate icons.

## Known broken — do not file these

| Step | What you may see | Gate |
|---|---|---|
| 22 | **Switching to the Map tab zooms the map far out** — ~7.5x the layer extent. Set *Link 2D and 3D views* to *Never* to avoid it | `test_shell_smoke.FramingSurvivesTheMapTabsFirstLayout` |

**`test_selectionhub` is a gate defect, not a selection bug.** The recursion
guard works — the crash is 13 frames deep, not a blown stack. The case builds
its node with `scene.addItem(node)`, which puts it in the *graphics* scene but
never registers it in `CompositionScene::m_nodes`, so `scene.node("catchment")`
returns null and the test dereferences it unguarded. It could not pass as
written. Cross-view selection itself is probably fine, and the five other cases
around it pass — but it is untested until that fixture is repaired, so treat
steps 5–7 as genuinely unverified rather than as known-good.

**`test_scene3d.SceneBoundsCountLayersThatCannotBeDrawn` looks like a real
product fault.** The fixture was repaired in `43370c2` (a plain probe layer
instead of a point layer, which gained a scene source in U3), and with a correct
fixture the assertion now runs and fails: a layer at x 900–1000 with no 3D form
leaves `sceneBounds().maximum().x()` at 100. The gate's own comment says such a
layer "still has to count toward the framing, or switching to the 3D tab would
cut off half of what the map was showing". Worth confirming by eye in step 22.

**The app segfaulted once, before this build,** on a mouse click: null
dereference inside `QWidget::focusOutEvent` for a `QLineEdit`, from
`giveFocusAccordingToFocusPolicy`. No frame of ours below `main`, which is the
signature of a widget destroyed while it still held focus — the U2b argument
windows and the U1 closeable Start tab are the new ways that can happen. It has
not recurred on the current build. If it does, **note what you clicked and what
had focus before it**. Report: `verification/walk/crash_focusout_215324.ips`.

## Worth your eyes

Nothing automated covers these:

- **Step 17 — the depth squeeze.** Orbit until terrain fills the cue's corner.
  Does any arm sink into the ground? The gate checks this against a flat opaque
  sheet, which is not real relief.
- **Step 27 — Cancel on an argument window.** Type an edit, press Cancel,
  confirm nothing changed. The plan calls this the one worth doing by hand.
- **Steps 13–15** — exaggeration to 10, tilt straight down, click the arms. The
  cue's arithmetic is gated; how it *reads* is not.
- **Step 22** — view linking set to *Never*, and whether framing survives a tab
  switch. Related to the scene-bounds failure above.
- **Step 31** — three argument windows at once over the map.

## Covered by green gates — skip unless something looks off

1–4 (`test_welcome`), 9–10 (`test_scenemarkers`), 11–12 and 18
(`test_axisgizmo` 17, `test_gizmodraw` 4), 19–21 and 23–24 (`test_navigation`
12, `test_icons`), 25–26 and 28–30 (`test_argumentdialogs` 11), 32
(`test_argumentkinds` 10, against the rebuilt SDK).
