# BUG FIX 02: Incorporate Original F144MIDI and F144SFX Into Stage 4

**Source of record:** user-supplied `F144MIDI_Prologue_v07.zip` (20 August
2026) and `F144SFX_v09.zip` (21 August 2026). This is integration, not a
replacement of the approved sounds.

## Diagnosis

Stage 4B originally shipped a silent `src/f144_win32_audio.c` adapter,
despite valid platform-facing init, music, SFX, settings and volume callbacks.
Game screens did not emit normal audio cues. Both music and SFX were therefore
silent globally. The root cause was the missing native production implementation
and event wiring, not a bad user setting or missing sound file.

## Architecture and original source preservation

- `src/f144audio.c/.h` are derived from the uploaded MIDI v0.7 originals.
  Their GM instrument choices, melody, clockwork/pizzicato and Act-corruption
  rules are retained; only duration, evolving secondary layers, restoration
  parameter, final-minute reset and preview seeking have been extended.
- `src/f144sfx.c/.h` retain all fourteen v0.9 procedural effects, including
  the approved **lower 640/790 Hz electromechanical telephone chirrup**.
  Eight small event generators are appended.
- `game/src/floppy144_audio.c/.h` is a game-facing semantic façade. Core
  does not import `windows.h`, `mmsystem.h`, `midiOut` or `waveOut`.
- `src/f144_win32_audio.c` opens exactly one MIDI output and one waveOut
  stream; WinMM is linked by the Windows executable. Neither original demo
  main function is linked into the game. No audio resource paths exist.
- `F144Platform` owns init, playback, volume and shutdown. An optional
  `audio_update` callback schedules music on the established monotonic clock.
  Audio devices are independent: a failed MIDI open may still allow waveOut.
  Both devices missing means silent, non-fatal gameplay.

## 30-minute Bureaucratic Loop

80 BPM, 4/4, 3 seconds/bar, **600 bars/9,600 sixteenth steps**.
The original v0.7 theme and parts are retained for the foundation.

| Time | Bars | Composition |
| --- | --- | --- |
| 0:00-5:00 | 0-99 | Original opening, paperwork and woodwind themes |
| 5:00-10:00 | 100-199 | Extra quiet brushed office percussion |
| 10:00-15:00 | 200-299 | Light pizzicato response |
| 15:00-20:00 | 300-399 | Occasional additional clarinet phrase |
| 20:00-25:00 | 400-499 | Sparse upper-register marimba response |
| 25:00-29:00 | 500-579 | Most layered, still restrained; occasional soft woodblock |
| 29:00-30:00 | 580-599 | Layer-by-layer unwind, fading bare clockwork and breathing space |

At step 9600 all notes stop and the score returns to the approved base.
Act I/II/III pitch corruption remains seeded and deterministic; the *additional*
reconstruction-linked intensity uses `floor(restoration_percent / 3)` exactly
(0%=0, 29%=9, 100%=33, 144%=48). Restoration adds an incremental chance of
pitch displacement; it never rewrites the authored patterns. No `rand()` calls.

## SFX catalogue and primary gameplay hooks

The original effects remain: terminal key, acceptance, blunt two-buzz error,
typewriter, stapler, printer, paper, relay, rubber stamp, filing cabinet,
office-door close, CRT wake, low telephone chirrup and power/system failure.

Added (procedural PCM only): footstep, file restore, blocked door, evidence
discovery, institutional door opening, Staff Room fridge, percolating coffee
machine, and floppy insertion.

| Game event | Cue |
| --- | --- |
| Intro floppy-drive transition | FLOPPY_INSERT |
| Intro power-up / entering a terminal | CRT_WAKE |
| Menu/non-intro scenes | Original F144MIDI, Act chosen from run state |
| Successful movement (rate-limited) | FOOTSTEP |
| Locked corridor door collision (rate-limited) | DOOR_BLOCKED |
| Printable Recovery Terminal text | TERMINAL_TYPE (original key) |
| Terminal command accepted/rejected | ACCEPT / ERROR |
| Collection restore genuinely completes | FILE_RESTORE |
| Previously absent evidence bit becomes set | EVIDENCE_FOUND |
| Authorised door-opening interaction | DOOR_OPEN |
| Entering/accessing cupboards | FILING_CABINET |
| Cabinet code rejected | ERROR |
| Physical inspection of printer/paper/stapler/stamp | Matching approved cues |
| Fridge, coffee-machine and phone inspection | FRIDGE_OPEN / COFFEE_MACHINE / TELEPHONE |
| Grey Door anomaly | POWER_FAIL |

Evidence detection is read-only and compares previous known bitmasks against
the current run state. It does not alter the RunState, save codec, progress or
seed. Intro music stays silent as in the intended presentation: cues only.

The v0.9 office door *closing* effect is not repurposed to sound like opening.
The new DOOR_OPEN cue lacks the heavy final frame impact.

## Settings and lifecycle

Persisted Music/SFX volume values 0-10 map to the original generator's
normalised 0.0-1.0 ranges. Defaults remain audible, and each channel mutes
independently. Music starts once, switches Act in place, and stops without
hanging notes. Existing settings/save formats remain unchanged.
The four-voice waveOut engine is non-blocking when occupied. It resets and
unprepares headers on shutdown; no external media files or worker threads.
The voice sample arrays occupy **211,680 bytes of BSS/runtime memory**
(4 x 26,460 x 2 bytes). This is **not executable file size**.

## Listening and tests

`tools/f144_audio_review.c` is a separate development-only WinMM test harness,
not part of the shipping game:

- 1-4: select Prologue / Acts I-III
- F1-F7: jump to 0, 5, 10, 15, 20, 25 or 29 minutes
- N/B: cycle through 22 effects; SPACE: play selected effect
- UP/DOWN: Music volume; LEFT/RIGHT: SFX volume
- R/E: raise/lower the synthetic restoration percentage by ten
- ESC: quit

Build it separately with MSVC and `winmm.lib user32.lib`.

Regression sources: `tools/stage4_audio_tests.c`,
`tools/test_stage4_audio.ps1`, and
`tools/test_stage4_build_architecture.ps1`. Validate the complete
`testbuild.ps1` and `cleanbuild.ps1` gate on a Windows/MSVC host.

**Validation limitation:** Linux-hosted mock-WinMM compile/test confirms
native source syntax, generated cue dispatch, restoration arithmetic,
sequence progression and all 22 procedural SFX with simulated devices.
Actual Windows device playback and final linked EXE size must be confirmed
on the Windows CI clean-build gate or local developer machine. Do not
claim acoustic acceptance without listening.

**Release size gate:** actual linked Release `Floppy144.exe` must remain
<= 1,474,560 bytes. Original .zip sources, source comments and runtime BSS
are not part of the single executable. Report before/after release sizes
from the clean build rather than inferring them from source bytes.

**Commit intent:** `Bug Fix: Incorporated sound into F144`.
