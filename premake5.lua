---@diagnostic disable: undefined-global, undefined-field

workspace("Floppy144")
configurations({"debug", "asan", "release"})
platforms({"windows"})
location("build")
architecture("x86_64")

project("F144 Runtime")
language("C")
cdialect("C99")
warnings("Extra")
kind("StaticLib")
targetname("f144runtime")

includedirs({
    "./include/"
})

files({
    "./src/f144_runtime.c",
    "./src/f144_win32_runtime.c",
    "./src/string_view.c",
    "./include/f144_runtime.h",
    "./include/string_view.h"
})

filter("platforms:Windows")
system("Windows")

defines({
    "BUILD_WINDOWS"
})

targetdir("bin/%{cfg.buildcfg}")
objdir("obj/Floppy144/%{cfg.buildcfg}/%{cfg.platform}")

buildoptions({
    "/wd4068"
})

filter("configurations:debug")
defines({"DEBUG"})
runtime("debug")
symbols("On")
optimize("Off")

filter("configurations:asan")
defines({"ASAN"})
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
staticruntime("off")
runtime("release")
symbols("Off")
optimize("Speed")
linkoptions("/NODEFAULTLIB:MSVCRTD")

filter({})


project("Floppy144")
language("C")
cdialect("C99")
kind("WindowedApp")
targetname("Floppy144")
warnings("Extra")

targetdir("bin/%{cfg.buildcfg}")
objdir("obj/Floppy144/%{cfg.buildcfg}/%{cfg.platform}")

files({
    "./game/src/**.c",
    "./game/src/**.h",
    "./src/f144_win32_platform.c",
    "./include/f144_win32_platform.h"
})

includedirs({
    "./include/"
})

libdirs({
    "./bin/%{cfg.buildcfg}/"
})

filter("platforms:Windows")
system("Windows")

defines({
    "BUILD_WINDOWS",
})

links({
    "F144 Runtime",
    "user32",
    "gdi32"
})

filter("configurations:debug")
runtime("debug")
symbols("On")
optimize("Off")

filter("configurations:release")
runtime("release")
symbols("Off")
optimize("Size")

filter({})
