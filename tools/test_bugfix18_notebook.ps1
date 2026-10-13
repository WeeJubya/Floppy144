param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot
$data = Get-Content -LiteralPath (Join-Path $root 'data\floppy144_game_data.json') -Raw | ConvertFrom-Json
$docs = @($data.documents)
if($docs.Count -ne 170) { throw "BF18 expected 170 authored documents, got $($docs.Count)" }

$ids = @{}
$summaries = @{}
foreach($doc in $docs) {
    if([string]::IsNullOrWhiteSpace([string]$doc.body)) {
        throw "BF18 authored body is empty: $($doc.id)"
    }
    if($ids.ContainsKey([string]$doc.id)) { throw "BF18 duplicate document ID: $($doc.id)" }
    $ids[[string]$doc.id] = 1
    $summary = [string]$doc.notebook_summary
    if([string]::IsNullOrWhiteSpace($summary) -or $summary.Length -lt 35) {
        throw "BF18 summary empty or too short: $($doc.id)"
    }
    if($summary -match '[.!?]\s+\S') {
        throw "BF18 summary must be a single sentence: $($doc.id)"
    }
    if($summaries.ContainsKey($summary)) { throw "BF18 duplicate summary: $($doc.id)" }
    $summaries[$summary] = 1
}
$lines = @(Get-Content -LiteralPath (Join-Path $root 'game\src\floppy144_game_data.generated.inc') |
    Where-Object { $_ -match '^FLOPPY144_DATA_RECORD\(FLOPPY144_DATA_NOTEBOOK, "[^"]+-RS-[^"]+", ' -and
                   $_ -match ', "DOCUMENT", ' })
if($lines.Count -ne $docs.Count) {
    throw "BF18 generated runtime has $($lines.Count) summaries for $($docs.Count) documents"
}
Write-Host "BF18 AUTHOR COVERAGE: $($docs.Count) document bodies; $($summaries.Count) unique summaries; $($lines.Count) runtime entries PASS"
