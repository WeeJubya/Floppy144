# FLOPPY//144 Regression Test Build
# Exact original root-script order: Stage 2, Stage 3A, Stage 3B.
# Stage 4 regression sequence follows the existing Windows CI, unchanged.
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$root = $PSScriptRoot
Push-Location $root
try {
    Write-Host ""
    Write-Host "=== FLOPPY//144 REGRESSION TEST BUILD ==="
    Write-Host "=== PREPARE CANONICAL GENERATED DATA ==="

    # Shared compile prerequisite, not a duplicated regression check.
    $global:LASTEXITCODE = 0
    & (Join-Path $root 'tools\build_game_data.ps1')
    if($LASTEXITCODE -ne 0) {
        throw "Game-data preparation failed with exit code $LASTEXITCODE."
    }

    # The final release-candidate Source phase additionally requires a clean,
    # pushed Git checkout; it stays in CI. Binary checks follow the build.
    $checks = @(
        [pscustomobject]@{ Label = 'Bug fix 15 chair occupancy'; Script = 'test_bugfix15_chairs.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 2 regression'; Script = 'test_stage2.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 3A regression'; Script = 'test_stage3a.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 3B regression'; Script = 'test_stage3b.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 platform boundary'; Script = 'test_stage4_platform.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 logical input'; Script = 'test_stage4_input.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 persistence paths'; Script = 'test_stage4_persistence.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 audio contract'; Script = 'test_stage4_audio.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 timing and lifecycle'; Script = 'test_stage4_timing_lifecycle.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 single-instance protection'; Script = 'test_stage4_single_instance.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 developer configuration'; Script = 'test_stage4_config.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E deterministic variation'; Script = 'test_stage4_variation.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E platform-neutral calendar'; Script = 'test_stage4_calendar.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E date-aware noticeboard'; Script = 'test_stage4_noticeboard.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E deterministic takeaway menu'; Script = 'test_stage4_takeaway.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E deterministic crossword variants'; Script = 'test_stage4_crossword.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E deterministic paperback titles'; Script = 'test_stage4_paperback.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E archive density and record-identity audit'; Script = 'test_stage4_archive.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C operator profile screen'; Script = 'test_stage4_profile_screen.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C operator name entry'; Script = 'test_stage4_profile_name.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C terminal authentication'; Script = 'test_stage4_terminal_auth.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C persistent settings'; Script = 'test_stage4_settings.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C credits and attribution'; Script = 'test_stage4_credits.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C restored-session presentation'; Script = 'test_stage4_reinstate_presentation.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C bespoke completion presentation'; Script = 'test_stage4_completion.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4D widened Help presentation'; Script = 'test_stage4_help_presentation.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4D office camera scale'; Script = 'test_stage4_office_camera.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4D directional player character'; Script = 'test_stage4_player_visual.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4D pseudo-isometric Inspection containers'; Script = 'test_stage4_inspection_25d.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4D presentation consistency'; Script = 'test_stage4_presentation_consistency.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4D integration sign-off audit'; Script = 'test_stage4d_integration.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4 build architecture'; Script = 'test_stage4_build_architecture.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4B integration sign-off audit'; Script = 'test_stage4b_integration.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4C integration sign-off audit'; Script = 'test_stage4c_integration.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4E integration, determinism and replay audit'; Script = 'test_stage4e_integration.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4F application icon source'; Script = 'test_stage4f_application_icon.ps1'; Parameters = @{ Phase = 'Source' } },
        [pscustomobject]@{ Label = 'Stage 4F GDR connection intro'; Script = 'test_stage4f_intro.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4F whole-game presentation journey'; Script = 'test_stage4f_presentation_journey.ps1'; Parameters = @{} },
        [pscustomobject]@{ Label = 'Stage 4G isolated Grey Door discovery'; Script = 'test_stage4g_grey_door.ps1'; Parameters = @{} }
    )
    foreach($check in $checks) {
        $path = Join-Path $root ("tools\" + $check.Script)
        if(-not (Test-Path -LiteralPath $path)) {
            throw "Required regression missing: $($check.Label) ($path)."
        }
        Write-Host ""
        Write-Host ("=== {0} ===" -f $check.Label)
        $parameters = $check.Parameters
        $global:LASTEXITCODE = 0
        try {
            & $path @parameters
        }
        catch {
            throw ("Regression failed [{0}] in {1}: {2}" -f $check.Label, $check.Script, $_)
        }
        if($LASTEXITCODE -ne 0) {
            throw ("Regression failed [{0}] with exit code {1}." -f $check.Label, $LASTEXITCODE)
        }
    }
    Write-Host ""
    Write-Host "FLOPPY//144 FULL REGRESSION SUITE: PASS" -ForegroundColor Green
}
finally {
    Pop-Location
}
