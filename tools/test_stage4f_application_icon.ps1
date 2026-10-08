param
(
    [ValidateSet("Source", "Generated", "Built")]
    [string]$Phase = "Source",

    [string]$Executable = ".\bin\release\Floppy144.exe"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$root = Split-Path -Parent $PSScriptRoot

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
    $masterPath = Join-Path $root "assets\branding\floppy144_app_icon.svg"
    $icoPath = Join-Path $root "platform\win32\floppy144.ico"
    $rcPath = Join-Path $root "platform\win32\floppy144_app.rc"
    $headerPath = Join-Path $root "platform\win32\floppy144_resource.h"
    $premakePath = Join-Path $root "premake5.lua"
    $mainPath = Join-Path $root "game\src\floppy144_main.c"

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
    Assert-True ($master.Contains("<svg")) "Canonical icon source is not SVG."
    Assert-True ($master.Contains('viewBox="0 0 64 64"')) "Canonical icon design grid changed."
    Assert-True ($master.Contains("#C2994C")) "Canonical icon lost the FLOPPY//144 amber accent."
    Assert-True (-not $master.Contains("<text")) "Canonical icon must not depend on tiny rendered text."

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

    Assert-True ($header.Contains("#define IDI_FLOPPY144_APP_ICON 101")) "Resource ID 101 is missing."
    Assert-True ($rc.Contains('IDI_FLOPPY144_APP_ICON ICON "floppy144.ico"')) "RC file does not bind the application icon."

    $coreStart = $premake.IndexOf('project("Floppy144Core")')
    $platformStart = $premake.IndexOf('project("Floppy144PlatformWin32")')
    $launcherStart = $premake.IndexOf('project("Floppy144")', $platformStart + 1)

    Assert-True ($coreStart -ge 0) "Floppy144Core project was not found in Premake."
    Assert-True ($platformStart -gt $coreStart) "Floppy144PlatformWin32 project was not found after Core."
    Assert-True ($launcherStart -gt $platformStart) "Floppy144 launcher project was not found after PlatformWin32."

    $corePremake = $premake.Substring($coreStart, $platformStart - $coreStart)
    $platformPremake = $premake.Substring($platformStart, $launcherStart - $platformStart)
    $launcherPremake = $premake.Substring($launcherStart)

    Assert-True (-not $corePremake.Contains("platform/win32")) "Platform include path leaked into Floppy144Core."
    Assert-True (-not $corePremake.Contains("floppy144_app.rc")) "Application icon resource leaked into Floppy144Core."
    Assert-True (-not $platformPremake.Contains("floppy144_app.rc")) "Application icon resource leaked into Floppy144PlatformWin32."
    Assert-True (-not $platformPremake.Contains("floppy144_resource.h")) "Application resource header leaked into Floppy144PlatformWin32."

    Assert-True ($launcherPremake.Contains("./platform/win32/")) "Windows launcher resource include path is missing."
    Assert-True ($launcherPremake.Contains("./platform/win32/floppy144_app.rc")) "Windows launcher does not include the icon resource."
    Assert-True ($launcherPremake.Contains("./platform/win32/floppy144_resource.h")) "Windows launcher does not include the resource header."
    Assert-True ($launcherPremake.Contains("resincludedirs({")) "Windows launcher resource include configuration is missing."

    Assert-True ($main.Contains('#include "floppy144_resource.h"')) "Win32 launcher does not include the resource ID header."
    Assert-True ($main.Contains("window_class.hIcon")) "Win32 window class icon assignment is missing."
    Assert-True ($main.Contains("LoadIconA(")) "Win32 window class does not load the FLOPPY//144 icon."
    Assert-True ($main.Contains("IDI_FLOPPY144_APP_ICON")) "Win32 launcher does not reference the FLOPPY//144 icon resource."

    Write-Host "S4F-01 ICON SOURCE: PASS"
    Write-Host "Canonical SVG: 64x64 design grid"
    Write-Host "ICO frames: $($actualSizes -join ', ')"
}

function Test-Floppy144GeneratedProject
{
    Test-Floppy144IconSource

    $projectPath = Join-Path $root "build\Floppy144.vcxproj"
    Assert-True (Test-Path -LiteralPath $projectPath) "Generated Floppy144.vcxproj is missing."

    $project = Get-Content -LiteralPath $projectPath -Raw
    $normalisedProject = $project.Replace("\", "/")

    Assert-True ($normalisedProject.Contains("floppy144_app.rc")) "Regenerated Visual Studio project dropped the icon RC."
    Assert-True ($normalisedProject.Contains("platform/win32")) "Regenerated Visual Studio project dropped the Win32 resource path."

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
