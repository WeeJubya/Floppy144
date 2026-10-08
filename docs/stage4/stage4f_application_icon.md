# S4F-01 — FLOPPY//144 Application Icon

**Branch:** `Stage4`  
**Canonical source:** `assets/branding/floppy144_app_icon.svg`  
**Windows derivative:** `platform/win32/floppy144.ico`

## Identity

The application icon is derived from the existing opening-screen **Disk 144**
art rather than introducing a separate logo language. It keeps the reversed
3.5-inch floppy silhouette, including the clipped lower-left corner, and uses
the splash palette: charcoal casing, paper-grey label, steel shutter and
muted amber accent.

At application-icon scale the written `DISK 144 / GDR RECOVERY` label is
deliberately replaced by the amber `//` mark from `FLOPPY//144`. This avoids
tiny unreadable text and leaves a recognisable mark at 16x16 and 32x32.

The canonical source is a lightweight SVG with a 64x64 design grid. It is the
cross-platform visual master. No editor-specific heavyweight design file is
required.

## Windows integration

`platform/win32/floppy144_app.rc` declares resource
`IDI_FLOPPY144_APP_ICON` (101) from `platform/win32/floppy144.ico`.

The `.ico` contains native frames at:

- 16x16
- 24x24
- 32x32
- 48x48
- 64x64
- 128x128
- 256x256

Each frame is 32-bit RGBA PNG data inside the ICO container. Small frames are
rendered deliberately instead of asking Windows to reduce a large image at
runtime.

Premake adds the resource script only to the Windows `Floppy144` launcher
target. `Floppy144Core` and `Floppy144PlatformWin32` do not own or compile
application branding. The Win32 window class loads resource 101 for its native
window/task-switcher icon, while the PE resource supplies the Explorer
executable icon.

Because the current competition size gate measures the complete Release
`Floppy144.exe`, the embedded Windows icon **does count** toward that measured
payload. The repository-side SVG and ICO do not count by themselves; only
resource bytes linked into the executable do. S4F-01 therefore preserves the
existing size-accounting rule rather than inventing a platform-wrapper
exception.

## Future platform derivation

Keep `assets/branding/floppy144_app_icon.svg` as the canonical visual source
and rasterise/adapt it for each package:

- **Linux:** generate PNGs for the required `hicolor` application-icon sizes,
  retaining the transparent outer area.
- **macOS:** derive the standard iconset sizes through 1024x1024 and package
  them as the platform app icon / ICNS resource.
- **iOS:** render a 1024x1024 source image, then produce an opaque,
  platform-compliant app-icon asset set. Do not ship alpha where Apple
  packaging forbids it.
- **Android:** derive launcher artwork from the same motif, adapting it to an
  adaptive-icon foreground/background pair and safe zone rather than blindly
  reusing the Windows ICO.

All platform packaging derivatives remain outside `Floppy144Core`. No
mobile/macOS/Linux packaging is introduced by S4F-01.

## Regression protection

`tools/test_stage4f_application_icon.ps1` verifies:

1. the canonical SVG identity contract;
2. every required ICO frame and its 32-bit PNG payload;
3. resource ID/path wiring;
4. that Premake keeps the resource on the final Windows launcher rather than
   Core;
5. that regenerated Visual Studio projects retain the resource;
6. that built Windows executables contain resource group 101
   (`RT_GROUP_ICON`).

CI runs the source check, regeneration check, a Debug launcher build/icon
probe, and the final Release icon probe.
