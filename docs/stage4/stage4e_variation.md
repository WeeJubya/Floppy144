# S4E-01: Deterministic variation service

## Seed source and persistence

Stage 3 already stores `Floppy144RunState.recovery_seed` as the first
32-bit word of both V1 and V2 save payloads. Ordinary new recoveries obtain
a nonzero seed from `f144PlatformMonotonicMs()` in the Windows coordinator.
Only that initial seed acquisition involves the platform clock. Stage 4E
choices themselves never read the clock or platform randomness.

The Stage 4B developer route `-debug -seed <nonzero uint32>` already overrides
the new-run seed. Portable tests use `F144StartupConfig` directly.
Reinstating a saved run retains its saved seed rather than the launch override.

Variation is **recovery-run-scoped**, not profile-scoped. No new persistent
field, save version, profile migration, storage path or player UI is necessary.
Legacy zero-seed saves get deterministic zero-seed output, without mutation.

## Portable Core API

`game/src/floppy144_variation.h/.c` implements:

```c
uint32_t value = Floppy144VariationValue(
    run_state->recovery_seed, "takeaway.menu.v1", "staff-room"
);
uint32_t slot = Floppy144VariationRange(
    run_state->recovery_seed, "takeaway.menu.v1", "staff-room", 5U
);
bool show = Floppy144VariationChance(
    run_state->recovery_seed, "ambient.notice.v1", "reception", 1U, 4U
);
```

- `Value`: 32-bit stable keyed output.
- `Range`: index in `[0,count)`, or zero when `count == 0`.
- `Chance`: numerator/denominator test. Zero denominator or numerator
  produces false; numerator >= nonzero denominator produces true.
- A feature ID must be nonempty, otherwise output is zero/false.
  `item_id == NULL` and `""` are equivalent.

Version 1 algorithm is FNV-1a over explicitly little-endian seed bytes,
fixed `0xA5` version/domain tag, NUL-separated feature and item bytes,
followed by fixed 32-bit avalanche mixing. Arithmetic is unsigned uint32.
It has no mutable PRNG state, heap allocation, time or Win32 dependency.
Modulo mapping is suitable for atmosphere/flavour choices, not fair gambling
or cryptographic random draws.

## Feature namespace policy

Use stable ASCII feature identifiers with explicit version suffixes, for
example `takeaway.menu.v1` versus `dr04.records.v1`. Use canonical stable
document/physical-item IDs for the optional key, not traversal order or
temporary array positions. Calls never consume a shared stream: modifying the
takeaway generator cannot change DR-04 results unless the DR-04 feature's
own inputs or selection rules are changed. Increment a namespace version
deliberately when its content mapping changes.

## Guarantees

- Same saved seed, feature/key, option count and **game build** produces the
  same Stage 4E variation.
- Repeated calls and calls to unrelated features do not reshuffle choices.
- V1/V2 save and restore retain the seed, so the current run retains choices.
- Tests can force known seeds using `-debug -seed 144` or the portable
  `f144StartupConfigSetRecoverySeedOverride()` interface.
- Cross-version reproducibility is **not promised** if IDs, option sets,
  algorithm or choice mappings change. Golden vectors protect this build's
  algorithm against accidental changes.

## Regression and content impact

`tools/test_stage4_variation.ps1` compiles/tests portable keyed variation,
golden vectors, seed changes, stable namespaces, optional item keys, invalid
arguments, range/chance and Stage 4B's debug seed injection.

The Stage 3B reconstruction test additionally round-trips real V1 and V2
RunState save payloads and compares variation choices before and after load.

This task adds **no generated atmosphere content** and changes no Stage 3
document bodies, document generator, trigger, interaction, collection,
evidence, size/capacity or gameplay rule. Profile/run/save schema: **unchanged**.
