# BF22: The Whimsical Assembly Line / A Completed Product Has Been Delivered

A 110 BPM, 4/4, 32-bar (69.818 s) General MIDI menu theme, generated
procedurally through the existing WinMM output. There are **no stored
MIDI, recorded sounds or sample assets**.

This supersedes the earlier **PLEASE HOLD** 70 BPM / 16-bar menu cue.
The approved 80 BPM, 600-bar gameplay composition remains unchanged.
The completion scene is now the independent 110 BPM, 32-bar
**A COMPLETED PRODUCT HAS BEEN DELIVERED**, with a perceived half-time pulse.
Both cues share a clock but retain their own harmonic and emotional character. Scene switching restores each cue's own GM patches,
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
Press M for the menu theme, C for the new completion theme,
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

## Completion companion: A Completed Product Has Been Delivered

The completion screen and its Credits continue to select the existing
completion scene through the unchanged game/audio façade. The new cue shares
110 BPM, 4/4, 16 steps/bar, 32 bars (69.818 s) with the menu, but is
perceived at 55 BPM through its two bass/tick beats per bar.

Zero-based GM patches on the original WinMM channels: ch1 Electric Piano 1
(#5, program 4), ch2 Acoustic Bass (#33, program 32), ch3 Pizzicato Strings
(#46, program 45), ch4 Music Box (#11, program 10), ch5 String Ensemble 1
(#49, program 48), ch6 Celesta (#9, program 8), ch7 Marimba (#13,
program 12). Ch10 has GM drums with low-velocity closed hi-hat (42),
side-stick (37) and a closing woodblock (76).

The four-part eight-bar harmonic phrase (four repetitions) is:
Cmaj9 / E7 over G# / Am9 / Fm6 / C over G / Abmaj7 /
Dm7 moving to G7sus4 halfway through the bar / C6/9.
Cmaj9 and C6/9 are voiced with explicit ninths; Fm6, Am9,
Abmaj7 and the secondary dominant retain their identity.
C6/9 resolves each phrase before the next Cmaj9 without stopping time.

**Bars 1–8: Your Work Has Been Recognised.** Strings underpin long
piano phrases derived from the menu's C-E-G-E-C, but with a final
homeward C. Two quiet bass pulses, sparse pizzicato offbeats and soft
clockwork support the melody. The intro *once per entry* uses piano C5
and E5, celesta G5 and music-box C6, followed by a soft side-stick
rubber-stamp cue in the first bar.

**Bars 9–24: A Proud Moment for the Department.** A fuller five-note
string chord, quicker piano interjections and alternating marimba,
music-box and celesta replies add warmth, without changing clock rate
or overwhelming UI sounds. The offbeat pizzicato continues to reference
the menu's typing rhythm.

**Bars 25–32: The Department Thanks You.** String chords and bass
soften, piano returns to spacious melody, with only occasional
celesta replies and muted machinery. In bar 32 the music box climbs
G5-A5-B5-C6 under a restrained final approval stamp. The C6/9 chord
flows into the next Cmaj9; gentle ticking continues across the loop.

The flourish is armed on completion-scene entry or a new playback
start after Stop, not rearmed on ordinary 32-bar wrap or seek.
All scene transitions use All Notes Off and restore scene-specific GM
programs. Act/restoration corruption, original gameplay pitch events,
save formats, and shared volume/mute settings are unaffected.

The BF22 PowerShell test compiles both the menu and completion native
MIDI-capture regressions; the latter explicitly checks intro-once
behaviour, sparse and rich sections, harmony, 110 BPM step cadence,
the 32-bar wrap, seek and return to gameplay. Musical approval is
subject to listening on a real GM device. Run
`tools/build_audio_review.ps1` and press **M/C/G** to compare the
menu/completion/gameplay tracks.
