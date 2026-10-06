/*
 * F144 Win32 persistence-path provider
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>

#include "f144_win32_storage.h"

#include <stddef.h>
#include <string.h>

static const char *f144Win32PersistenceLeafName(
    F144PersistenceFile file
)
{
    switch(file)
    {
        case F144_PERSISTENCE_MANUAL_SAVE:
            return "floppy144_manual.sav";

        case F144_PERSISTENCE_AUTOSAVE:
            return "floppy144_auto.sav";

        case F144_PERSISTENCE_PROFILE:
            return "floppy144_profile.dat";

        case F144_PERSISTENCE_SETTINGS:
            return "floppy144_settings.dat";

        case F144_PERSISTENCE_FILE_COUNT:
        default:
            return NULL;
    }
}

static bool f144Win32DirectoryExists(
    const char *path
)
{
    DWORD attributes;

    if(path == NULL || path[0] == '\0')
    {
        return false;
    }

    attributes =
        GetFileAttributesA(path);

    return
        attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0U;
}

static bool f144Win32JoinPath(
    const char *root,
    const char *leaf,
    char *path,
    uint32_t path_capacity
)
{
    size_t root_length;
    size_t leaf_length;
    size_t required;
    bool needs_separator;

    if(
        root == NULL ||
        leaf == NULL ||
        path == NULL ||
        path_capacity == 0U
    )
    {
        return false;
    }

    root_length =
        strlen(root);
    leaf_length =
        strlen(leaf);

    if(root_length == 0U || leaf_length == 0U)
    {
        return false;
    }

    needs_separator =
        root[root_length - 1U] != '\\' &&
        root[root_length - 1U] != '/';

    required =
        root_length +
        (needs_separator ? 1U : 0U) +
        leaf_length +
        1U;

    /*
     * The current persistence writer appends ".tmp" before atomic replacement.
     * Reject paths which cannot safely fit that legacy Win32 buffer as well as
     * the caller's platform-neutral output buffer.
     */
    if(
        required > (size_t)path_capacity ||
        required + 4U > (size_t)MAX_PATH
    )
    {
        path[0] = '\0';
        return false;
    }

    memcpy(
        path,
        root,
        root_length
    );

    if(needs_separator)
    {
        path[root_length++] =
            '\\';
    }

    memcpy(
        &path[root_length],
        leaf,
        leaf_length + 1U
    );

    return true;
}

bool f144Win32PersistencePathForRoot(
    const char *root,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
)
{
    const char *leaf;
    char data_directory[MAX_PATH];

    if(
        root == NULL ||
        path == NULL ||
        path_capacity == 0U
    )
    {
        return false;
    }

    path[0] =
        '\0';

    leaf =
        f144Win32PersistenceLeafName(file);

    if(leaf == NULL)
    {
        return false;
    }

    if(
        !f144Win32JoinPath(
            root,
            "Floppy144",
            data_directory,
            (uint32_t)sizeof(data_directory)
        )
    )
    {
        return false;
    }

    if(
        !CreateDirectoryA(
            data_directory,
            NULL
        )
    )
    {
        DWORD error =
            GetLastError();

        if(
            error != ERROR_ALREADY_EXISTS ||
            !f144Win32DirectoryExists(
                data_directory
            )
        )
        {
            return false;
        }
    }

    return f144Win32JoinPath(
        data_directory,
        leaf,
        path,
        path_capacity
    );
}

bool f144Win32PlatformPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    char *path,
    uint32_t path_capacity
)
{
    char app_data[MAX_PATH];

    (void)platform;

    if(
        path == NULL ||
        path_capacity == 0U
    )
    {
        return false;
    }

    path[0] =
        '\0';

    if(
        FAILED(
            SHGetFolderPathA(
                NULL,
                CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                NULL,
                SHGFP_TYPE_CURRENT,
                app_data
            )
        )
    )
    {
        return false;
    }

    return f144Win32PersistencePathForRoot(
        app_data,
        file,
        path,
        path_capacity
    );
}

static bool f144Win32LegacyPersistenceRoot(
    uint32_t candidate,
    char *root,
    uint32_t root_capacity
)
{
    DWORD length;

    if(
        root == NULL ||
        root_capacity == 0U
    )
    {
        return false;
    }

    root[0] =
        '\0';

    switch(candidate)
    {
        /*
         * Stage 3 used a bare relative filename, which means its exact historic
         * location is the process working directory for that launch.
         */
        case 0U:
        {
            length =
                GetCurrentDirectoryA(
                    root_capacity,
                    root
                );

            return
                length > 0U &&
                length < root_capacity;
        }

        /*
         * Most packaged/manual launches used the executable directory as their
         * working directory. Probe it second so an old save beside the EXE is
         * still recoverable even when Stage 4 is started from another CWD.
         */
        case 1U:
        {
            char *separator;

            length =
                GetModuleFileNameA(
                    NULL,
                    root,
                    root_capacity
                );

            if(
                length == 0U ||
                length >= root_capacity
            )
            {
                root[0] = '\0';
                return false;
            }

            separator =
                strrchr(
                    root,
                    '\\'
                );

            if(separator == NULL)
            {
                separator =
                    strrchr(
                        root,
                        '/'
                    );
            }

            if(separator == NULL)
            {
                root[0] = '\0';
                return false;
            }

            *separator =
                '\0';

            return root[0] != '\0';
        }

        default:
        {
            return false;
        }
    }
}

bool f144Win32PlatformLegacyPersistencePath(
    F144Platform *platform,
    F144PersistenceFile file,
    uint32_t candidate,
    char *path,
    uint32_t path_capacity
)
{
    const char *leaf;
    char root[MAX_PATH];

    (void)platform;

    leaf =
        f144Win32PersistenceLeafName(file);

    if(
        leaf == NULL ||
        !f144Win32LegacyPersistenceRoot(
            candidate,
            root,
            (uint32_t)sizeof(root)
        )
    )
    {
        if(path != NULL && path_capacity > 0U)
        {
            path[0] = '\0';
        }

        return false;
    }

    return f144Win32JoinPath(
        root,
        leaf,
        path,
        path_capacity
    );
}

