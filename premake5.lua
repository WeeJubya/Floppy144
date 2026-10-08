---@diagnostic disable: undefined-global, undefined-field

workspace("Floppy144")
configurations({"debug", "asan", "release"})
platforms({"windows"})
location("build")
architecture("x86_64")
startproject("Floppy144")

--[[
Floppy144Core

Portable gameplay, world state, generated-data interpretation, rendering,
logical input, timing/lifecycle state, and the platform/startup contracts.

The three excluded game files are deliberate:
  - floppy144_main.c is the Windows application/coordinator;
  - floppy144_persistence.c still mixes portable codecs with Win32 file I/O;
  - floppy144_storage.c is application-level persistence migration glue.

S4B-08 documents persistence.c as remaining portability debt rather than
pretending it belongs in the portable library.
]]
project("Floppy144Core")
language("C")
cdialect("C99")
warnings("Extra")
kind("StaticLib")
targetname("floppy144core")

targetdir("bin/%{cfg.buildcfg}")
objdir("obj/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}")

includedirs({
    "./include/",
    "./game/src/"
})

files({
    "./game/src/**.c",
    "./game/src/**.h",
    "./src/f144_startup_config.c",
    "./src/f144_platform.c",
    "./src/f144_calendar.c",
    "./include/f144_startup_config.h",
    "./include/f144_platform.h"
})

removefiles({
    "./game/src/floppy144_main.c",
    "./game/src/floppy144_persistence.c",
    "./game/src/floppy144_persistence.h",
    "./game/src/floppy144_storage.c",
    "./game/src/floppy144_storage.h"
})

buildoptions({
    "/utf-8"
})

filter("configurations:debug")
runtime("debug")
symbols("On")
optimize("Off")

filter("configurations:asan")
runtime("debug")
symbols("On")
optimize("Off")
editandcontinue("Off")
buildoptions({
    "/fsanitize=address",
    "/Zi",
    "/INCREMENTAL:NO"
})

filter("configurations:release")
runtime("release")
symbols("Off")
optimize("Size")

filter({})


--[[
Floppy144PlatformWin32

Native Windows implementation of the F144 platform boundary plus the legacy
F144 software-runtime support still used by the Windows presenter.

This project owns BUILD_WINDOWS. Floppy144Core intentionally does not.
]]
project("Floppy144PlatformWin32")
language("C")
cdialect("C99")
warnings("Extra")
kind("StaticLib")
targetname("floppy144platformwin32")

targetdir("bin/%{cfg.buildcfg}")
objdir("obj/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}")

includedirs({
    "./include/",
    "./game/src/"
})

files({
    "./src/f144_runtime.c",
    "./src/f144_win32_runtime.c",
    "./src/string_view.c",
    "./src/f144_win32_audio.c",
    "./src/f144_win32_calendar.c",
    "./src/f144_win32_startup_config.c",
    "./src/f144_win32_input.c",
    "./src/f144_win32_lifecycle.c",
    "./src/f144_win32_platform.c",
    "./src/f144_win32_single_instance.c",
    "./src/f144_win32_storage.c",
    "./src/f144_win32_timing.c",
    "./include/f144_runtime.h",
    "./include/string_view.h",
    "./include/f144_win32_audio.h",
    "./include/f144_win32_calendar.h",
    "./include/f144_win32_startup_config.h",
    "./include/f144_win32_input.h",
    "./include/f144_win32_lifecycle.h",
    "./include/f144_win32_platform.h",
    "./include/f144_win32_single_instance.h",
    "./include/f144_win32_storage.h",
    "./include/f144_win32_timing.h"
})

dependson({
    "Floppy144Core"
})

filter("platforms:Windows")
system("Windows")

defines({
    "BUILD_WINDOWS"
})

buildoptions({
    "/wd4068",
    "/utf-8"
})

filter("configurations:debug")
runtime("debug")
symbols("On")
optimize("Off")

filter("configurations:asan")
runtime("debug")
symbols("On")
optimize("Off")
editandcontinue("Off")
buildoptions({
    "/fsanitize=address",
    "/Zi",
    "/INCREMENTAL:NO"
})

filter("configurations:release")
runtime("release")
symbols("Off")
optimize("Size")

filter({})


--[[
Floppy144

Windows application/launcher and application-level persistence glue.
This target is the only final executable and therefore owns the native
system-library link. No DLL boundary is introduced.

floppy144_persistence.c remains here temporarily because it still combines
portable encoding with Win32 file operations. Moving the codec into Core and
file operations behind Platform storage is explicitly deferred portability
debt, not hidden inside Floppy144Core.
]]
project("Floppy144")
language("C")
cdialect("C99")
kind("WindowedApp")
targetname("Floppy144")
warnings("Extra")

targetdir("bin/%{cfg.buildcfg}")
objdir("obj/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}")

includedirs({
    "./include/",
    "./game/src/"
})

files({
    "./game/src/floppy144_main.c",
    "./game/src/floppy144_persistence.c",
    "./game/src/floppy144_persistence.h",
    "./game/src/floppy144_storage.c",
    "./game/src/floppy144_storage.h"
})

libdirs({
    "./bin/%{cfg.buildcfg}/"
})

links({
    "Floppy144Core",
    "Floppy144PlatformWin32"
})

filter("platforms:Windows")
system("Windows")

defines({
    "BUILD_WINDOWS"
})

links({
    "user32",
    "gdi32",
    "shell32"
})

buildoptions({
    "/utf-8"
})

postbuildcommands({
    '{COPYFILE} "%{wks.location}/../LICENSE" "%{cfg.targetdir}/LICENSE"'
})

filter("configurations:debug")
runtime("debug")
symbols("On")
optimize("Off")

filter("configurations:asan")
runtime("debug")
symbols("On")
optimize("Off")
editandcontinue("Off")
buildoptions({
    "/fsanitize=address",
    "/Zi",
    "/INCREMENTAL:NO"
})

filter("configurations:release")
runtime("release")
symbols("Off")
optimize("Size")

filter({})
