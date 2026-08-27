#include "floppy144_settings.h"

#include <string.h>

void Floppy144SettingsReset
(
    Floppy144Settings *settings
){
    if(settings == NULL)
    {
        return;
    }

    memset(
        settings,
        0,
        sizeof(*settings)
    );

    settings->crt_mode =
    (uint8_t)FLOPPY144_CRT_FULL;

    settings->text_speed =
    (uint8_t)FLOPPY144_TEXT_SPEED_NORMAL;

    /*
     * Ten preserves the current unattenuated audio behaviour.
     */

    settings->music_volume =
    FLOPPY144_SETTINGS_VOLUME_MAX;

    settings->sfx_volume =
    FLOPPY144_SETTINGS_VOLUME_MAX;

    settings->autosave_mode =
    (uint8_t)FLOPPY144_AUTOSAVE_5_MINUTES;
}

bool Floppy144SettingsSetCrtMode
(
    Floppy144Settings *settings,
 Floppy144CrtMode mode
){
    if(
        settings == NULL ||
        (uint32_t)mode >=
        (uint32_t)FLOPPY144_CRT_COUNT
    )
    {
        return false;
    }

    if(settings->crt_mode == (uint8_t)mode)
    {
        return false;
    }

    settings->crt_mode =
    (uint8_t)mode;

    settings->dirty =
    1U;

    return true;
}

bool Floppy144SettingsSetTextSpeed
(
    Floppy144Settings *settings,
 Floppy144TextSpeed speed
){
    if(
        settings == NULL ||
        (uint32_t)speed >=
        (uint32_t)FLOPPY144_TEXT_SPEED_COUNT
    )
    {
        return false;
    }

    if(settings->text_speed == (uint8_t)speed)
    {
        return false;
    }

    settings->text_speed =
    (uint8_t)speed;

    settings->dirty =
    1U;

    return true;
}

bool Floppy144SettingsSetMusicVolume
(
    Floppy144Settings *settings,
 uint8_t volume
){
    if(
        settings == NULL ||
        volume > FLOPPY144_SETTINGS_VOLUME_MAX
    )
    {
        return false;
    }

    if(settings->music_volume == volume)
    {
        return false;
    }

    settings->music_volume =
    volume;

    settings->dirty =
    1U;

    return true;
}

bool Floppy144SettingsSetSfxVolume
(
    Floppy144Settings *settings,
 uint8_t volume
){
    if(
        settings == NULL ||
        volume > FLOPPY144_SETTINGS_VOLUME_MAX
    )
    {
        return false;
    }

    if(settings->sfx_volume == volume)
    {
        return false;
    }

    settings->sfx_volume =
    volume;

    settings->dirty =
    1U;

    return true;
}

bool Floppy144SettingsSetAutosaveMode
(
    Floppy144Settings *settings,
 Floppy144AutosaveMode mode
){
    if(
        settings == NULL ||
        (uint32_t)mode >=
        (uint32_t)FLOPPY144_AUTOSAVE_MODE_COUNT
    )
    {
        return false;
    }

    if(settings->autosave_mode == (uint8_t)mode)
    {
        return false;
    }

    settings->autosave_mode =
    (uint8_t)mode;

    settings->dirty =
    1U;

    return true;
}

uint32_t Floppy144SettingsAutosaveIntervalMs
(
    const Floppy144Settings *settings
){
    if(settings == NULL)
    {
        return 300000U;
    }

    switch(
        (Floppy144AutosaveMode)
        settings->autosave_mode
    )
    {
        case FLOPPY144_AUTOSAVE_10_MINUTES:
        {
            return 600000U;
        }

        case FLOPPY144_AUTOSAVE_30_MINUTES:
        {
            return 1800000U;
        }

        case FLOPPY144_AUTOSAVE_OFF:
        {
            return 0U;
        }

        case FLOPPY144_AUTOSAVE_5_MINUTES:
        default:
        {
            return 300000U;
        }
    }
}
