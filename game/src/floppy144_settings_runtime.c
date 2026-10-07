/*
 * FLOPPY//144 persisted-settings runtime adapters.
 */

#include "floppy144_settings_runtime.h"

#include <limits.h>
#include <stddef.h>

uint32_t Floppy144SettingsTextElapsedMs(
    const Floppy144Settings *settings,
    uint32_t elapsed_ms
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
        case FLOPPY144_TEXT_SPEED_FAST:
        {
            if(elapsed_ms > UINT32_MAX / 2U)
            {
                return UINT32_MAX;
            }

            return elapsed_ms * 2U;
        }

        case FLOPPY144_TEXT_SPEED_INSTANT:
        {
            return UINT32_MAX;
        }

        case FLOPPY144_TEXT_SPEED_NORMAL:
        case FLOPPY144_TEXT_SPEED_COUNT:
        default:
        {
            return elapsed_ms;
        }
    }
}

static uint32_t Floppy144SettingsDimPixel(
    uint32_t pixel,
    uint32_t numerator,
    uint32_t denominator
)
{
    uint32_t red;
    uint32_t green;
    uint32_t blue;

    if(denominator == 0U)
    {
        return pixel;
    }

    red =
        ((pixel >> 16U) & 0xffU) *
        numerator /
        denominator;

    green =
        ((pixel >> 8U) & 0xffU) *
        numerator /
        denominator;

    blue =
        (pixel & 0xffU) *
        numerator /
        denominator;

    return
        (red << 16U) |
        (green << 8U) |
        blue;
}

void Floppy144SettingsApplyCrtFilter(
    Floppy144Surface *surface,
    const Floppy144Settings *settings
)
{
    Floppy144CrtMode mode =
        FLOPPY144_SETTINGS_DEFAULT_CRT_MODE;
    uint32_t y;
    uint32_t x;

    if(
        surface == NULL ||
        surface->pixels == NULL
    )
    {
        return;
    }

    if(
        settings != NULL &&
        settings->crt_mode <
            (uint8_t)FLOPPY144_CRT_COUNT
    )
    {
        mode =
            (Floppy144CrtMode)settings->crt_mode;
    }

    if(mode == FLOPPY144_CRT_OFF)
    {
        return;
    }

    for(y = 0U; y < surface->height; ++y)
    {
        bool dim_line;

        if(mode == FLOPPY144_CRT_FULL)
        {
            dim_line =
                (y & 1U) != 0U;
        }
        else
        {
            dim_line =
                (y & 3U) == 3U;
        }

        if(!dim_line)
        {
            continue;
        }

        for(x = 0U; x < surface->width; ++x)
        {
            uint32_t *pixel =
                &surface->pixels[
                    (uint64_t)y *
                    surface->width +
                    x
                ];

            *pixel =
                mode == FLOPPY144_CRT_FULL
                    ? Floppy144SettingsDimPixel(
                        *pixel,
                        3U,
                        4U
                    )
                    : Floppy144SettingsDimPixel(
                        *pixel,
                        7U,
                        8U
                    );
        }
    }
}
