/*
 * F144 Win32 single-instance ownership
 *
 * A named mutex is keyed to the Roaming AppData environment. The Global
 * namespace prevents two desktop sessions for the same profile environment
 * from writing the same FLOPPY//144 persistence set, while the profile-root
 * hash lets different Windows user profiles run independently.
 *
 * The mutex is kernel-owned. It leaves no lock file and is released
 * automatically if a process terminates unexpectedly.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>

#include "f144_win32_single_instance.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define F144_SINGLE_INSTANCE_NAME_CAPACITY 96U

static uint64_t f144Win32ProfileEnvironmentHash(
    const char *profile_root
)
{
    const uint64_t fnv_offset =
        UINT64_C(14695981039346656037);
    const uint64_t fnv_prime =
        UINT64_C(1099511628211);
    uint64_t hash =
        fnv_offset;
    const unsigned char *cursor =
        (const unsigned char *)profile_root;

    while(cursor != NULL && *cursor != 0U)
    {
        unsigned char value =
            *cursor++;

        /*
         * Windows paths are case-insensitive for the persistence environment.
         * Canonicalise simple ASCII case and slash direction so equivalent
         * roots generate one mutex name.
         */
        if(value >= (unsigned char)'A' && value <= (unsigned char)'Z')
        {
            value =
                (unsigned char)(
                    value -
                    (unsigned char)'A' +
                    (unsigned char)'a'
                );
        }
        else if(value == (unsigned char)'/')
        {
            value =
                (unsigned char)'\\';
        }

        hash ^=
            (uint64_t)value;
        hash *=
            fnv_prime;
    }

    /*
     * Include the game's data-directory leaf explicitly. The mutex therefore
     * represents the exact environment which owns %APPDATA%\\Floppy144.
     */
    {
        const char suffix[] =
            "\\floppy144";
        const unsigned char *suffix_cursor =
            (const unsigned char *)suffix;

        while(*suffix_cursor != 0U)
        {
            hash ^=
                (uint64_t)*suffix_cursor++;
            hash *=
                fnv_prime;
        }
    }

    return hash;
}

static bool f144Win32SingleInstanceNameForProfileRoot(
    const char *profile_root,
    char *name,
    uint32_t name_capacity
)
{
    uint64_t hash;
    uint32_t high;
    uint32_t low;
    int written;

    if(
        profile_root == NULL ||
        profile_root[0] == '\0' ||
        name == NULL ||
        name_capacity == 0U
    )
    {
        return false;
    }

    hash =
        f144Win32ProfileEnvironmentHash(
            profile_root
        );

    high =
        (uint32_t)(hash >> 32U);
    low =
        (uint32_t)(hash & UINT32_MAX);

    written =
        snprintf(
            name,
            name_capacity,
            "Global\\Floppy144.Instance.%08X%08X",
            (unsigned)high,
            (unsigned)low
        );

    if(
        written < 0 ||
        (uint32_t)written >= name_capacity
    )
    {
        name[0] =
            '\0';
        return false;
    }

    return true;
}

F144Win32SingleInstanceResult f144Win32SingleInstanceAcquireForProfileRoot(
    F144Win32SingleInstance *instance,
    const char *profile_root
)
{
    char name[F144_SINGLE_INSTANCE_NAME_CAPACITY];
    HANDLE mutex;
    DWORD error;

    if(instance == NULL)
    {
        return F144_WIN32_SINGLE_INSTANCE_ERROR;
    }

    instance->handle =
        NULL;

    if(
        !f144Win32SingleInstanceNameForProfileRoot(
            profile_root,
            name,
            (uint32_t)sizeof(name)
        )
    )
    {
        return F144_WIN32_SINGLE_INSTANCE_ERROR;
    }

    SetLastError(
        ERROR_SUCCESS
    );

    mutex =
        CreateMutexA(
            NULL,
            TRUE,
            name
        );

    if(mutex == NULL)
    {
        return F144_WIN32_SINGLE_INSTANCE_ERROR;
    }

    error =
        GetLastError();

    if(error == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(
            mutex
        );

        return
            F144_WIN32_SINGLE_INSTANCE_ALREADY_RUNNING;
    }

    instance->handle =
        (void *)mutex;

    return
        F144_WIN32_SINGLE_INSTANCE_ACQUIRED;
}

F144Win32SingleInstanceResult f144Win32SingleInstanceAcquire(
    F144Win32SingleInstance *instance
)
{
    char profile_root[MAX_PATH];

    if(instance == NULL)
    {
        return F144_WIN32_SINGLE_INSTANCE_ERROR;
    }

    instance->handle =
        NULL;

    if(
        SHGetFolderPathA(
            NULL,
            CSIDL_APPDATA,
            NULL,
            SHGFP_TYPE_CURRENT,
            profile_root
        ) != S_OK
    )
    {
        return F144_WIN32_SINGLE_INSTANCE_ERROR;
    }

    return
        f144Win32SingleInstanceAcquireForProfileRoot(
            instance,
            profile_root
        );
}

void f144Win32SingleInstanceRelease(
    F144Win32SingleInstance *instance
)
{
    HANDLE mutex;

    if(
        instance == NULL ||
        instance->handle == NULL
    )
    {
        return;
    }

    mutex =
        (HANDLE)instance->handle;

    (void)ReleaseMutex(
        mutex
    );

    CloseHandle(
        mutex
    );

    instance->handle =
        NULL;
}
