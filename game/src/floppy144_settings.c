/*
 * Portable settings defaults and mutations.
 *
 * Persistence encodes these values separately. Keep the enum ordinals and
 * stored byte meanings stable so older settings files remain readable.
 */
#include "floppy144_settings.h"

#include <string.h>

/* Every successful setter marks the settings for the existing save path. */
static bool Floppy144SettingsUpdateByte(
    Floppy144Settings *settings, uint8_t *field, uint8_t value)
{
    if(*field == value) return false;
    *field = value;
    settings->dirty = 1U;
    return true;
}

void Floppy144SettingsReset(Floppy144Settings *settings)
{
    if(settings == NULL) return;

    memset(settings, 0, sizeof(*settings));
    settings->crt_mode = (uint8_t)FLOPPY144_SETTINGS_DEFAULT_CRT_MODE;
    settings->text_speed = (uint8_t)FLOPPY144_SETTINGS_DEFAULT_TEXT_SPEED;
    /* Ten retains the established unattenuated music and SFX defaults. */
    settings->music_volume = FLOPPY144_SETTINGS_DEFAULT_MUSIC_VOLUME;
    settings->sfx_volume = FLOPPY144_SETTINGS_DEFAULT_SFX_VOLUME;
    settings->autosave_mode = (uint8_t)FLOPPY144_SETTINGS_DEFAULT_AUTOSAVE_MODE;
}

bool Floppy144SettingsSetCrtMode(Floppy144Settings *settings, Floppy144CrtMode mode)
{
    if(settings == NULL || (uint32_t)mode >= (uint32_t)FLOPPY144_CRT_COUNT)
        return false;
    return Floppy144SettingsUpdateByte(settings, &settings->crt_mode, (uint8_t)mode);
}

bool Floppy144SettingsSetTextSpeed(Floppy144Settings *settings, Floppy144TextSpeed speed)
{
    if(settings == NULL || (uint32_t)speed >= (uint32_t)FLOPPY144_TEXT_SPEED_COUNT)
        return false;
    return Floppy144SettingsUpdateByte(settings, &settings->text_speed, (uint8_t)speed);
}

bool Floppy144SettingsSetMusicVolume(Floppy144Settings *settings, uint8_t volume)
{
    if(settings == NULL || volume > FLOPPY144_SETTINGS_VOLUME_MAX) return false;
    return Floppy144SettingsUpdateByte(settings, &settings->music_volume, volume);
}

bool Floppy144SettingsSetSfxVolume(Floppy144Settings *settings, uint8_t volume)
{
    if(settings == NULL || volume > FLOPPY144_SETTINGS_VOLUME_MAX) return false;
    return Floppy144SettingsUpdateByte(settings, &settings->sfx_volume, volume);
}

bool Floppy144SettingsSetAutosaveMode(
    Floppy144Settings *settings, Floppy144AutosaveMode mode)
{
    if(settings == NULL || (uint32_t)mode >= (uint32_t)FLOPPY144_AUTOSAVE_MODE_COUNT)
        return false;
    return Floppy144SettingsUpdateByte(settings, &settings->autosave_mode, (uint8_t)mode);
}

uint32_t Floppy144SettingsAutosaveIntervalMs(const Floppy144Settings *settings)
{
    if(settings == NULL) return 300000U;

    switch((Floppy144AutosaveMode)settings->autosave_mode)
    {
        case FLOPPY144_AUTOSAVE_10_MINUTES: return 600000U;
        case FLOPPY144_AUTOSAVE_30_MINUTES: return 1800000U;
        case FLOPPY144_AUTOSAVE_OFF: return 0U;
        case FLOPPY144_AUTOSAVE_5_MINUTES:
        default: return 300000U;
    }
}

bool Floppy144SettingsSetInspectionIndicators(Floppy144Settings *settings, bool enabled)
{
    if(settings == NULL) return false;
    /* Stored as a single V1 settings byte; default remains OFF. */
    return Floppy144SettingsUpdateByte(
        settings, &settings->show_inspection_indicators, enabled ? 1U : 0U);
}
