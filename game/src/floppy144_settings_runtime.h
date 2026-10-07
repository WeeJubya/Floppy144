/*
 * FLOPPY//144 persisted-settings runtime adapters.
 *
 * These helpers translate portable settings into portable presentation/timing
 * behaviour. Native platform APIs remain outside this module.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_settings.h"

#include <stdint.h>

uint32_t Floppy144SettingsTextElapsedMs(
    const Floppy144Settings *settings,
    uint32_t elapsed_ms
);

void Floppy144SettingsApplyCrtFilter(
    Floppy144Surface *surface,
    const Floppy144Settings *settings
);
