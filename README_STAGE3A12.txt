Floppy//144 Stage 3A.1 - Issue #12 boundary ownership pass

Runtime contract:
- no anonymous shared geometry is emitted at runtime
- every rectangle has one owning room
- ordinary geometry has from_room == to_room == owner
- doors/windows carry explicit from/to endpoint rooms
- room-to-room boundaries are visible only when both rooms are reconstructed
- room-to-OUTSIDE boundaries are visible when the interior room is reconstructed
- 2D visibility no longer uses the old 1U boundary halo
- isometric visibility uses the same reconstruction rule

Compatibility:
- site_compiler.c still accepts the current generated top-level shared_geometry
  input and converts it to room-owned SITE_BOUNDARY records.
- site_layout.jsonc in this package shows the preferred migrated source format
  with boundary entries moved into room geometry arrays.
- floppy144_site_generated.def is generated output, included for comparison/
  immediate replacement only. Do not hand edit it.
