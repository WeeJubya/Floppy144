# river2D Stock Windows Baseline

## Source

- Repository: BadAcronym/river2D
- Branch baseline: gh-stable
- Source commit: dab1612e82053c91d818d8ec98677c0632297238
- Build configuration: Release / Windows
- Architecture: x86_64
- Renderer configuration: Stock dynamically loaded software renderer

## Build Results

| File | Purpose | Bytes | Shipping Status |
|---|---|---:|---|
| river2Dcommon.lib | Core static library | 125,674 | Build only |
| imgsurf.lib | Image-surface static library | 80,604 | Build only |
| imgsurftest.exe | Vendor test executable | 29,184 | Excluded |
| river2Dsoftware.dll | Software renderer | 14,336 | Required by stock architecture |
| river2Dsoftware.lib | Renderer import library | 2,588 | Build only |
| river2Dsoftware.exp | Renderer export data | 1,089 | Build only |

## Totals

- Total build output: 253,475 bytes
- Stock renderer DLL: 14,336 bytes
- Vendor test executable: 29,184 bytes
- Executable and DLL output combined: 43,520 bytes
- Contest ceiling: 1,474,560 bytes
- Prototype red line: 307,200 bytes

## Interpretation

The stock river2D build does not produce a game executable.

The only component currently relevant to a distributable river2D application is
river2Dsoftware.dll. The imgsurftest.exe file is a vendor test utility and must
not be treated as part of the Floppy144 runtime.

The first meaningful runtime measurement will be taken after adding the minimal
Floppy144 executable target.

The eventual one-binary build will compile the software renderer directly into
Floppy144.exe, removing the runtime DLL and its dynamic loading mechanism.

## File Verification

### river2Dcommon.lib

SHA-256:

B6A940F093F9D2EB53FCB9D60988C7B07B4C78E522FA30ABBC35BD84A2A547E1

### imgsurf.lib

SHA-256:

DADCA5F38754709C13C41E3E6437ED7B8F23CE3AA92AA85B8061B1D56974A6C7

### imgsurftest.exe

SHA-256:

4ABD10256FBAD198F8D406E33F5518CDB8EB4D9AB5A7021CC5B801617FDFED23

### river2Dsoftware.dll

SHA-256:

E7199BAC42F5A4497B299E91C8BF8D7D14C7D8E6A2B27A63B7EC5C142B616A26
