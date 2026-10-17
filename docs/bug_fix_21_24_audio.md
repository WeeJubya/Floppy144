# BF21–24 audio implementation and review

Branch: development/Stage-4. Baseline: b455b05.

## Root causes and scope

BF21's earlier linker issue was corrected in b455b05. BF22's initially
proposed 70 BPM menu cue was **superseded** by the later approved 110 BPM,
32-bar **Whimsical Assembly Line** (`docs/stage4/bf22_procedural_music.md`).
The 80 BPM gameplay composition and its Act-driven corruption are unchanged;
the independent 65 BPM completion music remains intact.

BF23 previously used linear elapsed-time presentation, without progress ticks
or temporary music tempo. It now uses per-percent presentation weights: constant
through 70%, decreasing to 90%, then increasing through 99%, with the longest
weight at 99–100%. Duration remains collection-size dependent, with a 1500 ms
base instead of 300 ms to make the crawl audible. Capacity, collection costs,
authoritative collection commit, save formats and restoration rules are unchanged.
Only the existing completion method commits a collection at 100%.

Ticks follow visible progress checkpoints: every four percent initially,
then increasingly dense checkpoints approaching 90%, then each final percent.
This makes acceleration audible even at the established 50 ms update quantum.
There is at most one short cue per update rather
than replaying skipped percentages after a stall. The last three percentages
use a subtly higher tick. Music scheduler tempo falls from 100% at 90% to 46%
at 99%; it slews back over at most 270 ms. Scene exit/stop requests normal tempo.
Menu/completion clocks ignore temporary gameplay tempo. No note programs,
composition events or corruption seed are changed by the slowdown.

BF24 footsteps used broadband noise with no low-pass treatment and a separate
180 ms movement timer. Playback now follows the established 160 ms gait frame
changes, gated by actual displacement and recent movement; blocked movement
clears the pending contact. Rendering and collision stay unchanged. **On 16 October 2026 the developer
approved browser Sound Lab Revised A. Gameplay footstep synthesis now uses
a C port of that authentic heel/sole design.** The old original remains
available through the review harness. Gameplay still uses the established
160 ms animation contact and displacement gate; the browser lab's
reference-paced 0.62 s demonstration is not yet the gameplay cadence.

## Review harness

From a Visual Studio Developer PowerShell:

```powershell
.\tools\build_audio_review.ps1
.\build\audio-review\f144_audio_review.exe
```

N/B select a named cue; SPACE replay. O = ORIGINAL; A = REVISED A: PHYSICALLY
AUTHENTIC; S = REVISED B: STYLISED RETRO. These labels describe intended design,
not verified physical fidelity. LEFT/RIGHT adjust SFX independently from UP/DOWN
music. M/C/G choose menu/completion/gameplay music; 1–4 select Acts. T runs the
same BF23 progress/tempo curve over three seconds; X interrupts and restores
tempo. R/E change recovery corruption. ESC shuts down waveOut and MIDI.

The harness uses a fixed synthesis seed for fair comparisons; successive feet
alternate subtly. A retains object-specific original event ordering but reduces
high-frequency noise and adds a quiet body resonance. B adds restrained
quantisation/sample-and-hold. Footsteps have separate rounded heel and quieter
sole events. Backspace reverses the chosen variant's keyboard PCM. No downloaded
recordings, stored audio, or external sample dependencies are used.

The game executable now uses **approved Revised A for footsteps only**.
`F144_SFX_REVIEW` enables all other unapproved candidates only in the
standalone harness/tests. The game retains original object sounds for the
other 15 effects; BF23's two new ticks remain unchanged. Four bounded waveOut buffers retain the existing busy-buffer policy;
no unbounded sound queue or new mixing is introduced.

## Active procedural SFX after 17 October 2026

The earlier 25-effect inventory has been superseded by **16 sounds with
direct gameplay triggers**, listed and reviewed in
`docs/stage4/bf24_foley_review.md`. Nine dormant/unused effects are
removed from the synthesiser, game audio IDs and review tools.

Only Revised A footsteps are approved for gameplay; the remaining
**15 active sounds** retain their established synthesiser recipes.
The native review harness includes 16 x 3 candidates, 51 metrics
rows including the alternate-foot rows.

## Research and limitations

Primary research describes heel/sole contact and shoe-floor excitation rather
than a single bright noise slap: https://pubmed.ncbi.nlm.nih.gov/21428494/
and https://pubmed.ncbi.nlm.nih.gov/16938964/ . Physical and physically inspired
footstep models are discussed at https://iris.unitn.it/handle/11572/226899 .
These support the separated heel/sole events and restrained friction model.

Period reference: Jordan Mechner's original Apple II Prince of Persia source,
https://github.com/jmechner/Prince-of-Persia-Apple-II . The economical short,
gait-triggered cue is a design inference from the period's limited playback
resources, not copied sound code or a claim to reproduce a specific port.

Contemporary evidence of dot-matrix printer noise:
https://archive.computerhistory.org/resources/access/text/2023/11/102803713-05-003-acc.pdf .
Telephone construction reference (two gongs / magneto ringer):
https://www.britishtelephones.com/bellst1.htm . Synthesis rationale for impacts,
noise and resonance: https://www.soundonsound.com/techniques/physics-percussion .

Remaining object profiles are physically inspired engineering interpretations
of source mechanisms, not measurements of particular props. No acoustic or
listening acceptance is claimed by waveform tests. Audition ORIGINAL/A/B at
normal and quiet volume, long cue followed by short cue, rapid typing/deletion,
walking/collision, interrupted/repeated restoration, and menu/ending transitions.
The waveform checks cover bounds, deterministic generation, clipping at tested
gain, mute and foot alternation; native device behaviour requires listening.

## Verification commands

```powershell
.\tools\test_bugfix21_backspace.ps1
.\tools\test_bugfix22_music.ps1
.\tools\test_bugfix23_24_audio.ps1
.\testbuild.ps1
.\run.ps1
```

Windows CI runs focused checks, the complete regression workflow, Release
rebuilds and the 1,474,560-byte gate. It uploads the **existing** standalone
review executable, waveform metrics CSV and detailed listening/approval guide
as `bf21-24-audio-review`. No comparison candidate enters gameplay without
specific developer approval. Inspect the run associated with the final commit for
actual results. Linux portable BF23 curve tests pass; Linux cannot run MSVC,
WinMM or the Windows executable and is not Windows acceptance.


## Browser Foley laboratory, 16 October 2026

Reusable tool: `tools/bf24_footsteps_listening_lab.html`, containing a
complete 25-effect registry, local audio imports for reference/original/A/B,
sequential listening, per-effect review notes, local decision storage and
JSON export. Footstep A is preselected. Other sounds remain pending and do not
have invented browser prototypes; use the existing Windows harness or import
recordings to compare them. The browser's authentic A was selected by the
developer, not the older generic filtered Revised A. The C port models the
same audible character rather than embedding any third-party MP3 or WAV.

The selected 0.62 s reference cadence is a **lab demonstration**, not proof
of the running game's 160 ms gait timing being correct. Windows native
listening, regression/build and size-gate results must be checked per commit.
