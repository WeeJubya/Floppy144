# B2-06: verify actual public Beta.1 binary and pinned release source.
# Reference is downloaded and hashed, NEVER executed.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = Split-Path -Parent $PSScriptRoot
$expectedSource = '0981e4ab3694a1cdb5b6338c199e0262a6ed14bf'
$expectedSize = 822272L
$expectedHash = '7daf713c4cbe5103b3bc7b1316140581614b484c397ca5a83352ec96fa8de749'
$asset = 'Floppy144.0.4.0-beta.1.exe'
$url = 'https://github.com/GreyDoorRepublik/Floppy144-beta/releases/download/Floppy144/' + $asset
$folder = Join-Path ([IO.Path]::GetTempPath()) ('F144Beta1Ref-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $folder | Out-Null
try {
    $exe = Join-Path $folder $asset
    Invoke-WebRequest -Uri $url -OutFile $exe -TimeoutSec 120 -MaximumRedirection 10
    $size = (Get-Item -LiteralPath $exe).Length
    $hash = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash.ToLowerInvariant()
    if($size -ne $expectedSize -or $hash -ne $expectedHash) {
        throw "Published Beta.1 binary mismatch: bytes=$size sha256=$hash"
    }
    Write-Host "B2-06 VERIFIED BETA.1 EXE: $size bytes, SHA256 $hash"

    $source = Join-Path $folder 'published-source'
    & git clone --quiet --depth 1 --branch Floppy144 'https://github.com/GreyDoorRepublik/Floppy144-beta.git' $source
    if($LASTEXITCODE -ne 0) { throw 'Cannot obtain published Beta.1 tag.' }
    $actual = (& git -C $source rev-parse HEAD).Trim()
    if($LASTEXITCODE -ne 0 -or $actual -ne $expectedSource) {
        throw "Published Beta.1 source tag has moved: $actual"
    }
    Write-Host "B2-06 VERIFIED BETA.1 SOURCE: $actual"
    # Only original game-facing files are expected byte-identical.
    # Exclude launcher/intro, intentionally changed since publication.
    $stable = @(
        'include/f144_platform.h',
        'src/f144_platform.c',
        'game/src/floppy144_draw.c',
        'game/src/floppy144_persistence.c',
        'game/src/floppy144_storage.c',
        'game/src/floppy144_profile.c',
        'game/src/floppy144_settings.c',
        'game/src/floppy144_site_isometric.c',
        'game/src/floppy144_grey_door.c',
        'game/src/floppy144_audio.c',
        'src/f144audio.c',
        'src/f144sfx.c',
        'src/f144_win32_audio.c'
    )
    foreach($path in $stable) {
        $published = Join-Path $source $path
        $current = Join-Path $root $path
        if(-not (Test-Path -LiteralPath $published) -or
           -not (Test-Path -LiteralPath $current)) {
            throw "Comparison source missing: $path"
        }
        $a = (Get-FileHash -LiteralPath $published -Algorithm SHA256).Hash
        $b = (Get-FileHash -LiteralPath $current -Algorithm SHA256).Hash
        if($a -ne $b) { throw "Unexpected stable Beta.1 source drift: $path" }
    }
    Write-Host ("B2-06 BETA.1 GAME CODE MATCH: PASS ({0} stable files)" -f $stable.Count)

    foreach($name in @('floppy144_manual.sav','floppy144_auto.sav',
                        'floppy144_profile.dat','floppy144_settings.dat')) {
        if(Test-Path -LiteralPath (Join-Path $source $name)) {
            Write-Host "B2-06 PUBLISHED SAVE FIXTURE PRESENT: $name"
        }
        else {
            Write-Host "B2-06 PUBLISHED SAVE FIXTURE MISSING: $name"
        }
    }
    Write-Host 'B2-06 BETA.1 REFERENCE: PASS (binary/source identity; visual and audio parity remain separate)'
}
finally {
    Remove-Item -LiteralPath $folder -Force -Recurse -ErrorAction SilentlyContinue
}
