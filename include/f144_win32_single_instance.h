/*
 * F144 Win32 single-instance ownership
 *
 * This is a launcher/platform concern. Game/Core code must not know the mutex
 * name, HANDLE, or Windows process semantics.
 */

#pragma once

#include <stdint.h>

typedef enum F144Win32SingleInstanceResult
{
    F144_WIN32_SINGLE_INSTANCE_ERROR = 0,
    F144_WIN32_SINGLE_INSTANCE_ACQUIRED,
    F144_WIN32_SINGLE_INSTANCE_ALREADY_RUNNING
} F144Win32SingleInstanceResult;

typedef struct F144Win32SingleInstance
{
    void *handle;
} F144Win32SingleInstance;

F144Win32SingleInstanceResult f144Win32SingleInstanceAcquire(
    F144Win32SingleInstance *instance
);

/*
 * Testable/profile-explicit form. Production resolves the same Roaming
 * AppData root used by persistence and passes it through this implementation.
 */
F144Win32SingleInstanceResult f144Win32SingleInstanceAcquireForProfileRoot(
    F144Win32SingleInstance *instance,
    const char *profile_root
);

void f144Win32SingleInstanceRelease(
    F144Win32SingleInstance *instance
);
