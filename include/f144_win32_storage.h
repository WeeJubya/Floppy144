/*
 * F144 Win32 persistence-path provider
 */

#pragma once

#include "f144_platform.h"

#include <stdbool.h>
#include <stdint.h>

bool f144Win32PlatformPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
);

bool f144Win32PlatformLegacyPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    uint32_t candidate,
    char *path,
    uint32_t path_capacity
);

/*
 * Testable constructor used with an isolated temporary root. Production calls
 * this after resolving roaming AppData with SHGetFolderPathA.
 */
bool f144Win32PersistencePathForRoot(
    const char *root,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
);
