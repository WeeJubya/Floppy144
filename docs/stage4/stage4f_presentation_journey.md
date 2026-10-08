# S4F-03 - Whole-Game Presentation Journey Pass

**Branch:** \`Stage4\`  
**Scope:** launch-to-completion presentation/state continuity only

## Journey reviewed

The pass traced the complete player-facing route:

\`Launch -> Intro -> Session Control -> Profile / Settings / Credits ->
New or Reinstated Recovery -> Terminal / Site / Catalogue / Documents /
Notebook / Contents / Site Directory -> Completion -> Final Note / Credits ->
Session Control\`.

The existing Stage 4D presentation grammar is already consistent across the
core screens, so this pass deliberately avoids cosmetic churn.

## Issues found and fixed

### 1. Completion could return to Session Control with a stale disabled selection

The Completion screen's explicit **RETURN TO MAIN MENU** option assigned
\`FLOPPY144_SCREEN_MAIN_MENU\` directly.

During an active recovery, Session Control normally defaults to **RETURN TO
ACTIVE SITE**. Completion marks the session inactive, making that option
unavailable. A direct screen assignment could therefore reopen Session Control
with its selection still sitting on the now-disabled row.

The Completion route now uses the canonical \`Floppy144OpenMainMenu()\` helper.
That clears transient menu notices/reinstate acknowledgement state and selects
**INITIATE NEW RECOVERY SESSION** for an inactive session.

No progression or completion semantics changed.

### 2. Intro Replay interrupted the completed-recovery Credits route

S4F-02 added **ENTER REPLAY INTRO** to the shared Credits screen. Because
Credits is also opened from Completion, pressing Enter there could replay the
startup sequence and finish at Session Control instead of preserving the
completed-recovery navigation context.

Credits is now parent-sensitive:

- opened from **Settings**: \`ENTER REPLAY INTRO   BACKSPACE BACK\`;
- opened from **Completion**: \`BACKSPACE BACK\`.

Confirm only replays the intro in the Settings-owned Credits route. Completion
Credits therefore remains part of the ending rather than becoming an accidental
restart shortcut.

### 3. Duplicate dead MENU transition paths

The coordinator already owns one universal logical \`MENU\`/Escape route to
Session Control. Older Office and Terminal screen-specific MENU paths remained
below that universal handler and were unreachable.

They were removed so there is one transition authority rather than multiple
contradictory-looking routes. Runtime behaviour is unchanged.

## Main Menu hierarchy decision

No additional top-level Credits item was added.

Session Control currently has seven options. The rows begin at y=170 with a
12-pixel stride; the seventh row sits at y=242 and the status divider begins at
y=252. An eighth item would collide with the established status area.

Moving Credits onto Session Control would therefore require a genuine layout
redesign and make the operational menu more crowded. The existing hierarchy is
retained deliberately:

- Session Control: recovery/session actions, Profile, Settings, Terminate;
- Settings: environment controls plus Credits / Attribution;
- Credits from Settings: optional Intro Replay.

This preserves discoverability without turning a presentation pass into a menu
redesign.

## Deliberate screen identities retained

The pass does not homogenise screens which are intentionally different:

- Terminal keeps command-line terminology and its own footer grammar.
- Notebook retains its paper treatment and in-session scroll position.
- Catalogue/document hierarchy retains list position while a document is open
  and resets document scroll on close/open as already designed.
- Site Directory remains a transient full-screen schematic closed by the next
  key.
- Inspection/Contents retains the Stage 4D pseudo-isometric presentation.
- Completion remains a bespoke terminal record with an explicit end-of-session
  choice rather than behaving like an ordinary Backspace-dismissable screen.

Profile/Settings selection/reset behaviour, terminal authentication,
reinstatement acknowledgement, first-run guidance and gameplay progression are
unchanged.

## Audio continuity

No audible transition defect exists in the current build because Stage 4B's
Win32 backend remains intentionally silent. Persisted music/SFX volumes are
still applied through the platform abstraction at startup.

S4F-03 adds no native audio call and no cue system. This avoids creating an
audio feature while auditing presentation. When generated audio is implemented,
the intro/menu/session/completion/Credits transition points documented here are
the places to validate cue ownership and stopping behavior.

## Regression coverage

\`tools/test_stage4f_presentation_journey.ps1\` protects the transition-level
contracts without snapshotting pixels:

- intro skip precedes the universal Session Control route;
- only one universal MENU transition authority remains;
- the seven-row Session Control hierarchy remains intact;
- Profile and Settings return to their corresponding Session Control rows;
- Settings-owned versus Completion-owned Credits behavior is explicit;
- Completion uses the canonical menu opener;
- new/reinstated sessions reset transient catalogue/notebook/notice state;
- Completion remains an explicit Final Note / Credits / Main Menu end screen;
- audio remains behind the Stage 4B abstraction with persisted volumes applied.

All inherited Stage 2 through Stage 4E, S4F-01 and S4F-02 regressions remain
authoritative for gameplay, persistence, progression and rendering behavior.
