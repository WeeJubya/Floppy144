# Stage 4C Operator Profile Screen

## Scope

S4C-01 exposes the already-persistent discovery profile through a read-only
player-facing GDR operator record.

No new persistent statistic is introduced by this task. The existing profile
file format remains version 1 with a 64-byte payload.

## Existing persistent profile structure

The authoritative persistent structure is
`Floppy144DiscoveryProfile` in `game/src/floppy144_profile.h`.

The following fields already existed before S4C-01:

| Field | Meaning | Displayed in S4C-01 |
| --- | --- | --- |
| `operator_name[32]` | Persistent operator identity, empty until assigned | Yes |
| `body_style` | Persistent operator body configuration A/B | Yes |
| `dirty` | Internal persistence bookkeeping | No |
| `recovery_sessions_begun` | Number of recovery sessions initiated, saturating at `UINT32_MAX` | Yes |
| `collections_ever_restored[]` | Cumulative bitset of distinct collections restored across recoveries | Yes, as a count |
| `evidence_ever_established[]` | Cumulative bitset of distinct evidence established across recoveries | Yes, as a count |
| `latest_completion_evidence[]` | Evidence mask from the most recent completed recovery | Not listed item-by-item |
| `completed_recoveries` | Number of completed recoveries | Yes |
| `latest_completion_evidence_percent` | Evidence percentage from the most recent completed recovery | Yes |
| `latest_completion_flags` | Latest completion outcome: evidence resolved and/or capacity exhausted | Yes, as outcome text |
| `latest_completion_recovered_kb` | Recovered KB in the most recent completed recovery | Yes |

The profile is encoded and loaded by
`game/src/floppy144_persistence.c` using the existing profile V1 codec:

```text
magic      0x34343150
version    1
payload    64 bytes
```

The final 12 bytes of that V1 payload were already allocated to the latest
completion snapshot during Stage 3C. S4C-01 does not alter the schema or
migration rules.

## Persistent profile versus current run

The Profile renderer accepts only:

```c
const Floppy144DiscoveryProfile *
```

It does not accept `Floppy144RunState`.

That boundary is intentional. The screen therefore cannot display transient
values such as:

- the active recovery seed;
- current reconstruction percentage;
- current-run-only collection state;
- current-run-only evidence state;
- current player position;
- current branch/act/capabilities;
- manual/autosave session state.

Those remain recovery-session data rather than operator history.

The existing coordinator continues to merge discovered collections/evidence
into the profile through `Floppy144DiscoveryProfileMergeRunState()` at the
same established lifecycle points. S4C-01 does not add another merge or save
when Profile is opened.

## Player-facing presentation

Session Control now includes:

```text
OPERATOR PROFILE
```

The option is available whether or not a recovery session is active.

The Profile screen uses the existing 640x360 software framebuffer and GDR/APS-12
visual language. It displays:

- operator name, or `UNASSIGNED` for a fresh profile;
- body configuration `TYPE A` or `TYPE B`;
- recoveries/sessions undertaken;
- cumulative distinct collections restored;
- cumulative distinct evidence established;
- completed recoveries;
- latest completion evidence percentage;
- latest completion recovered KB;
- latest completion outcome.

If there is no completed recovery, the latest-completion section explicitly
states that no completed recovery is on file.

`BACKSPACE` returns to Session Control. Escape also returns to Session
Control through the existing menu action.

Profile is a child of Session Control, not a suspended recovery-session screen.
Opening or escaping from Profile therefore does not overwrite
`global_resume_screen` for an active recovery.

## Editing

S4C-01 originally shipped this screen as read-only. S4C-02 now adds
player-facing operator-name entry/editing while leaving body-style editing for
S4C-03.

The S4C-01 persistent/current-run separation remains unchanged: editing affects
only `Floppy144DiscoveryProfile.operator_name` and never writes identity into
`Floppy144RunState`.

## Regression coverage

`tools/test_stage4_profile_screen.ps1` and
`tools/stage4_profile_screen_tests.c` verify:

- fresh/empty profile rendering;
- zero historical counters;
- existing operator name/body configuration;
- cumulative collection/evidence counters;
- completed-recovery display state;
- repeated drawing does not mutate profile history;
- deterministic repeated rendering;
- Profile does not consume `Floppy144RunState`;
- menu availability and activation;
- Backspace return to Session Control;
- Profile cannot replace the active recovery resume screen;
- opening Profile performs no profile save/merge/begin/completion mutation.

The existing persistence-path regression is also strengthened to round-trip
every persistent field displayed by Profile, including migration through a
legacy Stage 3 location.
