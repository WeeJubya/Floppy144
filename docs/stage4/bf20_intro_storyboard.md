# BF20: Four-screen intro implementation

Authoritative storyboard: the four-slide `Intro.pptx` supplied by the developer on 13 October 2026. The slide body text was verified; rendered PowerPoint reference images were not retrievable during this run, so the final visual match must be reviewed in Windows.

1. Cabinet with a disk sliver, held 3.8 seconds.
2. Zoom behind cabinet to reveal the labelled 3½-inch disk, with GDR / SITE RECOVERY DISK / DISK 144/233.
3. Cut to CRT and 3½-inch drive, disk arrives from bottom, shutter leading, and reaches drive with procedural insertion cue. Boot text types from a shared clock.
4. The CRT grows into the fixed-header full-screen terminal, continuing the exact same boot clock; messages scroll and end at editable LOGIN.

No input may skip the essential transition. The login is capped to the existing operator profile character set and 31-character limit; whitespace-only entries become GUEST. Profile save errors hold the login screen for retry. A successful save continues to the existing Session Control menu.

New intro timing constants are centralised in `floppy144_intro.h`. Gameplay and save-state progression are unaffected. Keyboard Backspace cue refinement remains BF21; approved gameplay music and sounds are untouched.

Automatic tests: Stage 4F updated intro vector suite plus BF20 source contracts, profile/persistence tests, stage 3 regression and Windows Release gate. Interactive visual acceptance, physical insertion animation and terminal scrolling need manual sign-off.
