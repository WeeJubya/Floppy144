# Stage 4C-04 — Terminal Operator Authentication

## Existing terminal lifecycle

Stage 3 creates a transient terminal state with `Floppy144TerminalReset` for
the initial recovery environment and `Floppy144TerminalResetAtRoom` for
physical Site terminals. The same reset path is rebuilt when a recorded
recovery is reinstated. Returning from a document does not reset the terminal,
so its output and command history remain intact.

Terminal UI state and command history are intentionally transient and are not
part of RunState/save persistence.

## Authentication boundary

A compact authentication sequence is shown once when the application
establishes a fresh recovery or reinstates a saved recovery:

```text
GDR NETWORK ACCESS
OPERATOR: <PROFILE NAME>
VERIFYING PROFILE...
ACCESS ACCEPTED
```

The sequence is immediate transcript output. It introduces no timer, modal
screen, input lock, or command delay.

After that first authentication, reopening physical terminals during the same
application recovery session shows only the current `OPERATOR:` line. This
keeps terminal identity visible and allows a renamed profile to appear on the
next terminal reset without replaying the verification sequence.

Returning from a document does not replay anything because the existing
terminal state is resumed rather than reset.

## Unnamed profiles

Profile naming remains optional. An empty persistent operator name is presented
as `GDR OPERATOR`; the same access-accepted sequence is used and gameplay is
not blocked.

## Functional boundary

The terminal module receives only an operator-name string. It does not depend
on profile storage or persistence.

No terminal command, collection/record ID, restore rule, LIST pager, document
access rule, command-history rule, or recovery save format is changed.
