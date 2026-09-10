# Floppy//144 - Stage 2 Data-Driven Refactor

This is the self-contained Stage 2 refactor of Floppy//144.

The central design rule is deliberately simple:

> **Game facts live in JSON. C implements reusable verbs.**

`data/floppy144_game_data.json` is the canonical source for collections,
documents and body text, triggers, interactions, evidence, rooms, connections,
furniture, fixtures, physical items, relationships, colours, drawing recipes
and ambient/seasonal content.

The released executable does **not** need to parse JSON. During development the
C99 `tools/game_data_compiler.c` validates the approved JSON and generates
compact C tables. Runtime systems interpret generic conditions and effects such
as `RECONSTRUCT_ROOM`, `UNLOCK_CONNECTION`, `ENABLE_COLLECTION`,
`REVEAL_PHYSICAL_ITEM`, `SET_ACT`, `SET_BRANCH`, `SET_PROJECTION`,
`GRANT_CAPABILITY` and `COMPLETE_INTERACTION`.

This means T-001, T-025, I-004, E-004 and their neighbours are data, not bespoke
C functions.

## Stage 2 scope

Stage 2 includes:

- canonical JSON -> generated C game data;
- stable collection, trigger, interaction and evidence IDs;
- generic trigger condition/effect interpretation;
- generic physical interaction interpretation;
- order-independent evidence synthesis;
- generated document catalogue metadata and all 157 readable document bodies;
- data-derived room connections, collection availability and physical reveals;
- generated Site geometry and validation;
- generic JSON drawing-recipe interpretation;
- persistent 2D / 2.5D projection state;
- FM-23 `SET_PROJECTION: ISOMETRIC` support;
- a lightweight isometric Site renderer over the same canonical geometry;
- Stage 2 headless regression tests;
- the 1,474,560-byte release size gate.

Stages 3, 4 and 5 are intentionally **not** implemented here.

## Folder location

The ZIP is packaged with a top-level `refactor` folder. Extract it into:

```text
C:\Dev\Floppy144
```

and the project will land at:

```text
C:\Dev\Floppy144\refactor
```

If you instead open an already-created `C:\Dev\Floppy144\refactor` folder in an
archive tool, extract the *contents* of the ZIP's `refactor` folder there.

## Build prerequisites

Use a Visual Studio Developer PowerShell / Developer Command Prompt with:

- Visual Studio 2022 C/C++ build tools (`cl.exe`, `MSBuild`);
- Premake 5 (`premake5`) on `PATH`.

No River2D source or Python runtime is required. The JSON compiler and Site
compiler are C source included in this package.

## Build and test

From the project root:

```powershell
.\run.ps1 -build release
```

The build performs these gates in order:

1. compile the C game-data compiler;
2. validate and compile `data/floppy144_game_data.json`;
3. synchronise generated stable-ID definition files;
4. compile and validate generated Site geometry;
5. compile and run the Stage 2 headless regression suite;
6. generate the VS2022 solution with Premake;
7. build Floppy144 with MSBuild;
8. enforce the 1.44 MB / 1,474,560-byte release executable limit.

A failure in an earlier gate stops the build.

To run only the data generation:

```powershell
.\tools\build_game_data.ps1
```

To run only the Stage 2 headless regression:

```powershell
.\tools\test_stage2.ps1
```

## Isometric state

Projection is stored once, as the existing persistent projection enum. Do not
store a second independent boolean. Code that wants the simple boolean view can
use:

```c
if(Floppy144RunStateIsIsometric(pRunState))
{
    /* FM-23 isometric projection is active. */
}
```

FM-23 reaches that state through its ordinary generated `SET_PROJECTION`
effect. There is no T-025-specific projection function.

## Editing game content

Edit **only** the canonical JSON for game-specific content wherever an existing
engine verb can express the desired behaviour. Generated files are marked as
generated and will be overwritten by the next build.

Add C code only when the design genuinely needs a new reusable engine verb or
new presentation capability.

See `STAGE2_REFACTOR_NOTES.md` and `data/DATA_SCHEMA_README.md` for additional
technical notes.
