# B2-02: Windows runtime acceptance matrix

**State:** test specification only. Dates: 21 October 2026. **No tests in this file were run for B2-02.** The existing Stage-4 baseline CI result is separate (see `01_baseline.md`). Normative specification: `02_runtime_specification.md`; API: `02_api_contract.md`.

## 1. Verification levels and evidence rules

- **L0** pure arithmetic/unit tests in portable C, deterministic fixture data; expected to run in the first implementation milestone.
- **L1** native Windows backend-integration tests on the VS2022/MSVC x64 runner; can use an offscreen memory-DC or test window when the OS allows it. No desktop-user screenshot is implied.
- **L2** full-game Windows automation and existing regression suite.
- **L3** actual Windows GUI/manual play, human listening and reference-image/video comparison against a verified released Beta.1 executable.
- **Gate notation:** `N` = not yet executed/new test; `E` = established source test exists (not rerun by B2-02); `B` = reference baseline absent/blocking final parity; `P` only if an actual test has passed and its log is linked. B2-02 sets no new test to P.

Each test result needs: test ID, runner Windows build/CPU/bitness/DPI, source commit SHA, configuration/toolchain version, command, result, artefact path and SHA-256 where applicable. Retain precise logs for failed tests, including any Windows GetLastError values without leaking user filesystem secrets. Do not modify old tests solely to make them pass.

## 2. Framebuffer allocation and memory safety

| ID | Level | Procedure / fixture | Required result | State |
|---|---|---|---|---|
| MEM-01 | L0/L1 | Initialised backend returns surface | Non-NULL pixels, width 640, height 360, address suitable for uint32_t and exactly 230400 accessible pixels | N |
| MEM-02 | L0/L1 | Check address first/last pixel; use sentinels or guard pages around bytes | No writes before start or after 921600-byte logical span; canary unmodified | N |
| MEM-03 | L0/L1 | Fill entire frame, clear, then read all values | No uninitialised bytes, deterministic black after clear | N |
| MEM-04 | L1 | Simulate framebuffer allocation failure/denied pages via test-injection seam | Startup aborts gracefully, no invalid `framebuffer()` and no OS resource leak | N |
| MEM-05 | L1 | Call init/teardown repeatedly, including teardown before init and double teardown | No double free/invalid Windows handles; pointer becomes unavailable after teardown | N |
| MEM-06 | L0/L1 | Supply invalid native dimensions: zero, negative, larger than signed DIB limit, arithmetic overflow | Explicit rejection or suppressed paint, no OOB, truncation, divide by zero or huge allocation | N |
| MEM-07 | L1 | 1000 window resizes while drawing patterned frames; same surface pointer/sizes checked | Logical 640x360 surface remains valid; no use-after-free/stale buffer | N |
| MEM-08 | L1 | ASan (/fsanitize=address) plus Windows handle/GDI object census before and after stress | No sanitizer findings attributable to new runtime and no monotonic handle growth | N |

## 3. Colour accuracy, origin and transfer

Fixture `COLOUR`: pixel (0,0)=red `0x00FF0000`; (639,0)=green `0x0000FF00`; (0,359)=blue `0x000000FF`; (639,359)=white `0x00FFFFFF`; centre (320,180)=yellow `0x00FFFF00`; all other pixels black. Pixel (10,10) tests `0xFF123456` with nonzero upper byte. Fixture `EDGE`: 1-pixel vertical red and blue stripes alternating, plus black/white checkerboard. Compute reference hashes over little-endian 32-bit values **after** standardising the unused high byte, before OS colour management; record hash algorithm and bytes in future test fixture.

| ID | Level | Procedure | Required result | State |
|---|---|---|---|---|
| PIX-01 | L0 | Read raw `COLOUR` memory indices | Exact 0x00RRGGBB values, top-left-origin, row order, no channel exchange | N |
| PIX-02 | L1 | Present `COLOUR` to a top-down 32-bit GDI reference DIB and read source/destination | Corner colours remain at correct corners, top/bottom not inverted, red/blue not swapped | N |
| PIX-03 | L1 | Present high-byte `0xFF123456` | Visible RGB `0x123456` only; no unexpected transparency | N |
| PIX-04 | L1 | Present `EDGE` at native, 2× and 1000×700 client | Only source colours appear; no blended intermediate colours or blurred edges | N |
| PIX-05 | L1/L3 | Re-present an unchanged published frame 20 times | Byte-identical client image (memory-DC L1); visually stable desktop L3 | N |
| PIX-06 | L3 | Compare the game intro, terminal, notebook, 2D/2.5D site against approved Beta.1 source fixture | No unapproved visual drift; reference screenshots/hashes are pending | B |

For L1 strict colour tests, measure an offscreen uncolour-managed 32-bit GDI target; for L3 screenshots record desktop colour-profile/HDR settings and avoid claiming exact RGB identity if compositing modifies them.

## 4. Client-area layout and resize

Use the geometry oracle from `02_runtime_specification.md`. Assert **client area**, not outer `GetWindowRect` bounds. Rectangles are x/y/w/h. For nonzero clients any odd residual band pixel lies on right or bottom.

| ID | Client size | Expected content rectangle | State |
|---|---|---|---|
| GEO-01 | 640×360 | 0,0,640,360 | N |
| GEO-02 | 1280×720 | 0,0,1280,720 | N |
| GEO-03 | 800×600 | 0,75,800,450 | N |
| GEO-04 | 1000×700 | 0,69,1000,562 | N |
| GEO-05 | 1200×500 | 156,0,888,500 | N |
| GEO-06 | 1024×768 | 0,96,1024,576 | N |
| GEO-07 | 700×1000 | 0,303,700,393; lower band 304 | N |
| GEO-08 | 320×180 | 0,0,320,180 | N |
| GEO-09 | 1×1 | 0,0,1,1; defined degenerate small window | N |
| GEO-10 | 0×360, 640×0, 0×0 | No GDI transfer; logical surface stays valid | N |
| GEO-11 | 1000×300 | 233,0,533,300; right band 234 | N |

| ID | Level | Procedure | Required result | State |
|---|---|---|---|---|
| RND-01 | L1 | Resize through every geometry size including maximise, minimise, restore | Full game rectangle fits; black bands only outside, no aspect creep | N |
| RND-02 | L1/L3 | Resize quickly across portrait/landscape and restore from minimised | No stale image outside current viewport, no divide-by-zero/crash | N |
| RND-03 | L1 | Trigger `WM_PAINT` without game update after uncovering window | Previously completed frame reappears, identical content | N |
| RND-04 | L1/L3 | Trigger background erasure and repeated repaint; inspect frame sequence | No full-client black clear between old and new image; unused bands black | N |
| RND-05 | L1/L3 | Change Windows display DPI / move between monitors | Recomputed client geometry; logical surface unchanged; no recursive DPI-resize problem | N, OS/version gate OPEN |
| RND-06 | L1 | Simulate GDI transfer failure (invalid HDC/test seam) | Error recognised, no dereference/corruption, later paint may recover | N |
| RND-07 | L3 | Record rapid window resizing and expose while animation continues | No visible black flashes or avoidable smearing as adjudicated by human review | B |

## 5. Input, text and lifecycle

| ID | Level | Procedure | Required result | State |
|---|---|---|---|---|
| INP-01 | L0/L1 | Existing stage4 input mapping regression (`tools/test_stage4_input.ps1`) | Arrow movement; W/S navigation; A/I distinct; Enter/Back/Page/N/Esc correct; unmapped Q/0/Space remain text | E |
| INP-02 | L1 | Down/up mapped and unmapped keys | Action/type identity preserved; tested physical_token equals tested VK value | N |
| INP-03 | L1 | Hold left+down, release left, hold up+down, focus loss | Diagonal combination, cancellation, correct independent release; no stuck direction | N |
| INP-04 | L1/L3 | Enter Q/0/Space, Backspace, Enter in terminal/cabinet with held-key repeats | Text stream separate from action stream; no duplicate game action or lost character | N |
| INP-05 | L1/L3 | Switch keyboard layouts, Alt/menu focus then reactivate | No stale input; non-ASCII expansion not accepted as completed requirement without separate test plan | N |
| LIFE-01 | L0/L1 | Stage 4 timing/lifecycle tests | `WM_ACTIVATEAPP` active/inactive; `WM_CLOSE` request; `WM_DESTROY` completed; `WM_SIZE` is not lifecycle | E |
| LIFE-02 | L1 | Close via title X, Escape->Quit, repeated close, destroy during partial startup | Request precedes exactly one completed shutdown; no use-after-free, no duplicate save | N |
| LIFE-03 | L1 | Window-class registration failure and CreateWindow failure injection | Clean nonzero startup failure, error available, no leaked resources | N |
| LIFE-04 | L1 | Force message-pump error, then normal WM_QUIT | -1 logged/handled, 0 terminates loop, no endless loop | N |
| LIFE-05 | L1 | Active->inactive->active and suspend/resume model events | Defined lifecycle transition, request_autosave retained only when requested | N |

## 6. Service integration, compatibility and packaging

| ID | Level | Procedure | Required result | State |
|---|---|---|---|---|
| API-01 | L0/L1 | Compile unchanged `include/f144_platform.h` and mock 15-slot vtable | ABI signatures and slot order identical; no Windows leakage into game-facing headers | E + N extension |
| API-02 | L1 | Verify dispatch of framebuffer/present/quit/time/calendar and null-safe optional façade use | No bypass, correct returns, no crash on unavailable optional callbacks | N |
| CLK-01 | L0/L1 | Fake monotonic timestamps 1000,1015,1016,1050,1500,2500 plus backwards 2000 | Same 16ms, 50ms and 500ms game cadence and clamp expectations from existing tests | E |
| CLK-02 | L1 | Timer setup failure, stop/restart, rapid wake and long idle | Correct error handling, no busy-spin or duplicate timer effects, game timing rules preserved | N |
| STO-01 | L1/L2 | Save/manual, autosave, operator profile, settings and legacy path round-trips using readonly Beta.1 fixtures | Exact format/read compatibility; no accidental rewrite of fixture; profile path preserved | N |
| STO-02 | L1 | Full/denied/absent storage paths, interrupted save simulation | No corruption, safe return/fallback, no writes outside allowed profile area | N |
| INST-01 | L1/L3 | Launch two processes with same profile; exit first; relaunch | Only one owns instance; no duplicate save/session or orphan lock | N |
| AUD-01 | L1 | Existing procedural MIDI and SFX test suites; no audio device/volume 0/10 | Existing cue semantics, clean failure and release; no external or stored sound | E + N integration |
| AUD-02 | L3 | Human listen to menu/game/completion, SFX and mute, alt-tab and shutdown | No unintended audio regression; approved listening fixture missing | B |
| BUILD-01 | L2 | `.\testbuild.ps1` then `.\cleanbuild.ps1` on Win2022 runner | All required regressions green; **known Stage-4 intro-label test presently fails**; do not mask | E (failure), N after fix |
| BUILD-02 | L2 | Premake 5.0.0-beta8/VS2022 x64; MSBuild strict Release clean rebuild | 0 errors, target 0 warnings, standalone EXE ≤ 1474560 bytes | N (prior unrelated CI size evidence only) |
| BUILD-03 | L2 | Scan binary imports, file list and source inclusion | No River2D-lineage compiled source/strings, additional graphics DLL or stored-audio asset; Windows system libraries permitted | N |
| BUILD-04 | L2 | Hash built EXE, separately download and hash published Beta.1 asset | Each hash logged; Beta.1 must match GitHub-reported `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749` or discrepancy investigated | B |
| BUILD-05 | L2 | Review new sources for provenance, copied comments/internal structures, licence and generated data changes | Independent expression, audited file changes, unchanged gameplay/save semantics | N |

## 7. Reference screenshot and audio fixture register

No L3 assets were independently captured in B2-01 or B2-02. **Do not invent screenshot hashes.** Future fixture record fields: `fixture_id, Beta.1 source SHA, executable SHA-256, Windows build, DPI, client size, capture time, reproducible seed/date/profile save hash, input script, image/audio file hash, reviewer/date, expected change rationale`. Needed coverage: intro disk approach and login, session control/carousel, profile, settings, credits, terminal, documents, Notebook/ledger, Site 2D/2.5D, inspection, Grey Room, final completion and restore/quit. Include native/2x/intermediate/window-resize captures and sound listening notes.

Baseline reference acquisition is a **separate human/manual verification task**. Unit tests may be implemented before these references exist, but **final parity certification is blocked**.

## 8. Acceptance gate summary

**Specification completeness: REVIEWED FOR IMPLEMENTER USE, subject to remaining baseline evidence.** A C99/Win32 developer can design the standalone display/runtime backend from these contracts, public headers and Microsoft API references without access to inherited runtime source. This is a **document review judgement**, not a demonstrated successful independent build. Require at minimum: all new L0-L2 tests passed, strict clean Release and size gate, source-difference/provenance review, Windows L3 visual and audio sign-off and published Beta.1 baseline hash verification before declaring runtime replacement done.
