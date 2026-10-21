# Beta.2 proprietary-licence guard; do not relicense Beta.1 history.
param(
    [ValidateSet('Source','Built')]
    [string]$Phase = 'Source'
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$activePath = Join-Path $root 'LICENSE'
$legacyRelative = 'licenses/history/Beta1-GPL-3.0.txt'
$legacyPath = Join-Path $root $legacyRelative
$historicGitBlob = 'b1b737ee5e3c21b4d2f41d38020d8214009019ae'

if(-not (Test-Path -LiteralPath $activePath)) {
    throw 'Beta.2 current root LICENSE is missing.'
}
$active = Get-Content -LiteralPath $activePath -Raw
foreach($required in @(
    'FLOPPY//144 BETA.2',
    'Grey Door Republik',
    'All rights reserved',
    'WeeJubya',
    'NOT apply retrospectively to Floppy//144 Beta.1',
    'licenses/history/Beta1-GPL-3.0.txt'
)) {
    if(-not $active.Contains($required)) {
        throw "Beta.2 proprietary LICENSE missing required text: $required"
    }
}
if($active.TrimStart().StartsWith('GNU GENERAL PUBLIC LICENSE')) {
    throw 'Beta.2 active root LICENSE still contains the previous GPLv3 grant.'
}
if(-not (Test-Path -LiteralPath $legacyPath)) {
    throw 'Original Beta.1 GPLv3 licence has not been preserved.'
}
$original = Get-Content -LiteralPath $legacyPath -Raw
if($original -notmatch 'GNU GENERAL PUBLIC LICENSE' -or
   $original -notmatch 'river2D. Copyright \(C\) 2026 BadAcronym') {
    throw 'Historical GPLv3 or River2D original copyright notice missing.'
}
$historicalSha = (& git -C $root hash-object -- $legacyRelative).Trim()
if($LASTEXITCODE -ne 0 -or $historicalSha -cne $historicGitBlob) {
    throw "Beta.1 GPLv3 historical licence altered: $historicalSha"
}

$premake = Get-Content -LiteralPath (Join-Path $root 'premake5.lua') -Raw
if(-not $premake.Contains('{COPYFILE} "%{wks.location}/../LICENSE" "%{cfg.targetdir}/LICENSE"')) {
    throw 'Premake must package the active Beta.2 LICENSE, not the historical one.'
}

if($Phase -eq 'Built') {
    $built = Join-Path $root 'bin/release/LICENSE'
    $exe = Join-Path $root 'bin/release/Floppy144.exe'
    if(-not (Test-Path -LiteralPath $exe) -or
       -not (Test-Path -LiteralPath $built)) {
        throw 'Release EXE and accompanying Beta.2 licence must be present.'
    }
    $a = (Get-FileHash -LiteralPath $activePath -Algorithm SHA256).Hash
    $b = (Get-FileHash -LiteralPath $built -Algorithm SHA256).Hash
    if($a -cne $b) {
        throw 'Release directory contains a stale or mismatched LICENSE.'
    }
    $installed = Get-Content -LiteralPath $built -Raw
    if(-not $installed.Contains('All rights reserved')) {
        throw 'Release package does not contain proprietary Beta.2 terms.'
    }
    Write-Host 'BETA.2 LICENCE PACKAGE: PASS; packaged licence matches current proprietary root LICENSE.'
}
Write-Host "BETA.2 LICENCE $($Phase): PASS; Beta.1 GPLv3 historical Git blob $historicalSha preserved."
