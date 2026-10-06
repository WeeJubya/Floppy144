/*
 * Floppy//144 - platform-neutral persistence path set and legacy migration
 */

#include "floppy144_storage.h"

#include "floppy144_persistence.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define FLOPPY144_STORAGE_LEGACY_CANDIDATES 2U

bool Floppy144StorageResolve(
    F144Platform *platform,
    Floppy144StoragePaths *paths
)
{
    uint32_t file;

    if(platform == NULL || paths == NULL)
    {
        return false;
    }

    memset(
        paths,
        0,
        sizeof(*paths)
    );

    for(
        file = 0U;
        file < (uint32_t)F144_PERSISTENCE_FILE_COUNT;
        ++file
    )
    {
        if(
            !f144PlatformPersistencePath(
                platform,
                (F144PersistenceFile)file,
                paths->current[file],
                F144_PLATFORM_PATH_CAPACITY
            )
        )
        {
            memset(
                paths,
                0,
                sizeof(*paths)
            );

            return false;
        }
    }

    return true;
}

const char *Floppy144StoragePath(
    const Floppy144StoragePaths *paths,
    F144PersistenceFile file
)
{
    if(
        paths == NULL ||
        (uint32_t)file >=
            (uint32_t)F144_PERSISTENCE_FILE_COUNT
    )
    {
        return "";
    }

    return
        paths->current[(uint32_t)file];
}

static bool Floppy144StorageMigrateRunState(
    const char *legacy_path,
    const char *current_path
)
{
    Floppy144RunState legacy_state;
    Floppy144RunState verified_state;

    memset(&legacy_state,0,sizeof(legacy_state));
    memset(&verified_state,0,sizeof(verified_state));

    if(
        !Floppy144PersistenceLoadRunState(
            legacy_path,
            &legacy_state
        )
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceSaveRunState(
            current_path,
            &legacy_state
        )
    )
    {
        return false;
    }

    return Floppy144PersistenceLoadRunState(
        current_path,
        &verified_state
    );
}

static bool Floppy144StorageMigrateProfile(
    const char *legacy_path,
    const char *current_path
)
{
    Floppy144DiscoveryProfile legacy_profile;
    Floppy144DiscoveryProfile verified_profile;

    memset(&legacy_profile,0,sizeof(legacy_profile));
    memset(&verified_profile,0,sizeof(verified_profile));

    if(
        !Floppy144PersistenceLoadProfile(
            legacy_path,
            &legacy_profile
        )
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceSaveProfile(
            current_path,
            &legacy_profile
        )
    )
    {
        return false;
    }

    return Floppy144PersistenceLoadProfile(
        current_path,
        &verified_profile
    );
}

static bool Floppy144StorageMigrateSettings(
    const char *legacy_path,
    const char *current_path
)
{
    Floppy144Settings legacy_settings;
    Floppy144Settings verified_settings;

    memset(&legacy_settings,0,sizeof(legacy_settings));
    memset(&verified_settings,0,sizeof(verified_settings));

    if(
        !Floppy144PersistenceLoadSettings(
            legacy_path,
            &legacy_settings
        )
    )
    {
        return false;
    }

    if(
        !Floppy144PersistenceSaveSettings(
            current_path,
            &legacy_settings
        )
    )
    {
        return false;
    }

    return Floppy144PersistenceLoadSettings(
        current_path,
        &verified_settings
    );
}

static bool Floppy144StorageMigrateFile(
    F144PersistenceFile file,
    const char *legacy_path,
    const char *current_path
)
{
    switch(file)
    {
        case F144_PERSISTENCE_MANUAL_SAVE:
        case F144_PERSISTENCE_AUTOSAVE:
        {
            return Floppy144StorageMigrateRunState(
                legacy_path,
                current_path
            );
        }

        case F144_PERSISTENCE_PROFILE:
        {
            return Floppy144StorageMigrateProfile(
                legacy_path,
                current_path
            );
        }

        case F144_PERSISTENCE_SETTINGS:
        {
            return Floppy144StorageMigrateSettings(
                legacy_path,
                current_path
            );
        }

        case F144_PERSISTENCE_FILE_COUNT:
        default:
        {
            return false;
        }
    }
}

uint32_t Floppy144StorageMigrateLegacy(
    F144Platform *platform,
    const Floppy144StoragePaths *paths
)
{
    uint32_t failures = 0U;
    uint32_t file;

    if(platform == NULL || paths == NULL)
    {
        return
            (1U << (uint32_t)F144_PERSISTENCE_FILE_COUNT) - 1U;
    }

    for(
        file = 0U;
        file < (uint32_t)F144_PERSISTENCE_FILE_COUNT;
        ++file
    )
    {
        F144PersistenceFile persistence_file =
            (F144PersistenceFile)file;
        const char *current_path =
            Floppy144StoragePath(
                paths,
                persistence_file
            );
        char first_legacy[F144_PLATFORM_PATH_CAPACITY] = {0};
        bool legacy_found = false;
        bool migrated = false;
        uint32_t candidate;

        /*
         * A new-location file always wins. This deliberately avoids replacing
         * a newer AppData file with an older Stage 3 copy, even when the new
         * file later proves malformed.
         */
        if(
            Floppy144PersistenceFileExists(
                current_path
            )
        )
        {
            continue;
        }

        for(
            candidate = 0U;
            candidate < FLOPPY144_STORAGE_LEGACY_CANDIDATES;
            ++candidate
        )
        {
            char legacy_path[F144_PLATFORM_PATH_CAPACITY] = {0};

            if(
                !f144PlatformLegacyPersistencePath(
                    platform,
                    persistence_file,
                    candidate,
                    legacy_path,
                    F144_PLATFORM_PATH_CAPACITY
                )
            )
            {
                continue;
            }

            if(
                legacy_path[0] == '\0' ||
                strcmp(
                    legacy_path,
                    current_path
                ) == 0
            )
            {
                continue;
            }

            if(
                first_legacy[0] != '\0' &&
                strcmp(
                    first_legacy,
                    legacy_path
                ) == 0
            )
            {
                continue;
            }

            if(first_legacy[0] == '\0')
            {
                (void)snprintf(
                    first_legacy,
                    sizeof(first_legacy),
                    "%s",
                    legacy_path
                );
            }

            if(
                !Floppy144PersistenceFileExists(
                    legacy_path
                )
            )
            {
                continue;
            }

            legacy_found =
                true;

            if(
                Floppy144StorageMigrateFile(
                    persistence_file,
                    legacy_path,
                    current_path
                )
            )
            {
                migrated =
                    true;

                break;
            }
        }

        if(legacy_found && !migrated)
        {
            failures |=
                1U << file;
        }
    }

    return failures;
}
