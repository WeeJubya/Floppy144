# BF39: Secretary's and Director's Office geometry

## Authority and root cause

Office Dimensions(1).xlsx, `Director's Office` worksheet, is the agreed room plan. Its two Excel grouped furniture arrangements are rotated (Director 315°, Secretary 45°); the prior BF12 conversion flattened them to orthogonal bounding boxes without preserving the relative positions of the individual chairs. That caused oversized gaps, the Secretary's chair being too far west, and the Director's three visitors appearing in one straight row.

The full-size workbook shapes were resolved to runtime Site-coordinate centres using the existing inverse transform from the legacy X/Y layout. The accepted implementation **does not add diagonal world furniture**: cardinal rectangles are positioned within approximately one Site unit of the corresponding original grouped-shape centres. Chairs retain their back/facing cardinal orientation, with the Director's west-side visitor chair now facing west (270°, back outwards towards the west wall).

## Canonical runtime geometry (X,Y; widths/heights unchanged)

| Furniture | Previous | New | Dimensions | Rotation |
| --- | --- | --- | --- | --- |
| Secretary desk | (45,36) | **(46,38)** | 6×4 | 0° |
| Secretary chair | (47,42) | **(50,42)** | 2×2 | 180° |
| Director desk | (43,8) | **(44,8)** | 8×4 | 0° |
| Director chair | (46,4) | **(49,6)** | 2×2 | 0° |
| Visitor chair 1 | (43,14) | **(44,12)** | 2×2 | 180° |
| Visitor chair 2 | (46,14) | **(46,15)** | 2×2 | 180° |
| Visitor chair 3 | (49,14) | **(41,9)** | 2×2 | 270° |

The parent furniture registry, runtime/generated Site layout, compiled geometry, compiled furniture records and legacy inverse-reference layout are maintained together. Existing room footprints, walls, doors, contents, interactions and other furniture are unchanged.

## Director's Office sofa (BF39 first commit)

The enlarged 7×4 sofa originally at (37,27) crossed the southern inner-floor limit at Y30. It is now (37,26), with its original X and footprint retained and no sideways shift.

## Verification

The BF12 native Site regression protects exact world rectangles and dimensions; the Stage 3B parent tests protect desk interaction geometry. The Stage 4D source audit now pins all seven desk/chair owners to their office geometry, additionally verifying cardinal rotations, mirrors, overlaps and floor boundaries, and retains the BF39 sofa regression.

Focused source-level geometry checks were performed before committing. Native Windows tests, Release compilation and exact executable size must be reported from GitHub Actions once complete. Manual visual acceptance in normal gameplay, full Site ISO and 2.5D inspection remains required, including approaching the Secretary's desk, all Director chairs and the sofa.
