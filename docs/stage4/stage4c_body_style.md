# Stage 4C-03 — Operator Body Style

## Existing contract

The persistent V1 profile already contains a one-byte `body_style` field at
payload offset 32. The valid values discovered in source are:

- `FLOPPY144_OPERATOR_BODY_STYLE_A` = 0
- `FLOPPY144_OPERATOR_BODY_STYLE_B` = 1

Type A is the historical/default silhouette. The profile format remains version
1 with a 64-byte payload (80 bytes including the existing header).

Before S4C-03 the field was displayed on the Profile screen but was not consumed
by the Site player renderer. An out-of-range stored value caused the complete
profile load to fail.

## Player-facing selector

When Profile is not editing the operator name, Left/Right cycles Body
Configuration. Selection is saved immediately through the existing profile
persistence path. Save failure restores the previous in-memory profile.

The Profile keeps the GDR personnel-record visual language: the field is shown
as `< TYPE A >` / `< TYPE B >` with a small preview based on the current
Stage 3 player motif. No Stage 4D sprite work is introduced.

## Rendering

The Stage 3 Type A player shape is preserved exactly. Type B changes only the
drawn torso width. Collision size, movement, interactions, recovery data,
triggers, capabilities and all gameplay rules remain identical.

The historical `Floppy144Site2DDraw` entry point remains available and
renders Type A so Stage 3 callers/regressions keep their original contract.
The Stage 4 application uses `Floppy144Site2DDrawForBodyStyle` with the
persistent profile preference.

## Compatibility

Fresh and old/uninitialised V1 profiles resolve byte 0 as Type A.

Out-of-range body-style bytes no longer invalidate an otherwise valid profile.
They normalise to Type A and the loaded profile is marked dirty. Startup then
uses the existing atomic profile-save path to repair the stored byte; if that
rewrite fails, the safe Type A profile remains usable in memory and the existing
profile persistence warning is raised.

No save-file or profile version was changed.
