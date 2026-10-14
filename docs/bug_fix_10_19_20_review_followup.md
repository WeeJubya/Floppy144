# BF10 / BF19 / BF20 user acceptance follow-up

Starting branch: development/Stage-4, 784ad53. References: the two screenshots
supplied on 14 October 2026, and the current authored Intro.pptx.

## BF10

The suppression panel still painted four schematic control blocks after the
new recovered-item grid was introduced. Its fixed spacing also put the last
block beyond the variable-width panel. Remove this extra block row; retain the
panel outline and the authoritative recovered-item grid.

Doors used the nominal FRONT plane even when authored width/depth made that
plane the thin edge. Choose the broader vertical plane for both door panelling,
catch and child plates. Regression tests now reject overflow beyond the panel
shadow and reject door items on the narrow edge. Existing desk, shelf and
fixture projection tests remain active.

## BF19

Selection alone previously enabled glow. Require the existing
Floppy144SiteAvailableActions result, plus the shared read-only secure-cabinet
Access eligibility, to expose Inspect or Access before the
selected object is highlighted. Reuse gameplay eligibility rather than changing
focus, proximity, visibility or interactions. The optional fixed inspection
symbols retain their established eligibility and setting.

## BF20: renderer investigation

Intro.pptx has layered vector shapes, not photographs. The Site renderer carries
world projection/camera logic; the Inspection pseudo-isometric renderer clips
its drawings at x<306 to protect the Contents list. Reusing either full renderer
would change the authored composition and require unrelated camera/clip work.
The chosen approach uses the existing software drawing primitives and compact
vector geometry flattened from the actual PowerPoint groups. No stored images
or new runtime library are introduced. The updated framing includes the narrow
disk sliver, a continuous zoom/reveal, the left-hand CRT and right-hand drive,
and the transition into the full-screen terminal. Small rounded/snipped shape
details are simplified at the logical 640x360 resolution. Visual approval is
still required; a framebuffer render is not an exact PowerPoint render.

The generated arrays can be regenerated from the supplied deck using the
optional development script (python-pptx required for this script only):

```sh
python tools/compile_intro_vectors.py /path/to/Intro.pptx
```

The runtime does not load the deck or use python-pptx.

Existing named profiles display their stored LOGIN name and enter the Main Menu
250 ms after boot finishes. They do not accept replacement text, require Enter,
or rewrite the profile. An unassigned profile still waits for explicit LOGIN,
with the existing validation, GUEST and failed-save retry behaviour. Credits
replay follows the same profile policy.

## Accepted and deferred audio/content

User accepted BF18, BF21 and BF23. BF18 notebook text will be reviewed during
play. BF22 and BF24 are deferred to separate chats. Their code and review status
are unchanged by this follow-up.

## Verification and remaining acceptance

Focused portable Inspection tests and intro tests pass. Windows CI must verify
the final commit's complete regression suite, Release build and 1,474,560-byte
size gate. Manually check the suppression panel, doors in both orientations,
nonactionable/actionable glow, all four intro scenes, first-run LOGIN, stored
profile automatic LOGIN and Credits replay. Animation framing and visual
fidelity remain user acceptance decisions.
