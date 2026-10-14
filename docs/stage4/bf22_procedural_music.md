# BF22: The Whimsical Assembly Line (main menu)

A 110 BPM, 4/4, 32-bar (69.818 s) General MIDI menu theme, generated
procedurally through the existing WinMM output. There are **no stored
MIDI, recorded sounds or sample assets**.

This supersedes the earlier **PLEASE HOLD** 70 BPM / 16-bar menu cue.
The approved 80 BPM, 600-bar gameplay composition and the independent
65 BPM, 16-bar **YOUR CONTRIBUTION HAS BEEN NOTED** completion music
are unchanged. Scene switching restores each cue's own GM patches,
ends previous notes and starts the appropriate loop; gameplay Act and
restoration-driven corruption remain gameplay-only. Music volume/mute
and existing scene routing remain shared and unchanged.

## GM orchestration

All program values sent by the engine are **zero-based**; GM display
numbers are one-based. Channels below are also described as one-based.

| Channel | GM instrument | Program value | Purpose |
|---|---|---:|---|
| 1 | #13 Marimba | 12 | five-note bureaucratic melody, with three-note sparse variants |
| 2 | #33 Acoustic Bass | 32 | clipped four-quarter conveyor ostinato |
| 3 | #46 Pizzicato Strings | 45 | eighth-note offbeat typing |
| 5 | #5 Electric Piano 1 | 4 | soft 3/4-note chord drone |
| 6 | #9 Celesta | 8 | notification replies in middle passage |
| 10 | GM percussion | N/A | note 42 closed hi-hat tick, 37 side-stick stamp, 76 high woodblock |

## Chord plan

C-major centre with E7 secondary dominant and borrowed iv / bVI.
One chord per bar, four repetitions:

| Within eight-bar phrase | Harmony | Roman numeral |
|---:|---|---|
| 1 | C6 (C-E-G-A) | I with added sixth |
| 2 | E7 (E-G#-B-D) | V7/vi |
| 3 | Am7 (A-C-E-G) | vi7 |
| 4 | Fm6 (F-Ab-C-D) | borrowed ivm6 |
| 5 | C/G (G-C-E-G) | I6/4 |
| 6 | Abmaj7 (Ab-C-Eb-G) | borrowed bVImaj7 |
| 7 | Dm7 (D-F-A-C) | ii7 |
| 8 | G7(b9) (G-B-D-Ab) | V7(b9) |

The last bass note approaches the following C from B natural; this
cadential hand-off makes bar 32 flow directly into bar 1.

## Precise sequencer plan

**Timing**: 16 equal sixteenth steps per 4/4 bar.
One step = 60 / (110 * 4) seconds = 136.364 ms.
Each bar = 2.181818 s. Loop boundary = 512 steps / 69.81818 s.

**Bass:** steps 0,4,8,12 are root, fifth, octave root and semitone
below the next bar's bass root. Notes release on steps 3,7,11,15.
The bass continues through the full piece; channel 2, MIDI velocity
59 on the downbeat and 43 on subsequent pulses.

**Typing:** channel 3 pizzicato at steps 2,6,10,14 using successive
chord tones one octave above the voicing; released at 3,7,11,15.
Velocities 25 / 31 for sparse/busy variants. Its placement is always
the offbeat between bass quarters.

**Clockwork and stamps:** channel 10 hi-hat note 42 on 0,4,8,12
(velocity 16 sparse, 21 busy). Side-stick note 37 on 12 in the first
eight bars, 4 and 12 in middle bars, and on selected 12s in the
closing passage. Woodblock 76 at step 14 every fourth bar, with
stronger final-bar punctuation.

**Harmony:** electric piano channel 5 starts a voiced triad at step
0 of every bar (velocity 23 for first chord note and 18 for the
others), with the fourth chord note added in bars 9–24. Previous
chord is explicitly released first.

**Melody:** marimba on steps 0,6,10 in bars 1–8 and 25–32;
on steps 0,3,6,10,13 in bars 9–24. Short staccato releases at
2,5,9,12,15. The seven other chord-specific motifs retain the
recognisable sequence in the same rhythm and range, punctuated
by E7's G#, Fm6's Ab and the G7(b9) chromatic tension.
The motif pitches are embedded as MIDI note numbers in f144audio.c.

**Notifications:** celesta channel 6 at steps 4 and 12 on
alternating bars during 9–24 only; short releases on steps 6/14.

### Arrangement

- Bars 1–8, *Morning Routine*: quiet three-note melody, pizzicato
  typing and simple stamp pattern.
- Bars 9–24, *Peak Administrative Efficiency*: five-note melody,
  celesta replies, fuller electric-piano voicings and two stamp hits.
- Bars 25–32, *End of Shift*: return to three-note melody and fewer
  stamps while bass and typing continue. Final G7(b9), raised
  woodblock and B-to-C approach restart the cycle without a pause.

## Review and verification

Use `tools/build_audio_review.ps1` and
`build/audio-review/f144_audio_review.exe` on Windows.
Press M for this new theme, C for the unchanged completion theme,
G for the original gameplay cue. Up/down adjusts music volume.
Pressing M after another scene restarts the main-menu loop from bar 1.

`tools/test_bugfix22_music.ps1` checks source/routing contracts and
compiles `tools/bf22_menu_midi_tests.c` with MSVC. The native test
intercepts short MIDI messages without depending on a real MIDI output
device and checks patches, harmony, rhythmic placement, 32-bar timing,
wrap, seek and restoration of original gameplay patches.

Run `tools/test_stage4_audio.ps1`, `tools/test_bugfix22_music.ps1`
and `testbuild.ps1` on Windows, plus the Release executable size
gate of 1,474,560 bytes. Automated checks cannot establish musical
quality: audition transitions, the 31/32/1 turnaround and long-running
loop continuity on a General MIDI device before approving the cue.
