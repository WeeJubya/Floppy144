# Floppy//144 Beta.2

**Release notes draft. Not published.**

Floppy//144 returns to the Government Department of Records with an updated Windows runtime. The department insists that the underlying paperwork remains exactly as unreasonable as before.

## What's changed

- **A new native Windows display system.** Beta.2 uses an independently implemented Win32 software framebuffer and GDI presenter in place of the previous River2D-derived runtime. It remains a small standalone Windows executable; no additional game engine or graphics framework is required.
- **Sharper, properly proportioned presentation.** The renderer is designed to retain the original 640×360 picture, crisp scaling and black presentation bands in differently sized windows.
- **Behind-the-scenes stability checks.** Input handling, saving and settings, procedural audio, terminal use, navigation and scene rendering have been subjected to a broad Windows regression suite.
- **Familiar gameplay.** No new puzzles, story branches, difficulty rules or intentional content revisions have been introduced by the runtime replacement.

The approved intro presentation includes an unlettered floppy label near the drive. A profile named `GUEST` still asks for the operator's preferred name when appropriate.

## Saves and settings

Beta.2 is designed to continue using the same local profile, Settings, manual-save and autosave locations under `%APPDATA%\Floppy144`. The game-facing save formats and migration logic are intentionally preserved.

**Before trying a new beta, back up your saves, profile and Settings.** Older internal development prototypes created before Beta.1 used different save payload formats and are not supported. Compatibility with files created by the public Beta.1 build must be confirmed before publication.

## What to look out for during testing

If anything looks or sounds wrong, we especially want to hear about window resizing and aspect ratio, unexpected black frames, movement and inspection, terminal and Notebook text, the Grey Door sequence, procedural music and sound effects, and loading or restoring a session.

Beta releases may contain bugs. If you encounter a crash or unexpected progress change, stop and make a copy of your save before trying again.

## Installing and reporting

Download the Windows release from the [Grey Door Republik releases page](https://github.com/GreyDoorRepublik/Floppy144-beta/releases). Run `Floppy144.exe` directly, or extract its ZIP first if the selected release is packaged as an archive. No installer is required. Consult the licence and accompanying notices for the specific published version.

Please report issues at [GitHub Issues](https://github.com/GreyDoorRepublik/Floppy144-beta/issues). Include the exact beta version, Windows version, steps to reproduce, expected versus observed behaviour and screenshots or a short video if useful. Mark story spoilers clearly, and do not post personal data or private save files publicly.

**Floppy//144** is a **Grey Door Republik** game, created by **WeeJubya**.

*The Government Department of Records thanks you for helping locate its missing functionality. Claims forms are available upon request, subject to availability.*
