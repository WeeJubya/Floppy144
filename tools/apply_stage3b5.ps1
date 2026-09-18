param()

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$Root = Split-Path -Parent $ScriptDir
$SourceDir = Join-Path $Root "game\src"

function Read-ProjectText {
    param([Parameter(Mandatory=$true)][string]$Path)
    if(-not (Test-Path $Path)) {
        throw "Stage 3B.5 patch target is missing: $Path"
    }
    return [System.IO.File]::ReadAllText($Path)
}

function Write-ProjectText {
    param(
        [Parameter(Mandatory=$true)][string]$Path,
        [Parameter(Mandatory=$true)][string]$Text
    )
    $Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $Utf8NoBom)
}

function Add-AfterOnce {
    param(
        [Parameter(Mandatory=$true)][string]$Text,
        [Parameter(Mandatory=$true)][string]$Anchor,
        [Parameter(Mandatory=$true)][string]$Addition,
        [Parameter(Mandatory=$true)][string]$Description
    )
    $Index = $Text.IndexOf($Anchor)
    if($Index -lt 0) {
        throw "Stage 3B.5 could not locate $Description. No files were intentionally guessed around this anchor."
    }
    return $Text.Insert($Index + $Anchor.Length, $Addition)
}

function Add-BeforeOnce {
    param(
        [Parameter(Mandatory=$true)][string]$Text,
        [Parameter(Mandatory=$true)][string]$Anchor,
        [Parameter(Mandatory=$true)][string]$Addition,
        [Parameter(Mandatory=$true)][string]$Description
    )
    $Index = $Text.IndexOf($Anchor)
    if($Index -lt 0) {
        throw "Stage 3B.5 could not locate $Description. No files were intentionally guessed around this anchor."
    }
    return $Text.Insert($Index, $Addition)
}

function Add-BeforeLastOnce {
    param(
        [Parameter(Mandatory=$true)][string]$Text,
        [Parameter(Mandatory=$true)][string]$Anchor,
        [Parameter(Mandatory=$true)][string]$Addition,
        [Parameter(Mandatory=$true)][string]$Description
    )
    $Index = $Text.LastIndexOf($Anchor)
    if($Index -lt 0) {
        throw "Stage 3B.5 could not locate $Description. No files were intentionally guessed around this anchor."
    }
    return $Text.Insert($Index, $Addition)
}

function Replace-Once {
    param(
        [Parameter(Mandatory=$true)][string]$Text,
        [Parameter(Mandatory=$true)][string]$Old,
        [Parameter(Mandatory=$true)][string]$New,
        [Parameter(Mandatory=$true)][string]$Description
    )
    $Index = $Text.IndexOf($Old)
    if($Index -lt 0) {
        throw "Stage 3B.5 could not locate $Description. No files were intentionally guessed around this anchor."
    }
    return $Text.Remove($Index, $Old.Length).Insert($Index, $New)
}

function Patch-RunStateHeader {
    $Path = Join-Path $SourceDir "floppy144_run_state.h"
    $Text = Read-ProjectText $Path
    if($Text.Contains("STAGE 3B.5 SECURE CABINET STATE")) { return }
    $NL = if($Text.Contains("`r`n")) { "`r`n" } else { "`n" }

    $Text = Add-AfterOnce $Text `
        "#define FLOPPY144_RUN_WORD_BITS             32" `
        ($NL + $NL + "/* STAGE 3B.5 SECURE CABINET STATE */" + $NL + "#define FLOPPY144_SECURE_CABINET_MAX          32U") `
        "RunState word-size declaration"

    $Text = Add-AfterOnce $Text `
        "    int32_t player_site_y;" `
        ($NL + $NL + "    /* Persistent per-cabinet unlocks for the current recovery. */" + $NL + "    uint32_t secure_cabinets_unlocked;") `
        "RunState player position fields"

    $Prototype = @(
        "/* Stage 3B.5: generated secure-cabinet unlock state. */",
        "bool Floppy144RunStateSecureCabinetUnlocked(",
        "    const Floppy144RunState *state,",
        "    uint32_t uCabinetIndex",
        ");",
        "",
        "bool Floppy144RunStateUnlockSecureCabinet(",
        "    Floppy144RunState *state,",
        "    uint32_t uCabinetIndex",
        ");",
        "",
        ""
    ) -join $NL

    $Text = Add-BeforeOnce $Text `
        "bool Floppy144RunStateBitGet" `
        $Prototype `
        "RunState bit-query API"

    Write-ProjectText $Path $Text
}

function Patch-RunStateSource {
    $Path = Join-Path $SourceDir "floppy144_run_state.c"
    $Text = Read-ProjectText $Path
    if($Text.Contains("STAGE 3B.5 SECURE CABINET STATE")) { return }
    $NL = if($Text.Contains("`r`n")) { "`r`n" } else { "`n" }

    $Block = @(
        "/* STAGE 3B.5 SECURE CABINET STATE",
        " *",
        " * Cabinet ordinals are derived from canonical generated secure-cabinet",
        " * furniture order. One 32-bit word is sufficient for the current Site.",
        " */",
        "bool Floppy144RunStateSecureCabinetUnlocked(",
        "    const Floppy144RunState *state,",
        "    uint32_t uCabinetIndex",
        ")",
        "{",
        "    uint32_t uMask;",
        "",
        "    if(state == NULL || uCabinetIndex >= FLOPPY144_SECURE_CABINET_MAX)",
        "    {",
        "        return false;",
        "    }",
        "",
        "    uMask = 1U << uCabinetIndex;",
        "    return (state->secure_cabinets_unlocked & uMask) != 0U;",
        "}",
        "",
        "bool Floppy144RunStateUnlockSecureCabinet(",
        "    Floppy144RunState *state,",
        "    uint32_t uCabinetIndex",
        ")",
        "{",
        "    uint32_t uMask;",
        "",
        "    if(state == NULL || uCabinetIndex >= FLOPPY144_SECURE_CABINET_MAX)",
        "    {",
        "        return false;",
        "    }",
        "",
        "    uMask = 1U << uCabinetIndex;",
        "",
        "    if((state->secure_cabinets_unlocked & uMask) != 0U)",
        "    {",
        "        return false;",
        "    }",
        "",
        "    state->secure_cabinets_unlocked |= uMask;",
        "    state->dirty = 1U;",
        "    return true;",
        "}",
        "",
        ""
    ) -join $NL

    $Text = Add-BeforeOnce $Text `
        "bool Floppy144RunStateBitGet" `
        $Block `
        "RunState bit helper implementation"

    Write-ProjectText $Path $Text
}

function Patch-PersistenceHeader {
    $Path = Join-Path $SourceDir "floppy144_persistence.h"
    $Text = Read-ProjectText $Path
    if($Text.Contains("secure_cabinets_unlocked)")) { return }
    $NL = if($Text.Contains("`r`n")) { "`r`n" } else { "`n" }

    $Old = "    16U +                                                         \" + $NL + "    sizeof(((Floppy144RunState *)0)->rooms) +"
    $New = "    16U +                                                         \" + $NL + "    sizeof(((Floppy144RunState *)0)->secure_cabinets_unlocked) +   \" + $NL + "    sizeof(((Floppy144RunState *)0)->rooms) +"
    $Text = Replace-Once $Text $Old $New "run-state payload-size formula"

    Write-ProjectText $Path $Text
}

function Patch-PersistenceSource {
    $Path = Join-Path $SourceDir "floppy144_persistence.c"
    $Text = Read-ProjectText $Path
    if($Text.Contains("STAGE 3B.5 SECURE CABINET PERSISTENCE")) { return }
    $NL = if($Text.Contains("`r`n")) { "`r`n" } else { "`n" }

    $WriteAnchor = @(
        "    Floppy144PersistenceWriteU32(",
        "        &payload[offset],",
        "        (uint32_t)state->player_site_y",
        "    );",
        "",
        "    offset += 4U;"
    ) -join $NL

    $WriteAddition = @(
        "",
        "",
        "    /* STAGE 3B.5 SECURE CABINET PERSISTENCE */",
        "    Floppy144PersistenceWriteU32(",
        "        &payload[offset],",
        "        state->secure_cabinets_unlocked",
        "    );",
        "",
        "    offset += 4U;"
    ) -join $NL

    $Text = Add-AfterOnce $Text $WriteAnchor $WriteAddition "RunState persistence encoder position fields"

    $ReadAnchor = @(
        "    decoded.player_site_y =",
        "    (int32_t)Floppy144PersistenceReadU32(",
        "        &payload[offset]",
        "    );",
        "",
        "    offset += 4U;"
    ) -join $NL

    $ReadAddition = @(
        "",
        "",
        "    decoded.secure_cabinets_unlocked =",
        "    Floppy144PersistenceReadU32(",
        "        &payload[offset]",
        "    );",
        "",
        "    offset += 4U;"
    ) -join $NL

    $Text = Add-AfterOnce $Text $ReadAnchor $ReadAddition "RunState persistence decoder position fields"

    Write-ProjectText $Path $Text
}

function Patch-MainCoordinator {
    $Path = Join-Path $SourceDir "floppy144_main.c"
    $Text = Read-ProjectText $Path
    $NL = if($Text.Contains("`r`n")) { "`r`n" } else { "`n" }

    if(-not $Text.Contains('#include "floppy144_cabinet.h"')) {
        $Text = Add-AfterOnce $Text `
            '#include "floppy144_catalogue.h"' `
            ($NL + '#include "floppy144_cabinet.h"') `
            "main coordinator catalogue include"
    }

    if(-not $Text.Contains('FLOPPY144_SCREEN_CABINET')) {
        $Text = Add-AfterOnce $Text `
            "    FLOPPY144_SCREEN_OFFICE," `
            ($NL + "    FLOPPY144_SCREEN_CABINET,") `
            "main screen enum Office entry"
    }

    if(-not $Text.Contains('static Floppy144CabinetState global_cabinet;')) {
        $Text = Add-AfterOnce $Text `
            "static Floppy144CatalogueState global_catalogue;" `
            ($NL + "static Floppy144CabinetState global_cabinet;") `
            "main catalogue state declaration"
    }

    if(-not $Text.Contains('STAGE 3B.5 CABINET DRAW')) {
        $DrawBlock = @(
            "        /* STAGE 3B.5 CABINET DRAW */",
            "        case FLOPPY144_SCREEN_CABINET:",
            "        {",
            "            Floppy144CabinetDraw(",
            "                global_runtime,",
            "                &global_cabinet,",
            "                &global_run_state",
            "            );",
            "",
            "            break;",
            "        }",
            "",
            ""
        ) -join $NL

        $Text = Add-BeforeOnce $Text `
            "        case FLOPPY144_SCREEN_NOTEBOOK:" `
            $DrawBlock `
            "Notebook draw case"
    }

    <#
        Stage 3B.2 moved Site interaction targeting away from the legacy
        Floppy144SiteInteractionTarget path. Patch A=Access at the player-facing
        Office key route instead, then preserve the existing dispatcher below it.
        Existing Site access keeps priority when an access target and cabinet overlap.
    #>
    if(-not $Text.Contains('STAGE 3B.5 CABINET ACCESS KEY')) {
        $OfficeStart = $Text.IndexOf("                case FLOPPY144_SCREEN_OFFICE:")
        if($OfficeStart -lt 0) {
            throw "Stage 3B.5 could not locate the Office key-routing case. No files were intentionally guessed around this anchor."
        }

        $NotebookStart = $Text.IndexOf("                case FLOPPY144_SCREEN_NOTEBOOK:", $OfficeStart)
        if($NotebookStart -le $OfficeStart) {
            throw "Stage 3B.5 could not locate the end of Office key routing. No files were intentionally guessed around this anchor."
        }

        $OfficeSource = $Text.Substring($OfficeStart, $NotebookStart - $OfficeStart)
        $AccessMatch = [regex]::Match(
            $OfficeSource,
            "case\s+'A'\s*:\s*\{",
            [System.Text.RegularExpressions.RegexOptions]::Singleline
        )

        if(-not $AccessMatch.Success) {
            throw "Stage 3B.5 could not locate the Office A=Access key case. No files were intentionally guessed around this anchor."
        }

        $CabinetAccess = @(
            "",
            "                            /* STAGE 3B.5 CABINET ACCESS KEY */",
            "                            {",
            "                                if(",
            "                                    (",
            "                                        Floppy144SiteAvailableActions(&global_run_state) &",
            "                                        FLOPPY144_SITE_ACTION_ACCESS",
            "                                    ) == 0U &&",
            "                                    Floppy144CabinetOpenNearby(",
            "                                        &global_cabinet,",
            "                                        &global_run_state",
            "                                    )",
            "                                )",
            "                                {",
            "                                    global_office_notice = NULL;",
            "                                    global_resume_screen = FLOPPY144_SCREEN_CABINET;",
            "                                    global_screen = FLOPPY144_SCREEN_CABINET;",
            "                                    Floppy144Redraw(window);",
            "                                    return 0;",
            "                                }",
            "                            }",
            ""
        ) -join $NL

        $InsertAt = $OfficeStart + $AccessMatch.Index + $AccessMatch.Length
        $Text = $Text.Insert($InsertAt, $CabinetAccess)
    }

    if(-not $Text.Contains('STAGE 3B.5 CABINET CHARACTER INPUT')) {
        $CharMatch = [regex]::Match(
            $Text,
            "case\s+WM_CHAR\s*:\s*\{",
            [System.Text.RegularExpressions.RegexOptions]::Singleline
        )

        if(-not $CharMatch.Success) {
            throw "Stage 3B.5 could not locate the WM_CHAR handler. No files were intentionally guessed around this anchor."
        }

        $CabinetChar = @(
            "",
            "            /* STAGE 3B.5 CABINET CHARACTER INPUT */",
            "            if(global_screen == FLOPPY144_SCREEN_CABINET)",
            "            {",
            "                if(w_param >= '0' && w_param <= '9')",
            "                {",
            "                    (void)Floppy144CabinetInputDigit(",
            "                        &global_cabinet,",
            "                        (char)w_param",
            "                    );",
            "                }",
            "",
            "                Floppy144Redraw(window);",
            "                return 0;",
            "            }",
            ""
        ) -join $NL

        $Text = $Text.Insert(
            $CharMatch.Index + $CharMatch.Length,
            $CabinetChar
        )
    }

    if(-not $Text.Contains('STAGE 3B.5 CABINET KEY ROUTING')) {
        $CabinetKeys = @(
            "                /* STAGE 3B.5 CABINET KEY ROUTING",
            "                 *",
            "                 * Keypad and Interior share one reusable screen state.",
            "                 * Escape remains the universal Session Control route.",
            "                 */",
            "                case FLOPPY144_SCREEN_CABINET:",
            "                {",
            "                    switch(w_param)",
            "                    {",
            "                        case VK_UP:",
            "                        {",
            "                            Floppy144CabinetMoveSelection(",
            "                                &global_cabinet,",
            "                                &global_run_state,",
            "                                -1",
            "                            );",
            "                            Floppy144Redraw(window);",
            "                            return 0;",
            "                        }",
            "",
            "                        case VK_DOWN:",
            "                        {",
            "                            Floppy144CabinetMoveSelection(",
            "                                &global_cabinet,",
            "                                &global_run_state,",
            "                                1",
            "                            );",
            "                            Floppy144Redraw(window);",
            "                            return 0;",
            "                        }",
            "",
            "                        case VK_RETURN:",
            "                        {",
            "                            if(Floppy144CabinetInteriorOpen(&global_cabinet))",
            "                            {",
            "                                (void)Floppy144CabinetInspectSelected(",
            "                                    &global_cabinet,",
            "                                    &global_world,",
            "                                    &global_run_state",
            "                                );",
            "                            }",
            "                            else",
            "                            {",
            "                                (void)Floppy144CabinetSubmitCode(",
            "                                    &global_cabinet,",
            "                                    &global_world,",
            "                                    &global_run_state",
            "                                );",
            "                            }",
            "",
            "                            Floppy144Redraw(window);",
            "                            return 0;",
            "                        }",
            "",
            "                        case 'I':",
            "                        {",
            "                            if(Floppy144CabinetInteriorOpen(&global_cabinet))",
            "                            {",
            "                                (void)Floppy144CabinetInspectSelected(",
            "                                    &global_cabinet,",
            "                                    &global_world,",
            "                                    &global_run_state",
            "                                );",
            "                                Floppy144Redraw(window);",
            "                            }",
            "                            return 0;",
            "                        }",
            "",
            "                        case VK_BACK:",
            "                        {",
            "                            if(!Floppy144CabinetBackspace(&global_cabinet))",
            "                            {",
            "                                global_resume_screen = FLOPPY144_SCREEN_OFFICE;",
            "                                global_screen = FLOPPY144_SCREEN_OFFICE;",
            "                            }",
            "",
            "                            Floppy144Redraw(window);",
            "                            return 0;",
            "                        }",
            "                    }",
            "",
            "                    break;",
            "                }",
            "",
            ""
        ) -join $NL

        $Text = Add-BeforeLastOnce $Text `
            "                case FLOPPY144_SCREEN_NOTEBOOK:" `
            $CabinetKeys `
            "Notebook key-routing case"
    }

    Write-ProjectText $Path $Text
}

function Patch-SitePrompts {
    foreach($FileName in @("floppy144_site_2d.c", "floppy144_site_isometric.c")) {
        $Path = Join-Path $SourceDir $FileName
        $Text = Read-ProjectText $Path
        $NL = if($Text.Contains("`r`n")) { "`r`n" } else { "`n" }

        if(-not $Text.Contains('#include "floppy144_cabinet.h"')) {
            $Text = Add-AfterOnce $Text `
                '#include "floppy144_draw.h"' `
                ($NL + '#include "floppy144_cabinet.h"') `
                "$FileName draw include"
        }

        if(-not $Text.Contains('Floppy144CabinetState sCabinetProbe;')) {
            $ActionDeclaration = [regex]::Match(
                $Text,
                "uint32_t\s+uActions\s*;"
            )

            if(-not $ActionDeclaration.Success) {
                throw "Stage 3B.5 could not locate $FileName interaction-prompt action declaration. No files were intentionally guessed around this anchor."
            }

            $Text = $Text.Insert(
                $ActionDeclaration.Index + $ActionDeclaration.Length,
                $NL + "    Floppy144CabinetState sCabinetProbe;"
            )
        }

        if(-not $Text.Contains('STAGE 3B.5 SECURE CABINET ACCESS PROMPT')) {
            if($FileName -eq "floppy144_site_2d.c") {
                $StateName = "run_state"
            }
            else {
                $StateName = "pRunState"
            }

            $Pattern =
                "uActions\s*=\s*Floppy144SiteAvailableActions\s*\(\s*" +
                [regex]::Escape($StateName) +
                "\s*\)\s*;"

            $ActionMatch = [regex]::Match(
                $Text,
                $Pattern,
                [System.Text.RegularExpressions.RegexOptions]::Singleline
            )

            if(-not $ActionMatch.Success) {
                throw "Stage 3B.5 could not locate $FileName Site action query. No files were intentionally guessed around this anchor."
            }

            $PromptAddition = @(
                "",
                "",
                "    /* STAGE 3B.5 SECURE CABINET ACCESS PROMPT */",
                "    Floppy144CabinetReset(&sCabinetProbe);",
                "    if(Floppy144CabinetOpenNearby(&sCabinetProbe, $StateName))",
                "    {",
                "        uActions |= FLOPPY144_SITE_ACTION_ACCESS;",
                "    }"
            ) -join $NL

            $Text = $Text.Insert(
                $ActionMatch.Index + $ActionMatch.Length,
                $PromptAddition
            )
        }

        Write-ProjectText $Path $Text
    }
}

Patch-RunStateHeader
Patch-RunStateSource
Patch-PersistenceHeader
Patch-PersistenceSource
Patch-MainCoordinator
Patch-SitePrompts

Write-Host "STAGE 3B.5 PATCH: APPLIED"
Write-Host "New module files are already present under game\src."
Write-Host "Next: .\tools\test_stage3b.ps1"
