$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$data = Get-Content -Raw -LiteralPath (Join-Path $root "data\floppy144_game_data.json") |
    ConvertFrom-Json
$compiled = Get-Content -Raw -LiteralPath (Join-Path $root "game\src\floppy144_documents.generated.inc")
$collDef = Get-Content -Raw -LiteralPath (Join-Path $root "game\src\floppy144_collections.def")
$catalogue = Get-Content -Raw -LiteralPath (Join-Path $root "game\src\floppy144_catalogue.c")
$compiler = Get-Content -Raw -LiteralPath (Join-Path $root "tools\game_data_compiler.c")

Write-Host "=== STAGE 4E ARCHIVE DENSITY / DISCOVERABILITY AUDIT ==="

$targets = @{ 'Prologue' = 15; 'Act I' = 25; 'Act II' = 50; 'Act III' = 75 }
if(@($data.collections).Count -ne 35 -or @($data.documents).Count -ne 170) {
    throw "Canonical collection or authored-document identities changed."
}

$rows = [regex]::Matches(
    $compiled,
    '(?m)^\s*\{ FLOPPY144_COLLECTION_([A-Z0-9]+), (\d+)U, "([^"]+)", "([^"]+)"'
)
if($rows.Count -ne 170) {
    throw "Generated authored registry must include all 170 canonical documents (found $($rows.Count))."
}

$byCollection = @{}
$authorIds = [System.Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal
)
foreach($m in $rows) {
    $symbol=$m.Groups[1].Value
    $slot=[int]$m.Groups[2].Value
    $id=$m.Groups[3].Value
    $title=$m.Groups[4].Value
    if(-not $authorIds.Add($id)) { throw "Duplicate authored record ID: $id" }
    if(-not $byCollection.ContainsKey($symbol)) {
        $byCollection[$symbol]=[System.Collections.Generic.List[object]]::new()
    }
    $byCollection[$symbol].Add([pscustomobject]@{
        Slot=$slot; Id=$id; Title=$title
    })
}

$allIDs = [System.Collections.Generic.HashSet[string]]::new(
    [StringComparer]::Ordinal
)
$total=0
$authored=0
$inversions=0
$spreadCollections=0
for($ci=0; $ci -lt @($data.collections).Count; ++$ci) {
    $c=$data.collections[$ci]
    $cid=[string]$c.id
    $symbol=$cid.Replace('-','')
    $docs=@($data.documents | Where-Object { $_.collection_id -eq $cid })
    $budget=0
    if($null -ne $c.content_budget) {
        $budget=[int]$c.content_budget.records
        $act=[string]$c.content_budget.act
        if(-not $targets.ContainsKey($act)) { throw "Unknown act on $cid." }
        if($budget -ne $targets[$act]) {
            throw "Wrong S4E-08 density for $cid : expected $($targets[$act]), got $budget."
        }
    }
    $count=[Math]::Max($budget,$docs.Count)
    $total+=$count
    $authored+=$docs.Count
    # PowerShell unwraps pipeline output from an if expression. Force
    # an array even for OS-41's single authored document and empty holdings.
    $compiledDocs = @(
        if($byCollection.ContainsKey($symbol)) {
            $byCollection[$symbol].ToArray()
        }
    )
    if($compiledDocs.Count -ne $docs.Count) {
        throw "$cid authored documents missing: $($compiledDocs.Count) != $($docs.Count)."
    }
    $slots=[System.Collections.Generic.HashSet[int]]::new()
    foreach($item in $compiledDocs) {
        if($item.Slot -lt 0 -or $item.Slot -ge $count -or
           -not $slots.Add($item.Slot)) {
            throw "Out-of-range or colliding authored slot in $cid : $($item.Slot)."
        }
    }
    if($docs.Count -gt 3 -and $count -gt 1) {
        $ordered=@($compiledDocs | Sort-Object Slot)
        if($ordered[0].Slot -le [int]($count / 3) -and
           $ordered[-1].Slot -ge [int](($count * 2) / 3)) {
            ++$spreadCollections
        }
        for($k=1;$k -lt $compiledDocs.Count;++$k) {
            if($compiledDocs[$k].Slot -lt $compiledDocs[$k-1].Slot) {
                ++$inversions
            }
        }
    }

    if($count -eq 0) { continue }
    $numbers=[System.Collections.Generic.List[int]]::new()
    $offset=11 + $ci*17
    for($i=0;$i -lt $count;++$i) {
        $number=1 + (($i*37 + $offset) % ($count*11))
        $numbers.Add([int]$number)
    }
    $sorted=@($numbers | Sort-Object)
    $slotToId=@{}
    foreach($item in $compiledDocs) { $slotToId[$item.Slot]=$item.Id }
    for($i=0;$i -lt $count;++$i) {
        $id=if($slotToId.ContainsKey($i)) {
            $slotToId[$i]
        } else {
            '{0}-RS-{1:D4}' -f $cid,$sorted[$i]
        }
        if(-not $allIDs.Add($id)) {
            throw "Duplicate / colliding player-facing catalogue ID: $id"
        }
    }
}
if($total -ne 1571 -or $authored -ne 170 -or
   $allIDs.Count -ne 1571) {
    throw "Archive total mismatch: $total rows / $authored authored / $($allIDs.Count) unique."
}
if($spreadCollections -lt 25 -or $inversions -lt 60) {
    throw "Authored material was not spread/reordered sufficiently: $spreadCollections spread, $inversions inversions."
}
foreach($fixedId in @('DR-01-RS-0001','FM-13-RS-0047','HR-01-RS-0107')) {
    if(-not $authorIds.Contains($fixedId)) {
        throw "Preserved explicit authored reference missing: $fixedId"
    }
}
if($collDef -notmatch 'floppy144_generated_generic_subjects, 36U' -or
   $catalogue -notmatch 'MISCELLANEOUS MINUTE' -or
   $catalogue -notmatch 'floppy144_number_cache' -or
   $compiler -notmatch 'size_t scrambled=authored_total>1U' -or
   $compiler -notmatch 'floppy144_generated_generic_subjects, 36U') {
    throw "New word pools, seeded authored placement or numbering cache not integrated."
}

# BUG FIX 07: verify HR-01-RS-0092 at the normal 536-pixel viewer width.
# The generated runtime body must match the canonical authored body.
$directory = @($data.documents | Where-Object { $_.id -eq 'HR-01-RS-0006' })
if($directory.Count -ne 1) {
    throw "Canonical Staff Contact Directory missing or duplicated."
}
$directory = $directory[0]
if($directory.title -ne 'Staff Contact Directory' -or
   $directory.collection_id -ne 'HR-01' -or
   [int]$directory.record_index -ne 5 -or
   $null -ne $directory.trigger_id) {
    throw "Staff Contact Directory identity or trigger changed."
}
$expectedDirectoryEntries = @(
    "Helen Cartwright | Director | Director's Office | Ext. 2909",
    "Priya Patel | Senior Archivist | Main Office / Records Office | Ext. 2509",
    "Daniel Price | Records Officer | Records Office / Main Office | Ext. 1109",
    "Claire Hughes | IT Manager | Main Office / IT Support | Ext. 2202",
    "Imran Shah | Facilities Liaison | Facilities / Main Office | Ext. 8822",
    "Martin Webb | Project Manager | Main Office | Ext. 2303",
    "Rachel Morgan | Security Manager | Security Office | Ext. 0806"
)
$staffRows = @(
    ([string]$directory.body -split "\r?\n") |
        Where-Object { $_ -match ' \| .* \| Ext\. \d{4}$' }
)
if($staffRows.Count -ne 7) {
    throw "Staff Contact Directory must contain seven formatted entries."
}
for($i = 0; $i -lt $staffRows.Count; ++$i) {
    $entry = [string]$staffRows[$i]
    if($entry -cne $expectedDirectoryEntries[$i]) {
        throw "Staff Contact Directory entry $($i+1) changed."
    }
    # The runtime bitmap font is six pixels per character minus one pixel.
    if(($entry.Length * 6 - 1) -gt 536) {
        throw "Staff Contact Directory entry $($i+1) exceeds the viewer width."
    }
    if($entry -match ' {2,}' -or $entry -match '(?i)\bDesk\s*\d+' -or
       $entry -notmatch '^[^|]+ \| [^|]+ \| [^|]+ \| Ext\. \d{4}$') {
        throw "Staff Contact Directory entry $($i+1) has inconsistent formatting."
    }
}
if([string]$directory.body -match '(?i)\bDesk\s*\d+') {
    throw "Staff Contact Directory still contains a desk number."
}
$compiledDirectory = [regex]::Match(
    $compiled,
    '(?m)^\s*\{ FLOPPY144_COLLECTION_HR01, 7U, "HR-01-RS-0092", "Staff Contact Directory", [^\r\n]*$'
)
if(-not $compiledDirectory.Success -or
   $compiledDirectory.Value -notmatch 'FLOPPY144_TRIGGER_COUNT') {
    throw "Generated HR-01-RS-0092 identity or trigger has changed."
}
$escapedBody = ([string]$directory.body).Replace('\','\\').Replace('"','\"').Replace([string][char]13,"").Replace([string][char]10,'\n')
if(-not $compiledDirectory.Value.Contains('"' + $escapedBody + '"')) {
    throw "Generated HR-01-RS-0092 body differs from canonical source."
}
Write-Host "HR-01-RS-0092 LAYOUT: PASS - 7 one-line staff entries"

Write-Host "S4E-08 ARCHIVE AUDIT: PASS - $total entries ($authored authored, $($total-$authored) generated), $inversions order inversions, $spreadCollections spread collections"
