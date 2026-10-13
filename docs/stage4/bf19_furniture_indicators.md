# BF19: Furniture selection glow and inspect indicators

- Live focus is read from the existing projection-neutral `Floppy144SiteFocusedParentId()` and matched to canonical Site rectangles; nothing changes in collision or interaction state.
- The selected furniture receives a subtle outline, including a shape-following quad for rotated furniture. Clipping is handled by the existing Site renderer.
- Optional small magnifier markings appear only on physically present furniture exposing actual Inspect contents. Secure Access/keypad cabinets remain unmarked, locked or unlocked.
- New `SHOW INSPECTION INDICATORS` setting defaults **OFF** and affects markers, not selection glows.
- Settings remain V1/16-byte compatible: formerly reserved byte 5 stores OFF=0 or ON=1; old files decode OFF without conversion.
- Stage 4 settings, Stage 3B site interaction, Stage 4 persistence and BF19 source-boundary audit cover the change.
- Manual visual sign-off still required: rotated furniture, wall-adjacent furniture, partially clipped furniture, and Site camera movement.
