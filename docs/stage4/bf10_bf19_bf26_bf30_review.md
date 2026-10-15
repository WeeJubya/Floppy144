# BF10 / BF19 / BF26 / BF30 inspection and Site rendering audit

**Branch:** development/Stage-4. The older BF26 reference HEAD (41705f53) was not current at this implementation pass; the latest branch was inspected before editing.

## Findings and changes

- **BF10:** the shared 2.5D drawing system already has coplanar monitor/desk-top child placement, projected bookshelf compartments and lips, parent-basis door fittings, and left-panel clipping. Retain these rather than adding per-object compensation. The common projected-box width now contracts for the real left-panel horizontal gutter, rather than letting long or mirrored furniture geometry bleed into the Contents column. This change applies consistently to all furniture classes. The named Reception Terminal Desk, Main Office Bookshelf 03, Desk 06 and corridor door framebuffer cases are preserved.
- **BF19:** selection glow, independent indicator toggle, default OFF, eligibility and secure-cabinet exclusion already existed. No gameplay/settings format changes. Existing source and runtime tests remain the regression evidence; visual shape acceptance is still a Windows manual check.
- **BF26:** the previous viewport fix deliberately defers culling of wall fixtures until their final mounted visual rectangle, while retaining clipped drawing. No object lifetime or coordinate override is added. A regression now identifies the Reception Directory from its canonical parent, tests its registered state and real inspect focus, sends the camera out of view, saves/loads and returns to a byte-identical framebuffer. This is an automated reproduction, not a substitute for review of the user video.
- **BF30:** entry leaves ENTRY_01/ENTRY_02 and emergency leaves EMER_01/EMER_02 are authoritative existing door fixtures. Both members of each pair map to one transient shared container ID via `floppy144_double_door.h`; no physical items, extra connections or persisted fields are created. The Site focus now advertises the unlocked exterior Main Entrance as inspectable. `Floppy144CabinetOpenParent` explicitly allows these four door fixtures with zero physical children, with their own presentation-only status and full pair names. The renderer draws two leaves, one joint, frame, handles, inset rails and exactly one sign per pair. Emergency sign: `EMERGENCY EXIT`; entrance sign: `MAIN ENTRANCE`. The pair uses the existing projection and face UV renderer, at every reconstruction stage. All four IDs preserve collision/traversal/locking behavior in RunState and Site data.

## Rendered comparison evidence

Run `tools/test_stage4_inspection_25d.ps1` with environment variable `F144_BF10_CAPTURE_DIR` set to a writable folder. The Windows BF10 GitHub Actions workflow does this automatically and uploads the resulting actual 640x360 RGB PPM framebuffers as the `bf10-bf30-inspection-reference-frames` artifact:

- reception_terminal_desk.ppm
- main_office_bookshelf_03.ppm
- main_office_desk_06.ppm
- corridor_records_door.ppm
- main_entrance_double_door.ppm
- emergency_exit_double_door.ppm

The automated framebuffer checks confirm both individual leaf IDs produce bit-identical projected pair images, that entrance and emergency pair images differ, and that nothing escapes x=306 into Contents. The captured frames are review evidence, **not final human visual sign-off**. If supplied screenshots are unavailable to CI they cannot be overlaid automatically; inspect manually alongside the original reference images/video.

## Required manual acceptance

Open Reception Terminal Desk, Main Office Bookshelf 03, Desk 06 and a corridor door with and without contents, inspect child occlusion, horizon and leg/shelf construction; inspect both outside entry leaves and both emergency leaves, with correct sign and no duplicated signage; walk away from Reception Directory in all directions and return, across room transitions and saved sessions. Repeat with inspection markers on/off and rotate/clip furniture near the camera edge. BF10, BF26 and BF30 must remain visually provisional until these checks are accepted.
