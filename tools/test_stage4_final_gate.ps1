param(
    [ValidateSet("Source","Built")]
    [string]$Phase = "Source",
    [string]$ReleaseDirectory = ".\bin\release",
    [string]$Executable = ".\bin\release\Floppy144.exe"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try
{
    function Require-File([string]$relative)
    {
        if(-not (Test-Path -LiteralPath (Join-Path $root $relative) -PathType Leaf))
        {
            throw "Required Stage 4 sign-off file is missing: $relative"
        }
    }

    function Require-Text([string]$text,[string]$needle,[string]$message)
    {
        if($text -notmatch [regex]::Escape($needle))
        {
            throw $message
        }
    }

    function Assert-CleanCheckout
    {
        # Several inherited cl.exe regression harnesses place only their
        # intermediate .obj files in the repository root. Their executables
        # already live in temporary/build directories. Remove those untracked
        # intermediates before proving repository cleanliness, but never remove
        # a tracked object file or any other untracked path.
        foreach($object in @(Get-ChildItem -LiteralPath $root -File -Filter "*.obj"))
        {
            $tracked = @(& git ls-files -- $object.Name)
            if($LASTEXITCODE -ne 0)
            {
                throw "Could not determine whether regression object is tracked: $($object.Name)"
            }

            if($tracked.Count -ne 0)
            {
                throw "Refusing to remove tracked root object during final gate: $($object.Name)"
            }

            Remove-Item -LiteralPath $object.FullName -Force
        }

        $diff = & git diff --exit-code -- 2>&1
        if($LASTEXITCODE -ne 0)
        {
            $diff | Write-Host
            throw "Tracked files differ after canonical generation/tests."
        }

        $status = @(& git status --porcelain --untracked-files=all)
        if($LASTEXITCODE -ne 0)
        {
            throw "git status failed."
        }

        if($status.Count -ne 0)
        {
            $status | Write-Host
            throw "Working tree contains uncommitted or untracked non-ignored files."
        }
    }

    if($Phase -eq "Source")
    {
        Write-Host "=== S4F-04 STAGE 4 FINAL SOURCE / RELEASE-CANDIDATE AUDIT ==="

        foreach($required in @(
            "docs\stage4\stage3_baseline.md",
            "docs\stage4\runtime_provenance.md",
            "docs\stage4\platform_dependency_inventory.md",
            "docs\stage4\build_architecture.md",
            "docs\stage4\stage4b_completion.md",
            "docs\stage4\stage4c_completion.md",
            "docs\stage4\stage4d_completion.md",
            "docs\stage4\stage4e_completion.md",
            "docs\stage4\stage4f_application_icon.md",
            "docs\stage4\stage4f_intro.md",
            "docs\stage4\stage4f_presentation_journey.md",
            "docs\stage4\stage4g_completion.md",
            "docs\stage4\stage4_completion.md"
        ))
        {
            Require-File $required
        }

        $workflowPath = Join-Path $root ".github\workflows\stage3c-ci.yml"
        $workflow = Get-Content -LiteralPath $workflowPath -Raw

        foreach($step in @(
            "Stage 2 regression",
            "Stage 3A regression",
            "Stage 3B regression",
            "Stage 4 platform boundary",
            "Stage 4 logical input",
            "Stage 4 persistence paths",
            "Stage 4 audio contract",
            "Stage 4 timing and lifecycle",
            "Stage 4 single-instance protection",
            "Stage 4 developer configuration",
            "Stage 4C operator profile screen",
            "Stage 4C operator name entry",
            "Stage 4C terminal authentication",
            "Stage 4C persistent settings",
            "Stage 4C credits and attribution",
            "Stage 4C restored-session presentation",
            "Stage 4C bespoke completion presentation",
            "Stage 4D widened Help presentation",
            "Stage 4D office camera scale",
            "Stage 4D directional player character",
            "Stage 4D pseudo-isometric Inspection containers",
            "Stage 4D presentation consistency",
            "Stage 4D integration sign-off audit",
            "Stage 4 build architecture",
            "Stage 4B integration sign-off audit",
            "Stage 4C integration sign-off audit",
            "Stage 4E integration, determinism and replay audit",
            "Stage 4F application icon source",
            "Stage 4F GDR connection intro",
            "Stage 4F whole-game presentation journey",
            "Stage 4G isolated Grey Door discovery",
            "Clean Release rebuild",
            "Verify Release application icon",
            "Stage 4 final built-artifact gate",
            "Verify Stage 4 source package"
        ))
        {
            Require-Text $workflow $step "Final workflow is missing required gate: $step"
        }

        $productionRoots = @(
            (Join-Path $root "game\src"),
            (Join-Path $root "src"),
            (Join-Path $root "include"),
            (Join-Path $root "platform\win32")
        )

        $markerHits = @()
        foreach($productionRoot in $productionRoots)
        {
            if(-not (Test-Path -LiteralPath $productionRoot)){ continue }

            $markerHits += Get-ChildItem -LiteralPath $productionRoot -Recurse -File |
                Where-Object {
                    $_.Extension -in @(".c",".h",".rc")
                } |
                Select-String -Pattern '\b(TODO|FIXME|HACK)\b' -CaseSensitive:$false
        }

        if($markerHits.Count -ne 0)
        {
            $markerHits |
                ForEach-Object {
                    Write-Host "$($_.Path):$($_.LineNumber):$($_.Line.Trim())"
                }
            throw "Temporary TODO/FIXME/HACK marker remains in production source."
        }

        $gameSources = Get-ChildItem -LiteralPath (Join-Path $root "game\src") -File |
            Where-Object { $_.Extension -in @(".c",".h") }

        $rawSwitchHits = @(
            $gameSources |
                Select-String -Pattern '"-(debug|seed|date)(?:=|")' -CaseSensitive:$false
        )

        if($rawSwitchHits.Count -ne 0)
        {
            $rawSwitchHits |
                ForEach-Object {
                    Write-Host "$($_.Path):$($_.LineNumber):$($_.Line.Trim())"
                }
            throw "Raw developer switch spelling leaked into game/Core source."
        }

        $calendar = Get-Content -LiteralPath (Join-Path $root "src\f144_calendar.c") -Raw
        Require-Text $calendar "f144StartupConfigDebugEnabled" "Fixed-date provider no longer requires developer configuration."
        Require-Text $calendar "f144StartupConfigFixedDateOverride" "Fixed-date override route is missing."

        $vendorTracked = @(& git ls-files -- "vendor/*")
        if($LASTEXITCODE -ne 0)
        {
            throw "Unable to audit tracked vendor source."
        }
        if($vendorTracked.Count -ne 0)
        {
            $vendorTracked | Write-Host
            throw "Unexpected active vendor source appeared during Stage 4."
        }

        $provenance = Get-Content -LiteralPath (Join-Path $root "docs\stage4\runtime_provenance.md") -Raw
        foreach($required in @(
            "REVIEW REQUIRED",
            "River2D-derived",
            "GPLv3",
            "embedded 5x7 glyph",
            "technical source-provenance audit, not legal advice"
        ))
        {
            Require-Text $provenance $required "Stage 4A provenance action disappeared: $required"
        }

        foreach($required in @(
            "Package Stage 4 source",
            "Floppy144_Stage4_Source",
            "Verify Stage 4 source package",
            "Floppy144-Stage4-Source"
        ))
        {
            Require-Text $workflow $required "Stage 4 source packaging is stale or missing: $required"
        }

        foreach($forbidden in @(
            "Floppy144_Stage3C_Source",
            "Floppy144-Stage3C-Source"
        ))
        {
            if($workflow.Contains($forbidden))
            {
                throw "Stale Stage 3C release-artifact label remains: $forbidden"
            }
        }

        Assert-CleanCheckout

        Write-Host "S4F-04 FINAL SOURCE / RELEASE-CANDIDATE AUDIT: PASS"
        Write-Host "source=clean generated=clean vendor=none-active developer-overrides=gated provenance=review-required"
        exit 0
    }

    Write-Host "=== S4F-04 STAGE 4 FINAL BUILT-ARTIFACT AUDIT ==="

    $releasePath = [IO.Path]::GetFullPath((Join-Path $root $ReleaseDirectory))
    $exePath = [IO.Path]::GetFullPath((Join-Path $root $Executable))

    if(-not (Test-Path -LiteralPath $releasePath -PathType Container))
    {
        throw "Release output directory is missing: $releasePath"
    }
    if(-not (Test-Path -LiteralPath $exePath -PathType Leaf))
    {
        throw "Release executable is missing: $exePath"
    }

    $exe = Get-Item -LiteralPath $exePath
    if($exe.Length -gt 1474560)
    {
        throw "Release executable exceeds the 1,474,560-byte floppy limit."
    }

    $licensePath = Join-Path $releasePath "LICENSE"
    if(-not (Test-Path -LiteralPath $licensePath -PathType Leaf))
    {
        throw "Release output is missing the root LICENSE copy."
    }

    $releaseFiles = @(Get-ChildItem -LiteralPath $releasePath -Recurse -File)
    $executables = @($releaseFiles | Where-Object { $_.Extension -ieq ".exe" })
    if(
        $executables.Count -ne 1 -or
        $executables[0].Name -ine "Floppy144.exe"
    )
    {
        $executables.FullName | Write-Host
        throw "Release output contains an unexpected executable."
    }

    $forbiddenExtensions = @(
        ".sav",".dat",".json",".jsonc",".c",".h",".ps1",
        ".log",".tmp",".obj",".zip"
    )
    $forbiddenFiles = @(
        $releaseFiles |
            Where-Object {
                $forbiddenExtensions -contains $_.Extension.ToLowerInvariant() -or
                $_.Name -match '(?i)(^|[-_.])(test|fixture|debug)([-_.]|$)'
            }
    )
    if($forbiddenFiles.Count -ne 0)
    {
        $forbiddenFiles.FullName | Write-Host
        throw "Release directory contains developer/test/source/runtime-state artefacts."
    }

    Write-Host "Release directory files:"
    $releaseFiles |
        Sort-Object FullName |
        ForEach-Object {
            Write-Host ("  {0} ({1} bytes)" -f $_.Name,$_.Length)
        }

    Assert-CleanCheckout

    $localHead = (& git rev-parse HEAD).Trim()
    if($LASTEXITCODE -ne 0){ throw "Could not resolve checked-out HEAD." }

    $remoteLine = (& git ls-remote origin refs/heads/Stage4 | Select-Object -First 1)
    if($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($remoteLine))
    {
        throw "Could not resolve remote Stage4 HEAD."
    }
    $remoteHead = ($remoteLine -split '\s+')[0].Trim()

    if($localHead -ne $remoteHead)
    {
        throw "CI checkout HEAD $localHead does not match remote Stage4 $remoteHead."
    }

    Write-Host "Release bytes: $($exe.Length)"
    Write-Host "Remaining headroom: $(1474560 - $exe.Length)"
    Write-Host "Checked-out/remote Stage4 HEAD: $localHead"
    Write-Host "S4F-04 FINAL BUILT-ARTIFACT AUDIT: PASS"
}
finally
{
    Pop-Location
}
