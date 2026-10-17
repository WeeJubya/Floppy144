# BF24: active procedural SFX and approval laboratory

**Branch:** `development/Stage-4`  
**Current scope:** 16 effects with direct game triggers. Nine dormant/unused
generators, semantic IDs and review entries were retired on 17 October 2026.
They remain recoverable through Git history, not as silent runtime placeholders.

The standalone Windows review harness is `tools/f144_audio_review.c`;
the browser laboratory is `tools/bf24_footsteps_listening_lab.html`.
Both list only **16 active effects**. All gameplay audio remains procedural
at 11,025 Hz mono, without stored samples or external audio dependencies.

## Current 16-effect catalogue

| ID | Effect | Game trigger | Foley status |
|---:|---|---|---|
| 0 | Terminal keyboard | Typing, login, profile | Original; re-Foley pending |
| 1 | Acceptance | Intro, successful commands | Original; pending |
| 2 | Error / buzzer | Invalid commands / cabinet code | Original; pending |
| 3 | Relay | Terminal restoration begins | Original; pending |
| 4 | Filing cabinet / cupboard | Secure cabinet access/keypad | Original; pending |
| 5 | CRT wake | Startup, terminal access | Original; pending |
| 6 | Power failure | Grey Door encounter | Original; pending |
| 7 | Footstep | First actual displacement and later gait | **Revised A approved** |
| 8 | File / collection restored | Successful recovery | Original; pending |
| 9 | Door blocked | Locked-door collision | Original; pending |
| 10 | Evidence discovered | New evidence | Original; pending |
| 11 | Door opening | Successful access | Original; pending |
| 12 | Floppy insertion | Intro floppy drive | Original; pending |
| 13 | Backspace | Actual text deletion | Original; pending |
| 14 | Restoration tick | Recovery progress | Original; pending |
| 15 | Final restoration tick | Late recovery progress | Original; pending |

**Remaining Foley work: 15 individually approved effects.**
Find a reference, prepare physically inspired Revised A and stylised Revised B,
audition against ORIGINAL in the Sound Laboratory, record a decision, integrate
approved changes one at a time and verify in a Release build. Merely showing a
sound in the review harness is not approval. The keyboard/Backspace relationship
must remain exact when either is later revised.

## Removed from runtime and review

- Typewriter
- Stapler
- Printer
- Paper
- Rubber stamp
- Door closing
- Telephone
- Fridge opening
- Coffee machine

Seven were only referenced by name-matching fallback notices that the active
Contents-screen inspection flow bypasses. Typewriter and door closing had no
gameplay call sites. All nine sound generators, dispatch cases and enum entries
are removed, without deleting any authored furniture/content or game logic.

## Verification commands

In a Visual Studio Developer PowerShell checkout:

```powershell
.\tools\test_bugfix23_24_audio.ps1
.\tools\build_audio_review.ps1
.\tools\test_bf24_review_package.ps1
.\tools\test_stage4_audio.ps1
```

Native review: `build/audio-review/f144_audio_review.exe --list` enumerates
16 effects without opening audio hardware. Press N/B to choose, O/A/S for
Original/Revised A/Revised B, Space to play. The CSV contains **51 rows**:
16 effects x 3 candidates, plus three extra alternate-foot rows.

Footstep Revised A uses an initial successful movement contact and the
unchanged 160 ms gait for subsequent footsteps. No footfall occurs against
an obstruction. Browser Revised A and C share the sound design rather
than byte-identical synthesis.

Music is separate, with approved menu, gameplay and completion tracks.
The sound review tests do not prove perceptual acoustic quality; normal
Windows gameplay listening remains an independent acceptance check.

Historical 25-effect inventories and 78-row snapshots are recoverable in
the repository's prior commit history, not the current registry.
