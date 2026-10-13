# BF21: Procedural reversed Backspace

The original F144SFX terminal key generator is unchanged. A new appended cue synthesises that exact approved 42ms waveform into an available WinMM voice and reverses the PCM sample array in memory before playback. No WAV assets are introduced.

Playback occurs only when a character is actually removed from BF20 LOGIN, the operator Profile editor or the archive terminal input. Empty buffers, terminal help paging and non-text interfaces do not generate false deletion sounds. Normal printable typing keeps the approved original cue.

The existing 4-voice non-overlapping waveOut queue drops a new 42ms key cue if all voices are occupied rather than mixing/clipping active voice buffers. Both forward and reverse cues use the existing SFX volume, including mute.

The native BF21 test checks exact reverse sample equality, 42ms length, determinism and mute; the source audit checks input routing and empty-buffer suppression. Manual headphone acceptance and rapid typing/Backspace are still recommended.
