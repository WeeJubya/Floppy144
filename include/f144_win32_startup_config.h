/*
 * F144 Win32 launch-configuration acquisition.
 *
 * Only this platform adapter understands the raw WinMain command-line string.
 */

#pragma once

#include "f144_startup_config.h"

#include <stdbool.h>

/*
 * Convert the raw Win32 command-line tail into portable F144 configuration.
 *
 * Supported developer options:
 *   -debug
 *   -seed <non-zero uint32>
 *   -date <YYYY-MM-DD>
 *
 * Seed/date overrides are applied only when -debug is present.
 */
bool f144Win32StartupConfigFromCommandLine(
    const char *command_line,
    F144StartupConfig *config
);
