# Stage 4C - Restored Session Presentation

S4C-07 is presentation-only. Successful reinstate still uses the established
Session Control screen and the existing two-message acknowledgement handshake.

## Previous presentation

After a successful load, Session Control drew normally and then placed a small
448x62 green-bordered box over only the lower status/capacity strip. It showed
`SESSION RESTORED` and `PRESS ANY KEY TO CONTINUE`. The menu remained visible
above it, while the restored reconstruction percentage was deliberately hidden
under the box.

The first key-down dismissed that box and forced one ordinary Session Control
frame to the window. That frame exposed the loaded reconstruction percentage.
The matching key-up then entered the restored terminal, preventing the continue
keypress from leaking into terminal text input.

## S4C-07 presentation

The same state and input handshake remain intact. The confirmation is now
treated as a proper Session Control state inside the existing outer frame:

- the GDR Session Control header, APS-12 marker, department title, environment
  subtitle and media metadata remain unchanged;
- confirmation uses the same 48/56 px inner margins as the menu/status system;
- the same panel, dark panel, border, green, amber, text and muted palette is
  reused;
- `SESSION RESTORED` sits in a full-width status header;
- the panel states that the recorded recovery state was reinstated and displays
  the already-loaded recovery seed;
- the normal reconstruction/capacity area remains covered until acknowledgement;
- the prompt remains `PRESS ANY KEY TO CONTINUE`.

This makes the success state look native to Session Control rather than like a
small modal box dropped onto it.

## Behaviour preserved

S4C-07 does not change save selection, manual-versus-autosave precedence,
binary decoding, RunState hydration, reconstruction percentage calculation,
autosave, discovery profile data, terminal authentication, or progression.

No-save still disables Reinstate. A failed/corrupt load still clears recorded
session availability and raises the existing persistence warning.

Successful restore still follows:

1. load and hydrate the RunState;
2. render the Session Control confirmation;
3. first key-down removes confirmation and paints one frame with the restored
   percentage;
4. matching key-up enters the restored terminal.

The renderer regression also guards the 640x360 framebuffer and verifies that
drawing the acknowledgement does not mutate the restored RunState.
