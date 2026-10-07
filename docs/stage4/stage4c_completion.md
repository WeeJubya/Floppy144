# Stage 4C Profile, Settings and Player Identity Completion Record

**Status:** **STAGE 4C: PASS**

Stage 4C is accepted as integrated. The dedicated S4C-09 gate reproduced the
complete Stage 3 gameplay regression suite, every Stage 4B platform/architecture
contract, every Stage 4C feature regression, a cross-surface fresh-profile
headless smoke route, independent portable Core and Win32 platform builds, a
clean production-equivalent Release rebuild, and the 1,474,560-byte size gate.

No Stage 4D presentation or visual-upgrade work is included in this sign-off.

## Baselines and commit range

**Immutable Stage 3 baseline:** `fe2239123f98346ed06b48bb500672a5e6e2e172`  
**Stage 4B completion:** `4c39cdbda0b9bbeeeb69d7acd08b3b0ae463b420`  
**Stage 4C implementation start / S4C-01:** `62eae1737681542baa417191b9551c7ee15b79ff`  
**Stage 4C feature completion / S4C-08:** `bb2ad1a682c6314c37b338ac89bc3f291b5ca8e5`  
**S4C-09 integration gate:** `9ee729453643f61b7f4cd9ba2fd87eaf39207d46`  
**Stage 4C final sign-off:** the commit containing this completion record.

The Stage 4C implementation range is therefore S4C-01 through S4C-09, followed
by this documentation/sign-off commit.

## Delivered feature sequence

| Task | Commit | Delivered |
| --- | --- | --- |
| S4C-01 | `62eae1737681542baa417191b9551c7ee15b79ff` | Persistent Operator Profile screen and recovery-history presentation |
| S4C-02 | `8b3a5ff37a2f927c59d0d3bf08e029fc48ce22d6` | Player-editable operator name, first-time setup, edit/cancel and persistence |
| S4C-03 | `437b70fc39e88f37a4142f9841e376c20f697fe0` | Persistent cosmetic Type A / Type B operator body style |
| S4C-04 | `18da0afc0ef2b417e7ad6614e5ab2dff9e648b4a` | Terminal operator identity and once-per-application-session authentication |
| S4C-05 | `57a444704fc85aa4cb30f3f13f0acd11ec974229` | Persistent CRT, text-speed, music/SFX-volume and autosave Settings |
| S4C-06 | `87c3b4dbe69c5a4d9b6bbfc696267b60d25043ce` | Credits/runtime attribution grounded in the Stage 4A provenance audit |
| S4C-07 | `ec19cab16daa3681c15f320abd680cc0cbfa3041` | Restored-session presentation integrated into Session Control |
| S4C-08 | `bb2ad1a682c6314c37b338ac89bc3f291b5ca8e5` | Bespoke completion summary, Final Note, Credits route and ending-flavour presentation |
| S4C-09 | `9ee729453643f61b7f4cd9ba2fd87eaf39207d46` | Cross-surface audit, fresh-profile integration smoke and final acceptance gate |

## S4C-09 audit result

The integration audit explicitly checked:

- duplicated/responsibility-leaking Stage 4C UI paths;
- screen-parent navigation and footer prompts;
- Profile versus RunState/session ownership;
- unbounded legacy C string calls in Stage 4C player-facing modules;
- Settings controls with no runtime consumer;
- Profile/Settings/terminal state written into the wrong persistence scope;
- repeated completion-history recording;
- terminal authentication leaking into RunState or replaying as persistent state;
- direct Win32 dependencies in Stage 4C player-facing/Core modules;
- Stage 4C TODO/FIXME/HACK markers;
- completed-session manual-save and autosave restrictions;
- the required Stage 3, Stage 4B and Stage 4C CI gates.

One genuine integration defect was found and fixed: the Settings footer said
`ENTER OPEN` even though Enter changes stepped Settings values and opens only
the Credits row. It now reads `ENTER CHANGE/OPEN`, matching actual input
behaviour.

No other Stage 4C integration defect required a production-code change.

## Profile acceptance

Validated coverage includes:

- completely fresh profile;
- migrated/legacy profile compatibility;
- optional unnamed/default operator state;
- first-time operator-name creation;
- later operator-name editing;
- edit cancellation without mutating the stored name;
- supported-character and maximum-length enforcement;
- Type A and Type B body-style selection;
- invalid body-style fallback to the historical Type A default;
- profile binary round trip and legacy migration;
- cumulative recovery-session counters;
- cumulative discovery history;
- latest-completion evidence snapshot;
- completion count and latest-completion flags/recovered KB;
- repeated Profile rendering without history mutation.

Profile identity and history remain Profile-scoped. The transient name editor
owns only an editable working copy; cancel does not write persistent state.

## Terminal acceptance

Validated coverage includes:

- named operator authentication;
- unnamed profile fallback to `GDR OPERATOR`;
- `GDR NETWORK ACCESS`, profile verification and access-accepted transcript;
- full authentication once when a fresh recovery application session is
  established;
- identity-only presentation on later physical-terminal access;
- fresh authentication once when a recorded recovery is reinstated;
- profile rename appearing on the next terminal reset;
- no authentication state stored in RunState/save files;
- existing terminal commands after authentication;
- `INITIATE`;
- `LIST` and LIST/document pager behaviour;
- terminal text entry/backspace;
- command-history traversal and unfinished-draft restoration;
- existing Stage 3 restoration/location policy.

The Terminal module receives only a plain operator-name string. It does not
include or own the Discovery Profile or persistence implementation.

## Settings acceptance

The persisted Settings remain:

- CRT: FULL / REDUCED / OFF;
- text speed: NORMAL / FAST / INSTANT;
- music volume: 0-10;
- SFX volume: 0-10;
- autosave: 5 MIN / 10 MIN / 30 MIN / OFF.

Validation confirms:

- defaults remain compatible with pre-Stage-4C behaviour;
- out-of-range checksum-valid fields are repaired individually to defaults;
- settings binary schema remains compatible;
- migrated/legacy settings survive the Stage 4 storage move;
- CRT is applied as a portable framebuffer post-filter;
- text speed drives the timed text/reveal clock without changing static screens;
- music/SFX values call the semantic F144 Platform volume API;
- autosave changes re-arm only the autosave deadline;
- OFF removes the autosave deadline;
- failed Settings persistence restores the previous in-memory value before a
  runtime consumer is changed.

The Win32 audio backend remains intentionally silent, as established in Stage
4B. The volume controls are nevertheless fully wired to the platform contract.

## Credits acceptance

Credits remain a compact single-page view, so no pager or scrolling state is
required.

Validation checks access from Settings and Completion, return to the correct
parent screen, framebuffer bounds, and attribution against the Stage 4A
provenance record and root GPLv3 licence.

Player-facing attribution includes Grey Door Republic, WeeJubya, River2D
lineage, BadAcronym, GNU GPL version 3, redistribution/full-licence reference
and no-warranty wording. No unsupported whole-game copyright or version claim
has been invented.

The full root `LICENSE` continues to be copied beside the Release executable.

## Restored-session acceptance

The established restore semantics remain unchanged:

1. select and decode the recorded RunState;
2. hydrate the world from the loaded RunState;
3. rebuild transient terminal state;
4. show the Session Control `SESSION RESTORED` acknowledgement;
5. first key-down dismisses the acknowledgement and paints the restored
   reconstruction percentage;
6. matching key-up enters the restored terminal without leaking the key into
   text input.

No-save, corrupt-save, manual-versus-autosave precedence and progression
semantics remain covered by the persistence and Stage 3 regressions.

Reinstating a recovery does not call `Floppy144DiscoveryProfileBeginRecovery`
and therefore does not increment the number of recoveries begun.

## Completion acceptance

The Stage 3 authority for completion is unchanged.

The authored resolved-investigation path remains:

`P-114 -> I-033 -> E-022 -> I-034(E-020/E-021/E-022) -> E-023 -> COMPLETE`.

The presentation transition still evaluates the independent existing predicates:

- authored Core-resolution evidence resolved;
- available recovery capacity exhausted.

The dedicated Completion tests cover all supported flavours:

- evidence resolved before capacity exhaustion;
- capacity exhausted before Core resolution;
- both predicates true together.

Before Completion is displayed, the coordinator still merges the completed run
into the Discovery Profile, records exactly one completion snapshot, saves the
Profile, clears RunState dirty state and marks the recovery inactive.

The Completion/Final Note/Credits views contain no persistence or completion-
counter mutation calls. Manual save still requires an active session, and
autosave returns when the session is inactive or clean.

The authored Core-resolution conclusion is preserved in Final Note when it was
actually established. Capacity-only endings explicitly report that no Core
resolution was established rather than inventing a conclusion.

A subsequent new recovery increments recovery-session history without
incrementing completed-recovery history again.

## Fresh-profile integration smoke

S4C-09 adds a deterministic headless cross-surface smoke route over the actual
portable game modules.

The route:

1. creates a fresh unnamed Profile and renders it;
2. enters and commits an operator name;
3. opens a later edit, changes the transient buffer and cancels it;
4. changes body style and renders the updated Profile;
5. changes all five Settings categories and renders Settings;
6. begins one recovery and authenticates the named operator;
7. executes established terminal `INITIATE` and `LIST` paths;
8. exercises command history, unfinished draft restoration and backspace;
9. re-enters a physical terminal without replaying full authentication;
10. renders `SESSION RESTORED` and establishes one restored application
    authentication without incrementing recovery history;
11. establishes E-020/E-021/E-022 and lets the real authored automatic
    synthesis produce the Core-resolution completion state;
12. records one Profile completion snapshot;
13. renders Completion summary, Final Note and Credits without mutating Profile
    history;
14. begins a subsequent recovery and confirms session count increments while
    completion count remains unchanged.

Real file round-trip and migrated-profile restore behaviour are exercised by the
Stage 4 persistence suite in the same CI gate; the smoke harness does not write
to the runner's or player's real Profile/AppData environment.

## Regression matrix

GitHub Actions run `37651626295` / workflow run **#379** validated the
integration commit before this sign-off record.

| Gate | Result |
| --- | --- |
| Canonical runtime-data regeneration | **PASS** |
| Stage 2 regression | **PASS** |
| Stage 3A regression | **PASS** |
| Complete Stage 3B regression | **PASS** |
| Stage 4 platform boundary | **PASS** |
| Stage 4 logical input | **PASS** |
| Stage 4 persistence paths/migration | **PASS** |
| Stage 4 audio contract | **PASS** |
| Stage 4 timing/lifecycle | **PASS** |
| Stage 4 single-instance protection | **PASS** |
| Stage 4 developer configuration | **PASS** |
| Stage 4C Profile screen | **PASS** |
| Stage 4C operator-name editing | **PASS** |
| Stage 4C terminal authentication | **PASS** |
| Stage 4C persistent Settings | **PASS** |
| Stage 4C Credits/provenance | **PASS** |
| Stage 4C restored-session presentation | **PASS** |
| Stage 4C bespoke Completion | **PASS** |
| Stage 4 build architecture | **PASS** |
| Stage 4B integration sign-off audit | **PASS** |
| Stage 4C integration/architecture audit | **PASS** |
| Stage 4C fresh-profile integration smoke | **PASS** |

## Architecture acceptance

The intended dependency direction remains:

```text
Player-facing Stage 4C UI
          |
          v
      Game / Core
          |
          v
 F144 Platform API
          |
          v
 Win32 Implementation
```

The S4C-09 audit scans Profile, Profile edit/view, Settings/runtime/view,
Terminal, Credits, Completion and Session Control renderer source for direct
Win32 API leakage. No Stage 4C player-facing/Core module includes
`windows.h`, owns HWND state or calls native file/audio/timing APIs directly.

### Existing architecture exceptions

These are inherited Stage 4B limitations, not Stage 4C regressions:

- `floppy144_main.c` remains the Win32 application shell/coordinator and still
  owns HWND/message-loop glue while routing migrated services through F144
  boundaries;
- `floppy144_persistence.c` still combines portable codecs with Windows file
  operations and remains deliberately outside `Floppy144Core`;
- the Win32 audio implementation remains a safe silent backend;
- the Premake workspace currently has only the Windows platform implementation.

No new Stage 4C architecture exception was introduced.

## Production build and payload

The sign-off build is the workflow's production-equivalent x64 Release rebuild
with warnings treated as errors.

| Metric | Stage 3 baseline | Stage 4B | Stage 4C |
| --- | ---: | ---: | ---: |
| Release executable | 659,968 B | 667,648 B | **681,984 B** |
| Delta from Stage 3 | - | +7,680 B | **+22,016 B** |
| Delta from Stage 4B | - | - | **+14,336 B** |
| Size limit | 1,474,560 B | 1,474,560 B | 1,474,560 B |
| Remaining headroom | 814,592 B | 806,912 B | **792,576 B** |
| Core warnings/errors | - | 0 / 0 | **0 / 0** |
| Win32 platform warnings/errors | - | 0 / 0 | **0 / 0** |
| Final application warnings/errors | 0 / 0 | 0 / 0 | **0 / 0** |

Stage 4C occupies 46.25% of the hard executable-size budget and remains well
inside the 1,474,560-byte gate.

## Known limitations and deferred items

The following are deliberately not failures of this gate:

- the current Win32 audio backend is silent even though music/SFX volume is
  persisted and wired through the semantic platform API;
- text speed affects timed presentation/reveal paths; already-static screens do
  not manufacture artificial character-by-character delays;
- Credits fit on one page and therefore do not have a pager;
- CI cannot drive an interactive desktop window as a human player would, so
  cross-surface acceptance combines deterministic Core/UI renderer smoke tests,
  persistence round trips and the existing behavioural regressions;
- the Type B body style remains the Stage 4C cosmetic variant; richer player,
  room and presentation artwork belongs to Stage 4D;
- the known Stage 4B persistence/coordinator portability debt remains deferred;
- no Stage 4D visual upgrade, animation system or new gameplay/content is part
  of this sign-off.

No unresolved Stage 4C correctness issue remains after the integration gate.

## Stage 4D readiness

Stage 3 gameplay remains green, the Stage 4B platform boundaries remain green,
all Stage 4C feature and integration tests are green, the production Release is
clean, and the executable remains comfortably below the floppy limit.

**Stage 4D Presentation and Visual Upgrade work can begin.**

## Gate result

**STAGE 4C: PASS**
