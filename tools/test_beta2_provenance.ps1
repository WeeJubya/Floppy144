# B2-05: inspect the current source tree and actual archive/import manifest.
# Historical provenance documents are intentionally preserved and excluded
# from active-source expression scans. This is technical, not legal sign-off.
param(
    [ValidateSet('Source','Linked')]
    [string]$Phase = 'Source'
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    if($Phase -eq 'Source') {
        $removed = @(
            'include/f144_runtime.h',
            'src/f144_runtime.c',
            'src/f144_win32_runtime.c',
            'src/f144_win32_platform.c',
            'include/f144_win32_platform.h',
            'src/string_view.c',
            'include/string_view.h',
            '.gitmodules'
        )
        foreach($path in $removed) {
            if(Test-Path -LiteralPath $path) {
                throw "Obsolete River2D/utility/submodule path remains: $path"
            }
        }
        if(Test-Path -LiteralPath 'vendor/puddle') {
            throw 'Obsolete puddle vendor checkout remains.'
        }
        $gitlinks = @(git ls-files --stage | Where-Object { $_ -match '^160000\s' })
        if($LASTEXITCODE -ne 0 -or $gitlinks.Count -ne 0) {
            throw 'Unexpected tracked submodule reference remains.'
        }

        $premake = Get-Content -LiteralPath 'premake5.lua' -Raw
        foreach($required in @(
            './src/floppy144_win32_display.c',
            './src/floppy144_win32_host.c',
            './src/f144_win32_timing.c'
        )) {
            if(-not $premake.Contains($required)) {
                throw "Production build omits independent source: $required"
            }
        }
        if(([regex]::Matches($premake, 'staticruntime\("On"\)')).Count -ne 3) {
            throw 'Three build targets must link the static C runtime.'
        }

        $roots = @('game/src','src','include','tools','platform/win32')
        $active = @(
            foreach($folder in $roots) {
                Get-ChildItem -LiteralPath $folder -File -Recurse |
                    Where-Object { $_.Extension -in @('.c','.h','.inc','.def','.rc') }
            }
        )
        $forbidden = @(
            '\bF144Runtime\b',
            '\bF144Image\b',
            '\bF144_MAX_PLANES\b',
            '\bf144Win32PlatformBind\b',
            '\bf144Win32BltBuffer\b',
            '\bf144Win32CompositeImage\b',
            '\bf144Win32LoadText\b',
            '\b(?:cstr_sv|sv_cstr|sv_find|StringView)\b',
            '(?i)#\s*include\s*[<"](?:f144_runtime|f144_win32_platform|string_view)\.h[>"]',
            '(?i)\briver2d\b',
            '(?i)\bpuddle\b',
            '(?i)\bimgsurf\b'
        )
        foreach($file in $active) {
            $contents = Get-Content -LiteralPath $file.FullName -Raw
            foreach($expression in $forbidden) {
                if($contents -match $expression) {
                    # The surviving Credits comment is an attribution pointer,
                    # not executable or copied runtime code. Preserve precisely
                    # that single reference, and reject additional occurrences.
                    if($expression -eq '(?i)\briver2d\b' -and
                       $file.Name -eq 'floppy144_credits_view.c' -and
                       $contents.Contains('detailed River2D / GPL notices remain in the') -and
                       ([regex]::Matches($contents, $expression)).Count -eq 1) {
                        continue
                    }
                    throw "Legacy or uncertain code symbol remains in active source: $($file.FullName) :: $expression"
                }
            }
        }

        if(-not (Test-Path 'docs/stage4/runtime_provenance.md') -or
           -not (Test-Path 'docs/beta2/01_provenance_inventory.md') -or
           -not (Test-Path 'LICENSE')) {
            throw 'Historical attribution and licence records must be preserved.'
        }
        Write-Host ("B2-05 ACTIVE-SOURCE PROVENANCE: PASS; scanned {0} source/header/generated/test files." -f $active.Count)
        Write-Host 'Historical licence and technical provenance records preserved.'
        return
    }

    $lib = '.\bin\release\floppy144platformwin32.lib'
    $exe = '.\bin\release\Floppy144.exe'
    if(-not (Test-Path $lib) -or -not (Test-Path $exe)) {
        throw 'Release binary and native platform archive must exist.'
    }
    $objects = & lib.exe /nologo /list $lib | Out-String
    if($LASTEXITCODE -ne 0) { throw 'Could not read static object manifest.' }
    foreach($required in @('floppy144_win32_display.obj','floppy144_win32_host.obj')) {
        if($objects -notmatch [regex]::Escape($required)) {
            throw "Independent compiled object not found: $required"
        }
    }
    foreach($forbidden in @('f144_runtime.obj','f144_win32_runtime.obj',
                            'f144_win32_platform.obj','string_view.obj')) {
        if($objects -match "(?im)(?:^|[\\/])$([regex]::Escape($forbidden))(?:\s|$)") {
            throw "Old derived object remains in platform archive: $forbidden"
        }
    }
    $imports = & dumpbin.exe /dependents $exe | Out-String
    if($LASTEXITCODE -ne 0) { throw 'Could not inspect executable imports.' }
    $allowed = @('USER32.DLL','KERNEL32.DLL','GDI32.DLL','SHELL32.DLL','WINMM.DLL')
    $observed = @([regex]::Matches($imports, '(?mi)^\s*([a-z0-9_.-]+\.dll)\s*$') |
                  ForEach-Object { $_.Groups[1].Value.ToUpperInvariant() })
    if($observed.Count -ne 5) { throw "Unexpected number of DLL imports: $($observed -join ', ')" }
    foreach($item in $observed) {
        if($item -notin $allowed) {
            throw "Unexpected third-party or CRT DLL imported: $item"
        }
    }
    $bytes = (Get-Item $exe).Length
    $remaining = 1474560 - $bytes
    Write-Host "B2-05 LINKED EXECUTABLE BYTES: $bytes"
    Write-Host "B2-05 EXECUTABLE REMAINING: $remaining"
    if($remaining -lt 0) { throw 'Executable exceeds 1,474,560 bytes.' }
    Write-Host ('B2-05 RELEASE SHA256: ' + (Get-FileHash $exe -Algorithm SHA256).Hash)
    Write-Host ("B2-05 SYSTEM IMPORTS: {0}" -f ($observed -join ', '))
    Write-Host 'B2-05 LINKED OBJECTS AND IMPORTS: PASS'
}
finally {
    Pop-Location
}
