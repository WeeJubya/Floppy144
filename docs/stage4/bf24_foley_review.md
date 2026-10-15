# BF24 procedural Foley: listening and approval pack

**Branch:** `development/Stage-4`  
**Decision status:** PROPOSED ONLY. **No replacement has developer approval.**

This guide accompanies the existing `tools/f144_audio_review.c` interactive WinMM
review harness and the generated `BF24_original_A_B_waveform_metrics.csv`.
The approved **Whimsical Assembly Line** menu (110 BPM/32 bars), original 80 BPM
gameplay score with Act/corruption, 65 BPM ending score, BF21 reversed Backspace
and BF23 recovery slowdown are outside this Foley approval request and remain untouched.

## Start the existing harness

Open **Visual Studio Developer PowerShell** in the checkout on Windows:

```powershell
.\tools\build_audio_review.ps1
.\build\audio-review\f144_audio_review.exe --list
.\build\audio-review\f144_audio_review.exe
```

The `--list` diagnostic displays all 25 names **without opening an audio
device**. Interactive launch requires Windows WinMM waveOut and a General MIDI
output device for music. **No actual speaker/device listening occurs in CI.**

Inside the interactive program: **N/B** next/previous sound; **O** Original,
**A** Revised A, **S** Revised B, **Space** replay; **Left/Right** SFX volume
and **Up/Down** music volume. **M/C/G** choose Menu/Completion/Gameplay.
**1–4** choose gameplay Act; **F1–F7** seek 0/5/10/15/20/25/29 min;
**R/E** increase/decrease recovery corruption. **T** previews BF23 progress
and tempo slowdown, **X** interrupts it, **Esc** shuts down MIDI and waveOut.

The bundled CSV is reproducible from the same C generator and fixed 0.8 output
gain/seed. A is **physically inspired** (low-pass plus body resonance), not a
measured recording of the actual prop. B is constrained-retro (sample/hold and
step quantisation). Both retain the original sound order and event timing except
for the purpose-built footstep candidate. All three versions exist for **every**
effect. An approval preference in this table is **not** a change to game audio.

## SFX catalogue and proposed *listening order*

"First choice" identifies a **candidate to audition first**, NOT a decision
to deploy. It is inferred from physical gesture, intended game era and
candidate synthesis, not claimed to be heard or professionally mixed. The
decision column must remain unapproved until the developer listens.

| ID | Effect | First choice | Intended character / reason | Particular listening risk |
|---:|---|---|---|---|
| 0 | Terminal key | A | Short damped key-switch/body, less electronic snap | May be too soft in rapid typing |
| 1 | Terminal OK | B | Crisp but restrained retro acknowledgement | Pitch/chime must not dominate music |
| 2 | Terminal error / buzzer | A | Low controlled buzzer rather than piercing alert | Still distinct at low SFX volume |
| 3 | Typewriter | A | Mechanical striker with brief carriage/body | Avoid a brittle bright click |
| 4 | Stapler | A | Dense lever impact and metal mechanism tail | Audible double-stage action |
| 5 | Printer | A | Motor plus muted pins/carriage texture | Must not turn into featureless hum |
| 6 | Paper | A | Light broadband friction, low rustle | A's filter might remove too much detail |
| 7 | Relay | A | Two damped armature clicks | Both contacts must remain separated |
| 8 | Rubber stamp | A | Rounded rubber/wood desk impact and release | Avoid an exaggerated bass thump |
| 9 | Filing cabinet / cupboard | A | Latch, runner friction, stop and body resonance | Preserve duration and slide motion |
| 10 | Door close | A | Weighty panel thud followed by latch | No overlong resonance |
| 11 | CRT wake | B | Expressive stepped startup electronics, period-correct | Avoid harsh whistle at loud volume |
| 12 | Telephone | A | Dual-band metal bell with restrained resonance | Two rings must remain recognisable |
| 13 | Power failure | A | Falling motor/mains body and click sequence | Don't obscure final shut-off |
| 14 | Footstep | **A, highest priority** | Low-frequency heel, delayed quieter sole, damped floor contact | Compare to original's slap; ensure not a musical hum |
| 15 | File / collection restored | B | Economical retro mechanical/confirmation flourish | Avoid sounding like evidence cue |
| 16 | Door blocked | A | Blunt panel impact and rattling latch | Clearly different from door close |
| 17 | Evidence discovered | B | Short discovery motif, game-first rather than literal Foley | Not too triumphant or too similar to restore |
| 18 | Door open | A | Latch release, hinge friction/body | Preserve approach/action clarity |
| 19 | Fridge open | A | Soft seal drag, hinge and low motor | Ensure seal remains identifiable |
| 20 | Coffee machine | A | Pump/motor with little bubbling pulses | Avoid continuous appliance drone |
| 21 | Floppy insertion | A | Drive rail, insertion click and end-stop | Must read as a 3.5-inch drive, not a door |
| 22 | Backspace | **Original** | Exact reverse of the active, approved original keyboard PCM | If key later changes, approve mirrored reverse together |
| 23 | Restoration tick | **Original** | Already integrated BF23 percent tick | Consistency with its final/high variant |
| 24 | Final restoration tick | **Original** | Same cue, subtly elevated pitch | Must remain distinct without brightness spike |

**Current shipping status for all 25 effects: ORIGINAL ONLY.** BF23 tick IDs
23 and 24 exist in normal gameplay, but **their candidate A/B waveforms are
review-only**. No candidate-selection flags are stored in saves or preferences.

## How the generated measurements are interpreted

CSV columns are duration, peak/rms in dBFS, crest factor, an adjacent-sample
difference-energy ratio (rough **high-frequency/roughness proxy**, **not** a
frequency-response measurement), zero-crossing count, and three local RMS
windows. Footstep has both alternating-foot rows for each candidate;
other effects have one row each. **78 data rows** = 25 × 3 + 3 additional feet.

Use measured changes to **prioritise listening**, not to prove acoustic realism.
Unclipped, quiet or spectrally dull audio can still sound inappropriate.
Compare ORIGINAL/A/B at 0.3, 0.6 and 1.0 SFX levels, with both music playing
and muted. Check rapid playback under the four bounded waveOut buffers; no
queue enlargement is proposed.

### Priority footstep listening checklist

Reference heel/toe construction in the current A candidate: a damped **64 Hz**
triangle heel (opposite foot 69 Hz), a lower-gain **105 Hz** sole/toe (opposite
112 Hz) starting ~32 ms later, with a low-pass/attenuated noise component and
a short ~105 ms total decay. B uses the same two contacts with stronger noise
and stepped quantisation. Original has a 78–93 Hz triangle body plus independent
broadband impact noise and a 108 ms linear decay: its higher-frequency slap was
the reason for reassessment.

- Walk at a slow rate on the institutional floor: does A sound like a shoe
  with an initial **heel** and a quieter **sole**, not a synth blip?
- Walk quickly, turn 90°, stop abruptly and walk against a wall: the step
  must only occur when real displacement/gait contact occurs and must not
  loop on blocked/stationary movement.
- Compare the alternating feet; the difference should be noticeable on
  attentive listening, never distracting.
- Repeat at low volume beneath the gameplay music and at SFX-only volume.
- Check no unwanted clicks at onset/end, no clipping, no lingering hum.

If A still sounds like two musical tones, **request a new footstep revision**
before approving. Do not lower the original's pitch indiscriminately or
replace the original based solely on lower difference-energy measurements.

## Recording the actual approval

For each cue, the developer should respond with one of:
`APPROVE ORIGINAL`, `APPROVE A`, `APPROVE B`, or `REWORK`. Until then,
**each item remains pending**. Prefer explicit approval of *each* changed cue,
not blanket approval of the library. If footwear and flooring differ between
areas, scope any future footstep material switch explicitly first.

After approval, integrate only named effects into the normal `F144_SFXPlay`
path and add regression checks for fallback/volume/seed; preserve IDs and save
compatibility. Do not remove test-only gating on unapproved effects.

## Current non-listening validation

- BF21 exact reversed PCM, text-delete-only activation and mute.
- BF22 110 BPM, 32 bars, patch mapping, scene/loop/Act integrity.
- BF23 90%/final 10% progress and temporary music tempo/cancellation.
- BF24 25 × 3 deterministic, bounded, unclipped, muted waveforms, alternating
  foot seed, waveOut voice policy.
- Stage 4 platform/audio, complete Release, standalone review harness build,
  reproducible CSV and device-free `--list` smoke.
- WinMM device and perceived acoustic quality **not listened to by automation**.
