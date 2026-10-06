/*
 * F144 Win32 logical-input adapter
 *
 * This header exposes no HWND/WPARAM/VK_* types. Native Win32 key identity is
 * translated inside the implementation into the platform-neutral F144Action.
 */

#pragma once

#include "f144_platform.h"

#include <stdint.h>

F144Action f144Win32ActionFromVirtualKey(
    uint32_t virtual_key
);

void f144Win32TranslateKeyEvent(
    uint32_t virtual_key,
    F144ActionEventType type,
    F144ActionEvent *event
);

void f144Win32TranslateTextEvent(
    uint32_t codepoint,
    F144TextInputEvent *event
);
