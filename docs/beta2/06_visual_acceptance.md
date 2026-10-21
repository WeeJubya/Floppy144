# B2-06 Windows visual and audio acceptance checklist

**Date:** 21 October 2026. **Status:** AUTOMATED FRAMEBUFFER TESTS PASSED; **HUMAN DESKTOP VISUAL/AUDIO ACCEPTANCE NOT PERFORMED**. Do not equate deterministic rendering checks with a human visual review or approval to release.

## 1. Reference and capture provenance

- Verified public **Beta.1**: `GreyDoorRepublik/Floppy144-beta`, release tag `Floppy144`, source commit `0981e4ab3694a1cdb5b6338c199e0262a6ed14bf`; downloaded public EXE **822,272 bytes**, independently SHA-256 checked as `7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749` in B2-06 Windows CI.
- **Beta.2 candidate**: `WeeJubya/Floppy144` `development/Beta-2-runtime`. The strict MSVC x64 Release at code commit `ba3674cc4f7e744e6b2c2c557955b09b9db77f50` was **973,312 bytes**, SHA-256 `E24B27EC4B4B97ACE9D5545001CC65C25BE2241490963CF9DDF7385BC751FFFB`. A later source/document-only build can have a different hash: use the **actual binary's** logged hash for each comparison.
- CI frame artefacts: the new `Beta.2 Release Candidate Verification` workflow runs deterministic intro frames through `tools/stage4f_intro_tests.c` with `F144_BETA2_CAPTURE_DIR`, and `tools/stage4_persistence_paths_tests.c` can emit a procedural Grey Door `bf33_grey_door.ppm` via `F144_BF33_CAPTURE_DIR`. Frames are P6/PPM **640×360 raw RGB**, not Windows desktop captures.
- The public Beta.1 release contains **no supplied save/profile/settings snapshots** or approved frame-pixel hashes. The B2-06 Windows job did independently download/hash the actual Beta.1 EXE and verify its source tag. No previous screenshot was silently substituted for a verified reference. Historical game source files matched in 13 stable code paths, and native offscreen pixel tests passed, but that is **not** an equivalent seeded desktop A/B recording.

## 2. Controlled Windows human test procedure

Use a Windows x64 GUI desktop session, not a headless Actions service. Preserve a **read-only** copy of the verified Beta.1 EXE. Record Windows build, graphics adapter, primary DPI %, screen recording frame rate, monitor/HDR/colour profile, game client-area dimensions (not outer window) and final EXE SHA for *each* run.

Use separate disposable Windows user profiles or clean VM snapshots for Beta.1/Beta.2, since both resolve the same `%APPDATA%\Floppy144` directory and share single-instance ownership. Seed equivalent recovery game states, calendars, operator names, difficulty and developer-mode flags only when verified for both builds. Preserve original save/profile/settings files before testing; never overwrite a real player's profile. Label deliberate Beta.1-to-Beta.2 approved content differences (such as the no-lettering disk insertion and later Stage-4 fixes) before comparing output.

At each checkpoint record: exact input script and frame/timestamp; expected and observed behaviour for **both** binaries; screenshot/video file path, SHA-256, observed difference, reviewer name/date, PASS/FAIL. A source screenshot or PPM from a regression harness is **supporting evidence only**.

## 3. Required human visual acceptance matrix

| ID | Scene/procedure | Pass criteria | Actual state |
|---|---|---|---|
| VIS-01 | Native 640×360, standard 1280×720 client | 640×360 logical image, correct RGB/top-down orientation; no crop, stretch, swapped channels or black partial content | **UNVERIFIED HUMAN** |
| VIS-02 | Resize client to 800×600 and 1000×700 | Viewports 800×450 at y=75 and 1000×562 at y=69; crisp nearest-pixel edges and equal permitted residual bands | **UNVERIFIED HUMAN** |
| VIS-03 | 1200×500 and 700×1000 clients, min/max/restore | Pillarbox x=156, width=888, and portrait viewport x=0,y=303,w=700,h=393; black unused bands, no stretched aspect | **UNVERIFIED HUMAN** |
| VIS-04 | Walk in office and ISO room, repeatedly resize/uncover/alt-tab | Frame-by-frame video shows no intermediate full-black game frames, tearing, uninitialised rows or palette corruption | **UNVERIFIED HUMAN** |
| VIS-05 | Intro discovery → reveal → insertion → terminal startup | Correct movement, logo, screen transitions, CRT effects; approved blank floppy label near drive, no accidental disappearing disk | **UNVERIFIED HUMAN** |
| VIS-06 | Empty/GUEST and named operator profile, restore session | Correct LOGIN prompt when name=GUEST, profile name typing, menus, Settings, CRT/text-speed options, persistence warning state | **UNVERIFIED HUMAN** |
| VIS-07 | Terminal authentication, documents, catalogue, keyboard text | Correct ASCII glyphs, colour/spacing, cursor, scroll, page changes and input repeat; no RGB inversion or text clipping | **UNVERIFIED HUMAN** |
| VIS-08 | Reception, Secretary, Director, Staff, server room Site movement | Canonical office geometry, close table/chair adjacency, walls, collision/occlusion and player facing/animation | **UNVERIFIED HUMAN** |
| VIS-09 | Notebook, Document Ledger, inspection overlays, ISO containers | Labels "RIGHT > Document Ledger" and "< Recovery Notebook"; no rear legs showing through desks/chairs, inspection markers not covered | **UNVERIFIED HUMAN** |
| VIS-10 | Grey Door discovery, office, secret content, effect transitions | Posters/screen in 2.5D and all effects render crisply, no clipping or unexpected tint/black flicker | **UNVERIFIED HUMAN** |
| VIS-11 | Recovery completion, Credits, final note, return to menu | All end screens match approved scenes and transitions; no stale geometry or inappropriate Credits replay route | **UNVERIFIED HUMAN** |

**Automated supporting tests:** B2-03 Win32 offscreen GDI colour/top-down/bands/resize test (79 assertions) and B2-04 host API/paint integration (25 assertions) passed in Windows CI. Full Stage 2/3/4 screen-source/pixel tests are separately recorded in `06_test_results.md`. None of those tests inspect a user's actual desktop composite or all intermediate animation frames.

## 4. Separate audio/hardware acceptance

| ID | Procedure | Pass criteria | State |
|---|---|---|---|
| AUD-H01 | Menu music, procedural recovery track and completion cue | Musical acts, tempo/progress changes and scene boundaries are heard correctly | **UNVERIFIED HUMAN** |
| AUD-H02 | Step/inspection/terminal/backspace SFX | Correct cue identities and mix; no missing/duplicated notes | **UNVERIFIED HUMAN** |
| AUD-H03 | Music/SFX volume 0 and 10, mute, settings restart | Independent, correctly persisted volume controls; mute silence | **UNVERIFIED HUMAN** |
| AUD-H04 | Alt-tab, device unavailable/changed, close/relaunch | No stuck WinMM waveOut/MIDI handle, notes continuing after quit or crash | **UNVERIFIED HUMAN** |
| AUD-T01 | Automated procedural audio and scheduling tests | Source/API/synthesis cue and deterministic timing invariants | **AUTOMATED PASS**, not audible sign-off |

Audio capture must be recorded separately from visual videos where feasible; note sound device, playback driver and listening reviewer.

## 5. Persistence, lifecycle and startup manual validation

| ID | Test | Status |
|---|---|---|
| CMP-01 | Backup; restore actual **published Beta.1-created** manual save and autosave in Beta.2, reload, re-save, compare progress and bytes-on-disk expectations | **UNVERIFIED**: published release contains no save files |
| CMP-02 | Repeat for Beta.1 profile and Settings; confirm operator name, control volumes and indicators | **UNVERIFIED** public Beta.1 data |
| CMP-03 | Check `%APPDATA%\Floppy144` stable location, no files written to install directory | **AUTOMATED PATH CONTRACT PASS**; human installed-game test **UNVERIFIED** |
| CMP-04 | Quit/relaunch, rapid X-close, minimise/restore, single-instance second launch | **AUTOMATED PLATFORM/INSTANCE TESTS PASS**; real UI **UNVERIFIED** |
| CMP-05 | Existing approved developer command-line options (`-GDR-CinderEllie`, `-GDR-Hathaway`), fixed-date/seed; verify mode does not persist unintentionally | **AUTOMATED CONFIG TESTS PASS**; actual launch **UNVERIFIED** |
| CMP-06 | Tracked **older development** prototype samples: manual V1/64 and auto V1/68 versus Beta.1 V1/72 | **Known compatibility limit**: old samples are rejected; Beta.1/current decoder matches. Approval needed if importing these older formats is required. Historical profile/settings samples load and round-trip. |

## 6. Manual sign-off (uncompleted)

- [ ] Reviewer performed both EXEs on Windows GUI under equivalent documented conditions.
- [ ] Screenshots/videos uploaded and hashed with reproducible seed/profile records.
- [ ] All VIS-01..VIS-11 items reviewed and signed, or individually fail with follow-up defect.
- [ ] All AUD-H01..AUD-H04 checks completed on real audio hardware.
- [ ] Public Beta.1 **created** save/profile/settings data restored in Beta.2 with no loss.
- [ ] Reviewers adjudicated any intentional approved Beta.1-versus-current differences.
- [ ] No untriaged flicker, input, pixel corruption, crash or content regression remains.

**Disposition:** HUMAN ACCEPTANCE **OPEN**. The existence of automated frame artefacts, successful strict Release and verified binary baseline **must not** be read as this checklist being completed.
