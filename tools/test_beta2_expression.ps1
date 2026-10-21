# B2-07 exact-expression regression guard. Mechanical check, not legal clearance.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$historical = '38afcfe5b6ed8ac5da08039479c205ab883239d9'
$legacyPaths = @(
    'include/f144_runtime.h',
    'src/f144_runtime.c',
    'src/f144_win32_runtime.c',
    'src/f144_win32_platform.c',
    'include/f144_win32_platform.h'
)
$replacementPaths = @(
    'src/floppy144_win32_display.c',
    'src/floppy144_win32_host.c',
    'src/f144_win32_timing.c',
    'include/floppy144_win32_display.h',
    'include/floppy144_win32_host.h',
    'game/src/floppy144_main.c'
)
# Shared platform ABI glue, not rasterisation; reject any additional match.
$sharedInterfaceLines = @(
    'return &platform->surface;',
    'f144Win32PlatformPersistencePath,',
    'f144Win32PlatformLegacyPersistencePath,',
    'f144Win32AudioPlayMusic,',
    'f144Win32AudioStopMusic,',
    'f144Win32AudioSetMusicVolume,',
    'f144Win32AudioSetSfxVolume,',
    'f144Win32LocalCalendarDate,'
)

function Get-SubstantiveLines([string]$source) {
    foreach($raw in ($source -split '\r?\n')) {
        $line = ($raw.Trim() -replace '\s+',' ')
        if($line.Length -lt 24 -or $line -match '^(//|/\*|\*|\*/|#)') { continue }
        $line
    }
}

Push-Location $root
try {
    $legacy = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::Ordinal)
    foreach($path in $legacyPaths) {
        $spec = "$($historical):$path"
        $source = (& git show $spec) -join [Environment]::NewLine
        if($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($source)) {
            throw "Cannot access preserved historical source: $spec"
        }
        foreach($line in (Get-SubstantiveLines $source)) {
            [void]$legacy.Add($line)
        }
    }
    $allowed = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::Ordinal)
    foreach($line in $sharedInterfaceLines) { [void]$allowed.Add($line) }
    $foundAllowed = New-Object 'System.Collections.Generic.HashSet[string]' ([StringComparer]::Ordinal)
    foreach($path in $replacementPaths) {
        $source = Get-Content -LiteralPath $path -Raw
        $matches = 0
        foreach($line in (Get-SubstantiveLines $source)) {
            if(-not $legacy.Contains($line)) { continue }
            if($path -ne 'src/floppy144_win32_host.c' -or
               -not $allowed.Contains($line)) {
                throw "B2-07 unexpected historical source expression in $($path): $line"
            }
            [void]$foundAllowed.Add($line)
            ++$matches
        }
        Write-Host "B2-07 EXPRESSION COMPARISON: $path shared interface lines=$matches"
    }
    foreach($line in $sharedInterfaceLines) {
        if(-not $foundAllowed.Contains($line)) {
            throw "B2-07 obsolete/unobserved shared-interface exception: $line"
        }
    }
    Write-Host ("B2-07 EXACT-EXPRESSION RECHECK: PASS; {0} preserved old files, {1} current files, {2} acknowledged shared-interface lines." -f $legacyPaths.Count,$replacementPaths.Count,$foundAllowed.Count)
    Write-Host 'LIMIT: textual identity only; not semantic copying or independent legal rights certification.'
}
finally {
    Pop-Location
}
