# Floppy144 Stock Runtime Baseline

## Purpose

This measurement records the first working Floppy144 Windows application using
river2D's stock dynamically loaded software-renderer architecture.

The runtime consists of two files:

- Floppy144.exe
- river2Dsoftware.dll

No external images, configuration files, fonts, audio files, or other game
resources are required.

## Build

- Configuration: Release
- Platform: Windows x64
- Compiler: Microsoft Visual C
- Source branch: floppy144/vs001-static-win32
- Internal canvas: 640 x 360
- Application type: Windows GUI application

## Runtime Files

| File | Bytes | SHA-256 |
|---|---:|---|
| Floppy144.exe | 14,848 | 45FC45EA9FF87148358C3191FA851A34FA74CDC4CD3BDD76AA1FC61421434EE6 |
| river2Dsoftware.dll | 14,336 | E7199BAC42F5A4497B299E91C8BF8D7D14C7D8E6A2B27A63B7EC5C142B616A26 |

## Totals

- Stock runtime: 29,184 bytes
- Contest ceiling: 1,474,560 bytes
- Remaining capacity: 1,445,376 bytes
- Disk capacity used: 1.98%
- Prototype red line: 307,200 bytes
- Space below prototype red line: 278,016 bytes

## Verification

The executable was tested from a clean runtime directory containing only
Floppy144.exe and river2Dsoftware.dll.

The application successfully:

- loaded the software renderer
- created a 640 x 360 software framebuffer
- opened a window titled Floppy//144
- displayed the framebuffer
- resized correctly
- closed normally
- produced no compiler warnings or errors

## Next Comparison

The next build will compile the Windows software renderer directly into
Floppy144.exe.

The resulting one-file runtime will be compared against this 29,184-byte
two-file baseline.
