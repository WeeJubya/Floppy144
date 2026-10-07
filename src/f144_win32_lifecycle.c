/*
 * F144 Win32 lifecycle-message adapter.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "f144_win32_lifecycle.h"

#include <stddef.h>

bool f144Win32TranslateLifecycleEvent(
    uint32_t message,
    uintptr_t parameter,
    F144LifecycleEvent *event
)
{
    if(event == NULL)
    {
        return false;
    }

    event->type =
        F144_LIFECYCLE_NONE;
    event->request_autosave =
        0U;

    switch(message)
    {
        case WM_ACTIVATEAPP:
        {
            event->type =
                parameter != 0U
                ? F144_LIFECYCLE_ACTIVE
                : F144_LIFECYCLE_INACTIVE;

            return true;
        }

        case WM_CLOSE:
        {
            event->type =
                F144_LIFECYCLE_SHUTDOWN_REQUESTED;

            return true;
        }

        case WM_DESTROY:
        {
            event->type =
                F144_LIFECYCLE_SHUTDOWN;

            return true;
        }

        default:
        {
            return false;
        }
    }
}
