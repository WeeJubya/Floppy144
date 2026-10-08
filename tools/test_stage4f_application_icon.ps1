param
(
    [ValidateSet("Source", "Generated", "Built")]
    [string]$Phase = "Source",

    [string]$Executable = ".\bin\release\Floppy144.exe"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Assert-True
{
    param
    (
        [bool]$Condition,
        [string]$Message
    )

    if(-not $Condition)
    {
        throw $Message
    }
}

function Test-Floppy144IconSource
{
    $masterPath = ".\assets\branding\floppy144_app_icon.svg"
    $icoPath = ".\platform\win32\floppy144.ico"
    $rcPath = ".\platform\win32\floppy144_app.rc"
    $headerPath = ".\platform\win32\floppy144_resource.h"
    $premakePath = ".\premake5.lua"
    $mainPath = ".\game\src\floppy144_main.c"

    foreach($path in @(
        $masterPath,
        $icoPath,
        $rcPath,
        $headerPath,
        $premakePath,
        $mainPath
    ))
    {
        Assert-True (Test-Path -LiteralPath $path) "Missing S4F-01 icon asset/wiring: $path"
    }

    $master = Get-Content -LiteralPath $masterPath -Raw
    Assert-True ($master -match '<svg\b') "Canonical icon source is not SVG."
    Assert-True ($master -match 'viewBox="0 0 64 64"') "Canonical icon design grid changed."
    Assert-True ($master -match '#C2994C') "Canonical icon lost the FLOPPY//144 amber accent."
    Assert-True ($master -notmatch '<text\b') "Canonical icon must not depend on tiny rendered text."

    $ico = [System.IO.File]::ReadAllBytes($icoPath)
    Assert-True ($ico.Length -gt 6) "Windows ICO is unexpectedly short."
    Assert-True ([BitConverter]::ToUInt16($ico, 0) -eq 0) "ICO reserved word must be zero."
    Assert-True ([BitConverter]::ToUInt16($ico, 2) -eq 1) "ICO type must be 1."
    Assert-True ([BitConverter]::ToUInt16($ico, 4) -eq 7) "ICO must contain exactly seven frames."

    $pngSignature = [byte[]](137, 80, 78, 71, 13, 10, 26, 10)
    $expectedSizes = @(16, 24, 32, 48, 64, 128, 256)
    $actualSizes = @()

    for($i = 0; $i -lt 7; $i++)
    {
        $entry = 6 + ($i * 16)
        $width = [int]$ico[$entry]
        $height = [int]$ico[$entry + 1]

        if($width -eq 0) { $width = 256 }
        if($height -eq 0) { $height = 256 }

        Assert-True ($width -eq $height) "ICO frame $i is not square."
        Assert-True ([BitConverter]::ToUInt16($ico, $entry + 6) -eq 32) "ICO frame $width must be 32-bit."

        $bytesInResource = [BitConverter]::ToUInt32($ico, $entry + 8)
        $imageOffset = [BitConverter]::ToUInt32($ico, $entry + 12)

        Assert-True (($imageOffset + $bytesInResource) -le $ico.Length) "ICO frame $width exceeds file bounds."

        for($j = 0; $j -lt $pngSignature.Length; $j++)
        {
            Assert-True ($ico[$imageOffset + $j] -eq $pngSignature[$j]) "ICO frame $width is not PNG encoded."
        }

        $actualSizes += $width
    }

    Assert-True (($actualSizes -join ",") -eq ($expectedSizes -join ",")) "ICO frame sizes changed: $($actualSizes -join ', ')."

    $rc = Get-Content -LiteralPath $rcPath -Raw
    $header = Get-Content -LiteralPath $headerPath -Raw
    $premake = Get-Content -LiteralPath $premakePath -Raw
    $main = Get-Content -LiteralPath $mainPath -Raw

    Assert-True ($header -match "#define\s+IDI_FLOPPY144_APP_ICON\s+101") "Resource ID 101 is missing."
    Assert-True ($rc -match 'IDI_FLOPPY144_APP_ICON\s+ICON\s+"floppy144\.ico"') "RC file does not bind the application icon."

    $launcherStart = $premake.IndexOf('project("Floppy144")')
    Assert-True ($launcherStart -ge 0) "Floppy144 launcher project was not found in Premake."
    $launcherPremake = $premake.Substring($launcherStart)

    Assert-True ($launcherPremake -match 'filter\("platforms:Windows"\)[\s\S]*floppy144_app\.rc') "Windows launcher does not include the icon resource."
    Assert-True ($launcherPremake -match 'resincludedirs\([\s\S]*platform/win32') "Windows resource include directory is missing."

    $coreStart = $premake.IndexOf('project("Floppy144Core")')
    $platformStart = $premake.IndexOf('project("Floppy144PlatformWin32")')
    Assert-True ($coreStart -ge 0 -and $platformStart -gt $coreStart) "Premake Core/platform project boundaries are missing."
    $corePremake = $premake.Substring($coreStart, $platformStart - $coreStart)
    Assert-True ($corePremake -notmatch 'floppy144_app\.rc|floppy144\.ico|floppy144_resource\.h') "Application resources leaked into Floppy144Core."

    Assert-True ($main -match '#include\s+"floppy144_resource\.h"') "Win32 launcher does not include the resource ID header."
    Assert-True ($main -match 'hIcon\s*=\s*LoadIconA\(\s*instance,\s*MAKEINTRESOURCEA\(\s*IDI_FLOPPY144_APP_ICON\s*\)') "Win32 window class does not load the FLOPPY//144 icon."

    Write-Host "S4F-01 ICON SOURCE: PASS"
    Write-Host "Canonical SVG: 64x64 design grid"
    Write-Host "ICO frames: $($actualSizes -join ', ')"
}

function Test-Floppy144GeneratedProject
{
    Test-Floppy144IconSource

    $projectPath = ".\build\Floppy144.vcxproj"
    Assert-True (Test-Path -LiteralPath $projectPath) "Generated Floppy144.vcxproj is missing."

    $project = Get-Content -LiteralPath $projectPath -Raw
    Assert-True ($project -match 'floppy144_app\.rc') "Regenerated Visual Studio project dropped the icon RC."
    Assert-True ($project -match 'platform[\\/]+win32') "Regenerated Visual Studio project dropped the Win32 resource include path."

    Write-Host "S4F-01 GENERATED PROJECT: PASS"
}

function Test-Floppy144BuiltExecutable
{
    param
    (
        [string]$Path
    )

    Test-Floppy144IconSource

    Assert-True (Test-Path -LiteralPath $Path) "Built executable is missing: $Path"

    if(-not ("Floppy144NativeResourceProbe" -as [type]))
    {
        Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

public static class Floppy144NativeResourceProbe
{
    private const uint LOAD_LIBRARY_AS_DATAFILE = 0x00000002;

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern IntPtr LoadLibraryExW(string fileName, IntPtr file, uint flags);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr FindResourceW(IntPtr module, IntPtr name, IntPtr type);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern uint SizeofResource(IntPtr module, IntPtr resource);

    [DllImport("kernel32.dll")]
    private static extern bool FreeLibrary(IntPtr module);

    public static uint GroupIconSize(string fileName, int resourceId)
    {
        IntPtr module = LoadLibraryExW(fileName, IntPtr.Zero, LOAD_LIBRARY_AS_DATAFILE);
        if(module == IntPtr.Zero)
        {
            return 0;
        }

        try
        {
            IntPtr resource = FindResourceW(module, new IntPtr(resourceId), new IntPtr(14));
            if(resource == IntPtr.Zero)
            {
                return 0;
            }

            return SizeofResource(module, resource);
        }
        finally
        {
            FreeLibrary(module);
        }
    }
}
"@
    }

    $resolvedPath = (Resolve-Path -LiteralPath $Path).Path
    $groupSize = [Floppy144NativeResourceProbe]::GroupIconSize($resolvedPath, 101)

    Assert-True ($groupSize -gt 0) "Built executable does not contain RT_GROUP_ICON resource 101."

    Write-Host "S4F-01 BUILT EXECUTABLE ICON: PASS"
    Write-Host "Executable: $resolvedPath"
    Write-Host "RT_GROUP_ICON 101 size: $groupSize bytes"
}

switch($Phase)
{
    "Source"
    {
        Test-Floppy144IconSource
    }

    "Generated"
    {
        Test-Floppy144GeneratedProject
    }

    "Built"
    {
        Test-Floppy144BuiltExecutable $Executable
    }
}
