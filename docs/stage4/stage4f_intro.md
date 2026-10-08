# S4F-02 - FLOPPY//144 Intro Sequence

**Branch:** \`Stage4\`  
**Insertion point:** existing \`FLOPPY144_SCREEN_SPLASH\` startup screen  
**Duration:** 10.4 seconds at normal scene timing

## Sequence

The old standalone splash presentation is replaced by one compact procedural
intro drawn entirely into the existing 640x360 software framebuffer.

1. **Discovery, 0.0-2.6 s**  
   A dim desk/found-media scene reveals the reversed Disk 144. The only
   contextual text is that unidentified removable media has been found, with
   the handwritten \`144\` marking and no catalogue entry.

2. **Insertion, 2.6-4.8 s**  
   Disk 144 moves horizontally into a simple computer removable-media drive.
   The drive engages and activity is detected.

3. **Program start, 4.8-7.0 s**  
   The view becomes the FLOPPY//144 computer presentation. It reports
   \`BOOTSTRAP FOUND\`, validates the media header, locates the recovery
   executable and initialises the system.

4. **GDR connection, 7.0-10.4 s**  
   The program brings a network interface online, searches without player
   instruction, finds the GDR network, connects and receives a remote session.
   The last line introduces \`GDR SESSION CONTROL SYSTEM\`, which is the screen
   the player enters next.

No later-game clue, evidence identity, collection identity or explanation of
what the GDR is has been added.

## Launch, skip and replay policy

The intro plays on every application launch. This preserves the "found floppy"
framing for a new installation without adding a profile-schema field solely to
hide presentation on later launches.

Repeat players are never trapped in it. The established logical actions:

- Confirm / Enter
- Back / Backspace
- Menu / Escape

all skip immediately to GDR Session Control from any beat.

Normal completion also opens Session Control automatically. There is no
framebuffer reset outside the normal complete-screen redraw path.

Credits now advertises **ENTER REPLAY INTRO**. Confirm from Credits re-arms
only the existing splash/intro animation deadline and starts the intro at zero.
It does not reset autosave, profile, run state, evidence, notebook or recovery
state. Completing or skipping a replay goes to Session Control.

## Settings and platform boundaries

Text reveal timing uses \`Floppy144SettingsTextElapsedMs\` on each beat's local
elapsed time. Normal, Fast and Instant therefore change staged boot/network
text without accelerating the physical discovery/insertion scene clock.

The existing coordinator applies \`Floppy144SettingsApplyCrtFilter\` after
drawing, so Full, Reduced and Off CRT modes affect the intro exactly as they do
the other screens.

Stage 4B's Win32 audio backend is intentionally silent and defines no shipping
music/SFX cues or generated-audio playback. S4F-02 therefore adds no direct
WinMM, PlaySound, waveOut, MCI or other native sound path, and it does not
invent semantic cue IDs that cannot yet be heard. Music/SFX volume settings
remain applied through the existing platform abstraction at startup. When the
separately approved generated-audio backend arrives, intro cue events can be
added through that semantic boundary.

## Regression coverage

\`tools/test_stage4f_intro.ps1\` and \`tools/stage4f_intro_tests.c\` verify:

- all four beat boundaries and normal completion;
- immediate skip actions for Confirm, Back and Menu;
- non-skip movement action;
- representative rendering from each beat;
- Normal, Fast and Instant text reveal variation;
- muted-vs-non-muted SFX setting leaves presentation deterministic while the
  backend is silent;
- Full, Reduced and Off CRT processing;
- replay re-arms intro timing;
- replay does not disturb the existing autosave deadline;
- automatic completion routes to Session Control;
- Credits replay wiring and footer;
- the intro module has no profile, run-state, persistence or native-platform
  dependency.

The full inherited Stage 2 through Stage 4E gates and S4F-01 application-icon
gate remain part of CI.
