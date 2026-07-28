# Floppy144 Static Runtime Baseline

## Purpose

This measurement records the first standalone Floppy144 Windows executable
with the river2D software renderer compiled directly into the application.

No river2D renderer DLL or external game resources are required.

## Build

- Configuration: Release
- Platform: Windows x64
- Source branch: floppy144/vs001-static-win32
- Source commit: b4240befdcdad162fc3d5957e1ba66c9d9a96087
- Internal canvas: 640 x 360
- Application type: Windows GUI application
- Runtime files: 1

## Runtime File

| File | Bytes | SHA-256 |
|---|---:|---|
| Floppy144.exe | 15872 | 574AEFEB87A7120C29718D063A3E859F845580911F718EE75AD4A04087546436 |

## Comparison

- Previous two-file runtime: 29184 bytes
- Standalone executable: 15872 bytes
- Space saved: 13312 bytes
- Runtime reduction: 45.61%
- Contest ceiling: 1474560 bytes
- Remaining capacity: 1458688 bytes
- Disk capacity used: 1.08%

## Runtime Dependencies

- USER32.dll
- GDI32.dll
- KERNEL32.dll
- VCRUNTIME140.dll
- api-ms-win-crt-stdio-l1-1-0.dll
- api-ms-win-crt-heap-l1-1-0.dll
- api-ms-win-crt-runtime-l1-1-0.dll
- api-ms-win-crt-math-l1-1-0.dll
- api-ms-win-crt-locale-l1-1-0.dll

These are operating-system or Microsoft runtime dependencies. The executable
does not depend on river2Dsoftware.dll.

## Verification

The executable was tested from a clean directory containing only
Floppy144.exe.

The application successfully:

- created the software framebuffer
- opened a window titled Floppy//144
- displayed and resized the framebuffer
- closed normally
- ran without river2Dsoftware.dll
- built with no warnings or errors

## Next Experiment

Remove imgsurf.lib from the Floppy144 link and build process while retaining
the existing source tree for compatibility and rollback.
