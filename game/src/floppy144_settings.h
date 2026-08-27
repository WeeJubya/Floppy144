#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FLOPPY144_SETTINGS_VOLUME_MAX 10U

typedef enum Floppy144CrtMode
{
    FLOPPY144_CRT_FULL = 0,
    FLOPPY144_CRT_REDUCED,
    FLOPPY144_CRT_OFF,

    FLOPPY144_CRT_COUNT
}
Floppy144CrtMode;

typedef enum Floppy144TextSpeed
{
    FLOPPY144_TEXT_SPEED_NORMAL = 0,
    FLOPPY144_TEXT_SPEED_FAST,
    FLOPPY144_TEXT_SPEED_INSTANT,

    FLOPPY144_TEXT_SPEED_COUNT
}
Floppy144TextSpeed;

typedef enum Floppy144AutosaveMode
{
    FLOPPY144_AUTOSAVE_5_MINUTES = 0,
    FLOPPY144_AUTOSAVE_10_MINUTES,
    FLOPPY144_AUTOSAVE_30_MINUTES,
    FLOPPY144_AUTOSAVE_OFF,

    FLOPPY144_AUTOSAVE_MODE_COUNT
}
Floppy144AutosaveMode;

typedef struct Floppy144Settings
{
    uint8_t crt_mode;
    uint8_t text_speed;

    uint8_t music_volume;
    uint8_t sfx_volume;

    uint8_t autosave_mode;

    uint8_t dirty;
}
Floppy144Settings;

void Floppy144SettingsReset
(
    Floppy144Settings *settings
);

bool Floppy144SettingsSetCrtMode
(
    Floppy144Settings *settings,
 Floppy144CrtMode mode
);

bool Floppy144SettingsSetTextSpeed
(
    Floppy144Settings *settings,
 Floppy144TextSpeed speed
);

bool Floppy144SettingsSetMusicVolume
(
    Floppy144Settings *settings,
 uint8_t volume
);

bool Floppy144SettingsSetSfxVolume
(
    Floppy144Settings *settings,
 uint8_t volume
);

bool Floppy144SettingsSetAutosaveMode
(
    Floppy144Settings *settings,
 Floppy144AutosaveMode mode
);

uint32_t Floppy144SettingsAutosaveIntervalMs
(
    const Floppy144Settings *settings
);
