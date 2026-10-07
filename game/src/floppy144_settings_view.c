/*
 * FLOPPY//144 persistent settings screen.
 */

#include "floppy144_settings_view.h"

#include <stdio.h>

static const char *Floppy144SettingsViewCrtText(
    const Floppy144Settings *settings
)
{
    Floppy144CrtMode mode =
        FLOPPY144_SETTINGS_DEFAULT_CRT_MODE;

    if(
        settings != NULL &&
        settings->crt_mode <
            (uint8_t)FLOPPY144_CRT_COUNT
    )
    {
        mode =
            (Floppy144CrtMode)settings->crt_mode;
    }

    switch(mode)
    {
        case FLOPPY144_CRT_REDUCED: return "REDUCED";
        case FLOPPY144_CRT_OFF: return "OFF";
        case FLOPPY144_CRT_FULL:
        case FLOPPY144_CRT_COUNT:
        default: return "FULL";
    }
}

static const char *Floppy144SettingsViewTextSpeedText(
    const Floppy144Settings *settings
)
{
    Floppy144TextSpeed speed =
        FLOPPY144_SETTINGS_DEFAULT_TEXT_SPEED;

    if(
        settings != NULL &&
        settings->text_speed <
            (uint8_t)FLOPPY144_TEXT_SPEED_COUNT
    )
    {
        speed =
            (Floppy144TextSpeed)settings->text_speed;
    }

    switch(speed)
    {
        case FLOPPY144_TEXT_SPEED_FAST: return "FAST";
        case FLOPPY144_TEXT_SPEED_INSTANT: return "INSTANT";
        case FLOPPY144_TEXT_SPEED_NORMAL:
        case FLOPPY144_TEXT_SPEED_COUNT:
        default: return "NORMAL";
    }
}

static const char *Floppy144SettingsViewAutosaveText(
    const Floppy144Settings *settings
)
{
    Floppy144AutosaveMode mode =
        FLOPPY144_SETTINGS_DEFAULT_AUTOSAVE_MODE;

    if(
        settings != NULL &&
        settings->autosave_mode <
            (uint8_t)FLOPPY144_AUTOSAVE_MODE_COUNT
    )
    {
        mode =
            (Floppy144AutosaveMode)settings->autosave_mode;
    }

    switch(mode)
    {
        case FLOPPY144_AUTOSAVE_10_MINUTES: return "10 MIN";
        case FLOPPY144_AUTOSAVE_30_MINUTES: return "30 MIN";
        case FLOPPY144_AUTOSAVE_OFF: return "OFF";
        case FLOPPY144_AUTOSAVE_5_MINUTES:
        case FLOPPY144_AUTOSAVE_MODE_COUNT:
        default: return "5 MIN";
    }
}

static void Floppy144SettingsViewVolumeText(
    char *buffer,
    uint32_t capacity,
    uint8_t volume
)
{
    uint32_t index;
    char bar[11];

    if(
        buffer == NULL ||
        capacity == 0U
    )
    {
        return;
    }

    if(volume > FLOPPY144_SETTINGS_VOLUME_MAX)
    {
        volume =
            FLOPPY144_SETTINGS_DEFAULT_MUSIC_VOLUME;
    }

    for(index = 0U; index < 10U; ++index)
    {
        bar[index] =
            index < (uint32_t)volume
                ? '#'
                : '-';
    }

    bar[10] = '\0';

    (void)snprintf(
        buffer,
        capacity,
        "%s %u/10",
        bar,
        (unsigned)volume
    );
}

void Floppy144SettingsViewDraw(
    Floppy144Surface *surface,
    const Floppy144Settings *settings,
    Floppy144SettingsOption selected_option,
    const char *status_text
)
{
    const uint32_t background=FLOPPY144_RGB(17,23,28);
    const uint32_t panel=FLOPPY144_RGB(24,33,39);
    const uint32_t panel_dark=FLOPPY144_RGB(12,17,21);
    const uint32_t border=FLOPPY144_RGB(86,103,107);
    const uint32_t text=FLOPPY144_RGB(202,211,205);
    const uint32_t muted=FLOPPY144_RGB(118,133,132);
    const uint32_t amber=FLOPPY144_RGB(194,153,76);
    const char *labels[FLOPPY144_SETTINGS_OPTION_COUNT]=
    {
        "CRT MODE",
        "TEXT SPEED",
        "MUSIC VOLUME",
        "SFX VOLUME",
        "AUTOSAVE INTERVAL",
        "CREDITS / ATTRIBUTION"
    };
    char values[FLOPPY144_SETTINGS_OPTION_COUNT][32];
    uint32_t option;

    if(surface==NULL||surface->pixels==NULL||settings==NULL) return;

    if(
        selected_option < 0 ||
        selected_option >= FLOPPY144_SETTINGS_OPTION_COUNT
    )
    {
        selected_option=FLOPPY144_SETTINGS_OPTION_CRT;
    }

    (void)snprintf(values[FLOPPY144_SETTINGS_OPTION_CRT],sizeof(values[0]),"%s",Floppy144SettingsViewCrtText(settings));
    (void)snprintf(values[FLOPPY144_SETTINGS_OPTION_TEXT_SPEED],sizeof(values[0]),"%s",Floppy144SettingsViewTextSpeedText(settings));
    Floppy144SettingsViewVolumeText(values[FLOPPY144_SETTINGS_OPTION_MUSIC_VOLUME],(uint32_t)sizeof(values[0]),settings->music_volume);
    Floppy144SettingsViewVolumeText(values[FLOPPY144_SETTINGS_OPTION_SFX_VOLUME],(uint32_t)sizeof(values[0]),settings->sfx_volume);
    (void)snprintf(values[FLOPPY144_SETTINGS_OPTION_AUTOSAVE],sizeof(values[0]),"%s",Floppy144SettingsViewAutosaveText(settings));
    (void)snprintf(values[FLOPPY144_SETTINGS_OPTION_CREDITS],sizeof(values[0]),"%s","ENTER");

    Floppy144DrawClear(surface,background);
    Floppy144DrawFillRect(surface,0U,0U,640U,16U,panel_dark);
    Floppy144DrawText(surface,10U,5U,"GDR ENVIRONMENT CONFIGURATION",1U,muted);
    Floppy144DrawText(surface,556U,5U,"APS-12",1U,amber);
    Floppy144DrawFillRect(surface,72U,34U,496U,286U,panel);
    Floppy144DrawRect(surface,72U,34U,496U,286U,border);
    Floppy144DrawText(surface,320U-Floppy144DrawTextWidth("SETTINGS",2U)/2U,48U,"SETTINGS",2U,text);
    Floppy144DrawFillRect(surface,96U,78U,448U,1U,border);

    for(option=0U;option<(uint32_t)FLOPPY144_SETTINGS_OPTION_COUNT;++option)
    {
        uint32_t row_y=96U+option*31U;
        bool selected=option==(uint32_t)selected_option;
        bool credits=option==(uint32_t)FLOPPY144_SETTINGS_OPTION_CREDITS;

        if(selected)
        {
            Floppy144DrawFillRect(surface,92U,row_y-8U,456U,24U,panel_dark);
            Floppy144DrawText(surface,102U,row_y,">",1U,amber);
        }

        Floppy144DrawText(surface,122U,row_y,labels[option],1U,selected?amber:text);

        if(credits)
        {
            Floppy144DrawText(surface,402U,row_y,"[ ENTER ]",1U,selected?amber:muted);
        }
        else
        {
            Floppy144DrawText(surface,374U,row_y,"<",1U,selected?amber:muted);
            Floppy144DrawText(surface,390U,row_y,values[option],1U,selected?amber:text);
            Floppy144DrawText(surface,526U,row_y,">",1U,selected?amber:muted);
        }
    }

    Floppy144DrawText(surface,96U,286U,status_text!=NULL?status_text:"CHANGES ARE SAVED AUTOMATICALLY",1U,status_text!=NULL?amber:muted);
    Floppy144DrawText(surface,10U,346U,"UP/DOWN SELECT   LEFT/RIGHT CHANGE   ENTER OPEN   BACKSPACE BACK",1U,muted);
}
