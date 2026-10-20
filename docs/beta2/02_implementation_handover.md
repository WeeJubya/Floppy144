# B2-02: Clean-room Windows implementer handover

**Purpose:** hand to an engineer who has **never opened** the River2D-derived F144 runtime implementation. This is a self-contained technical brief together with `02_runtime_specification.md`, `02_api_contract.md`, `02_acceptance_matrix.md` and the publicly permitted headers. **Do not implement or alter any production source in B2-02.** Implementation is a later authorised stage (B2-03 or subsequently approved task).

## 1. Repository and reference identifiers

- Development: `WeeJubya/Floppy144`, **only** existing `development/Beta-2-runtime` branch. Do not develop against the old WeeJubya `main` or push to the release account.
- Published read-only Beta.1 reference: `GreyDoorRepublik/Floppy144-beta` `main` and tag `Floppy144` at `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`.
- Beta.1 GitHub release EXE metadata: `Floppy144.0.4.0-beta.1.exe`, **822272 bytes**, SHA-256 `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749`. This hash is **unverified from downloaded bytes**. Executable maximum **1474560 bytes**.
- Starting Stage-4 development history used in B2-01: `46c2a81e097abf69734a4b2f0b7bc91f9142c256`. Published tree differs from Stage-4 in seven files; use explicit SHA per evidence.
- The owner confirms GPLv3 is intended for Beta.1; accidental omission of `LICENSE` in the GDR publication is scheduled for separate correction. Do not unilaterally relicense or claim compliance resolved.
- Existing Windows Actions baseline `39486754386` includes a **failed** intro floppy-label assertion, although a separate audio/Release job rebuilt an 822272-byte EXE with zero warnings/errors. This failure must not be ignored or rewritten merely to obtain green CI.

## 2. Permitted clean implementer inputs (allow-list)

**Primary design and acceptance documents** (all project-authored):
- `docs/beta2/02_runtime_specification.md`: complete behavioural requirements, pixel format/geometry/window/error semantics and official Win32 references.
- `docs/beta2/02_api_contract.md`: exact platform callback order, fields, types, input/lifecycle semantics and native-service boundaries.
- `docs/beta2/02_acceptance_matrix.md`: deterministic, native, regression and human comparison test IDs and oracles.
- `docs/beta2/02_implementation_handover.md`: this division of work and allowed/forbidden sources.
- `docs/beta2/01_baseline.md` **only its observable behaviour, SHA/source drift, build/test and capture sections**; do not use historical provenance percentages/source matching to design code.

**Permitted project-owned public contracts and game-neutral code**, where necessary for compilation/integration:
- `include/f144_platform.h`, `include/f144_startup_config.h` and `src/f144_platform.c` (portable game-facing interface/dispatch, not derived engine).
- `game/src/floppy144_draw.h`, `game/src/floppy144_input.h`, `game/src/floppy144_lifecycle.h`, `game/src/floppy144_timing.h` (types and drawing/cadence obligations; no game implementation need be transplanted).
- `include/f144_win32_input.h`, `include/f144_win32_lifecycle.h`, `include/f144_win32_timing.h`, `include/f144_win32_storage.h`, `include/f144_win32_single_instance.h`, `include/f144_win32_audio.h` (project-owned service **declarations**).
- Existing platform/input/lifecycle/timing/persistence/audio *tests* to establish behaviour; see `02_acceptance_matrix.md`. Existing `premake5.lua`, `testbuild.ps1`, `cleanbuild.ps1`, `run.ps1`, `tools/build_game_data.ps1` and `.github/workflows/stage3c-ci.yml` are authorised only for build/test orchestration, not as engine design templates.
- Microsoft Learn Win32 documentation linked in `02_runtime_specification.md`, the Windows SDK declarations and general C99 references. Independently authored public-domain-like standard solutions may be considered only after licensing/source review; project design permits **no added third-party engine/library**.

**Existing project components to reuse, not copy into a new graphics runtime:** portable `Floppy144Core` gameplay renderers, canonical data pipeline, project-owned Win32 storage/calendar/single-instance/startup configuration/input/lifecycle service implementations and WinMM procedural music/SFX where their provenance is separately approved. Reusing existing compiled modules is allowed; reimplementing them is not part of the display replacement. `src/f144_win32_timing.c` currently has a legacy-context dependency: its header is permitted, but source is **integration-review only**, not an implementer design aid.

## 3. Forbidden references (clean-room firewall)

**Do not open, read, import, reformat, translate, port, compare line by line, paraphrase or copy any source/comment/data structure from:**
- `include/f144_runtime.h`
- `src/f144_runtime.c`
- `src/f144_win32_runtime.c`
- `src/f144_win32_platform.c`
- `include/f144_win32_platform.h`
- historic `BadAcronym/river2D` source, dynamic/static River2D software-renderer source and renamed previous engine versions, including commit diffs showing derived implementations.
- historical `docs/stage4/runtime_provenance.md` and `docs/beta2/01_provenance_inventory.md` as **implementation inputs**. They belong to a distinct audit/reviewer role.
- the old `game/src/floppy144_main.c` implementation body and any private F144 runtime fields/callback installation during the isolated implementer phase. The main coordinator is largely game-owned but contains tainted legacy glue; a separate maintainer will perform the integration with independent interface work.

Do not use renamed `F144Runtime`/`F144Image` layouts, engine image/composition abstractions, their private function signatures, internal algorithms, error strings or copied comments. A familiar name is not authorisation to preserve implementation expression. The clean backend must have new, independently designed private types and lifetime; only the **public** `F144PlatformApi` surface is fixed.

Maintain an evidence log recording what each new source file was designed from (B2-02 requirement/test ID or official API page), with author/review date. A reviewer **separate from the clean implementer** may analyse source provenance and independently compare submitted code to forbidden code after implementation; the comparison results must not be used to coach a line-by-line rewrite.

## 4. Functional build contract from first principles

The independent engineer must build a fresh native Win32/GDI implementation, without needing to inspect any forbidden file. This is a task-level description of **outputs**, not an instruction to mimic a previous call graph:

1. **Window host:** native class registration; a single ordinary resizable application window; message processing; client-area size and DPI change notifications; OS paint/expose; orderly close/destroy and errors.
2. **Frame store:** independently chosen checked allocation of **640×360×4 = 921600** addressable bytes, writable as `uint32_t`; top row first; x/y bounds; colour `0x00RRGGBB`; stable borrowed `Floppy144Surface` until teardown; no inconsistent/partially initialised view.
3. **GDI presenter:** transfer the latest fully completed pixels into the window client area with nearest/crisp scaling; no crop; centre maximum 16:9 rectangle; residual opaque black bands; support all intermediate sizes including cases smaller than native and zero-size suppression. Verify the exact geometry table in the spec/acceptance matrix. Never clear the entire destination to black immediately before redrawing its content.
4. **Platform binding:** populate all 15 `F144PlatformApi` callbacks and existing `F144Platform` fields with new backend storage behind the opaque `state` pointer. Reuse portable dispatch and project-owned platform services. Do not enlarge the public API table.
5. **Event bridge:** independent Windows message-to-existing-action/text/lifecycle delivery, with exact existing tests for arrows, W/S, A/I, navigation, held direction cancellation, `WM_CHAR` and lifecycle transitions. Keep game screen policies outside native code. Source input/lifecycle service implementations may be linked unchanged if they pass tests and provenance review.
6. **Services and shutdown:** reuse timing wake and monotonic calendar providers (adapt legacy private context access with **new code**, not copied glue), keep local storage and single-instance rules, and rebind WinMM procedural music/SFX; close/reset them once on shutdown, including partial failures.
7. **Integration shim:** expose newly authored private bootstrap/bind/present/teardown interfaces to a separately responsible coordinator integrator. The clean implementer defines their own **private** signatures (not derived originals); the integrator changes the Windows launcher to call these and remove old private-runtime references. Gameplay/drawing functions and save codecs must be left alone.

**Acceptable implementation variability:** own Win32 window class/context types, allocation method, whether frame publication uses a separate copy or synchronised UI thread, whether GDI uses `StretchDIBits`, and internal error objects. **Not variable:** public ABI, logical pixels, colour/orientation, visible geometry, input semantics, legacy save/audio compatibility, independent expression, single EXE and byte cap.

## 5. Minimal required tests built by the implementer

New deterministic L0 math/colour tests and Windows L1 memory/GDI/lifecycle/error tests must cover every `MEM-`, `PIX-`, `GEO-`, `RND-`, `INP-` and `LIFE-` case in `02_acceptance_matrix.md`, particularly:
- guarded first/last pixel writes, overflow/fault allocation, zero-size and double teardown;
- pure channel/orientation fixture and top-down GDI target capture;
- intermediate size `1000×700 -> (0,69,1000,562)` and pillarbox `1200×500 -> (156,0,888,500)`;
- repaint of unchanged frame after exposing/resizing; no black flash;
- physical key down/up and text separation, lost focus releasing holds;
- startup, close request, destruction, message-pump -1 failure, audio-device absent and storage errors;
- strict x64 Release 0-error/0-warning build and ≤1474560-byte EXE; compare imports/source file list against audit inventory **by an independent reviewer**.

Reuse existing regression suites without weakening. A documented test-injection seam for allocator/native failures is acceptable; no changes to public API solely to make tests easier.

## 6. Build, verification and clean-room review instructions

Windows VS2022 Developer PowerShell with MSVC x64, Premake 5.0.0-beta8:
- Regenerate canonical data with `.\tools\build_game_data.ps1` and ensure no unintended changes to generated files.
- Run the existing independent regression suite with `.\testbuild.ps1`. The current **known intro-label failure** is a baseline discrepancy; document it independently if still present.
- Generate VS2022 solution via `premake5 vs2022` and run the existing project build `.\run.ps1 -build release`; CI strict step is `msbuild .\build\Floppy144.sln /t:Rebuild /m /p:Configuration=release /p:Platform=windows /p:WarningLevel=Level4 /p:TreatWarningsAsErrors=true`.
- For end-to-end clean validation use `.\cleanbuild.ps1`, but only after the pre-existing failing regression has been resolved via an **authorised separate change** or clearly reported.
- Record warnings, tests, size, SHA-256, changed files, the final integration diff and an explicit clean-room source review. Match public release hashes only after independently verifying source fixture identity.
- Avoid relying on GitHub Actions branch filters for `development/Beta-2-runtime`; explicitly document whether manual CI dispatch/test setup ran. Do not claim a build or GUI approval without actual evidence.

## 7. Division of labour and deliverables for the subsequent stage

**Clean implementer:** brand-new native Windows display/backend source and *private* headers; test-only harnesses; optional new win32 platform binding file; clear author log referencing `02_...` requirements and Microsoft APIs. May request a semantic change to the public contract but must not implement it without approval.

**Integration maintainer (can inspect original game launcher for wiring only):** remove inherited runtime project/source includes from active build; adapt `game/src/floppy144_main.c` and the native timing binding to independently authored backend interfaces; retain `src/f144_platform.c` and actual game renderers/audio/save modules; ensure one window, clean lifecycle, resource order and no gameplay changes. May not transplant implementation bodies from forbidden files.

**Independent provenance reviewer:** inspect added files and final link manifest for absence of derived code/expression, accidental copies and third-party introductions; maintain historical source-audit evidence separately from the implementer's brief. Review existing licence obligations and published Beta.1 corrections outside the clean code specification.

**QA:** execute L0-L3 matrix; reproduce and review actual Beta.1 captures from a hash-verified EXE; capture before/after screenshots, full audio and save round-trip; record failures; approve only evidence-backed parity.

Required future change-set output: source/tests, Release executable size/hash, clean-room provenance log, source-manifest diff, regression logs, baseline visual/audio sign-off, documentation/STATUS update, commit and push **only on the development branch**.

## 8. Final specification gate and outstanding inputs

**Completeness review:** every required host/framebuffer/presentation/input/lifecycle/service/error behaviour has an observable contract and test oracle. A Windows C99 engineer with the permit-listed public interface headers and this handover can produce an independently designed backend without reading River2D/F144 engine source. This is a **static documentation assessment**, not proof that such a build has already occurred.

**Not yet cleared for final behavioural equivalence:** the published Beta.1 executable's hash has not been independently calculated, visual/framebuffer reference fixtures have not been captured or signed off, DPI/Windows minimum/version-specific details remain OPEN, and the pre-existing Stage-4 intro-label regression fails. The intended GPLv3 licence's missing public file also requires verification of the owner's promised separate correction. These are outstanding verification/governance decisions, **not** permission to consult the inherited implementation.

**Stage boundary:** B2-02 ends at committed documents. Do not delete or replace engine files, modify build targets, change gameplay, fix the intro test or add implementation code in B2-02.
