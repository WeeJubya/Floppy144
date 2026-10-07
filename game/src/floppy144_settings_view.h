/*
 * FLOPPY//144 persistent settings screen.
 */

#pragma once

#include "floppy144_draw.h"
#include "floppy144_settings.h"

#include <stdint.h>

typedef enum Floppy144SettingsOption
{
    FLOPPY144_SETTINGS_OPTION_CRT = 0,
    FLOPPY144_SETTINGS_OPTION_TEXT_SPEED,
    FLOPPY144_SETTINGS_OPTION_MUSIC_VOLUME,
    FLOPPY144_SETTINGS_OPTION_SFX_VOLUME,
    FLOPPY144_SETTINGS_OPTION_AUTOSAVE,
    FLOPPY144_SETTINGS_OPTION_CREDITS,
    FLOPPY144_SETTINGS_OPTION_COUNT
}
Floppy144SettingsOption;

void Floppy144SettingsViewDraw(
    Floppy144Surface *surface,
    const Floppy144Settings *settings,
    Floppy144SettingsOption selected_option,
    const char *status_text
);
