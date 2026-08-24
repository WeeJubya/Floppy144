---@diagnostic disable: undefined-global, undefined-field

workspace("river2D")
configurations({"debug", "asan", "release"})
platforms({"windows"})
location("build")
architecture("x86_64")

project("river2D common functions")
language("C")
cdialect("C99")
warnings("Extra")
kind("StaticLib")
targetname("river2Dcommon")

includedirs({
    "./include/"
})

files({
    "./src/river2Dcommon_main.c",
    "./src/win32_river2Dcommon.c",
    "./src/string_view.c",
    "./include/river2D_main.h",
    "./include/string_view.h"
})

filter("platforms:Windows")
system("Windows")

defines({
    "BUILD_WINDOWS",
    "RIVER2D_NO_IMAGE_IO",
    "RIVER2D_NO_TILEMAP"
})

targetdir("bin/%{cfg.buildcfg}")
objdir("obj/river2Dcommon/%{cfg.buildcfg}/%{cfg.platform}")

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
    "./src/win32_river2Dsoftware_platform.c",
    "./include/win32_river2Dsoftware_platform.h"
})

includedirs({
    "./include/"
})

libdirs({
    "./bin/%{cfg.buildcfg}/"
})

dependson({
    "river2D common functions"
})

filter("platforms:Windows")
system("Windows")

defines({
    "BUILD_WINDOWS",
    "RIVER2D_STATIC_RENDERER"
})

links({
    "river2Dcommon.lib",
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
