# BF35: Grey Door Republik Easter Egg overhaul

Branch: `development/Stage-4`. Starting HEAD: `bed1b7e71cb44e1eccc44d9629c7596d11cb8065`.

## Root causes and solution

The old impossible room was a largely flat composition, separate from the
Site ISO projection. The percentage also increased during the Developer's
dialogue, and glitch distortion did not begin until the false counter was
already 144%. The normal Site HUD was subsequently painted over the
corruption, making that counter look immune to the breakdown.

BF35 reuses the Site ISO line, prism and projection rasterisers via a
scene-local projection context. The existing Site camera globals are restored
after every temporary prism draw. Floor, only the two rear walls, and
volumetric furniture have bright bespoke colours. The Developer remains the
shared procedural office character, enlarged without adding a new model.
The concurrent BF36 neck-geometry fix is already present in the shared renderer;
hairstyle reference verification remains an outstanding BF36 task.

The Developer automatically turns after five seconds without Inspect.
Inspecting first preserves identification, turn and the existing spoken line.
The true restoration percentage is frozen through dialogue; an accelerating
cubic curve then runs from the true saved percentage to **exactly 144%**
across the capacity and interference phases, with a 300 ms terminal hold.
The original red flash for values over 100% is unchanged.

Glitches begin while the counter is still moving. Intensity increases with
the same scene clock, using deterministic displaced horizontal slices, short
colour-corrupted rectangles, sparse vertical-sync tears and intermittent
corrupted warning text. The interference pass runs after the Site HUD, so
the false counter is affected too. Frames are freshly rendered on each tick,
without retaining mutated pixel state or introducing stored assets.

No persistent world position, recovery capacity, evidence, collection,
profile, or save schema has changed. The original two-second blackout,
return, save lockout, immediate one-shot autosave, and existing legacy save
compatibility all remain in place.

## Verification

The persistence regression adds focused 4.9/5.0-second reaction checks,
player Inspect path, completed dialogue gating, strict acceleration and
visible exact-144 checks. Existing candidate-matrix tests retain world,
evidence, save, one-shot and blackout coverage. The Grey Door source audit
checks the ISO reuse and interference-after-HUD path.

Windows Release compilation, size gate and actual on-screen colour,
occlusion and visual degradation must be verified on Windows CI and in
a real game session. BF36 hairstyle accuracy remains pending original-artwork comparison, while
its shared neck removal is inherited by this scene.
