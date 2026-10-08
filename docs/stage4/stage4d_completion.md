# Stage 4D Presentation and Visual Upgrade Completion Record

**Status:** **STAGE 4D: PASS**

Stage 4D is accepted as integrated. The dedicated S4D-06 gate reproduces the
complete Stage 3 gameplay regression suite, all Stage 4B and Stage 4C gates,
every Stage 4D visual regression, the fresh-profile Stage 4C integration smoke,
independent portable Core and Win32 platform builds, a clean production-
equivalent Release rebuild, and the 1,474,560-byte executable-size gate.

No Stage 4E atmosphere, content or replay-variation work is included in this
sign-off.

## Baselines and commit range

**Immutable Stage 3 baseline:** `fe2239123f98346ed06b48bb500672a5e6e2e172`  
**Stage 4C completed baseline:** `dc2ab35d4dd481e3c46a688ed6053be7c800f3ae`  
**Stage 4D implementation start / S4D-01:** `386e52e026a3b5194bc30f86bdc4eb77d2fefc5d`  
**S4D-05 validated presentation head:** `370ef65d66920120a53b9ec6acb42800cbf38551`  
**S4D-06 integration-gate commit:** `bd289e0f219c51f6fb2129a1ff9e35788c7315db`  
**Stage 4D final sign-off:** the commit containing this completion record.

The production Stage 4D feature range is therefore S4D-01 through S4D-05,
followed by the S4D-06 acceptance gate and this completion record.

The pre-documentation S4D-06 validation was GitHub Actions run
`37733081136`, which completed successfully against
`bd289e0f219c51f6fb2129a1ff9e35788c7315db`.

## Delivered Stage 4D sequence

| Task | Primary commit | Delivered |
| --- | --- | --- |
| S4D-01 | `386e52e026a3b5194bc30f86bdc4eb77d2fefc5d` | Widened three-page Terminal Help presentation without changing Help content or pager behaviour |
| S4D-02 | `493f16476414d34dac601ef93f5044d1af87c723` | Reduced Site camera scale and extracted fixed-point camera transform |
| S4D-03 | `11182a9f65a323a7c2872bfb2ed4a297971fda57` | Procedural directional player, animation and Profile body-style rendering |
| S4D-04 | `34934638a8e1774dfecf7736fd10bffe71df4529` | Pseudo-isometric Inspection parent rendering with Main Office presentation unlock |
| S4D-05 | `b5d76d18473e408d10e3f76fed9b2acbecf706c2` | Cross-screen presentation grammar, shared scrollbar and prompt consistency |
| S4D-06 | `bd289e0f219c51f6fb2129a1ff9e35788c7315db` | Dedicated Stage 4D integration and visual-acceptance gate |

Follow-up commits within each task corrected or hardened regression harnesses,
clipping and presentation contracts; they did not introduce Stage 4E content.

## Final Help layout

Terminal Help remains three pages.

The established Terminal shell remains unchanged:

- panel: 620 x 280 px;
- Help text origin: x=34;
- safe right edge: x=606;
- usable Help text width: **572 px**;
- output begins at y=84;
- output row step: 15 px;
- divider: y=260;
- footer remains in the Terminal-specific footer band.

Adjacent authored Help rows may be packed together only when the resulting
rendered line remains inside the 572-pixel safe width. Authored blank rows still
force paragraph breaks. The content, page order and three-page pager semantics
remain unchanged.

The dedicated Help regression drives all three pages, both page-3 capability
states, paging/back behaviour, fragment preservation, text-width bounds and
rendered right-edge clipping.

## Final exploration camera

The Stage 4D Site presentation scale is:

**12 pixels per Site unit, or 60% of the historical 20 px/unit view.**

The exploration viewport remains 576 x 252 px at x=32, y=32, yielding an exact
visible span of:

- **48 Site units horizontally**;
- **21 Site units vertically**.

The camera remains a presentation transform over canonical x16 fixed-point
world coordinates. It does not rescale or rewrite world geometry.

The transform path remains:

1. canonical Site x16 world coordinates;
2. existing Site-view transform;
3. subtract camera top-left x16;
4. multiply by pixels-per-unit;
5. divide by the fixed-point unit;
6. add the fixed viewport origin.

The regression builds the production camera for every registered room,
including corridors, room edges and the exact-fit Records Office case. Existing
Stage 3B interaction/collision tests remain authoritative for doorways,
furniture, prompts and interaction distances.

## Player rendering architecture

The Stage 4D player is procedural software rendering; there is no sprite sheet
or external bitmap asset.

The transient presentation state contains only:

- facing;
- moving/not moving;
- one two-frame walk phase;
- a millisecond animation accumulator.

Supported visual facings are:

- left;
- right;
- up;
- down;
- idle while retaining the last valid facing.

Horizontal movement alternates arms and legs. Vertical movement also changes
limb/body presentation so the player does not slide as a static pawn.

The gait frame interval is **160 ms**. Elapsed time comes from the Stage 4B
timing abstraction through `presentation_elapsed_ms`; the player renderer does
not call Win32 timing APIs.

The Site player presentation remains approximately **42 x 72 px** at the final
12 px/unit camera scale. The visual size does not redefine the gameplay
collision footprint.

## Supported body styles

The Stage 4C Profile continues to expose exactly two cosmetic body styles:

- **TYPE A**: narrower torso with a central uniform detail;
- **TYPE B**: broader torso with twin shoulder details.

Both styles use the same canonical:

- world position;
- visual outer footprint;
- collision width/depth;
- movement step;
- movement speed;
- interaction range.

The Profile preview and Site exploration use the same procedural player
renderer. Profile persistence round-trips the body-style value independently of
RunState.

## Inspection renderer

The enhanced Inspection renderer is procedural and parameterised. It reads
existing parent dimensions and the projection-neutral Site rectangle to recover
authored footprint and rotation without changing either.

Current renderer families cover:

1. desks;
2. GDR/IT terminal desks;
3. workbenches;
4. secure/non-secure cabinets;
5. bookcases and shelving;
6. tables, worktops and sink furniture;
7. chairs;
8. trolleys;
9. sofas;
10. servers/fridges;
11. wall-mounted fixtures;
12. doors;
13. small appliances;
14. generic future/unknown fallback.

The current generated Site contains **32 canonical visual variants** across
furniture and fixtures. The dedicated Inspection regression enumerates every
current canonical variant and requires each current class to render.

Dimensions remain presentation inputs, so examples such as a 6x4 desk, 2x6
bookcase and 18x5 worktop retain visibly different proportions.

Signed authored rotation is recovered through
`Floppy144SiteRectForParentId()`. The diagonal Secretary Office desk is
explicitly regression-tested.

Wall-mounted objects use a shallow panel vocabulary instead of freestanding
cabinet geometry.

Before Main Office reconstruction the established Stage 3 schematic remains
active. After Main Office reconstruction, the exact same CabinetState,
selection, contents and gameplay semantics use the enhanced 2.5D presentation.
The unlock is presentation-only.

Unknown/future classes render through a bounded generic fallback rather than
crashing or rendering nothing.

## Presentation consistency result

The final shared visual grammar includes:

- one formal GDR footer baseline for Session Control, Profile, Settings,
  Credits and Completion;
- consistent `UP/DOWN`, `LEFT/RIGHT`, `PGUP/PGDN`, `ENTER`,
  `BACKSPACE` and `ESC` key spelling;
- consistent Back/Confirm action wording where the same interaction is meant;
- one shared proportional scrollbar primitive used by Catalogue, document
  viewer, Notebook, Completion and Inspection;
- truthful Site `ESC SESSION CONTROL` wording;
- Terminal pager Q consistently described as a return action.

Deliberate screen identities remain intact. Terminal still reads as a terminal;
Notebook keeps its paper treatment; Completion remains special; Site Directory
remains a full-screen map; Inspection retains its 2.5D furniture view; and the
opening title remains an unboxed presentation sequence.

## S4D-06 integration audit

The dedicated gate reviews Stage 4D production paths for:

- direct Win32 rendering/timing dependencies;
- TODO/FIXME/HACK markers;
- heap allocation in camera/player/Inspection rendering;
- camera mutation of RunState/world state;
- body-style ownership of collision or persistent RunState;
- animation timing bypassing the Stage 4B clock;
- Inspection rendering mutating selection/gameplay state;
- object-specific IDs embedded in the production 2.5D renderer;
- duplicate/abandoned presentation routes;
- loss of the deliberate pre-Main-Office schematic path;
- loss of the thin compatibility wrapper for the historical body-style Site
  draw entry point;
- loss of Stage 4D visual coverage from CI.

### Duplicate and legacy path review

No abandoned Stage 4D drawing path remains.

`Floppy144Site2DDrawForBodyStyle()` is retained deliberately as a thin
compatibility wrapper around `Floppy144Site2DDrawForPlayerState()`; it is not
a second player implementation.

`Floppy144CabinetDrawContainerBody()` remains deliberately live as the
pre-Main-Office schematic. It is not dead code.

The old Site camera calculations were removed from the monolithic Site renderer
when the fixed-point camera module was extracted.

Scrollbar rendering is centralised in `Floppy144DrawScrollbar()`.

No Stage 4D TODO/FIXME/HACK marker remains in the production rendering paths.

## Visual acceptance coverage

### Help

Validated:

- page 1;
- page 2;
- page 3 before capability unlock;
- page 3 after capability unlock;
- next/previous paging;
- no clipping;
- no overlap with the divider;
- complete authored fragments.

### Exploration

The camera regression builds every registered room using the production
camera. Stage 3B continues to exercise canonical geometry, doors, collision,
interaction targeting, reconstructed/unreconstructed state and prompts.

The final 12 px/unit scale preserves whole-pixel movement for the existing
half-unit movement step.

### Player

Validated:

- left;
- right;
- up;
- down;
- idle;
- rapid direction transitions;
- simultaneous/diagonal input presentation;
- TYPE A;
- TYPE B;
- animation accumulation;
- delayed/variable frame timing;
- viewport clipping;
- Profile persistence;
- unchanged movement/collision constants.

### Inspection

Validated:

- every current canonical parent variant;
- dimensional differences;
- diagonal orientation;
- wall-mounted fixture;
- unusually wide parent;
- empty contents;
- one item;
- many items;
- selected item marker;
- scrolling/list separation;
- unknown fallback;
- pre-Main-Office schematic;
- post-Main-Office 2.5D presentation;
- unchanged contents count/selection across the visual unlock;
- secure/locked container behaviour through Stage 3B.

### Cross-surface smoke

CI remains headless and does not automate a visible Windows desktop with mouse
or keyboard injection.

The existing Stage 4C fresh-profile integration smoke still runs in the same
sign-off workflow and exercises Profile creation/editing/body style, Settings,
Terminal, restored-session state and Completion over the actual portable
modules.

The same run combines that fresh-profile route with Stage 3B and Stage 4D
headless renderer tests covering exploration, Main Office reconstruction,
Inspection, Notebook/document presentation contracts and the upgraded visual
systems. A separate brittle desktop-driving harness was not introduced.

## Regression matrix

GitHub Actions run `37733081136` validated the S4D-06 integration gate.

| Gate | Result |
| --- | --- |
| Canonical runtime-data regeneration | **PASS** |
| Stage 2 regression | **PASS** |
| Stage 3A regression | **PASS** |
| Complete Stage 3B regression | **PASS** |
| Stage 4 platform boundary | **PASS** |
| Stage 4 logical input | **PASS** |
| Stage 4 persistence paths | **PASS** |
| Stage 4 audio contract | **PASS** |
| Stage 4 timing/lifecycle | **PASS** |
| Stage 4 single-instance protection | **PASS** |
| Stage 4 developer configuration | **PASS** |
| Stage 4C Profile | **PASS** |
| Stage 4C operator-name editing | **PASS** |
| Stage 4C Terminal authentication | **PASS** |
| Stage 4C Settings | **PASS** |
| Stage 4C Credits | **PASS** |
| Stage 4C restored-session presentation | **PASS** |
| Stage 4C Completion | **PASS** |
| S4D-01 Help presentation | **PASS** |
| S4D-02 exploration camera | **PASS** |
| S4D-03 directional player | **PASS** |
| S4D-04 2.5D Inspection | **PASS** |
| S4D-05 presentation consistency | **PASS** |
| S4D-06 integration/visual-acceptance audit | **PASS** |
| Stage 4 build architecture | **PASS** |
| Stage 4B integration sign-off | **PASS** |
| Stage 4C integration/fresh-profile smoke | **PASS** |

## Architecture acceptance

The intended presentation dependency remains:

```text
Game State / World Data
          |
          v
Portable Rendering Logic
          |
          v
640x360 Software Framebuffer
          |
          v
F144 Platform Presentation
```

The Stage 4D camera, procedural player, Help reflow, shared scrollbar and 2.5D
Inspection renderer operate entirely in portable software-rendering code.

They do not call Win32 GDI, native Win32 timers, window APIs or platform input
APIs directly.

### Architecture exceptions

No new Stage 4D architecture exception was introduced.

The inherited Stage 4B/4C exceptions remain:

- `floppy144_main.c` is still the Win32 application-shell/coordinator and owns
  HWND/message-loop/redraw glue while Stage 4 services migrate behind F144
  boundaries;
- `floppy144_persistence.c` still combines portable persistence codecs with
  Windows filesystem operations and remains outside `Floppy144Core`;
- the current platform implementation is Win32 only;
- the Win32 audio backend remains intentionally silent.

None of these exceptions is used by the camera, player or 2.5D renderer to draw
their presentation.

## Performance sanity

No obvious Stage 4D performance regression was found.

- The camera uses integer/fixed-point arithmetic and performs no heap
  allocation.
- The player renderer uses compact primitives, no external assets and no heap
  allocation.
- Player animation redraw is requested only when the 160 ms visible walk phase
  changes; idle state does not animate continuously.
- The 2.5D renderer uses bounded software scanlines/lines inside the left
  Inspection panel and allocates no per-object heap memory.
- Inspection 2.5D work occurs when the Inspection screen is rendered; it does
  not add a continuously running world-render pass.
- Shared scrollbars replace duplicated drawing code rather than adding parallel
  UI systems.
- CI visual tests and the production-equivalent Release build complete without
  timing or resource failures.

No optimisation change was required for sign-off.

## Production build and size

The S4D-06 validation build uses the workflow's x64 production-equivalent
Release rebuild with Level 4 warnings and warnings treated as errors.

| Metric | Stage 3 baseline | Stage 4C | Stage 4D |
| --- | ---: | ---: | ---: |
| Release executable | 659,968 B | 681,984 B | **695,808 B** |
| Delta from Stage 3 | - | +22,016 B | **+35,840 B** |
| Delta from Stage 4C | - | - | **+13,824 B** |
| Size limit | 1,474,560 B | 1,474,560 B | **1,474,560 B** |
| Remaining headroom | 814,592 B | 792,576 B | **778,752 B** |
| Core warnings/errors | - | 0 / 0 | **0 / 0** |
| Win32 platform warnings/errors | - | 0 / 0 | **0 / 0** |
| Final application warnings/errors | 0 / 0 | 0 / 0 | **0 / 0** |

The Stage 4D executable occupies **47.1875%** of the hard floppy-size budget.

The complete Stage 4D visual programme adds 13,824 bytes over the accepted
Stage 4C baseline while introducing no external sprite-sheet or bitmap payload.

## Known visual limitations

The following are deliberate compact-renderer limitations, not sign-off
failures:

- player facing is four-way; diagonal movement uses the most recent axis as the
  facing tie-break;
- the walk cycle is deliberately two-frame rather than biomechanically rich;
- Inspection is pseudo-isometric, not a general 3D renderer;
- Inspection rotation is represented with a compact deterministic orientation
  treatment rather than arbitrary perspective projection;
- furniture families are parameterised; individual parent objects do not each
  carry bespoke artwork;
- CI validates deterministic framebuffer/layout contracts but cannot replace a
  human aesthetic review of a live desktop build.

## Deferred items

Stage 4D leaves atmosphere, additional content treatment, replay variation and
other Stage 4E work untouched.

No unresolved Stage 4D correctness defect is deferred into Stage 4E.

Inherited portability/persistence/audio limitations remain tracked by the
earlier Stage 4 architecture records and are not presentation regressions.

## Stage 4E readiness

Stage 3 gameplay remains green. Stage 4B architecture remains green. Stage 4C
identity/settings/completion remains green. Every Stage 4D presentation gate is
green, the clean production build is warning/error free, and 778,752 bytes of
the executable budget remain.

**Stage 4E Atmosphere, Content and Replay Variation can begin.**

## Gate result

**STAGE 4D: PASS**
