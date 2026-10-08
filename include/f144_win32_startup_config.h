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
 *   -GDR-CinderEllie
 *   -GDR-Hathaway
 *   -seed <non-zero uint32>
 *   -date <YYYY-MM-DD>
 *
 * Seed/date overrides require exact -GDR-CinderEllie without Hathaway.
 */
bool f144Win32StartupConfigFromCommandLine(
    const char *command_line,
    F144StartupConfig *config
);
