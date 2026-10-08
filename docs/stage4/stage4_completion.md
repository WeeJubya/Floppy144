# Stage 4 Completion and Stage 5 Handover

Repository: WeeJubya/Floppy144  
Branch: Stage4  
Formal gate: S4F-04 Stage 4 Final Integration, Release-Candidate and Sign-Off  
Scope: Stage 4A through Stage 4G  
Immutable Stage 3 baseline: fe2239123f98346ed06b48bb500672a5e6e2e172  
Final production implementation entering this sign-off: d79280a63db280d4cec7421649e1f16a7055ac99

> This is an engineering/release-candidate record. It does not provide legal
> advice and it does not claim that CI is a human-observed Windows playthrough.

## Gate decision

The Stage 4 engineering baseline satisfies the automated release-candidate gate:

**STAGE 4: PASS**

The sign-off workflow runs every established Stage 3 and Stage 4 regression,
the cross-phase S4F-04 source audit, independent Core and Win32 platform builds,
a clean production-equivalent Windows Release rebuild, application-icon checks,
the release-output audit, the decompressed floppy-size gate, and a filtered
Stage 4 source-package sanity check.

The final remote branch SHA is the commit containing this handover plus any
strictly sign-off-only correction required by that workflow. Because a Git
commit cannot contain its own SHA, the exact final branch HEAD and successful
Actions run are recorded in the external S4F-04 completion report and Git
history. The production implementation SHA above is immutable input to this
gate.

## Stage 4 starting point

Stage 4 began from the completed Stage 3 source:

| Item | Baseline |
| --- | --- |
| Branch source | development/stage-3c-complete |
| Commit | fe2239123f98346ed06b48bb500672a5e6e2e172 |
| Release executable | **659,968 bytes** |
| Hard decompressed limit | **1,474,560 bytes** |
| Starting headroom | **814,592 bytes** |
| Compiler warnings/errors | **0 / 0** |

The Stage 3 gameplay/progression contract remains the authority wherever Stage 4
did not explicitly change presentation, platform abstraction, identity,
deterministic atmosphere or the isolated Grey Door end-case.

## Major features delivered

| Phase | Delivered |
| --- | --- |
| **4A** | Immutable Stage 3 baseline, platform-dependency inventory, runtime provenance/licensing audit and architecture proposal |
| **4B** | Floppy144Core, F144 platform API, Win32 platform implementation, logical input, AppData paths/migration, semantic audio boundary, monotonic timing/lifecycle, single-instance protection and startup configuration |
| **4C** | Persistent Profile/operator identity, body style, Settings, Credits/provenance presentation, terminal authentication, restored-session presentation and bespoke Completion/Profile history |
| **4D** | Widened Help, reduced exploration scale/camera, procedural directional player, 2.5D Inspection/Contents and cross-screen presentation grammar |
| **4E** | Stateless deterministic flavour, portable calendar/date override, seasonal noticeboard, takeaway/crossword/paperback variants, seeded DR-04 arrangement and archive expansion |
| **4F** | Native application icon, 10.4-second procedural found-floppy/GDR intro and launch-to-completion presentation journey pass |
| **4G** | Hidden orphan record, deterministic Grey Door corridor placement, impossible-office/Developer vignette, 144% gag, CRT glitch, one-shot persistence and save/reload hardening |

No Stage 5 balancing or broad release cleanup is included.

## Final architecture

The accepted dependency direction is broadly:

    FLOPPY//144 portable game systems
                  |
                  v
            Floppy144Core
                  |
                  v
           F144 Platform API
                  ^
                  |
         platform implementation
                  ^
                  |
         Windows launcher / app

The Windows build contains three targets:

    Floppy144Core             portable static library
    Floppy144PlatformWin32    Win32 implementation static library
    Floppy144.exe             Windows launcher/application

Accepted contracts:

- logical input reaches game code as platform-neutral actions/text events;
- persistence roots are provided by platform storage and Stage 3 locations are migrated conservatively;
- music/SFX volume and semantic audio requests use the platform API;
- monotonic timing and lifecycle events use Stage 4B abstractions;
- seed variation is stateless/keyed and portable;
- calendar/date-aware content uses the F144 calendar provider;
- game presentation renders into the existing 640x360 software framebuffer;
- Win32 single-instance ownership remains outside Core;
- the icon/resource file belongs only to the Windows launcher;
- Grey Door placement/scene code is Core-side and contains no direct Win32 or uncontrolled PRNG dependency.

## Remaining portability exceptions

These are explicit debt, not hidden release failures:

1. floppy144_main.c still combines WinMain/message-loop duties with some top-level game coordination. A future port should extract the remaining portable coordinator policy into a platform-independent application module.
2. floppy144_persistence.c still mixes portable binary codec/schema logic with Windows/MSVC file I/O and atomic replacement. It remains quarantined in the application target, outside Floppy144Core.
3. floppy144_storage.c remains application-level migration glue rather than a fully Core-owned storage policy.
4. The Win32 presenter/runtime remains the only shipping platform implementation. The portable Core boundary is real, but Linux/macOS/mobile backends do not yet exist.
5. Legacy F144 presenter/runtime structures remain on the Win32 side. They must not simply be copied into new platform backends.
6. The audio API is abstracted but the current Win32 backend is intentionally silent. There is no shipping music/SFX playback backend.
7. Mobile/background lifecycle policy is not defined. Stage 4 provides the abstraction but Windows does not invent mobile pause/background semantics.
8. Windows application resources and single-instance behaviour are natively platform-specific by design.

The architecture gate scans the declared Core source/header ownership and independently compiles Floppy144Core with strict warnings.

## Combined regression result

The final Stage4 workflow is the single combined acceptance path. It runs the canonical runtime-data regeneration and generated-registry drift check, the complete Stage 2/3A/3B regression contract, all Stage 4B platform/input/persistence/audio/timing/single-instance/configuration/architecture gates, all Stage 4C Profile/identity/Settings/Credits/reinstate/Completion/integration gates, all Stage 4D presentation gates, the complete Stage 4E deterministic/date/archive/replay matrix, all Stage 4F icon/intro/journey gates, the complete Stage 4G Grey Door matrix, and this final S4F-04 release-candidate audit.

Stage 4G includes 22 derived corridor candidates and 12 fixed seeds x four pre-completion contexts, for **48 complete headless journeys**, including physical save/reload boundaries.

A successful final workflow is the combined regression result: **PASS**.

## Fresh-profile route

The strongest automated equivalent is the Stage 4C integration smoke combined with the inherited gameplay/rendering suites.

It starts from a zero-history unnamed Profile, renders Profile setup, enters and commits an operator name, changes body style, exercises all Settings categories, begins a fresh recovery, authenticates the named operator, runs the actual INITIATE/LIST terminal path, exercises terminal editing/history, renders a restored-session acknowledgement, re-authenticates after restore, reaches the authoritative Core-resolution completion state, records one Profile completion snapshot, renders Completion/Final Note/Credits, and begins a subsequent recovery without duplicating history.

Stage 3B and Stage 4D regressions provide the complementary exploration, physical-item, cabinet/Contents, documents/catalogue, notebook, room/collision and Site rendering coverage. S4F-02 covers the launch intro, while S4F-03 protects its transition into Session Control.

**Result: PASS (automated/headless equivalent).**

A physically observed human Windows playthrough remains separate Stage 5 QA.

## Existing-profile and migration route

The persistence suite uses isolated real files and exercises:

- no-existing-data startup;
- Stage 3-style manual-save migration;
- Stage 3-style autosave migration;
- legacy Profile migration with full history;
- legacy Settings migration;
- legacy source retained after migration;
- current AppData data winning over older legacy copies;
- corrupt/malformed legacy data rejection;
- manual and autosave round trips/reinstate;
- Profile round trip and restart;
- missing/new Stage 4 Profile fields normalising/defaulting safely;
- body-style persistence and default compatibility;
- V1/V2/V3 RunState compatibility, including Grey Door V2 defaulting;
- different launch working directories;
- continued terminal/run-state operation after restore.

The fresh/integration and Stage 3 progression suites then exercise the same runtime systems through completion.

**Result: PASS.**

## Alternate-route and seed variation

Stage 4E deliberately uses seeds **146** and **144** because they select different DR-04 arrangements.

| Seed | DR-04 arrangement | Representative deterministic flavour |
| ---: | --- | --- |
| **146** | swap 0 | FERRET CHIP SHOP; QUEUE/SHELF crossword |
| **144** | swap 1 | HEDGEHOG PIE OFFICE; ORDER/INDEX crossword |

The Stage 3B terminal/reconstruction fixtures exercise both Records-first and Technology-first access against both seed classes, including deferred alternate access, save/reload, terminal re-entry, document identity and completion dependencies.

Stage 4G broadens deterministic coverage to 12 fixed seeds and proves multiple safe Grey Door wall positions without consuming or perturbing other feature streams.

**Alternate-route result: PASS (automated).**  
**Seed variation result: PASS.**

The previously documented human-observed seed-146 Records-first and seed-144 Technology-first full playthroughs remain Stage 5 release QA rather than being misreported as completed by CI.

## Calendar variation

The fixed-date matrix is developer-gated and exercises:

| Date | Case |
| --- | --- |
| 2026-01-15 | ordinary winter |
| 2026-04-21 | ordinary spring |
| 2026-07-15 | summer |
| 2026-09-15 | ordinary autumn |
| 2026-10-31 | Halloween |
| 2026-11-05 | Bonfire |
| 2026-12-20 | Christmas |
| 2026-01-01 / 2026-12-31 | New Year |
| 2026-02-14 / 2026-03-31 | Valentine / spring notice |

The noticeboard variant is read-only presentation. It does not alter collections, evidence, triggers, capacity, notebook or completion.

**Result: PASS.**

## Ending states

The Stage 3/4 completion authority remains the two independent predicates:

- Core-resolution evidence resolved;
- available recovery capacity exhausted.

All supported display flavours are exercised:

1. **EVIDENCE RESOLVED**
2. **RECOVERY CAPACITY EXHAUSTED**
3. **EVIDENCE RESOLVED / CAPACITY EXHAUSTED**

The Completion screen, archival status and Final Note are rendered distinctly. A capacity-only ending does not invent a Core-resolution conclusion.

Stage 4G is explicitly progression-neutral and cannot modify these predicates.

**Result: PASS.**

## Save, completion and subsequent-run behaviour

Validated:

- manual save;
- autosave;
- manual/autosave reinstate;
- corrupt/missing save handling;
- restored-session acknowledgement/key ownership;
- post-completion manual/autosave restrictions;
- one Profile completion snapshot per completed recovery;
- no Completion/Credits view-side history mutation;
- same Profile can begin a subsequent recovery;
- Grey Door cannot be saved during its temporary scene;
- completed Grey Door autosave beats only an older same-seed pre-event manual checkpoint, preventing one-shot resurrection without changing ordinary save precedence.

**Result: PASS.**

## UI / presentation review

Automated framebuffer, bounds, source-contract and transition tests cover icon, intro, Session Control/Main Menu, Profile, Settings, Credits, Help, terminal and help/list pagers, catalogue, document viewer, notebook, Inspection/Contents, Site Directory, exploration shell/camera/player, restored-session acknowledgement, and Completion summary/Final Note/Credits return.

S4F-03 removed the final known stale-selection and Credits/intro transition seams. Stage 4D protects common footer grammar, scrollbar bounds and screen identities. Site Directory and Terminal deliberately retain their specialised presentation rather than being forced into the formal-screen layout.

No automated test claims subjective human judgement of animation pacing, legibility on every physical display or Windows resize feel.

**Result: PASS for the automated presentation gate; human visual smoke remains Stage 5 QA.**

## Provenance and licensing status

The Stage 4A conclusion is unchanged by Stage 4B-G.

No new active vendor source tree, audio library, video/image library, font file, SDL dependency or other third-party runtime was introduced during Stage 4. The application icon, intro, deterministic generators, player/inspection rendering and Grey Door sequence are source/procedural additions.

The existing technical provenance review still concludes:

- currently compiled F144 runtime/platform material is demonstrably River2D-derived;
- the repository root licence is GPLv3 material inherited from that lineage;
- the repository does not establish a separate relicensing/dual-licensing permission;
- string_view has historical third-party lineage requiring confirmation if non-GPL relicensing is intended;
- the embedded 5x7 glyph table still needs authorship/provenance confirmation;
- inactive puddle .gitmodules metadata remains;
- tracked tools/site_compiler.exe/.obj remain known repository artefacts.

This is **not legal advice**. Before public/commercial distribution, the project should resolve the River2D rights/licensing strategy and channel-specific compliance obligations. If non-GPL distribution is intended and no separate permission exists, a clean replacement of the derived runtime/platform layer should be treated as a dedicated project.

Stage 4 source packaging now excludes the tracked compiler binaries and tracked runtime save/profile/settings files. They are not shipping game dependencies.

## Clean Windows build

The authoritative production-equivalent gate uses GitHub-hosted windows-2022, x86-64, Premake 5.0.0-beta8, Visual Studio 2022/MSVC, Release configuration, /W4 and TreatWarningsAsErrors=true. It independently builds Floppy144Core and Floppy144PlatformWin32 before the clean full-solution Release rebuild.

The historical Stage 3B test harnesses emit four pre-existing C4701 diagnostics outside the production build. Production Debug/Core/Win32/Release builds are required to remain warning-clean.

## Payload growth

| Baseline | Release bytes | Phase growth | Growth vs Stage 3 | Remaining headroom |
| --- | ---: | ---: | ---: | ---: |
| Stage 3 immutable | 659,968 | - | - | 814,592 |
| Stage 4A | 659,968 | 0 | 0 | 814,592 |
| Stage 4B | 667,648 | +7,680 | +7,680 | 806,912 |
| Stage 4C | 681,984 | +14,336 | +22,016 | 792,576 |
| Stage 4D | 695,808 | +13,824 | +35,840 | 778,752 |
| Stage 4E | 709,632 | +13,824 | +49,664 | 764,928 |
| Stage 4F | 716,800 | +7,168 | +56,832 | 757,760 |
| Stage 4G / final production implementation | **725,504** | **+8,704** | **+65,536** | **749,056** |

Hard decompressed executable limit: **1,474,560 bytes**.

The S4F-04 sign-off adds tests/documentation/release-package hygiene only and is not intended to add shipping executable bytes. The final workflow remeasures the executable rather than assuming that expectation.

## Release-output sanity

The final built-artifact audit requires exactly one Release executable (Floppy144.exe), the copied root LICENSE, the embedded application icon, size within the hard limit, and no runtime saves, source files, PowerShell scripts, JSON, logs, temporary objects, zip files or test/debug executables under bin/release. Build-only static libraries may coexist in the build output but are not runtime dependencies.

The Stage 4 source artifact is created in the runner temporary directory as Floppy144_Stage4_Source.zip and filters tracked compiler .exe/.obj products and tracked runtime .sav/.dat state. A package verification step rejects those artefacts before upload.

## Git hygiene

The final source and built gates both require git diff clean after canonical generation/regressions, no non-ignored untracked files, generated runtime files matching committed canonical output, and checked-out CI HEAD equal to remote refs/heads/Stage4.

The connector-based implementation does not maintain a separate mutable local developer checkout. The CI checkout is therefore the authoritative clean-tree proof for this sign-off.

No merge away from Stage4 is part of this gate.

## Known limitations / deliberate deferrals to Stage 5

1. Complete two human-observed end-to-end replay routes, seed 146 Records-first and seed 144 Technology-first, including a save/reinstate and ending capture.
2. Perform a human Windows visual smoke of intro pacing, every major screen, resize/presentation behaviour and the Grey Door sequence.
3. Resolve River2D/GPL/relicensing strategy and 5x7 glyph provenance before public/commercial distribution.
4. Decide whether to remove stale .gitmodules metadata and tracked compiler artefacts from the repository tree.
5. Convert the current build output into an explicitly curated end-user release package/installer if Stage 5 requires one.
6. Keep the roughly 749 KB payload headroom as QA/fix reserve rather than filling it merely because space exists.
7. Address portability debt only as an explicit Stage 5/later architecture task; do not spread the River2D-derived Win32 presenter into new platforms.
8. Audible music/SFX remain unimplemented behind the established semantic audio boundary.

## Recommended Stage 5 starting priorities

1. Human end-to-end fresh/alternate-route replay and screenshot/visual sign-off.
2. Release packaging and Windows distribution smoke on a clean machine.
3. Provenance/licensing decision and any required source/notices work.
4. Final bug/balance QA only, with Stage 3/4 regressions treated as immutable release contracts.
5. Any portability/runtime replacement as a separately scoped workstream after the provenance decision.

## Formal handover

Stage 4 has a reproducible automated release-candidate gate, a fixed Stage 3 comparison point, explicit architecture debt, explicit provenance debt, and enough remaining floppy capacity for Stage 5 QA corrections without reopening feature development.

**STAGE 4 COMPLETE**

**READY FOR STAGE 5**
