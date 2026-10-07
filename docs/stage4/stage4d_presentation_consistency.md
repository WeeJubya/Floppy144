# S4D-05 Stage 4 Presentation Consistency Pass

## Scope

S4D-05 is a presentation-only audit. It does not redesign the application and
does not change progression, content, IDs, evidence, collection structure,
save semantics, terminal commands or physical-item behaviour.

The pass compares the accumulated Stage 3 and Stage 4 screens for repeated UI
concepts and corrects only accidental differences.

## Shared grammar established

### Formal GDR screens

Session Control, Profile, Settings, Credits and Completion now share one
formal-screen footer baseline:

- `FLOPPY144_UI_FORMAL_FOOTER_Y` = 346.

This keeps their bottom instruction line in the same visual band while leaving
their individual panel widths, headings and internal layouts intact.

### Key naming

Repeated navigation labels now use the same compact notation:

- `UP/DOWN`;
- `LEFT/RIGHT`;
- `PGUP/PGDN`;
- `ENTER`;
- `BACKSPACE`;
- `ESC`.

The older `UP DOWN`, `PGUP PGDN` and inconsistent double-space forms were
removed from shared UI footers.

### Back wording

Where Backspace moves up one UI level, the common action word is now `BACK`.
Context is retained where useful, for example:

- `BACKSPACE BACK`;
- `BACKSPACE BACK TO CONTENTS`;
- `BACKSPACE BACK TO CABINET`;
- `ENTER/BACKSPACE BACK TO SUMMARY`.

### Shared scrollbar

`Floppy144DrawScrollbar()` now owns the six-pixel scrollbar visual:

- track;
- one-pixel outline;
- two-pixel inset thumb;
- proportional thumb height;
- clamped thumb position.

It is used by:

- Catalogue;
- document viewer;
- Notebook;
- Completion Final Note;
- Inspection/Contents.

This removes four subtly different scrollbar dialects while leaving each
screen's scroll state and input model untouched.

## Screen-specific corrections

### Session Control

The footer moved to the common formal baseline and now truthfully advertises
both navigation and activation:

`UP/DOWN SELECT   ENTER CONFIRM`

SESSION RESTORED remains an intentionally distinct modal state inside Session
Control and still closes on any subsequent key.

### Profile / Profile editing

Double spaces inside key/action pairs were removed. Edit notices now use
imperative action wording, matching the actual controls:

- `ENTER SAVE`;
- `ESC CANCEL`;
- `BACKSPACE DELETE`.

Normal Profile retains its body-style control and Back action.

### Settings

No behavioural or layout redesign was needed. Its existing footer already used
the intended key vocabulary and now shares the formal footer baseline.

### Credits

Only the accidental double space in `BACKSPACE  BACK` was corrected.

### Completion

The summary now uses the shared formal footer baseline and
`ENTER CONFIRM`, which applies correctly to Final Note, Credits and Return to
Main Menu.

The Final Note uses the shared scrollbar and explicitly advertises both real
ways to return to the summary:

`ENTER/BACKSPACE BACK TO SUMMARY`

Its green archival emphasis remains deliberate.

### Catalogue / document viewer

Catalogue retained its archive/table identity. Its old key spellings were
normalised:

- `UP/DOWN SELECT`;
- `PGUP/PGDN PAGE`;
- `BACKSPACE BACK`.

Catalogue and authored document scrolling now use the shared scrollbar.

### Notebook

Notebook keeps its paper palette and page framing. Its footer now documents the
existing return controls instead of omitting them:

`N/BACKSPACE BACK`

Notebook scrolling uses the shared scrollbar.

### Inspection / Contents

The S4D-04 pseudo-isometric furniture presentation is unchanged.

The Contents list now gains the same scrollbar affordance used elsewhere when
more than ten physical items are present. Existing item selection and scrolling
remain authoritative.

The secure keypad's ambiguous `0-9 ENTER CODE` wording was corrected to
`0-9 CODE`; Enter remains the submit key.

Backspace wording now follows the common Back grammar.

### Terminal / Help

Terminal retains its command-line identity, transcript punctuation and command
language.

The record/help pager footer now consistently calls Q a return action, matching
what Q actually does. Record-page status text likewise uses `Q: RETURN`
instead of `Q: EXIT`.

### Site exploration

The HUD's `ESC RECOVERY` label was ambiguous. Escape actually opens the GDR
Session Control screen, so the footer now says:

`ESC SESSION CONTROL`

The label is right-aligned from its measured width rather than relying on a
hard-coded x-coordinate.

### Site Directory

No change. The Directory deliberately consumes the full screen, including the
usual footer band, and closes on any subsequent key. Adding standard footer
chrome would reduce map space and erase a deliberate overlay identity.

### Title screen

No change. The animated floppy, staged title and `PRESS ENTER` prompt are an
intentional opening presentation rather than a standard application panel.

## Deliberate differences retained

The consistency goal is shared grammar, not identical screens:

- Terminal remains terminal-like.
- Notebook remains paper-like.
- Completion keeps its special green archival emphasis.
- Site exploration keeps its reconstruction HUD.
- Site Directory remains a full-screen map.
- Credits retains its larger FLOPPY//144 title treatment.
- Inspection retains its dedicated 2.5D furniture view.
- The splash/title sequence remains cinematic and unboxed.

## Regression protection

The dedicated S4D-05 regression protects:

- removal of legacy footer spellings;
- use of the common formal footer baseline;
- truthful Back/Confirm/navigation wording;
- Notebook return controls;
- Completion Enter/Backspace summary return;
- Terminal Q return wording;
- Site Session Control wording;
- shared scrollbar use;
- footer overflow safety;
- shared scrollbar no-overflow, thumb movement and clamping.

Existing Stage 3 and Stage 4 tests remain responsible for screen-state
transitions, gameplay interactions, terminal behaviour, scrolling semantics and
persistence.
