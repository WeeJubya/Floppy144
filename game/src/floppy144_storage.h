/*
 * Floppy//144 - platform-neutral persistence path set and legacy migration
 */

#pragma once

#include "f144_platform.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct Floppy144StoragePaths
{
    char current[F144_PERSISTENCE_FILE_COUNT][F144_PLATFORM_PATH_CAPACITY];
} Floppy144StoragePaths;

bool Floppy144StorageResolve(
    F144Platform *platform,
    Floppy144StoragePaths *paths
);

const char *Floppy144StoragePath(
    const Floppy144StoragePaths *paths,
    F144PersistenceFile file
);

/*
 * Returns a bit mask of files whose legacy data existed but could not be
 * validated and copied to the new platform-selected location.
 */
uint32_t Floppy144StorageMigrateLegacy(
    F144Platform *platform,
    const Floppy144StoragePaths *paths
);
