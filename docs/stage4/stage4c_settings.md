# Stage 4C - Persistent Settings

S4C-05 exposes the existing V1 settings through a compact Session Control child screen.

Choices: CRT FULL/REDUCED/OFF; text speed NORMAL/FAST/INSTANT; music and SFX volume 0-10; autosave 5/10/30 minutes or OFF. Defaults remain FULL, NORMAL, 10, 10 and 5 minutes.

CRT is now a portable post-render framebuffer filter and previews immediately. Text speed affects only the timed opening splash/reveal; static documents, notebook, catalogue and terminal text remain immediate. Music and SFX changes go only through the Stage 4B platform-neutral audio API. The current Win32 backend is still intentionally silent because no shipping generated-audio backend or semantic production cues exist, so S4C-05 does not invent an irritating fake SFX preview. Autosave changes re-arm only the autosave deadline through the monotonic timing layer.

Changes save immediately. If saving fails, the complete previous settings object is restored before any runtime consumer is updated.

The settings file remains V1 with its existing 16-byte payload. Missing or structurally corrupt files use defaults. Checksum-valid V1 files containing out-of-range individual fields now retain their valid fields, normalise only invalid fields to defaults, mark themselves dirty and are repaired on startup.

No Restore Defaults command was added because the five settings are already compact stepped controls.
