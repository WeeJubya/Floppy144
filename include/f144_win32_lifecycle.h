/*
 * F144 Win32 lifecycle-message adapter.
 */

#pragma once

#include "f144_platform.h"

#include <stdbool.h>
#include <stdint.h>

bool f144Win32TranslateLifecycleEvent(
    uint32_t message,
    uintptr_t parameter,
    F144LifecycleEvent *event
);
