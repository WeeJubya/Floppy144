# Floppy144 Static Renderer Baseline

## Purpose

This measurement records the first working one-file Floppy144 Windows runtime.

The river2D software renderer is compiled directly into Floppy144.exe.
The executable does not load or require river2Dsoftware.dll.

## Build

- Configuration: Release
- Platform: Windows x64
- Compiler: Microsoft Visual C
- Branch: floppy144/vs001-static-win32
- Internal canvas: 640 x 360
- Application type: Windows GUI application
- Runtime files: 1

## Runtime File

| File | Bytes | SHA-256 |
|---|---:|---|
| Floppy144.exe | 15,872 | 3E55C0685CB74A20F02E3A3C8E4A4B3DED4F4621E54158DF1D18A1ACDEC3F7BF |

## Comparison

| Build | Bytes |
|---|---:|
| Stock EXE plus renderer DLL | 29,184 |
| Static renderer EXE | 15,872 |
| Saving | 13,312 |

The static renderer build is 45.61 percent smaller than the original
two-file runtime.

## Capacity

- Contest ceiling: 1,474,560 bytes
- Runtime size: 15,872 bytes
- Remaining capacity: 1,458,688 bytes
- Disk capacity used: 1.08 percent
- Prototype red line: 307,200 bytes
- Space below prototype red line: 291,328 bytes

## Verified Dependencies

The executable depends on standard Windows and Microsoft runtime components:

- USER32.dll
- GDI32.dll
- KERNEL32.dll
- VCRUNTIME140.dll
- Universal C Runtime API sets

It does not depend on river2Dsoftware.dll.

## Verification

A clean directory containing only Floppy144.exe was tested successfully.

The application:

- opened a window titled Floppy//144
- created and displayed the software framebuffer
- resized correctly
- closed normally
- produced no compiler warnings or errors

## Next Investigation

Test whether imgsurf.lib is genuinely required by the current executable.

Floppy144 does not currently load external images. Its visuals are generated
procedurally in memory.
